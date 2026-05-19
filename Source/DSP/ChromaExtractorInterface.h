#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>

class ChromaExtractorInterface {
  public:
      virtual ~ChromaExtractorInterface() = default;

      virtual void extractChroma(const juce::AudioBuffer<float> &spectogram, juce::AudioBuffer<float> &chroma) = 0;
};
