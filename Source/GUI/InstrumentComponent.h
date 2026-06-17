#pragma once

#include "../Audio/AudioEngine.h"
#include "../DSP/Classificator.h"
#include "ChordInfoPanel.h"
#include "GuitarView.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <vector>

class InstrumentComponent : public juce::Component, public juce::Timer {
public:
  InstrumentComponent(AudioEngine &engine);
  ~InstrumentComponent() override;

  GuitarView &getGuitarView() { return guitarView; }
  ChordInfoPanel &getChordInfoPanel() { return chordInfoPanel; }

  void setTimeline(const std::vector<Classificator::ChordSegment> &segments,
                   double sampleRate, int hopSize);

  void paint(juce::Graphics &g) override;
  void resized() override;
  void timerCallback() override;

private:
  AudioEngine &audioEngine;
  GuitarView guitarView;
  ChordInfoPanel chordInfoPanel;

  std::vector<Classificator::ChordSegment> timeline;
  double analysisSampleRate = 44100.0;
  int analysisHopSize = 512;

  int currentSegmentIndex = -1;
  void updateUIForSegment(int segmentIndex);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentComponent)
};
