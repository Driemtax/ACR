#pragma once

#include "../DSP/AnalyzerConfig.h"
#include "../DSP/Classificator.h"
#include "juce_core/juce_core.h"
#include <JuceHeader.h>
#include <vector>

class Test {
public:
  struct TestConfig {
    int method = 0;
    juce::String testName = "";

    // FFT
    int fftSize = 4096;
    int fftOrder = 12;
    int hopSize = 512;

    // flags
    bool medianFilter = false;
    int medianWindowSize = 5;

    // Parameter for harmonic weight calculation of hpcp
    int chromaRes = 1;
    float s = 0.6f;

    bool useDeepChroma = false;

    // Classification
    float similarityThreshold = 0.3f;
  };

  Test() = default;
  ~Test() = default;

  void runAllTests() const;
  float runTests(AnalyzerConfig &config, const juce::String testFileName,
                 bool logToConsole) const;
  void findMaxima() const;

private:
  struct GroundTruthLabel {
    float startTimeSec;
    float endTimeSec;
    juce::String chordName;
  };

  void findMaximaMedianWindowSize() const;
  void findMaximaFloatParameters() const;

  std::vector<GroundTruthLabel>
  parseGroundTruth(const juce::File &labelFile) const;
  juce::var createJSONForTrack(const juce::String &songName,
                               const std::vector<GroundTruthLabel> &groundTruth,
                               const std::vector<int> &predictions,
                               double sampleRate, int hopSize,
                               float trackAccuracy) const;

  void saveResultsToJSON(const juce::var &resultsData,
                         const juce::File &outputFile) const;
  void evaluateTrackAccuracy(const std::vector<GroundTruthLabel> &groundTruth,
                             const std::vector<int> &predictions,
                             double sampleRate, int hopSize,
                             int &outCorrectFrames, int &outTotalFrames) const;

  const juce::File appDir = juce::File::getSpecialLocation(
      juce::File::SpecialLocationType::currentApplicationFile);
  const juce::File projectDir = appDir.getParentDirectory()
                                    .getParentDirectory()
                                    .getParentDirectory()
                                    .getParentDirectory();
  const juce::File testDataDir = projectDir.getFullPathName() +
                                 juce::File::getSeparatorString() + "Testfiles";
  const juce::File outputDir = projectDir.getFullPathName() +
                               juce::File::getSeparatorString() + "TestResults";
};
