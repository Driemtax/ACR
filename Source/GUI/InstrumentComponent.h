#pragma once

#include "../Audio/AudioEngine.h"
#include "ChordInfoPanel.h"
#include "GuitarView.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>

class InstrumentComponent : public juce::Component {
public:
  InstrumentComponent(AudioEngine &engine);

  GuitarView &getGuitarView() { return guitarView; }
  ChordInfoPanel &getChordInfoPanel() { return chordInfoPanel; }

  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  GuitarView guitarView;
  ChordInfoPanel chordInfoPanel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentComponent)
};
