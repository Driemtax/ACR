#pragma once

#include "../GUI/GuitarView.h"
#include "ScaleDatabase.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include <JuceHeader.h>
#include <vector>

// Utility class: maps chord names to fretboard positions.
class FretboardMapper {
public:
  static std::vector<GuitarView::FretLabel>
  getLabelsForChord(const juce::String &chordName, const juce::Colour &colour,
                    int maxFret = 22);

  static std::vector<GuitarView::FretLabel>
  getLabelsForScale(int rootNote, ScaleDatabase::ScaleType scaleType,
                    const juce::Colour &colour, int maxFret = 22);

  static int getNoteAt(int string, int fret);
  static const char *getNoteName(int noteIndex);

private:
  // Standard tuning: open string semitones indices (1=high E, 6=low E)
  static constexpr int openStringNotes[6] = {4, 11, 7, 2, 9, 4};

  static int parseRootNote(const juce::String &root);
  static std::vector<int> getIntervalsForQuality(const juce::String &quality);
  static int getNoteIndex(const juce::String &noteName);
  static std::vector<std::vector<int>> getEveryNoteAppearance(int noteIndex);
};
