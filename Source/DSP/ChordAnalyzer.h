#pragma once

#include "../ML/DeepChromaExtractor.h"
#include "AnalyzerConfig.h"
#include "ChromaAnalyzer.h"
#include "ChromaExtractorInterface.h"
#include "Classificator.h"
#include "KeyEstimator.h"
#include "SpectogramAnalyzer.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>
#include <memory>
#include <optional>
#include <vector>

class ChordAnalyzer {
public:
  ChordAnalyzer(AnalyzerConfig &config);
  ~ChordAnalyzer() = default;

  struct AnalysisResult {
    juce::AudioBuffer<float> spectogramData;
    juce::AudioBuffer<float> chromagramData;
    std::vector<Classificator::ChordSegment> chordSegments;
    std::vector<int> rawClassifications;
    double sampleRate;
    int hopSize;

    std::optional<KeyEstimator::Key> estimatedKey;
  };

  AnalysisResult runAnalysis(const juce::File &audioFile);

private:
  SpectogramAnalyzer spectoAnalyzer;
  Classificator classifier;
  KeyEstimator keyEstimator;

  double sampleRate = 44100.0;
  int fftOrder = 12;
  int fftSize = 4096; // 2^12 = 4096
  int hopSize = 512;

  bool medianFilter = true;
  int medianWindowSize = 5;
  float s = 0.6f;
  float similarityThreshold = 0.8f;
  int chromaRes = 1;
  int chromaSize = 12;
  bool tuningShift = true;
  float ratio = 3.8f;

  // Deep Chroma Extractor
  bool useDeepLearning = false;
  std::unique_ptr<ChromaExtractorInterface> chromaProcessor;

  bool useKeyEstimator = false;

  // Extraction for verification
  void exportBufferToJson(const juce::AudioBuffer<float> &buffer,
                          const juce::File &outputFile);
};
