#pragma once

#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <atomic>
#include <memory>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::AudioDeviceSelectorComponent audioSetupComp;

    // app state
    enum class TransportState {
      Stopped,
      Recording,
      Playing
    };

    TransportState state = TransportState::Stopped;

    // Buttons for recording and playing audio
    juce::TextButton recordButton { "Record" };
    juce::TextButton playButton { "Play" };


    // Recording audio to temp wav files
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" };

    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    // thread-safe pointer for GUI and audio thread to communicate
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
