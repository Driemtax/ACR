#include "ScienceView.h"
#include "../TestSetup/Test.h"
#include "ChromaDisplay.h"
#include "ConfigPopup.h"
#include "SpectogramDisplay.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <algorithm>
#include <memory>
#include <thread>

ScienceView::ScienceView(AudioEngine &engine, AnalyzerConfig &cfg)
    : audioEngine(engine), config(cfg), chromaDisplay(engine) {

  addAndMakeVisible(analyzeButton);
  addAndMakeVisible(configButton);
  addAndMakeVisible(testButton);
  addAndMakeVisible(loadingText);
  addAndMakeVisible(spectogramDisplay);
  addAndMakeVisible(chromaDisplay);

  loadingText.setJustificationType(juce::Justification::centred);
  loadingText.setColour(juce::Label::textColourId, juce::Colours::orange);
  loadingText.setVisible(false);

  analyzeButton.onClick = [this] {
    auto file = audioEngine.getAudioFilePath();
    if (!file.existsAsFile())
      return;

    analyzeButton.setEnabled(false);
    loadingText.setVisible(true);
    spectogramDisplay.setVisible(false);
    chromaDisplay.setVisible(false);

    if (config.useDeepLearning) {
      config.setToDeepLearningDefaults();
    }

    chordAnalyzer = std::make_unique<ChordAnalyzer>(config);
    std::thread([this]() { runAnalysisOffline(); }).detach();
  };

  configButton.onClick = [this] {
    if (settingsWindow != nullptr) {
      return;
    }
    settingsWindow = new SettingsWindow("Analysis Settings", config);
  };

  testButton.onClick = [this] {
    testButton.setEnabled(false);
    loadingText.setText("Running Tests...", juce::dontSendNotification);
    loadingText.setVisible(true);
    analyzeButton.setEnabled(false);

    std::thread([this]() {
      Test tester;
      // tester.runAllTests();
      tester.findMaxima();

      juce::MessageManager::callAsync([this]() {
        loadingText.setText("Tests Finished!", juce::dontSendNotification);
        testButton.setEnabled(true);
        analyzeButton.setEnabled(true);
      });
    }).detach();
  };
}

void ScienceView::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ScienceView::resized() {
  auto area = getLocalBounds();

  // Top row: buttons and loading label
  auto topRow = area.removeFromTop(40);
  analyzeButton.setBounds(topRow.removeFromLeft(120).reduced(2));
  configButton.setBounds(topRow.removeFromLeft(120).reduced(2));
  testButton.setBounds(topRow.removeFromLeft(120).reduced(2));
  loadingText.setBounds(topRow.removeFromLeft(200).reduced(2));

  // Remaining space: spectogram and chromagram side by side
  int halfWidth = area.getWidth() / 2;
  spectogramDisplay.setBounds(area.removeFromLeft(halfWidth).reduced(4));
  chromaDisplay.setBounds(area.reduced(4));
}

void ScienceView::showAnalysisResults(ChordAnalyzer::AnalysisResult &result) {
  loadingText.setVisible(false);
  analyzeButton.setEnabled(true);

  spectogramDisplay.setSpectogramData(result.spectogramData, result.sampleRate,
                                      result.hopSize);
  spectogramDisplay.setVisible(true);

  chromaDisplay.setChromaData(result.chromagramData, result.chordSegments,
                              result.sampleRate, result.hopSize);
  chromaDisplay.setVisible(true);
}

void ScienceView::runAnalysisOffline() {
  auto file = audioEngine.getAudioFilePath();
  auto result = chordAnalyzer->runAnalysis(file);

  auto segmentsCopy = result.chordSegments;
  double sampleRate = result.sampleRate;
  int hopSize = result.hopSize;

  juce::MessageManager::callAsync([this, res = std::move(result),
                                   segments = std::move(segmentsCopy),
                                   sampleRate, hopSize]() mutable {
    showAnalysisResults(res);
    if (onAnalysisCompletion) {
      onAnalysisCompletion(segments, sampleRate, hopSize);
    }
  });
}
