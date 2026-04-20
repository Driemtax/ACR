#pragma once
#include <JuceHeader.h>
#include "../Audio/AudioEngine.h"

class WaveformDisplay : public juce::Component, public juce::Timer
{
public:
    WaveformDisplay(AudioEngine& engineToUse);
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void timerCallback() override;

private:
    AudioEngine& audioEngine;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
