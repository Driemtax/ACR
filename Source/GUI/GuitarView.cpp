#include "GuitarView.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <cmath>
#include <map>
#include <set>
#include <utility>
#include <vector>

GuitarView::GuitarView(AudioEngine &engine) : audioEngine(engine) {
  setCurrentLabels({
      {1, 0, "E", juce::Colours::steelblue},
      {2, 0, "B", juce::Colours::steelblue},
      {3, 0, "G", juce::Colours::steelblue},
      {4, 0, "D", juce::Colours::steelblue},
      {5, 0, "A", juce::Colours::steelblue},
      {6, 0, "E", juce::Colours::steelblue},
  });
}

void GuitarView::setCurrentLabels(const std::vector<FretLabel> &labels) {
  currentLabels = labels;
  repaint();
}

void GuitarView::setNextLabels(const std::vector<FretLabel> &labels) {
  nextLabels = labels;
  repaint();
}

void GuitarView::clearLabels() {
  currentLabels.clear();
  nextLabels.clear();
  repaint();
}

// ==================================================================================================
// Coordinate helpers
// ==================================================================================================

juce::Rectangle<float> GuitarView::getFretboardBounds() const {
  // Margins: left for open-string labels, top for fret numbers
  const float left = 45.0f;
  const float right = 15.0f;
  const float top = 38.0f;
  const float bottom = 20.0f;

  return {left, top, getWidth() - left - right, getHeight() - top - bottom};
}

float GuitarView::getFretX(int fret) const {
  auto fb = getFretboardBounds();

  if (fret <= 0)
    return fb.getX();

  // 12-TET formula: fet n sits at L * (1 - 2^(-n/12)) from the nut where L is
  // the scale size. Normalize so that fret `numFrets` maps to the right edge
  float maxPos = 1.0f - std::powf(2.0f, -static_cast<float>(numFrets) / 12.0f);

  float fretPos = 1.0f - std::powf(2.0f, -static_cast<float>(fret) / 12.0f);

  return fb.getX() + fb.getWidth() * (fretPos / maxPos);
}

float GuitarView::getStringY(int string) const {
  auto fb = getFretboardBounds();

  // String 1 (high E) at top, string 6 (low E) at bottom.
  // Add inner padding so strings don't sit in the very edge of the wood.
  float padding = fb.getHeight() * 0.08f;
  float usable = fb.getHeight() - 2.0f * padding;
  float spacing = usable / static_cast<float>(numStrings - 1);
  return fb.getY() + padding + (string - 1) * spacing;
}

// =================================================================================================
// Paint
// =================================================================================================

void GuitarView::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(0xFF1A1A2E)); // dark background

  drawFretboard(g);
  drawInlays(g);
  drawFrets(g);
  drawNut(g);
  drawStrings(g);
  drawFretNumbers(g);
  drawAllLabels(g);
}

void GuitarView::resized() {}

// =================================================================================================
// Drawing Layers
// =================================================================================================
void GuitarView::drawFretboard(juce::Graphics &g) {
  auto fb = getFretboardBounds();

  // wood body
  g.setColour(juce::Colour(0xFF3D2B1F));
  g.fillRoundedRectangle(fb, 4.0f);

  // Subtle border
  g.setColour(juce::Colour(0xFF2A1F14));
  g.drawRoundedRectangle(fb, 4.0f, 1.5f);
}

void GuitarView::drawNut(juce::Graphics &g) {
  auto fb = getFretboardBounds();
  float nutX = fb.getX();

  // Bone-coloured nut
  g.setColour(juce::Colour(0xFF0E8D0));
  g.fillRect(nutX - 1.0f, fb.getY(), 4.0f, fb.getHeight());
}

void GuitarView::drawFrets(juce::Graphics &g) {
  auto fb = getFretboardBounds();

  for (int fret = 1; fret <= numFrets; fret++) {
    float x = getFretX(fret);

    // Silver fret wire
    g.setColour(juce::Colour(0xFFC0C0C0));
    g.drawLine(x, fb.getY(), x, fb.getBottom(), 2.0f);
  }
}

void GuitarView::drawStrings(juce::Graphics &g) {
  auto fb = getFretboardBounds();

  // Thickness per string (high E -> low E)
  const float gauges[] = {1.0f, 1.2f, 1.5f, 2.0f, 2.5f, 3.0f};

  for (int s = 1; s <= numStrings; s++) {
    float y = getStringY(s);
    float thickness = gauges[s - 1];

    // plain steel (strings 1-3) vs wound bronze (strings 4-6)
    g.setColour(s <= 3 ? juce::Colour(0xFFD4D4D4) : juce::Colour(0xFFB8860B));
    g.drawLine(fb.getX(), y, fb.getRight(), y, thickness);
  }
}

void GuitarView::drawInlays(juce::Graphics &g) {
  g.setColour(juce::Colour(0xFFF5F5F5).withAlpha(0.25f));

  float dotRadius = 5.0f;

  // Single dots at frets 3,5,7,9,15,17,19,21
  // centred between strings 3 and 4
  const int singleDotFrets[] = {3, 5, 7, 9, 15, 17, 19, 21};
  float centerY = (getStringY(3) + getStringY(4)) / 2.0f;

  for (int fret : singleDotFrets) {
    float cx = (getFretX(fret - 1) + getFretX(fret)) / 2.0f;
    g.fillEllipse(cx - dotRadius, centerY - dotRadius, dotRadius * 2.0f,
                  dotRadius * 2.0f);
  }

  // Double dots at fret 12 - on string 2 and string 5
  {
    float cx = (getFretX(11) + getFretX(12)) / 2.0f;
    float y1 = getStringY(2);
    float y2 = getStringY(5);

    g.fillEllipse(cx - dotRadius, y1 - dotRadius, dotRadius * 2.0f,
                  dotRadius * 2.0f);
    g.fillEllipse(cx - dotRadius, y2 - dotRadius, dotRadius * 2.0f,
                  dotRadius * 2.0f);
  }
}

void GuitarView::drawFretNumbers(juce::Graphics &g) {
  auto fb = getFretboardBounds();

  g.setColour(juce::Colours::lightgrey);
  g.setFont(22.0f);

  for (int fret = 1; fret <= numFrets; fret++) {
    float left = getFretX(fret - 1);
    float right = getFretX(fret);
    float fretWidth = right - left;

    g.drawText(juce::String(fret), static_cast<int>(left),
               static_cast<int>(fb.getY() - 30), static_cast<int>(fretWidth),
               26, juce::Justification::centred);
  }
}

void GuitarView::drawAllLabels(juce::Graphics &g) {
  // Build a lookup of next-label positions for fast overlap detection
  std::map<std::pair<int, int>, const FretLabel *> nextMap;

  for (const auto &label : nextLabels) {
    nextMap[{label.string, label.fret}] = &label;
  }

  // Identify overlapping positions
  std::set<std::pair<int, int>> overlaps;
  for (const auto &cl : currentLabels) {
    if (nextMap.count({cl.string, cl.fret})) {
      overlaps.insert({cl.string, cl.fret});
    }
  }

  // 1. Draw non-overlapping labels
  for (const auto &label : nextLabels) {
    if (overlaps.count({label.string, label.fret}) == 0) {
      drawSingleLabel(g, label);
    }
  }

  // 2. Draw non-overlapping current labels
  for (const auto &label : currentLabels) {
    if (overlaps.count({label.string, label.fret}) == 0) {
      drawSingleLabel(g, label);
    }
  }

  // 3. Draw overlapping labels as split ellipses
  for (const auto &cl : currentLabels) {
    auto it = nextMap.find({cl.string, cl.fret});
    if (it != nextMap.end()) {
      drawSplitLabel(g, cl, *it->second);
    }
  }
}

juce::Rectangle<float>
GuitarView::getLabelBounds(const FretLabel &label) const {
  float y = getStringY(label.string);
  float x;

  if (label.fret == 0) {
    x = getFretX(0) - 18.0f;
  } else {
    x = (getFretX(label.fret - 1) + getFretX(label.fret)) / 2.0f;
  }

  float fretWidth = (label.fret > 0)
                        ? getFretX(label.fret) - getFretX(label.fret - 1)
                        : 30.0f;

  float labelW = juce::jlimit(18.0f, 45.0f, fretWidth * 0.7f);
  float labelH = labelW * 0.75f;

  return {x - labelW / 2.0f, y - labelH / 2.0f, labelW, labelH};
}

void GuitarView::drawSingleLabel(juce::Graphics &g, const FretLabel &label) {
  if (label.string < 1 || label.string > numStrings)
    return;
  if (label.fret < 0 || label.fret > numFrets)
    return;

  auto bounds = getLabelBounds(label);

  // Filled ellipse
  g.setColour(label.colour);
  g.fillEllipse(bounds);

  // Borders
  g.setColour(label.colour.brighter(0.3f));
  g.drawEllipse(bounds, 1.5f);

  // Text
  g.setColour(juce::Colours::white);
  g.setFont(juce::jlimit(9.0f, 12.0f, bounds.getWidth() * 0.5f));
  g.drawText(label.text, bounds.toNearestInt(), juce::Justification::centred,
             false);
}

void GuitarView::drawSplitLabel(juce::Graphics &g, const FretLabel &current,
                                const FretLabel &next) {
  auto bounds = getLabelBounds(current);
  float midX = bounds.getCentreX();

  // Left half: current chord colour
  g.saveState();
  g.reduceClipRegion(static_cast<int>(bounds.getX()),
                     static_cast<int>(bounds.getY()),
                     static_cast<int>(midX - bounds.getX()),
                     static_cast<int>(bounds.getHeight()) + 1);
  g.setColour(current.colour);
  g.fillEllipse(bounds);
  g.restoreState();

  // Right half: next chord colour
  g.saveState();
  g.reduceClipRegion(static_cast<int>(midX), static_cast<int>(bounds.getY()),
                     static_cast<int>(bounds.getRight() - midX) + 1,
                     static_cast<int>(bounds.getHeight() + 1));
  g.setColour(next.colour);
  g.fillEllipse(bounds);
  g.restoreState();

  // Border
  g.setColour(juce::Colours::white.withAlpha(0.7f));
  g.drawEllipse(bounds, 1.5f);

  // Center divider line
  g.setColour(juce::Colours::white.withAlpha(0.4f));
  g.drawLine(midX, bounds.getY() + 2.0f, midX, bounds.getBottom() - 2.0f, 1.0f);

  // Text
  g.setColour(juce::Colours::white);
  g.setFont(juce::jlimit(9.0f, 12.0f, bounds.getWidth() + 0.5f));
  g.drawText(current.text, bounds.toNearestInt(), juce::Justification::centred,
             false);
}

void GuitarView::drawLabels(juce::Graphics &g,
                            const std::vector<FretLabel> &labels) {
  for (const auto &label : labels) {
    if (label.string < 1 || label.string > numStrings)
      continue;

    if (label.fret < 0 || label.fret > numFrets)
      continue;

    float y = getStringY(label.string);
    float x;

    if (label.fret == 0) {
      // Open string: to the left of the nut
      x = getFretX(0) - 18.0f;
    } else {
      // Centred between previous fret and this fret
      x = (getFretX(label.fret - 1) + getFretX(label.fret)) / 2.0f;
    }

    // Scale label to fit the fret width (clamped)
    float fretWidth = (label.fret > 0)
                          ? getFretX(label.fret) - getFretX(label.fret - 1)
                          : 30.0f;
    float labelW = juce::jlimit(18.0f, 45.0f, fretWidth * 0.7f);
    float labelH = labelW * 0.75f;

    auto bounds = juce::Rectangle<float>(x - labelW / 2.0f, y - labelH / 2.0f,
                                         labelW, labelH);

    // Filled ellipse
    g.setColour(label.colour);
    g.fillEllipse(bounds);

    // Border
    g.setColour(label.colour.brighter(0.3f));
    g.drawEllipse(bounds, 1.5f);

    // Text
    g.setColour(juce::Colours::white);
    g.setFont(juce::jlimit(9.0f, 12.0f, labelW * 0.5f));
    g.drawText(label.text, bounds.toNearestInt(), juce::Justification::centred,
               false);
  }
}
