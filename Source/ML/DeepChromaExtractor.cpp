#include "DeepChromaExtractor.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "onnxruntime_c_api.h"
#include "onnxruntime_cxx_api.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

DeepChromaExtractor::DeepChromaExtractor(int binSize)
    : chromaBinSize(binSize)
{ loadModel(); }

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

void DeepChromaExtractor::extractChroma(
    const juce::AudioBuffer<float> &spectogram,
    juce::AudioBuffer<float> &chroma) {}

int DeepChromaExtractor::getChromaBinSize() const {
   return chromaBinSize;
}
