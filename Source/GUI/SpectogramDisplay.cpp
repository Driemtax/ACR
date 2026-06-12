#include "SpectogramDisplay.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_graphics/juce_graphics.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ostream>

static void drawLabelsYAxis(juce::Graphics &g, const juce::Image &img,
                            int width, int height,
                            double sampleRate = 44100.0) {
  g.fillAll(juce::Colours::black);

  if (!img.isValid()) {
    g.setColour(juce::Colours::grey);
    g.drawFittedText("No Spectogram Data", 0, 0, width, height,
                     juce::Justification::centred, 1);
    return;
  }

  g.setImageResamplingQuality(
      juce::Graphics::ResamplingQuality::lowResamplingQuality);

  const int yAxisWidth = 50;
  juce::Rectangle<int> imageBounds(yAxisWidth, 0, width - yAxisWidth, height);

  g.drawImage(img, imageBounds.toFloat(),
              juce::RectanglePlacement::stretchToFit, false);

  int numTicks = 40;
  double nyquistFreq = sampleRate / 2.0;
  double minFreq = 50.0;

  g.setFont(10.0f);
  int lastTextYPos = imageBounds.getBottom() + 50;

  for (int i = 0; i <= numTicks; ++i) {
    double freq =
        minFreq * std::pow(nyquistFreq / minFreq, (double)i / numTicks);
    float fraction = (float)(freq / nyquistFreq);
    int yPos =
        imageBounds.getBottom() - (int)(fraction * imageBounds.getHeight());

    juce::String freqStr = (freq >= 1000.0f)
                               ? juce::String(freq / 1000.0f, 1) + " k"
                               : juce::String((int)freq);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawLine(yAxisWidth - 5, yPos, yAxisWidth, yPos);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawLine(yAxisWidth, yPos, width, yPos);

    // collision detection
    if (lastTextYPos - yPos >= 14) {
      g.setColour(juce::Colours::white);
      g.drawText(freqStr, 0, yPos - 6, yAxisWidth - 8, 12,
                 juce::Justification::centredRight);
      lastTextYPos = yPos;
    }
  }
}

// =====================================================================================
// Auxilary class to spawnm a new window of the needed size to display the
// spectogram
class FullSizeImageComponent : public juce::Component {
public:
  FullSizeImageComponent(juce::Image img) : image(img) {
    setSize(image.getWidth(), image.getHeight());
  }

  void paint(juce::Graphics &g) override {
    drawLabelsYAxis(g, image, getWidth(), getHeight());
  }

private:
  juce::Image image;
};

class SpectogramWindow : public juce::DocumentWindow {
public:
  SpectogramWindow(const juce::String &name, juce::Image img, int screenHeight)
      : juce::DocumentWindow(name, juce::Colours::darkgrey,
                             DocumentWindow::allButtons) {
    auto *content = new FullSizeImageComponent(img);
    viewport.setViewedComponent(content, true);
    setContentOwned(&viewport, false);

    int w = std::min(1000, img.getWidth() + 20);
    int h = std::min(screenHeight - 100, img.getHeight() + 40);

    setResizable(true, true);
    centreWithSize(w, h);
    setVisible(true);
  }

  void closeButtonPressed() override { delete this; }

private:
  juce::Viewport viewport;
};

// =============================================================================

SpectogramDisplay::SpectogramDisplay() {
  setOpaque(true);
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

// This function will be called from the thread analysing the audio, when the
// Analysis has finished.
void SpectogramDisplay::setSpectogramData(const juce::AudioBuffer<float> &data,
                                          double sampleRate, int hopSize) {
  if (data.hasBeenCleared())
    return;

  int numFrames = static_cast<int>(data.getNumChannels());
  int numBins = static_cast<int>(data.getNumSamples());

  if (numFrames == 0 || numBins == 0) {
    return;
  }

  this->sampleRate = sampleRate;
  this->hopSize = hopSize;

  float framesPerSecond =
      static_cast<float>(sampleRate) / static_cast<float>(hopSize);
  float targetFPS = 43.0f;

  int pixelsPerFrame =
      std::max(1, static_cast<int>(targetFPS / framesPerSecond));
  int imageWidth = numFrames * pixelsPerFrame;

  // create an image of size (imageWidth * numBins)
  spectogramImage = juce::Image(juce::Image::RGB, imageWidth, numBins, true);

  float maxDb = -1000.0f;
  int maxDbFrame = -1;
  int maxDbBin = -1;
  for (int f = 1; f < numFrames - 1; f++) {
    const float *frame = data.getReadPointer(f);
    for (int bin = 0; bin < numBins; bin++) {
      float val = frame[bin];
      if (val > maxDb) {
        maxDb = val;
        maxDbFrame = f;
        maxDbBin = bin;
      }
    }
  }

  // set dynamic range
  const float dynamicRange = 90.0f;
  float minDb = maxDb - dynamicRange;

  // draw image pixel by pixel
  for (int x = 0; x < numFrames; x++) {
    const auto &frame = data.getReadPointer(x);
    for (int y = 0; y < numBins; y++) {
      float val = frame[y];

      // clipping prevents negative values after normalizing.
      if (val < minDb)
        val = minDb;

      float normalizedValue = (val - minDb) / dynamicRange;

      // This is to dampen the noise. Everything near to 1 stays there
      float contrastValue = std::pow(normalizedValue, 2.0f);

      juce::Colour pixelColour =
          juce::Colours::black
              .interpolatedWith(juce::Colours::cyan,
                                std::min(1.0f, contrastValue * 2.0f))
              .interpolatedWith(juce::Colours::white,
                                std::max(0.0f, contrastValue * 2.0f - 1.0f));

      // draw this pixelsPerFrame times
      for (int px = 0; px < pixelsPerFrame; px++) {
        // the axis in graphics and audio spectogram are mirrored on the y-axis,
        // we need to mirror it back.
        spectogramImage.setPixelAt(x * pixelsPerFrame + px, numBins - 1 - y,
                                   pixelColour);
      }
    }
  }

  repaint();
}

void SpectogramDisplay::mouseDown([[maybe_unused]] const juce::MouseEvent &e) {
  if (spectogramImage.isValid()) {
    int screenHeight = getParentMonitorArea().getHeight();
    new SpectogramWindow("High-Res Spectogram", spectogramImage, screenHeight);
  }
}

void SpectogramDisplay::paint(juce::Graphics &g) {
  drawLabelsYAxis(g, spectogramImage, getWidth(), getHeight());
}
