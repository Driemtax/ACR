#include "InstrumentComponent.h"
#include "GuitarView.h"
#include "juce_graphics/juce_graphics.h"

InstrumentComponent::InstrumentComponent(AudioEngine &engine)
    : guitarView(engine) {
  addAndMakeVisible(guitarView);
  addAndMakeVisible(chordInfoPanel);
}

void InstrumentComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(0xFF1A1A2E));
}

void InstrumentComponent::resized() {
  auto area = getLocalBounds();

  chordInfoPanel.setBounds(
      area.removeFromTop(static_cast<int>(getHeight() * 0.4f)));

  guitarView.setBounds(area);
}
