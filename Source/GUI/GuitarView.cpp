#include "GuitarView.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <cmath>
#include <vector>

GuitarView::GuitarView(AudioEngine &engine) : audioEngine(engine) {
  setLabels({
      {1, 0, "E", juce::Colours::steelblue},
      {2, 0, "A", juce::Colours::steelblue},
      {3, 0, "D", juce::Colours::steelblue},
      {4, 0, "G", juce::Colours::steelblue},
      {5, 0, "B", juce::Colours::steelblue},
      {6, 0, "E", juce::Colours::steelblue},
  });
}

void GuitarView::setLabels(const std::vector<FretLabel> &labels) {
  activeLabels = labels;
  repaint();
}

void GuitarView::clearLabels() {
  activeLabels.clear();
  repaint();
}

// ==================================================================================================
// Coordinate helpers
// ==================================================================================================

juce::Rectangle<float> GuitarView::getFretboardBounds() const {
  // Margins: left for open-string labels, top for fret numbers
  const float left = 45.0f;
  const float right = 15.0f;
  const float top = 28.0f;
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

  // String 1 (low E) at top, string 6 (high E) at bottom.
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
  drawLabels(g);
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

  // Thickness per string (low E -> high E)
  const float gauges[] = {3.0f, 2.5f, 2.0f, 1.5f, 1.2f, 1.0f};

  for (int s = 1; s <= numStrings; s++) {
    float y = getStringY(s);
    float thickness = gauges[s - 1];

    // wound bronze (strings 1-3) vs plain steel (strings 4-6)
    g.setColour(s <= 3 ? juce::Colour(0xFFB8860B) : juce::Colour(0xFFD4D4D4));
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

  g.setColour(juce::Colours::grey);
  g.setFont(11.0f);

  for (int fret = 1; fret <= numFrets; fret++) {
    float cx = (getFretX(fret - 1) + getFretX(fret)) / 2.0f;

    g.drawText(juce::String(fret), static_cast<int>(cx - 10),
               static_cast<int>(fb.getBottom() + 3), 20, 18,
               juce::Justification::centredTop);
  }
}

void GuitarView::drawLabels(juce::Graphics &g) {
  for (const auto &label : activeLabels) {
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
    float labelW = juce::jlimit(18.0f, 28.0f, fretWidth * 0.7f);
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
