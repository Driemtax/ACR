#pragma once

#include <juce_core/juce_core.h>
#include <vector>

class ScaleDatabase {
public:
  enum class ScaleType {
    MinorPantatonic,
    MajorPantatonic,
    // For later
    // Major,
    // NaturalMinor,
    // HarmonicMinor,
    // Dorian,
    // Mixolydian,
  };

  struct ScaleInfo {
    ScaleType type;
    juce::String displayName;
    std::vector<int> intervals; // semitones in respect to root
  };

  static const std::vector<ScaleInfo> &getAllScales();
  static std::vector<int> getScaleNotes(int rootNote, ScaleType type);
  static const std::vector<int> &getIntervals(ScaleType type);
};
