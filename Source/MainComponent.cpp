#include "MainComponent.h"
#include "Audio/AudioEngine.h"
#include "DSP/ChordAnalyzer.h"
#include "TestSetup/Test.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>
#include <thread>

const int WIDTH = 800;
const int HEIGHT = 800;

//==============================================================================
// Parameters of audioSetupComp: minInput, maxInput, minOutput, maxOutput, showMidiIn, showMidiOut, showStereo, hideAdvancedOptions
MainComponent::MainComponent() : audioSetupComp(audioEngine.getDeviceManager()
    , 0, 2, 0, 2, false, false, true, true)
{
    // Initialize and show Audio Device Selector dropdown
    addAndMakeVisible(audioSetupComp);

    // Add Buttons for playing and recording audio
    addAndMakeVisible(recordButton);
    addAndMakeVisible(playButton);

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

    loadingText.setJustificationType(juce::Justification::centred);
    loadingText.setColour(juce::Label::textColourId, juce::Colours::orange);
    loadingText.setVisible(false);

    // click functions for buttons
    recordButton.onClick = [this] {
        if (audioEngine.getState() == AudioEngine::TransportState::Stopped) {
            audioEngine.startRecording();
        } else if (audioEngine.getState() == AudioEngine::TransportState::Recording) {
            audioEngine.stop();
        }
        updateTransportState();
    };

    playButton.onClick = [this] {
      if (audioEngine.getState() == AudioEngine::TransportState::Stopped) {
          audioEngine.startPlayback();
      } else if (audioEngine.getState() == AudioEngine::TransportState::Playing) {
          audioEngine.stop();
      }
      updateTransportState();
    };

    analyzeButton.onClick = [this] {
        analyzeButton.setEnabled(false);
        loadingText.setVisible(true);
        spectogramDisplay.setVisible(false);
        chromaDisplay.setVisible(false);

        std::thread([this]() {
            runAnalysisOffline();
        }).detach();
    };

    fileButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select a WAV file",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory),
            "*.wav"
        );

        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc) {
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
            Test::TestConfig config;
            config.testName = "Baseline_Run_1";
            config.medianFilter = true;
            config.medianWindowSize = 5;
            config.similarityThreshold = 0.8f;
            config.s = 0.6f;

            juce::File testDataDir("C:/Users/a930084/OneDrive - ATOS/Dokumente/ACR_App");
            juce::File outputDir("C:/dev/ACR/TestResults");

            if (!outputDir.exists()) outputDir.createDirectory();

            tester.runTests(config, testDataDir, outputDir);

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

    // Make sure you set the size of the component after
    // you add any child components.
    setSize (WIDTH, HEIGHT);
}

MainComponent::~MainComponent()
{
    audioEngine.getTransportSource().removeChangeListener(this);
}

//==============================================================================
void MainComponent::updateTransportState()
{
    auto state = audioEngine.getState();

    if (state == AudioEngine::TransportState::Recording) {
        recordButton.setButtonText("Stop");
        recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled(false);
    }
    else if (state == AudioEngine::TransportState::Playing) {
        playButton.setButtonText("Stop");
        playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::greenyellow);
        recordButton.setEnabled(false);
    } else {
        // Stopped state
        recordButton.setButtonText("Record");
        recordButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::TextButton::buttonColourId));
        recordButton.setEnabled(true);

        playButton.setButtonText("Play");
        playButton.setColour(juce::TextButton::buttonColourId, getLookAndFeel().findColour(juce::TextButton::buttonColourId));
        playButton.setEnabled(true);
    }

    auto file = audioEngine.getAudioFilePath();
    if (file.existsAsFile()) {
        fileToAnalyze.setText("File: " + file.getFileName(), juce::dontSendNotification);
        analyzeButton.setEnabled(state == AudioEngine::TransportState::Stopped);
    } else {
        fileToAnalyze.setText("No file recorded yet.", juce::dontSendNotification);
        analyzeButton.setEnabled(false);
    }
}

void MainComponent::handleAsyncUpdate() {
    updateTransportState();
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster *source) {
    if (source == &audioEngine.getTransportSource()) {
        if (!audioEngine.getTransportSource().isPlaying()) {
            audioEngine.stop();
            triggerAsyncUpdate();
        }
    }
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    // Audio Device Selector on top
    audioSetupComp.setBounds(10, 10, getWidth() - 20, 200);

    // Play and Record Buttons
    recordButton.setBounds(10, 380, 120, 40);
    playButton.setBounds(140, 380, 120, 40);

    // waveform component
    waveformDisplay.setBounds(10, 220, getWidth() - 20, 150);

    fileToAnalyze.setBounds(270, 380, 200, 40);
    analyzeButton.setBounds(480, 380, 120, 40);
    loadingText.setBounds(270, 430, 180, 40);
    int spectoWidth = (getWidth() - 20) / 2;
    spectogramDisplay.setBounds(10, 430, spectoWidth, 350);

    chromaDisplay.setBounds(spectoWidth + 10, 430, spectoWidth, 350);

    fileButton.setBounds(620, 380, 120, 40);

    testButton.setBounds(780, 380, 120, 40);
}

void MainComponent::runAnalysisOffline() {
   auto file = audioEngine.getAudioFilePath();
   auto result = chordAnalyzer.runAnalysis(file);
   // This functions runs in a seperate thread and notifys the calling thread (GUI-Thread) when it has finished.
   // Then this function will be executed.
   juce::MessageManager::callAsync([this, res = std::move(result)]() {
       loadingText.setVisible(false);
       analyzeButton.setEnabled(true);

       spectogramDisplay.setSpectogramData(res.spectogramData);
       spectogramDisplay.setVisible(true);

       chromaDisplay.setChromaData(res.chromagramData, res.chordSegments, res.sampleRate, res.hopSize);
       chromaDisplay.setVisible(true);
   });
}
