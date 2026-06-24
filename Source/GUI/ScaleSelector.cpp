#include "ScaleSelector.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"

static const char *noteNames[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                    "F#", "G",  "G#", "A",  "A#", "B"};

ScaleSelector::ScaleSelector() {
  // Key selector
  for (int i = 0; i < 12; i++) {
    keySelector.addItem(noteNames[i], i + 1);
  }
  keySelector.setSelectedId(0); // default: no selection
  keySelector.onChange = [this] { selectionChanged(); };

  // Scale type selector
  const auto &scales = ScaleDatabase::getAllScales();
  for (int i = 0; i < static_cast<int>(scales.size()); i++) {
    scaleTypeSelector.addItem(scales[i].displayName, i + 1);
  }
  scaleTypeSelector.setSelectedId(0); // default: no selection
  scaleTypeSelector.onChange = [this] { selectionChanged(); };

  // Labels
  keyLabel.setText("Key", juce::dontSendNotification);
  keyLabel.setJustificationType(Justification::centredRight);
  keyLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

  scaleLabel.setText("Scale", juce::dontSendNotification);
  scaleLabel.setJustificationType(juce::Justification::centredRight);
  scaleLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

  addAndMakeVisible(keyLabel);
  addAndMakeVisible(keySelector);
  addAndMakeVisible(scaleLabel);
  addAndMakeVisible(scaleTypeSelector);
}

void ScaleSelector::resized() {
  auto area = getLocalBounds().reduced(4);

  auto leftHalf = area.removeFromLeft(area.getWidth() / 2);
  auto rightHalf = area;

  // Left half: Key label + dropdown
  keyLabel.setBounds(leftHalf.removeFromLeft(40));
  keySelector.setBounds(leftHalf.reduced(2));

  // Right: Scale label + combo
  scaleLabel.setBounds(rightHalf.removeFromLeft(45));
  scaleTypeSelector.setBounds(rightHalf.reduced(2));
}

void ScaleSelector::selectionChanged() {
  int rootId = keySelector.getSelectedId();
  int scaleId = scaleTypeSelector.getSelectedId();

  if (rootId <= 0 || scaleId <= 0) {
    return;
  }

  // Since the dropdown forbids id == 0, we added 1 to the actual id and have to
  // remove it here again.
  int rootNote = rootId - 1;
  int scaleIndex = scaleId - 1;

  const auto &scales = ScaleDatabase::getAllScales();

  if (scaleIndex < 0 || scaleIndex >= static_cast<int>(scales.size())) {
    return;
  }

  ScaleDatabase::ScaleType type = scales[scaleIndex].type;

  if (onScaleChanged) {
    onScaleChanged(rootNote, type);
  }
}
