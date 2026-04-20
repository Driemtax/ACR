#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay(AudioEngine& engineToUse) : audioEngine(engineToUse)
{
    startTimerHz(30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    // background dark grey
    g.fillAll(juce::Colours::darkgrey.darker());

    auto& thumbnail = audioEngine.getThumbnail();

    // If there is no data in thumbnail buffer the just display text
    if (thumbnail.getTotalLength() <= 0.0)
    {
        g.setColour(juce::Colours::white);
        g.drawFittedText("No Audio Recorded", getLocalBounds(), juce::Justification::centred, 1);
        return;
    }

    // 1. draw wave
    g.setColour(juce::Colours::lightblue);
    thumbnail.drawChannels(g, getLocalBounds(), 0.0, thumbnail.getTotalLength(), 1.0f);

    // 2. draw playhead
    auto state = audioEngine.getState();
    double currentPosition = 0.0;

    if (state == AudioEngine::TransportState::Playing) {
        // if we are playing audio get current position
        currentPosition = audioEngine.getTransportSource().getCurrentPosition();
    } else if (state == AudioEngine::TransportState::Recording) {
        // if we ware recording the playhead is always at the end of the wave where we currentyl write data
        currentPosition = thumbnail.getTotalLength();
    }

    // calculate the x-position of the playhead
    if (currentPosition > 0.0 && thumbnail.getTotalLength() > 0.0)
    {
        auto playheadPositionX = (currentPosition / thumbnail.getTotalLength()) * getWidth();

        g.setColour(juce::Colours::white);
        g.drawLine(playheadPositionX, 0, playheadPositionX, getHeight(), 2.0f);
    }
}
