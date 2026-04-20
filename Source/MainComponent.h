#pragma once

#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include "Audio/AudioEngine.h"
#include "GUI/WaveformDisplay.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,
                        private juce::AsyncUpdater,
                        private juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void handleAsyncUpdate() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    //==============================================================================
    AudioEngine audioEngine;
    juce::AudioDeviceSelectorComponent audioSetupComp;

    // Buttons for recording and playing audio
    juce::TextButton recordButton { "Record" };
    juce::TextButton playButton { "Play" };

    // Waveform component
    WaveformDisplay waveformDisplay { audioEngine };

    void updateTransportState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
