#pragma once

#include <JuceHeader.h>

class ChordInfoPanel : public juce::Component {
public:
  ChordInfoPanel();

  void setCurrentChord(const juce::String &name);
  void setNextChord(const juce::String &name);
  void clearChords();

  void paint(juce::Graphics &g) override;
  void resized() override;

private:
  juce::String currentChordName;
  juce::String nextChordName;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordInfoPanel)
};
