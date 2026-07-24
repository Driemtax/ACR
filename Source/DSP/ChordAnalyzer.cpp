#include "ChordAnalyzer.h"
#include "AnalyzerConfig.h"
#include "ChromaAnalyzer.h"
#include "Classificator.h"
#include "KeyEstimator.h"
#include "SpectogramAnalyzer.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_core/juce_core.h"
#include <memory>

ChordAnalyzer::ChordAnalyzer(AnalyzerConfig &config)
    : classifier(config.similarityThreshold), spectoAnalyzer(config),
      fftOrder(config.fftOrder), fftSize(config.fftSize),
      hopSize(config.hopSize), medianFilter(config.medianFilter),
      medianWindowSize(config.medianWindowSize), s(config.s),
      similarityThreshold(config.similarityThreshold),
      chromaRes(config.chromaRes), useDeepLearning(config.useDeepLearning),
      tuningShift(config.tuningShift), useKeyEstimator(config.useKeyEstimator),
      keyEstimator(config.profileType), ratio(config.sptRatio) {}

/**
 * Runs the complete chord analysis process on a given audio file.
 *
 * This method reads the audio file, computes its spectrogram, extracts
 * the chromagram, and classifies the resulting data into chord segments.
 *
 * @param audioFile The audio file to be analyzed.
 * @return An AnalysisResult object containing the computed spectrogram,
 * chromagram, and chord segments.
 */
ChordAnalyzer::AnalysisResult
ChordAnalyzer::runAnalysis(const juce::File &audioFile) {
  AnalysisResult result;

  // 1. Read in audio file
  juce::AudioFormatManager formatManager;
  formatManager.registerBasicFormats();

  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(audioFile));
  if (reader == nullptr)
    return result;

  double sourceSampleRate = reader->sampleRate;
  const double targetSampleRate = 44100.0;

  juce::AudioBuffer<float> buffer;

  // If the file is not at sample rate 44,1kHz we need to resample!
  if (sourceSampleRate != targetSampleRate) {
    double ratio = sourceSampleRate / targetSampleRate;

    // calculatet sample count new buffer needs
    int targetLength = static_cast<int>(reader->lengthInSamples / ratio);
    buffer.setSize(reader->numChannels, targetLength);

    // temp buffer for original data
    juce::AudioBuffer<float> tempBuffer(
        reader->numChannels, static_cast<int>(reader->lengthInSamples));
    reader->read(&tempBuffer, 0, static_cast<int>(reader->lengthInSamples), 0,
                 true, true);

    // JUCE Interpolator for resampling every channel
    for (int ch = 0; ch < reader->numChannels; ch++) {
      juce::WindowedSincInterpolator interpolator;
      interpolator.process(ratio, tempBuffer.getReadPointer(ch),
                           buffer.getWritePointer(ch), targetLength);
    }
  } else {
    buffer.setSize(reader->numChannels,
                   static_cast<int>(reader->lengthInSamples));
    reader->read(&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true,
                 true);
  }

  // 2. Create Spectogram
  auto spectogramData = spectoAnalyzer.processFullFile(buffer);

  int numFrames = static_cast<int>(spectogramData.getNumChannels());
  int numBins =
      spectogramData.hasBeenCleared() ? 0 : (int)spectogramData.getNumSamples();

  // std::cout << "=== SPEKTOGRAMM BERECHNET ===" << std::endl;
  // std::cout << "FFT Size : " << fftSize
  //           << ", Hop Size: " << spectoAnalyzer.getHopSize() << std::endl;
  // std::cout << "Anzahl Frames (Zeit): " << numFrames << std::endl;
  if (!spectogramData.hasBeenCleared()) {
    // std::cout << "Anzahl Bins (Frequenz): " << numBins << std::endl;
  }

  if (useDeepLearning) {
    chromaProcessor = std::make_unique<DeepChromaExtractor>(
        chromaRes * chromaSize, medianWindowSize);
  } else {
    chromaProcessor = std::make_unique<ChromaAnalyzer>(
        static_cast<float>(targetSampleRate), static_cast<float>(fftSize), s,
        chromaRes, medianWindowSize, medianFilter, tuningShift, ratio);
  }

  // Create Chromagram
  int chromaBins = chromaProcessor->getChromaBinSize();

  auto chromagram = juce::AudioBuffer<float>(numFrames, chromaBins);
  chromaProcessor->extractChroma(spectogramData, chromagram);

  // Key estimation
  result.estimatedKey = keyEstimator.estimateKey(chromagram);

  // smooth chroma with key profile
  if (useKeyEstimator && result.estimatedKey.has_value()) {
    keyEstimator.applyKeyWeights(chromagram, result.estimatedKey.value());
  }

  // classify
  std::vector<int> classifiedFrames;
  classifiedFrames.resize(chromagram.getNumChannels());
  classifier.classifyFullChroma(chromagram, classifiedFrames);
  std::vector<Classificator::ChordSegment> chordSegments =
      classifier.getGroupedSegments(classifiedFrames);

  // Since the DeepLearning model uses linear scaled magnitudes we need to
  // convert them to logarithmic scaled decibels before visualizing them.
  if (useDeepLearning) {
    for (int f = 0; f < numFrames; f++) {
      float *frame = spectogramData.getWritePointer(f);
      for (int b = 0; b < numBins; b++) {
        frame[b] = juce::Decibels::gainToDecibels(frame[b], -100.0f);
      }
    }
  }

  // Debug verification
  // Set this to true if you want to export the chroma as a json.
  // I used the json to verify my model outputs with the madmom library outputs.
  bool exportForPython = false;
  if (exportForPython) {
    juce::File desktop =
        juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
    // juce::File spectoFile = desktop.getChildFile(
    // audioFile.getFileNameWithoutExtension() + "_specto.json");
    juce::File chromaFile = desktop.getChildFile(
        audioFile.getFileNameWithoutExtension() + "_chroma.json");

    // exportBufferToJson(spectogramData, spectoFile);
    exportBufferToJson(chromagram, chromaFile);
    std::cout << "JSON Files exported to Desktop!" << std::endl;
  }
  // save all results
  result.spectogramData = std::move(spectogramData);
  result.chromagramData = std::move(chromagram);
  result.chordSegments = std::move(chordSegments);
  result.rawClassifications = std::move(classifiedFrames);
  result.sampleRate = spectoAnalyzer.getSampleRate();
  result.hopSize = spectoAnalyzer.getHopSize();

  return result;
}

void ChordAnalyzer::exportBufferToJson(const juce::AudioBuffer<float> &buffer,
                                       const juce::File &outputFile) {
  juce::DynamicObject::Ptr jsonRoot = new juce::DynamicObject();
  juce::Array<juce::var> framesArray;

  int numFrames = buffer.getNumChannels();
  int numBins = buffer.getNumSamples();

  for (int t = 0; t < numFrames; t++) {
    const float *frameData = buffer.getReadPointer(t);
    juce::Array<juce::var> binsArray;

    for (int b = 0; b < numBins; b++) {
      binsArray.add(juce::var(static_cast<double>(frameData[b])));
    }
    framesArray.add(juce::var(binsArray));
  }

  juce::String jsonString = juce::JSON::toString(juce::var(framesArray));
  outputFile.replaceWithText(jsonString);
}
