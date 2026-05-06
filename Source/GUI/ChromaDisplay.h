#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <vector>
#include "../DSP/Classificator.h"

class ChromaDisplay : public juce::Component {
    public:
    ChromaDisplay();

    void setChromaData(const juce::AudioBuffer<float> &chroma, const std::vector<Classificator::ChordSegment> &segments,
        double sampleRate, int hopSize);
    void mouseDown(const juce::MouseEvent &e) override;
    void paint(juce::Graphics &g) override;

    private:
    juce::Image chromaImage;
    std::vector<Classificator::ChordSegment> currentSegments;

    double sampleRate = 0.0f;
    int hopSize = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChromaDisplay);
};
