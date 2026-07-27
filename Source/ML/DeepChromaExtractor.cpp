#include "DeepChromaExtractor.h"
#include "FilterbankWeights.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "onnxruntime_c_api.h"
#include "onnxruntime_cxx_api.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

DeepChromaExtractor::DeepChromaExtractor(int binSize, int medianSize)
    : chromaBinSize(binSize), medianSize(medianSize) {
  loadModel();
}

/**
 * @brief Loads the ONNX model into memory and initializes the runtime session.
 *
 * This method sets up the ONNX Runtime environment, configures session options
 * such as the number of intra-operation threads and graph optimization levels,
 * and loads the model from the specified file path into RAM. It also verifies
 * the input tensor shape for debugging purposes. Any exceptions encountered
 * during the loading process are caught and logged to standard output.
 */
void DeepChromaExtractor::loadModel() {
  try {
    ortEnv = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "DeepChromaEnv");

    // options like how many threads to use
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(
        GraphOptimizationLevel::ORT_ENABLE_ALL);

    // load model into ram
    ortSession =
        std::make_unique<Ort::Session>(ortEnv, modelPath, sessionOptions);

    DBG("SUCCESS: ONNX model loaded successfully into ram.");

    // Debug verification of correct model input size
    // This is optional and can be removed.
    size_t numInputTensors = ortSession->GetInputCount();
    DBG("Count of input nodes: " << numInputTensors);

    Ort::TypeInfo inputTypeInfo = ortSession->GetInputTypeInfo(0);
    auto tensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();

    std::vector<int64_t> inputDims = tensorInfo.GetShape();
    DBG("Shape der Input Node 0: ");
    for (size_t i = 0; i < inputDims.size(); i++) {
      DBG("Dimension " << i << ": " << inputDims[i]);
    }

  } catch (const Ort::Exception &e) {
    std::cout << "ERROR loading the ONNX model: " << e.what() << std::endl;
  }
}

/**
 * @brief Extracts chroma features from a given spectrogram.
 *
 * This method processes the provided linear spectrogram by computing a
 * logarithmic filterbank representation, setting up sliding context windows,
 * and running inference through the loaded ONNX model to generate the final
 * chroma features.
 *
 * @param spectogram The input linear spectrogram (channels correspond to
 * frames, samples correspond to linear frequency bins).
 * @param chroma     The output buffer where the extracted chroma features will
 *                   be stored (channels correspond to frames, samples
 * correspond to the 12 chroma bins).
 */
void DeepChromaExtractor::extractChroma(
    const juce::AudioBuffer<float> &spectogram,
    juce::AudioBuffer<float> &chroma) {
  if (!ortSession) {
    std::cout << "Model not loaded. Cannot extract chroma." << std::endl;
    return;
  }

  int numFrames = spectogram.getNumChannels();
  int numLogBins = static_cast<int>(MADMOM_FILTERBANK.size());

  // =================================================================================
  // 1. Logarithmic Filterbank Calculation (Extracting unique bins)
  // =================================================================================
  // For each of the 105 output bands, convert the contributing FFT bins from
  // dB back to linear magnitude, compute the weighted sum using the triangular
  // filter coefficients, then apply madmom's log compression: log10(1 + 100*x).
  //
  // This is equivalent to: log_spec = dot(linear_mag, filterbank_matrix)
  //                         compressed = log10(1 + 100 * log_spec)

  // Build the compressed log-spectogram (Frames x 105)
  std::vector<std::vector<float>> logSpectogram(
      numFrames, std::vector<float>(numLogBins, 0.0f));
  for (int t = 0; t < numFrames; t++) {
    const float *frameData = spectogram.getReadPointer(t);

    for (int b = 0; b < numLogBins; b++) {
      const FilterBand &band = MADMOM_FILTERBANK[b];
      float linearSum = 0.0f;

      for (size_t k = 0; k < band.bins.size(); k++) {
        float linearMag = frameData[band.bins[k]];
        linearSum += band.weights[k] * linearMag;
      }

      // apply logarithmic compression of madmom (multiplier = 100) or is it
      // just 1?
      logSpectogram[t][b] = std::log10(1.0f + linearSum);
    }
  }

  // TODO: Export Spectogram in 2D Array structure in Json, same structure as in
  // Scripts/*_specto.json

  // =================================================================================
  // 2. ONNX Inference Preparation
  // =================================================================================
  Ort::MemoryInfo memoryInfo =
      Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
  Ort::AllocatorWithDefaultOptions allocator;

  // Safely allocate and retain strings for input and output node names
  auto inputNamePtr = ortSession->GetInputNameAllocated(0, allocator);
  auto outputNamePtr = ortSession->GetOutputNameAllocated(0, allocator);
  const char *inputNamesArray[] = {inputNamePtr.get()};
  const char *outputNamesArray[] = {outputNamePtr.get()};

  // Get shape requested by model, replace dynamic batch dimension (-1 or 0)
  // with 1
  Ort::TypeInfo inputTypeInfo = ortSession->GetInputTypeInfo(0);
  std::vector<int64_t> expectedShape =
      inputTypeInfo.GetTensorTypeAndShapeInfo().GetShape();
  for (auto &dim : expectedShape) {
    if (dim <= 0)
      dim = 1;
  }

  int contextSize = 15;              // specified by model
  int halfContext = contextSize / 2; // 7 frames look-back, 7 frames look-ahead
  std::vector<float> inputTensorValues(contextSize * numLogBins,
                                       0.0f); // Size: 1575

  // =================================================================================
  // 3. Sliding Window Inference
  // =================================================================================
  for (int t = 0; t < numFrames; t++) {
    // Reset the tensor with zeros (handles padding at start and end
    // automatically)
    std::fill(inputTensorValues.begin(), inputTensorValues.end(), 0.0f);

    int tensorIdx = 0;
    for (int c = -halfContext; c <= halfContext; c++) {
      int frameIndex = t + c;
      // Only copy data if the context frame is within the song bounds
      if (frameIndex >= 0 && frameIndex < numFrames) {
        std::copy(logSpectogram[frameIndex].begin(),
                  logSpectogram[frameIndex].end(),
                  inputTensorValues.begin() + tensorIdx * numLogBins);
      }

      tensorIdx++;
    }

    // Create ONNX Tensor wrapper around our memory
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo, inputTensorValues.data(), inputTensorValues.size(),
        expectedShape.data(), expectedShape.size());

    // Run Inference
    auto outputTensors =
        ortSession->Run(Ort::RunOptions{nullptr}, inputNamesArray, &inputTensor,
                        1, outputNamesArray, 1);

    // Map Output to Chroma Buffer
    float *outputData = outputTensors.front().GetTensorMutableData<float>();
    int outSize = static_cast<int>(
        outputTensors.front().GetTensorTypeAndShapeInfo().GetElementCount());

    float *chromaWrite = chroma.getWritePointer(t);

    for (int i = 0; i < std::min(12, outSize); i++) {
      chromaWrite[i] = outputData[i];
    }
  }

  // Apply Median Filter
  // applyMedianFilter(chroma);
}

void DeepChromaExtractor::applyMedianFilter(
    juce::AudioBuffer<float> &chroma) const {
  juce::AudioBuffer<float> ogChroma;
  ogChroma.makeCopyOf(chroma);

  const int medianWindowSize = medianSize;
  const int medianMid = medianWindowSize / 2;
  const int maxFrames = chroma.getNumChannels();
  int currentIndex = 0;

  // the loop from -medianMid to medianMid always produces 2 * medianMid + 1
  // iterations
  std::vector<float> currentValues(2 * medianMid + 1, 0.0f);

  for (int frame = 0; frame < maxFrames; frame++) {
    float *outputFrame = chroma.getWritePointer(frame);
    for (int bin = 0; bin < chroma.getNumSamples(); bin++) {
      // iterate over the bin of the left and right neighbours and calculate the
      // median.
      for (int i = -medianMid; i <= medianMid; i++) {
        // This ensures we dont access out of bounds elements!
        currentIndex = std::max(0, std::min(maxFrames - 1, frame + i));
        const float *neighbourFrame = ogChroma.getReadPointer(currentIndex);
        currentValues[i + medianMid] = neighbourFrame[bin];
      }

      auto first = currentValues.begin();
      auto nth = first + medianMid;
      auto last = currentValues.end();
      std::nth_element(first, nth, last);

      // The result of nth_element is garanteed to be at index medianMid.
      outputFrame[bin] = currentValues[medianMid];
    }
  }
}

int DeepChromaExtractor::getChromaBinSize() const { return chromaBinSize; }
