#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>

class SpectogramDisplay : public juce::Component {
public:
  SpectogramDisplay();

  void setSpectogramData(const juce::AudioBuffer<float> &data,
                         double sampleRate, int hopSize);
  void mouseDown(const juce::MouseEvent &e) override;
  void paint(juce::Graphics &g) override;

private:
  juce::Image spectogramImage;
  double sampleRate = 0.0;
  int hopSize = 0;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectogramDisplay);
};
