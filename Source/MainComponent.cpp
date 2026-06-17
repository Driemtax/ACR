#include "MainComponent.h"
#include "Audio/AudioEngine.h"
#include "DSP/Classificator.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include <memory>
#include <vector>

// =============================================================================
// Helper: wraps AudioDeviceSelectorComponent in its own window
// =============================================================================
class AudioSettingsWindow : public juce::DocumentWindow {
public:
  AudioSettingsWindow(juce::AudioDeviceManager &dm)
      : juce::DocumentWindow("Audio Settings", juce::Colours::darkgrey,
                             DocumentWindow::closeButton),
        audioSetupComp(dm, 0, 2, 0, 2, false, false, true, true) {
    setContentNonOwned(&audioSetupComp, true);
    setResizable(true, false);
    centreWithSize(450, 350);
    setVisible(true);
  }

  void closeButtonPressed() override { delete this; }

private:
  juce::AudioDeviceSelectorComponent audioSetupComp;
};

// =============================================================================
// MainComponent
// =============================================================================

MainComponent::MainComponent() {
  // Transport controls
  addAndMakeVisible(recordButton);
  addAndMakeVisible(playButton);
  addAndMakeVisible(stopButton);
  addAndMakeVisible(fileToAnalyze);
  addAndMakeVisible(fileButton);
  addAndMakeVisible(settingsButton);

  // Waveform
  addAndMakeVisible(waveformDisplay);

  // Tab buttons
  addAndMakeVisible(instrumentTab);
  addAndMakeVisible(scienceTab);

  // Views (both added, visibility controlled by switchToView)
  addAndMakeVisible(instrumentView);
  addAndMakeVisible(scienceView);

  // --- Button callbacks ---

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

  settingsButton.onClick = [this] {
    if (audioSettingsWindow != nullptr)
      return;
    audioSettingsWindow =
        new AudioSettingsWindow(audioEngine.getDeviceManager());
  };

  // Tab switching
  instrumentTab.onClick = [this] { switchToView(ActiveView::Instrument); };
  scienceTab.onClick = [this] { switchToView(ActiveView::Science); };

  // Callback for analysis results
  scienceView.onAnalysisCompletion =
      [this](const std::vector<Classificator::ChordSegment> &segments,
             double sampleRate, int hopSize) {
        instrumentView.setTimeline(segments, sampleRate, hopSize);
      };

  // Initial state
  switchToView(ActiveView::Instrument);
  updateTransportState();
  audioEngine.getTransportSource().addChangeListener(this);
}

MainComponent::~MainComponent() {
  audioEngine.getTransportSource().removeChangeListener(this);
}

// =============================================================================
// View switching
// =============================================================================

void MainComponent::switchToView(ActiveView view) {
  activeView = view;

  instrumentView.setVisible(view == ActiveView::Instrument);
  scienceView.setVisible(view == ActiveView::Science);

  // Highlight active tab
  auto defaultColour =
      getLookAndFeel().findColour(juce::TextButton::buttonColourId);
  auto activeColour = juce::Colours::steelblue;

  instrumentTab.setColour(juce::TextButton::buttonColourId,
                          view == ActiveView::Instrument ? activeColour
                                                         : defaultColour);
  scienceTab.setColour(juce::TextButton::buttonColourId,
                       view == ActiveView::Science ? activeColour
                                                   : defaultColour);

  resized();
}

// =============================================================================
// Transport state
// =============================================================================

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
    // Stopped
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
  } else {
    fileToAnalyze.setText("No file loaded.", juce::dontSendNotification);
  }
}

void MainComponent::handleAsyncUpdate() { updateTransportState(); }

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster *source) {
  if (source == &audioEngine.getTransportSource()) {
    if (!audioEngine.getTransportSource().isPlaying()) {
      double currentPos = audioEngine.getTransportSource().getCurrentPosition();
      double totalLen = audioEngine.getTransportSource().getLengthInSeconds();

      if (currentPos >= totalLen - 0.1) {
        audioEngine.stop();
        triggerAsyncUpdate();
      }
    }
  }
}

// =============================================================================
// Paint & Layout
// =============================================================================

void MainComponent::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
  auto area = getLocalBounds();

  // --- Transport Bar (top, 40px) ---
  auto transportBar = area.removeFromTop(40);
  recordButton.setBounds(transportBar.removeFromLeft(100).reduced(4));
  playButton.setBounds(transportBar.removeFromLeft(100).reduced(4));
  stopButton.setBounds(transportBar.removeFromLeft(100).reduced(4));
  fileToAnalyze.setBounds(transportBar.removeFromLeft(200).reduced(4));
  fileButton.setBounds(transportBar.removeFromLeft(110).reduced(4));
  settingsButton.setBounds(transportBar.removeFromRight(100).reduced(4));

  // --- Waveform (120px) ---
  waveformDisplay.setBounds(area.removeFromTop(120).reduced(4));

  // --- Tab Bar (36px) ---
  auto tabBar = area.removeFromTop(36);
  instrumentTab.setBounds(tabBar.removeFromLeft(150).reduced(2));
  scienceTab.setBounds(tabBar.removeFromLeft(150).reduced(2));

  // --- Active View (remaining space) ---
  if (activeView == ActiveView::Instrument)
    instrumentView.setBounds(area);
  else
    scienceView.setBounds(area);
}
