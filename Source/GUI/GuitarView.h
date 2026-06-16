#pragma once

#include "../Audio/AudioEngine.h"
#include "juce_core/juce_core.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <vector>

class GuitarView : public juce::Component {
public:
  struct FretLabel {
    int string; // 1-6 (1 = low E, 6 = high E)
    int fret;   // 0-22 (0 = open string)
    juce::String text;
    juce::Colour colour = juce::Colours::steelblue;
  };

  GuitarView(AudioEngine &engine);

  void setLabels(const std::vector<FretLabel> &labels);
  void clearLabels();

  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  AudioEngine &audioEngine;
  std::vector<FretLabel> activeLabels;

  static constexpr int numStrings = 6;
  static constexpr int numFrets = 22;

  // Coordinate helpers
  juce::Rectangle<float> getFretboardBounds() const;
  float getFretX(int fret) const;
  float getStringY(int string) const;

  // Drawing layers (painted back-to-front)
  void drawFretboard(juce::Graphics &g);
  void drawInlays(juce::Graphics &g);
  void drawFrets(juce::Graphics &g);
  void drawNut(juce::Graphics &g);
  void drawStrings(juce::Graphics &g);
  void drawFretNumbers(juce::Graphics &g);
  void drawLabels(juce::Graphics &g);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuitarView)
};
