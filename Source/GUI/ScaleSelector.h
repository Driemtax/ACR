#pragma once

#include "../Util/ScaleDatabase.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <functional>

class ScaleSelector : public juce::Component {
public:
  ScaleSelector();
  void resized() override;

  // callback if user changes scale
  std::function<void(int rootNote, ScaleDatabase::ScaleType type)>
      onScaleChanged;

  void setScale(int rootNote, ScaleDatabase::ScaleType type);

private:
  juce::ComboBox keySelector;
  juce::ComboBox scaleTypeSelector;
  juce::Label keyLabel, scaleLabel;

  void selectionChanged();
};
