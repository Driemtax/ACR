#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>

class ChromaDisplay : public juce::Component {
    public:
    ChromaDisplay();

    void setChromaData(const juce::AudioBuffer<float> &chroma);
    void mouseDown(const juce::MouseEvent &e) override;
    void paint(juce::Graphics &g) override;

    private:
    juce::Image chromaImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChromaDisplay);
};
