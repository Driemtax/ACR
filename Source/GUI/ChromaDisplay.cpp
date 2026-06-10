#include "ChromaDisplay.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <algorithm>
#include <vector>

//==============================================================================
class FullChromaImageComponent : public juce::Component, public juce::Timer {
public:
  FullChromaImageComponent(juce::Image img,
                           const std::vector<Classificator::ChordSegment> &segs,
                           double sampleRate, int hopSize, int ppf,
                           AudioEngine &engine)
      : image(img), segments(segs), sampleRate(sampleRate), hopSize(hopSize),
        pixelsPerFrame(ppf), audioEngine(engine) {
    setSize(image.getWidth() + leftMargin,
            image.getHeight() + bottomMargin + topMargin);

    // This allows us to receive keyboard events
    setWantsKeyboardFocus(true);

    addAndMakeVisible(playButton);
    playButton.setBounds(5, 5, 30, 20);
    updateButtonState();

    playButton.onClick = [this] {
      if (audioEngine.getState() == AudioEngine::TransportState::Playing) {
        audioEngine.stop();
      } else {
        audioEngine.startPlayback();
      }
      updateButtonState();
    };

    startTimerHz(30);
  }

  ~FullChromaImageComponent() override { stopTimer(); }

  void timerCallback() override {
    updateButtonState();

    auto state = audioEngine.getState();
    if (state == AudioEngine::TransportState::Playing ||
        state == AudioEngine::TransportState::Paused) {
      repaint();
    } else if (state == AudioEngine::TransportState::Stopped) {
      repaint();
    }
  }

  void updateButtonState() {
    if (audioEngine.getState() == AudioEngine::TransportState::Playing) {
      playButton.setButtonText("Stop");
      playButton.setColour(juce::TextButton::buttonColourId,
                           juce::Colours::red);
    } else {
      playButton.setButtonText("Play");
      playButton.setColour(juce::TextButton::buttonColourId,
                           juce::Colours::darkgreen);
    }
  }

  /**
   * @brief Handles key press events for the component.
   *
   * Intercepts keyboard events and toggles the audio playback state when the
   * spacebar is pressed.
   *
   * @param key The key press event containing information about the key
   * pressed.
   * @return true if the key event was consumed (spacebar), false otherwise.
   */
  bool keyPressed(const juce::KeyPress &key) override {
    if (key.isKeyCode(juce::KeyPress::spaceKey)) {
      auto currentState = audioEngine.getState();

      if (currentState == AudioEngine::TransportState::Playing) {
        audioEngine.pausePlayback();
      } else if (currentState == AudioEngine::TransportState::Stopped ||
                 currentState == AudioEngine::TransportState::Paused) {
        audioEngine.startPlayback();
      }

      updateButtonState();
      repaint();

      return true; // consumed key event
    }

    return false; // ignore all other keys
  }

  void paint(juce::Graphics &g) override {
    g.fillAll(juce::Colours::black);
    g.drawImageAt(image, leftMargin, topMargin);

    // --- LABELS ---

    // 1. y-Axis: noteNames
    const char *noteNames[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                 "F#", "G",  "G#", "A",  "A#", "B"};

    g.setColour(juce::Colours::white);
    g.setFont(20.0f);

    float binHeight = static_cast<float>(image.getHeight() / 12.0f);

    for (int i = 0; i < 12; ++i) {
      float yPos = static_cast<float>(image.getHeight() -
                                      ((i + 1) * binHeight) + topMargin);

      g.drawText(noteNames[i], 10, static_cast<int>(yPos), leftMargin - 8,
                 static_cast<int>(binHeight), juce::Justification::centredLeft,
                 false);

      g.setColour(juce::Colours::white.withAlpha(0.8f));
      g.drawLine(static_cast<float>(leftMargin), yPos,
                 static_cast<float>(getWidth()), yPos, 1.0f);
      g.setColour(juce::Colours::white);
    }

    // 2. x-axis: frames
    float totalFrames = static_cast<float>(image.getWidth()) /
                        static_cast<float>(pixelsPerFrame);

    float framesPerSecond =
        static_cast<float>(sampleRate) / static_cast<float>(hopSize);
    int totalSeconds = static_cast<int>(totalFrames / framesPerSecond);

    g.setFont(14.0f);

    for (int s = 0; s < totalSeconds; s++) {
      float frameIndex = s * framesPerSecond;
      float xPos = (frameIndex * pixelsPerFrame) + leftMargin;
      float yBottom = static_cast<float>(image.getHeight() + topMargin);

      g.drawLine(xPos, yBottom, xPos, yBottom + 5, 2.0f);

      juce::String timeString = juce::String(s);
      g.drawText(timeString, (int)xPos + 5, (int)yBottom + 5, 50, 20,
                 juce::Justification::topLeft, false);
    }

    // 3. Chord Labels

    g.setColour(juce::Colours::orange);

    for (const auto &seg : segments) {
      if (seg.chordName.isEmpty() || seg.chordName == "")
        continue;

      float xStart =
          static_cast<float>(leftMargin + (seg.startFrame * pixelsPerFrame));
      float width =
          static_cast<float>((seg.endFrame - seg.startFrame) * pixelsPerFrame);

      g.drawText(seg.chordName, static_cast<int>(xStart + 5), 0,
                 static_cast<int>(width), topMargin,
                 juce::Justification::centredLeft, true);

      g.setColour(juce::Colours::white.withAlpha(0.8f));
      g.drawLine(xStart, 0, xStart, static_cast<float>(getHeight()), 1.0f);
      g.setColour(juce::Colours::orange);
    }

    // 4. Playhead
    auto currentState = audioEngine.getState();
    if (currentState == AudioEngine::TransportState::Playing ||
        currentState == AudioEngine::TransportState::Paused) {

      double currentPos = audioEngine.getTransportSource().getCurrentPosition();
      double totalLen = audioEngine.getTransportSource().getLengthInSeconds();

      if (currentPos > 0.0 && totalLen > 0.0) {
        // time (percentage) * width of image + margin
        float playheadX =
            static_cast<float>((currentPos / totalLen) * image.getWidth()) +
            leftMargin;

        g.setColour(juce::Colours::white);
        g.drawLine(playheadX, 0.0f, playheadX, static_cast<float>(getHeight()),
                   2.0f);
      }
    }
  }

private:
  AudioEngine &audioEngine;
  juce::TextButton playButton{"Play"};

  juce::Image image;
  const std::vector<Classificator::ChordSegment> &segments;
  int leftMargin = 40;
  int bottomMargin = 30;
  int topMargin = 30;

  int pixelsPerFrame;

  double sampleRate;
  int hopSize;
};

class ChromaWindow : public juce::DocumentWindow {
public:
  ChromaWindow(const juce::String &name, juce::Image img,
               std::vector<Classificator::ChordSegment> &segments,
               int screenHeight, double sampleRate, int hopSize, int ppf,
               AudioEngine &engine)
      : juce::DocumentWindow(name, juce::Colours::darkgrey,
                             DocumentWindow::allButtons) {
    auto *content = new FullChromaImageComponent(img, segments, sampleRate,
                                                 hopSize, ppf, engine);
    viewport.setViewedComponent(content, true);
    setContentOwned(&viewport, false);

    int w = std::min(1200, content->getWidth() + 20);
    int h = std::min(screenHeight - 100, content->getHeight() + 40);

    setResizable(true, true);
    centreWithSize(w, h);
    setVisible(true);
    content->grabKeyboardFocus();
  }

  void closeButtonPressed() override { delete this; }

private:
  juce::Viewport viewport;
};

// =============================================================================

ChromaDisplay::ChromaDisplay(AudioEngine &engine) : audioEngine(engine) {
  setOpaque(true);
  setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void ChromaDisplay::setChromaData(
    const juce::AudioBuffer<float> &chroma,
    const std::vector<Classificator::ChordSegment> &segments,
    double sampleRateInput, int hopSizeInput) {
  if (chroma.hasBeenCleared())
    return;

  currentSegments = segments;
  this->sampleRate = sampleRateInput;
  this->hopSize = hopSizeInput;

  int numFrames = chroma.getNumChannels();
  int numBins = chroma.getNumSamples();

  if (numFrames == 0 || numBins == 0)
    return;

  float framesPerSecond =
      static_cast<float>(this->sampleRate) / static_cast<float>(this->hopSize);
  float targetPixelsPerSecond = 86.0f;
  pixelsPerFrame =
      std::max(1, static_cast<int>(targetPixelsPerSecond / framesPerSecond));
  int imageWidth = numFrames * pixelsPerFrame;

  // Every bin is 66 pixel high, e.g. 12*66 = 792 pixel total
  int pixelsPerBin = 66;
  int imageHeight = numBins * pixelsPerBin;

  chromaImage = juce::Image(juce::Image::RGB, imageWidth, imageHeight, true);

  // draw pixel by pixel
  for (int frame = 0; frame < numFrames; ++frame) {
    for (int bin = 0; bin < numBins; ++bin) {
      float val = chroma.getSample(frame, bin);

      // clip value between 0.0 and 1.0
      val = juce::jlimit(0.0f, 1.0f, val);

      // Interpolation: Black (0) -> Red (0.33) -> Yellow (0.66) -> White (1)
      juce::Colour pColour;
      if (val < 0.33f) {
        pColour = juce::Colours::black.interpolatedWith(juce::Colours::red,
                                                        val * 3.0f);
      } else if (val < 0.66f) {
        pColour = juce::Colours::red.interpolatedWith(juce::Colours::yellow,
                                                      (val - 0.33f) * 3.0f);
      } else {
        pColour = juce::Colours::yellow.interpolatedWith(juce::Colours::white,
                                                         (val - 0.66f) * 3.0f);
      }

      // draw whole bin at once
      for (int py = 0; py < pixelsPerBin; ++py) {
        // invert y-axis, so that low sounds (Bin 0) are at the bottom
        int yPos = imageHeight - 1 - (bin * pixelsPerBin + py);

        // draw every frame pixelsPerFrame times
        for (int px = 0; px < pixelsPerFrame; px++) {
          chromaImage.setPixelAt(frame * pixelsPerFrame + px, yPos, pColour);
        }
      }
    }
  }

  repaint();
}

void ChromaDisplay::mouseDown([[maybe_unused]] const juce::MouseEvent &e) {
  if (chromaImage.isValid()) {
    int screenHeight = getParentMonitorArea().getHeight();
    new ChromaWindow("Chromagram", chromaImage, currentSegments, screenHeight,
                     sampleRate, hopSize, pixelsPerFrame, audioEngine);
  }
}

void ChromaDisplay::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::black);

  if (chromaImage.isValid()) {
    g.setImageResamplingQuality(
        juce::Graphics::ResamplingQuality::lowResamplingQuality);
    g.drawImage(chromaImage,
                juce::Rectangle<float>(0, 0, (float)chromaImage.getWidth(),
                                       (float)chromaImage.getHeight()),
                juce::RectanglePlacement::stretchToFit, false);

    // --- LABELS ---

    // 1. y-Axis: Note names
    const char *noteNames[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                 "F#", "G",  "G#", "A",  "Bb", "B"};

    g.setColour(juce::Colours::white);
    g.setFont(14.0f);

    float binHeight = (float)getHeight() / 12.0f;

    for (int i = 0; i < 12; ++i) {
      float yPos =
          (float)getHeight() -
          ((i + 1) * binHeight); // top edge of current bin (inverted y-axis)
      g.drawText(noteNames[i], 5, (int)yPos, 30, (int)binHeight,
                 juce::Justification::centredLeft, false);

      // optional
      // g.setColour(juce::Colours::white.withAlpha(0.2f));
      // g.drawLine(0, yPos, (float)getWidth(), yPos, 1.0f);
      // g.setColour(juce::Colours::white);
    }

    // 2. x-axis: frame count
    int totalFrames = chromaImage.getWidth() / pixelsPerFrame;
    int frameStep = 100;

    for (int f = 0; f < totalFrames; f += frameStep) {
      float xPos = (static_cast<float>(f) / totalFrames) * getWidth();

      g.drawLine(xPos, static_cast<float>(getHeight() - 5), xPos,
                 static_cast<float>(getHeight()), 2.0f);

      juce::String frameText = juce::String(f);
      g.drawText(frameText, static_cast<int>(xPos + 2), getHeight() - 20, 50,
                 20, juce::Justification::bottomLeft, false);
    }
  } else {
    g.setColour(juce::Colours::grey);
    g.drawFittedText("No Chromagram Data", getLocalBounds(),
                     juce::Justification::centred, 1);
  }
}
