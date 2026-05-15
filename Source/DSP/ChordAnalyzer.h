#pragma once

#include <JuceHeader.h>
#include <vector>
#include "Classificator.h"
#include "ChromaAnalyzer.h"
#include "SpectogramAnalyzer.h"
#include "../Testing/Test.h"
#include "juce_audio_basics/juce_audio_basics.h"

class ChordAnalyzer {
  public:
      ChordAnalyzer();
      ChordAnalyzer(Test::TestConfig &config);
      ~ChordAnalyzer() = default;

      struct AnalysisResult {
        std::vector<std::vector<float>> spectogramData;
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

      double sampleRate;
      float fftSize;

      bool medianFilter = true;
      int medianWindowSize = 5;
      float s = 0.6f;
      float similarityThreshold = 0.8f;
      int chromaRes = 1;

};
