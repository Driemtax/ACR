#include "ChromaDisplay.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"

//==============================================================================
class FullChromaImageComponent : public juce::Component {
    public:
    FullChromaImageComponent(juce::Image img) : image(img) {
        setSize(image.getWidth() + leftMargin, image.getHeight() + bottomMargin);
    }

    void paint(juce::Graphics &g) override {
        g.fillAll(juce::Colours::black);
        g.drawImageAt(image, leftMargin, 0);

        // --- LABELS ---

        // 1. y-Axis: noteNames
        const char* noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

        g.setColour(juce::Colours::white);
        g.setFont(20.0f);

        float binHeight = (float)image.getHeight() / 12.0f;

        for (int i = 0; i < 12; ++i) {
                    float yPos = (float)image.getHeight() - ((i + 1) * binHeight);

                    g.drawText(noteNames[i], 10, (int)yPos, leftMargin - 8, (int)binHeight, juce::Justification::centredLeft, false);

                    g.setColour(juce::Colours::white.withAlpha(0.2f));
                    g.drawLine(leftMargin, yPos, (float)getWidth(), yPos, 1.0f);
                    g.setColour(juce::Colours::white);
                }

                // 2. x-axis: frames
                int totalFrames = image.getWidth(); // 1 Pixel = 1 Frame
                int frameStep = 100;

                g.setFont(14.0f);

                for (int f = 0; f < totalFrames; f += frameStep) {
                    float xPos = (float)f + leftMargin;
                    float yBottom = (float)image.getHeight();

                    g.drawLine(xPos, yBottom, xPos, yBottom + 5, 2.0f);
                    g.drawText(juce::String(f), (int)xPos + 5, (int)yBottom + 5, 50, 20, juce::Justification::topLeft, false);
                }
    }
    private:
    juce::Image image;
    int leftMargin = 40;
    int bottomMargin = 30;
};

class ChromaWindow : public juce::DocumentWindow {
    public:
    ChromaWindow(const juce::String &name, juce::Image img, int screenHeight)
        : juce::DocumentWindow(name, juce::Colours::darkgrey, DocumentWindow::allButtons) {
            auto *content = new FullChromaImageComponent(img);
            viewport.setViewedComponent(content, true);
            setContentOwned(&viewport, false);

            int w = std::min(1200, content->getWidth() + 20);
            int h = std::min(screenHeight - 100, content->getHeight() + 40);

            setResizable(true, true);
            centreWithSize(w, h);
            setVisible(true);
        }

        void closeButtonPressed() override { delete this; }
    private:
    juce::Viewport viewport;
};

// =============================================================================

ChromaDisplay::ChromaDisplay() {
    setOpaque(true);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void ChromaDisplay::setChromaData(const juce::AudioBuffer<float> &chroma) {
    if (chroma.hasBeenCleared()) return;

    int numFrames = chroma.getNumChannels();
    int numBins = chroma.getNumSamples();

    if (numFrames == 0 || numBins == 0) return;

    // Every bin is 66 pixel high, e.g. 12*66 = 792 pixel total
    int pixelsPerBin = 66;
    int imageHeight = numBins * pixelsPerBin;

    chromaImage = juce::Image(juce::Image::RGB, numFrames, imageHeight, true);

    // draw pixel by pixel
    for (int frame = 0; frame < numFrames; ++frame) {
        for (int bin = 0; bin < numBins; ++bin) {
           float val = chroma.getSample(frame, bin);

           // clip value between 0.0 and 1.0
           val = juce::jlimit(0.0f, 1.0f, val);

           // Interpolation: Black (0) -> Red (0.33) -> Yellow (0.66) -> White (1)
           juce::Colour pColour;
           if (val < 0.33f) {
               pColour = juce::Colours::black.interpolatedWith(juce::Colours::red, val * 3.0f);
           } else if (val < 0.66f) {
               pColour = juce::Colours::red.interpolatedWith(juce::Colours::yellow, (val - 0.33f) * 3.0f);
           } else {
               pColour = juce::Colours::yellow.interpolatedWith(juce::Colours::white, (val - 0.66f) * 3.0f);
           }

           // draw whole bin at once
           for (int py = 0; py < pixelsPerBin; ++py) {
               // invert y-axis, so that low sounds (Bin 0) are at the bottom
               int yPos = imageHeight - 1 - (bin * pixelsPerBin + py);
               chromaImage.setPixelAt(frame, yPos, pColour);
           }
        }
    }

    repaint();
}

void ChromaDisplay::mouseDown(const juce::MouseEvent &e) {
    if (chromaImage.isValid()) {
        int screenHeight = getParentMonitorArea().getHeight();
        new ChromaWindow("Chromagram", chromaImage, screenHeight);
    }
}

void ChromaDisplay::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::black);

    if (chromaImage.isValid()) {
        g.setImageResamplingQuality(
            juce::Graphics::ResamplingQuality::lowResamplingQuality);
        g.drawImage(chromaImage,
            juce::Rectangle<float>(0,0, (float)chromaImage.getWidth(), (float)chromaImage.getHeight()),
            juce::RectanglePlacement::stretchToFit, false);

        // --- LABELS ---

        // 1. y-Axis: Note names
        const char* noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "Bb", "B"};

        g.setColour(juce::Colours::white);
        g.setFont(14.0f);

        float binHeight = (float)getHeight() / 12.0f;

        for (int i = 0; i < 12; ++i) {
            float yPos = (float)getHeight() - ((i + 1) * binHeight); // top edge of current bin (inverted y-axis)
            g.drawText(noteNames[i], 5, (int)yPos, 30, (int) binHeight, juce::Justification::centredLeft, false);

            // optional
            //g.setColour(juce::Colours::white.withAlpha(0.2f));
            //g.drawLine(0, yPos, (float)getWidth(), yPos, 1.0f);
            //g.setColour(juce::Colours::white);
        }

        // 2. x-axis: frame count
        int totalFrames = chromaImage.getWidth();
        int frameStep = 100;

        for (int f = 0; f < totalFrames; f+= frameStep) {
            float xPos = ((float)f / totalFrames) * getWidth();

            g.drawLine(xPos, (float)getHeight() - 5, xPos, (float)getHeight(), 2.0f);

            juce::String frameText = juce::String(f);
            g.drawText(frameText, (int)xPos + 2, getHeight() - 20, 50, 20, juce::Justification::bottomLeft, false);

        }
    } else {
        g.setColour(juce::Colours::grey);
        g.drawFittedText("No Chromagram Data", getLocalBounds(), juce::Justification::centred, 1);
    }
}
