#pragma once

#include <JuceHeader.h>
#include <vector>

class SpectogramDisplay : public juce::Component {
public:
  SpectogramDisplay();

  void setSpectogramData(const std::vector<std::vector<float>> &data);
  void mouseDown(const juce::MouseEvent& e) override;
  void paint(juce::Graphics &g) override;

private:
  juce::Image spectogramImage;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectogramDisplay);
};
