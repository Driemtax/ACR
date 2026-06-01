#include "SpectogramDisplay.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <algorithm>

// =====================================================================================
// Auxilary class to spawnm a new window of the needed size to display the
// spectogram
class FullSizeImageComponent : public juce::Component {
public:
  FullSizeImageComponent(juce::Image img) : image(img) {
    setSize(image.getWidth(), image.getHeight());
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colours::black);
    g.drawImageAt(image, 0, 0);
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
void SpectogramDisplay::setSpectogramData(
    const juce::AudioBuffer<float> &data) {
  if (data.hasBeenCleared())
    return;

  int numFrames = (int)data.getNumChannels();
  int numBins = (int)data.getNumSamples();

  // dynamic scaling of width: at least 800 pixel
  int pixelsPerFrame = std::max(1, 800 / std::max(1, numFrames));
  int imageWidth = numFrames * pixelsPerFrame;

  // create an image of size (imageWidth * numBins)
  spectogramImage = juce::Image(juce::Image::RGB, imageWidth, numBins, true);

  float minDb = 1000.0f;
  float maxDb = -1000.0f;

  for (int f = 0; f < numFrames; f++) {
    const float *frame = data.getReadPointer(f);
    for (int bin = 0; bin < numBins; bin++) {
      float val = frame[bin];
      if (val < minDb)
        minDb = val;
      if (val > maxDb)
        maxDb = val;
    }
  }

  if (maxDb - minDb < 0.1f)
    maxDb = minDb + 1.0f;
  const float dbRange = maxDb - minDb;

  // draw image pixel by pixel
  for (int x = 0; x < numFrames; x++) {
    const auto &frame = data.getReadPointer(x);
    for (int y = 0; y < numBins; y++) {
      float dbValue = frame[y];
      float normalizedValue = (dbValue - minDb) / dbRange;

      // This is to dampen the noise. Everything near to 1 stays there
      float contrastValue = std::pow(normalizedValue, 3.0f);

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
  g.fillAll(juce::Colours::black);

  if (spectogramImage.isValid()) {
    g.setImageResamplingQuality(
        juce::Graphics::ResamplingQuality::lowResamplingQuality);
    g.drawImage(spectogramImage,
                juce::Rectangle<float>(0, 0, (float)spectogramImage.getWidth(),
                                       (float)spectogramImage.getHeight()),
                juce::RectanglePlacement::stretchToFit, false);
  } else {
    g.setColour(juce::Colours::grey);
    g.drawFittedText("No Spectogram Data", getLocalBounds(),
                     juce::Justification::centred, 1);
  }
}
