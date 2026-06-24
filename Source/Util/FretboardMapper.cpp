#include "FretboardMapper.h"
#include "ScaleDatabase.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include <algorithm>
#include <regex>
#include <vector>

static const char *noteNames[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                    "F#", "G",  "G#", "A",  "A#", "B"};

const char *FretboardMapper::getNoteName(int noteIndex) {
  return noteNames[((noteIndex % 12) + 12) % 12];
}

int FretboardMapper::getNoteAt(int string, int fret) {
  int stringRoot = openStringNotes[string - 1];
  return (stringRoot + fret) % 12;
}

int FretboardMapper::getNoteIndex(const juce::String &noteName) {
  int index = -1;
  for (int i = 0; i < 12; i++) {
    if (noteName == noteNames[i]) {
      return i;
    }
  }

  return index;
}

int FretboardMapper::parseRootNote(const juce::String &root) {
  return getNoteIndex(root);
}

std::vector<std::vector<int>>
FretboardMapper::getEveryNoteAppearance(int noteIndex) {
  // TODO: put it in some kind of config
  const int maxFrets = 22;

  std::vector<std::vector<int>> result(6);

  for (int i = 0; i < result.size(); i++) {
    int openNote = openStringNotes[i];

    // find the first fret on this string that our note is on.
    int firstFret = (noteIndex - openNote + 12) % 12;

    // add this fret in every octave
    for (int fret = firstFret; fret <= maxFrets; fret += 12) {
      result[i].push_back(fret);
    }
  }
  return result;
}

std::vector<int>
FretboardMapper::getIntervalsForQuality(const juce::String &quality) {
  std::vector<int> intervals;
  // If its a major chord we need to add major third and fifth, otherwise minor
  // third and fifth.
  if (quality == "Maj") {
    intervals = std::vector<int>{0, 4, 7};
  } else {
    intervals = std::vector<int>{0, 3, 7};
  }

  return intervals;
}

/**
 * @brief Generates fretboard labels for a given chord up to a maximum fret.
 *
 * @param chordName The name of the chord to generate labels for.
 * @param maxFret The maximum fret number to include.
 * @return A vector of GuitarView::FretLabel objects representing the notes of
 * the chord.
 */
std::vector<GuitarView::FretLabel>
FretboardMapper::getLabelsForChord(const juce::String &chordName,
                                   const juce::Colour &colour, int maxFret) {
  std::vector<GuitarView::FretLabel> labels;
  std::string stdChordName = chordName.toStdString();

  /// Regex checks the following:
  ///   1. Letter A-G for root name followed by an optional #
  ///   2. Optional space followed by Maj|Min|5
  std::regex pattern("^([A-G]#?)\\s*(Maj|Min|5)$");
  std::smatch match;
  // Checks if the whole string matches the pattern
  bool foundMatch = std::regex_match(stdChordName, match, pattern);

  // If there is no match, meaning no chord, we return an empty label list
  if (!foundMatch) {
    return labels;
  }

  // match[1] is rootName and match[2] is qualityName
  juce::String rootName = match[1].str();
  juce::String qualityName = match[2].str();

  // Ignore powerchords for now
  if (qualityName == "5") {
    qualityName = "Maj";
  }

  int rootNote = parseRootNote(rootName);
  if (rootNote < 0)
    return labels;

  std::vector<int> intervals = getIntervalsForQuality(qualityName);

  // Every Appearance of root
  // Every Appearance of next intervall and so on ..
  for (int i = 0; i < intervals.size(); i++) {
    std::vector<std::vector<int>> noteAppearances =
        getEveryNoteAppearance((rootNote + intervals[i]) % 12);

    for (int s = 0; s < 6; s++) {
      for (int fret : noteAppearances[s]) {
        GuitarView::FretLabel label = {
            s + 1, fret, getNoteName(getNoteAt(s + 1, fret)), colour};
        labels.push_back(label);
      }
    }
  }

  return labels;
}

std::vector<GuitarView::FretLabel>
FretboardMapper::getLabelsForScale(int rootNote,
                                   ScaleDatabase::ScaleType scaleType,
                                   const juce::Colour &colour, int maxFret) {
  std::vector<GuitarView::FretLabel> labels;
  std::vector<int> notes = ScaleDatabase::getScaleNotes(rootNote, scaleType);

  for (int note : notes) {
    std::vector<std::vector<int>> noteAppearances =
        getEveryNoteAppearance(note);
    bool isRoot = (note == rootNote);
    // juce::Colour c = isRoot ? rootColour : colour;

    for (int s = 0; s < 6; s++) {
      for (int fret : noteAppearances[s]) {
        if (fret <= maxFret) {
          GuitarView::FretLabel label = {s + 1, fret, getNoteName(note),
                                         colour};
          labels.push_back(label);
        }
      }
    }
  }

  return labels;
}
