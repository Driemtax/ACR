#include "ChordInfoPanel.h"

ChordInfoPanel::ChordInfoPanel() {}

void ChordInfoPanel::setCurrentChord(const juce::String &name) {
  currentChordName = name;
  repaint();
}

void ChordInfoPanel::setNextChord(const juce::String &name) {
  nextChordName = name;
  repaint();
}

void ChordInfoPanel::clearChords() {
  currentChordName.clear();
  nextChordName.clear();
  repaint();
}

void ChordInfoPanel::paint(juce::Graphics &g) {
  auto area = getLocalBounds();

  // Background
  g.fillAll(juce::Colour(0xFF16213E));

  // Subtle top border to separate from fretboard
  g.setColour(juce::Colours::white.withAlpha(0.1f));
  g.drawLine(0.0f, 0.0f, static_cast<float>(getWidth()), 0.0f, 1.0f);

  // Split into left (current) and right (next)
  auto leftHalf = area.removeFromLeft(area.getWidth() / 2).reduced(10);
  auto rightHalf = area.reduced(10);

  // --- Current Chord ---
  g.setColour(juce::Colours::grey);
  g.setFont(14.0f);
  g.drawText("Now Playing", leftHalf.removeFromTop(24),
             juce::Justification::centred);

  if (currentChordName.isNotEmpty()) {
    g.setColour(juce::Colours::white);
    g.setFont(48.0f);
    g.drawText(currentChordName, leftHalf, juce::Justification::centred);
  } else {
    g.setColour(juce::Colours::darkgrey);
    g.setFont(28.0f);
    g.drawText("--", leftHalf, juce::Justification::centred);
  }

  // --- Next Chord ---
  g.setColour(juce::Colours::grey);
  g.setFont(14.0f);
  g.drawText("Up Next", rightHalf.removeFromTop(24),
             juce::Justification::centred);

  if (nextChordName.isNotEmpty()) {
    g.setColour(juce::Colours::orange);
    g.setFont(36.0f);
    g.drawText(nextChordName, rightHalf, juce::Justification::centred);
  } else {
    g.setColour(juce::Colours::darkgrey);
    g.setFont(28.0f);
    g.drawText("--", rightHalf, juce::Justification::centred);
  }
}

void ChordInfoPanel::resized() {}
