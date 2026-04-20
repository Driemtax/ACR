#include "MainComponent.h"
#include "Audio/AudioEngine.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

const int WIDTH = 800;
const int HEIGHT = 600;

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

    // add waveform component
    addAndMakeVisible(waveformDisplay);

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
}
