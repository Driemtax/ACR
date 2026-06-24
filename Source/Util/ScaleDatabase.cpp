#include "ScaleDatabase.h"
#include <vector>

const std::vector<ScaleDatabase::ScaleInfo> &ScaleDatabase::getAllScales() {
  static std::vector<ScaleInfo> scales = {
      {ScaleType::MinorPantatonic, "Minor Pentatonic", {0, 3, 5, 7, 10}},
      {ScaleType::MajorPantatonic, "Major Pentatonic", {0, 2, 4, 7, 9}}};

  return scales;
}

const std::vector<int> &ScaleDatabase::getIntervals(ScaleType type) {
  for (const auto &info : getAllScales()) {
    if (info.type == type) {
      return info.intervals;
    }
  }

  // Fallback to empty if not found
  static std::vector<int> empty;
  return empty;
}

std::vector<int> ScaleDatabase::getScaleNotes(int rootNote, ScaleType type) {
  std::vector<int> notes;
  const auto &intervals = getIntervals(type);

  for (int interval : intervals) {
    notes.push_back((rootNote + interval) % 12);
  }

  return notes;
}
