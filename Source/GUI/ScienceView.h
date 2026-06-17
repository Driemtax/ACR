#pragma once

#include "../Audio/AudioEngine.h"
#include "../DSP/AnalyzerConfig.h"
#include "../DSP/ChordAnalyzer.h"
#include "../DSP/Classificator.h"
#include "ChromaDisplay.h"
#include "SpectogramDisplay.h"
#include <JuceHeader.h>
#include <functional>
#include <memory>
#include <vector>

class ScienceView : public juce::Component {
public:
  ScienceView(AudioEngine &engine, AnalyzerConfig &config);

  std::function<void(const std::vector<Classificator::ChordSegment> &, double,
                     int)>
      onAnalysisCompletion;

  void resized() override;
  void paint(juce::Graphics &g) override;

private:
  AudioEngine &audioEngine;
  AnalyzerConfig &config;

  // Analysis controls
  juce::TextButton analyzeButton{"Analyze"};
  juce::TextButton configButton{"Config"};
  juce::TextButton testButton{"Test"};
  juce::Label loadingText{"Loading", "Analysing..."};

  // Displays
  SpectogramDisplay spectogramDisplay;
  ChromaDisplay chromaDisplay;

  // Analysis
  std::unique_ptr<ChordAnalyzer> chordAnalyzer;

  // Config window
  juce::Component::SafePointer<juce::DocumentWindow> settingsWindow;

  void runAnalysisOffline();
  void showAnalysisResults(ChordAnalyzer::AnalysisResult &result);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScienceView)
};
