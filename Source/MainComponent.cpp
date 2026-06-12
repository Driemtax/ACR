#include "MainComponent.h"
#include "Audio/AudioEngine.h"
#include "DSP/AnalyzerConfig.h"
#include "DSP/ChordAnalyzer.h"
#include "GUI/ConfigPopup.h"
#include "TestSetup/Test.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>
#include <thread>

//==============================================================================
// Parameters of audioSetupComp: minInput, maxInput, minOutput, maxOutput,
// showMidiIn, showMidiOut, showStereo, hideAdvancedOptions
MainComponent::MainComponent()
    : audioSetupComp(audioEngine.getDeviceManager(), 0, 2, 0, 2, false, false,
                     true, true) {
  // Initialize and show Audio Device Selector dropdown
  addAndMakeVisible(audioSetupComp);

  // Add Buttons for playing and recording audio
  addAndMakeVisible(recordButton);
  addAndMakeVisible(playButton);
  addAndMakeVisible(stopButton);

  // waveform component
  addAndMakeVisible(waveformDisplay);

  // spectogram components
  addAndMakeVisible(fileToAnalyze);
  addAndMakeVisible(analyzeButton);
  addAndMakeVisible(loadingText);
  addAndMakeVisible(spectogramDisplay);

  // chromagram components
  addAndMakeVisible(chromaDisplay);

  // File selection
  addAndMakeVisible(fileButton);

  // Test button
  addAndMakeVisible(testButton);

  // config Button
  addAndMakeVisible(configButton);

  loadingText.setJustificationType(juce::Justification::centred);
  loadingText.setColour(juce::Label::textColourId, juce::Colours::orange);
  loadingText.setVisible(false);

  // click functions for buttons
  recordButton.onClick = [this] {
    if (audioEngine.getState() == AudioEngine::TransportState::Stopped) {
      audioEngine.startRecording();
    } else if (audioEngine.getState() ==
               AudioEngine::TransportState::Recording) {
      audioEngine.stop();
    }
    updateTransportState();
  };

  playButton.onClick = [this] {
    auto currentState = audioEngine.getState();
    if (currentState == AudioEngine::TransportState::Stopped ||
        currentState == AudioEngine::TransportState::Paused) {
      audioEngine.startPlayback();
    } else if (currentState == AudioEngine::TransportState::Playing) {
      audioEngine.pausePlayback();
    }
    updateTransportState();
  };

  stopButton.onClick = [this] {
    auto currentState = audioEngine.getState();
    if (currentState == AudioEngine::TransportState::Playing ||
        currentState == AudioEngine::TransportState::Paused) {
      audioEngine.stop();
    }

    updateTransportState();
  };

  analyzeButton.onClick = [this] {
    analyzeButton.setEnabled(false);
    loadingText.setVisible(true);
    spectogramDisplay.setVisible(false);
    chromaDisplay.setVisible(false);

    if (config.useDeepLearning) {
      config.setToDeepLearningDefaults();
    }

    // initialize chordAnalyzer
    chordAnalyzer = std::make_unique<ChordAnalyzer>(config);

    std::thread([this]() { runAnalysisOffline(); }).detach();
  };

  configButton.onClick = [this] {
    if (settingsWindow != nullptr) {
      return;
    }

    settingsWindow = new SettingsWindow("Analysis Settings", config);
  };

  fileButton.onClick = [this] {
    juce::File appDir =
        audioEngine.getDefaultRecordingFile().getParentDirectory();
    fileChooser = std::make_unique<juce::FileChooser>("Select an audio file",
                                                      appDir, "*.wav,*.mp3");

    auto chooserFlags = juce::FileBrowserComponent::openMode |
                        juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser &fc) {
      auto file = fc.getResult();
      if (file.existsAsFile()) {
        audioEngine.setAudioFile(file);
        updateTransportState();
      }
    });
  };

  testButton.onClick = [this] {
    testButton.setEnabled(false);
    loadingText.setText("Running Tests...", juce::dontSendNotification);
    loadingText.setVisible(true);
    analyzeButton.setEnabled(false);
    fileButton.setEnabled(false);
    playButton.setEnabled(false);
    recordButton.setEnabled(false);

    std::thread([this]() {
      Test tester;
      // tester.runAllTests();
      tester.findMaxima();

      // Update GUI, when Tests have finished
      juce::MessageManager::callAsync([this]() {
        loadingText.setText("Tests Finished!", juce::dontSendNotification);
        testButton.setEnabled(true);
        recordButton.setEnabled(true);
        playButton.setEnabled(true);
        analyzeButton.setEnabled(true);
        fileButton.setEnabled(true);
      });
    }).detach();
  };

  updateTransportState();
  audioEngine.getTransportSource().addChangeListener(this);
}

MainComponent::~MainComponent() {
  audioEngine.getTransportSource().removeChangeListener(this);
}

//==============================================================================
void MainComponent::updateTransportState() {
  auto state = audioEngine.getState();

  if (state == AudioEngine::TransportState::Recording) {
    recordButton.setButtonText("Stop");
    recordButton.setColour(juce::TextButton::buttonColourId,
                           juce::Colours::green);
    playButton.setEnabled(false);
  } else if (state == AudioEngine::TransportState::Playing) {
    playButton.setButtonText("Pause");
    playButton.setColour(juce::TextButton::buttonColourId,
                         juce::Colours::greenyellow);
    stopButton.setButtonText("Stop");
    recordButton.setEnabled(false);
  } else if (state == AudioEngine::TransportState::Paused) {
    playButton.setButtonText("Resume");
    playButton.setColour(juce::TextButton::buttonColourId,
                         juce::Colours::orange);
    stopButton.setButtonText("Stop");

  } else {
    // Stopped state
    recordButton.setButtonText("Record");
    recordButton.setColour(
        juce::TextButton::buttonColourId,
        getLookAndFeel().findColour(juce::TextButton::buttonColourId));
    recordButton.setEnabled(true);

    playButton.setButtonText("Play");
    playButton.setColour(
        juce::TextButton::buttonColourId,
        getLookAndFeel().findColour(juce::TextButton::buttonColourId));
    playButton.setEnabled(true);
  }

  auto file = audioEngine.getAudioFilePath();
  if (file.existsAsFile()) {
    fileToAnalyze.setText("File: " + file.getFileName(),
                          juce::dontSendNotification);
    analyzeButton.setEnabled(state == AudioEngine::TransportState::Stopped);
  } else {
    fileToAnalyze.setText("No file recorded yet.", juce::dontSendNotification);
    analyzeButton.setEnabled(false);
  }
}

void MainComponent::handleAsyncUpdate() { updateTransportState(); }

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster *source) {
  if (source == &audioEngine.getTransportSource()) {
    // check if the end of file was reached
    if (!audioEngine.getTransportSource().isPlaying()) {
      double currentPos = audioEngine.getTransportSource().getCurrentPosition();
      double totalLen = audioEngine.getTransportSource().getLengthInSeconds();

      // only stop if the end of file was reached
      if (currentPos >= totalLen - 0.1) {
        audioEngine.stop();
        triggerAsyncUpdate();
      }
    }
  }
}

//==============================================================================
void MainComponent::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  // You can add your drawing code here!
}

void MainComponent::resized() {
  // This is called when the MainContentComponent is resized.
  // If you add any child components, this is where you should
  // update their positions.

  // Audio Device Selector on top
  audioSetupComp.setBounds(10, 10, getWidth() - 20, 200);

  // Play and Record Buttons
  recordButton.setBounds(10, 380, 120, 40);
  playButton.setBounds(140, 380, 120, 40);
  stopButton.setBounds(270, 380, 120, 40);

  // waveform component
  waveformDisplay.setBounds(10, 220, getWidth() - 20, 150);

  fileToAnalyze.setBounds(400, 380, 200, 40);
  analyzeButton.setBounds(610, 380, 120, 40);
  loadingText.setBounds(270, 430, 180, 40);
  int spectoWidth = (getWidth() - 20) / 2;
  spectogramDisplay.setBounds(10, 430, spectoWidth, 350);

  chromaDisplay.setBounds(spectoWidth + 10, 430, spectoWidth, 350);

  fileButton.setBounds(750, 380, 120, 40);

  testButton.setBounds(910, 380, 120, 40);

  configButton.setBounds(1040, 380, 120, 40);
}

void MainComponent::runAnalysisOffline() {
  auto file = audioEngine.getAudioFilePath();
  auto result = chordAnalyzer->runAnalysis(file);
  // This functions runs in a seperate thread and notifys the calling thread
  // (GUI-Thread) when it has finished. Then this function will be executed.
  juce::MessageManager::callAsync([this, res = std::move(result)]() {
    loadingText.setVisible(false);
    analyzeButton.setEnabled(true);

    spectogramDisplay.setSpectogramData(res.spectogramData, res.sampleRate,
                                        res.hopSize);
    spectogramDisplay.setVisible(true);

    chromaDisplay.setChromaData(res.chromagramData, res.chordSegments,
                                res.sampleRate, res.hopSize);
    chromaDisplay.setVisible(true);
  });
}
