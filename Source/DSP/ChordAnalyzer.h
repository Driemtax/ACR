#pragma once

#include <JuceHeader.h>
#include <memory>
#include <vector>
#include "ChromaExtractorInterface.h"
#include "Classificator.h"
#include "ChromaAnalyzer.h"
#include "../ML/DeepChromaExtractor.h"
#include "SpectogramAnalyzer.h"
#include "../TestSetup/Test.h"
#include "juce_audio_basics/juce_audio_basics.h"

class ChordAnalyzer {
  public:
      ChordAnalyzer();
      ChordAnalyzer(Test::TestConfig &config);
      ~ChordAnalyzer() = default;

      struct AnalysisResult {
        juce::AudioBuffer<float> spectogramData;
        juce::AudioBuffer<float> chromagramData;
        std::vector<Classificator::ChordSegment> chordSegments;
        std::vector<int> rawClassifications;
        double sampleRate;
        int hopSize;
      };

      AnalysisResult runAnalysis(const juce::File& audioFile);

  private:
      SpectogramAnalyzer spectoAnalyzer;
      Classificator classifier;

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

      // Deep Chroma Extractor
      bool useDeepLearning = false;
      std::unique_ptr<ChromaExtractorInterface> chromaProcessor;
};
