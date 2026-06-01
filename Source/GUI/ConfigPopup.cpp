#include "ConfigPopup.h"
// clang-format off
#include <JuceHeader.h>
// clang-format on
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <functional>

#ifdef always_inline
#undef always_inline
#endif

SettingsComponent::SettingsComponent(AnalyzerConfig &configToEdit,
                                     std::function<void()> onSave)
    : config(configToEdit), onSaveCallback(onSave) {
  addAndMakeVisible(useDeepLearningToggle);
  useDeepLearningToggle.setToggleState(config.useDeepLearning,
                                       juce::dontSendNotification);
  useDeepLearningToggle.onClick = [this] {
    if (useDeepLearningToggle.getToggleState()) {
      // auto-set madmom defaults
      fftOrderSlider.setValue(13);
      hopSizeSlider.setValue(4410);

      fftOrderSlider.setEnabled(false);
      hopSizeSlider.setEnabled(false);
    } else {
      fftOrderSlider.setEnabled(true);
      hopSizeSlider.setEnabled(true);
    }
  };

  // configure sliders for range and default values
  auto setupSlider = [this](juce::Slider &sl, juce::Label &label, double min,
                            double max, double step, double val) {
    addAndMakeVisible(label);
    addAndMakeVisible(sl);
    sl.setRange(min, max, step);
    sl.setValue(val);
  };

  setupSlider(fftOrderSlider, fftOrderLabel, 8, 14, 1, config.fftOrder);
  setupSlider(hopSizeSlider, hopSizeLabel, 128, 8192, 1, config.hopSize);
  setupSlider(medianWindowSlider, medianWindowLabel, 1, 21, 1,
              config.medianWindowSize);
  setupSlider(sSlider, sLabel, 0.1, 1.0, 0.05, config.s);
  setupSlider(thresholdSlider, thresholdLabel, 0.0, 1.0, 0.05,
              config.similarityThreshold);
  setupSlider(chromaResSlider, chromaResLabel, 1, 3, 1, config.chromaRes);

  addAndMakeVisible(medianFilterToggle);
  medianFilterToggle.setToggleState(config.medianFilter,
                                    juce::dontSendNotification);

  addAndMakeVisible(saveButton);
  saveButton.onClick = [this] {
    config.useDeepLearning = useDeepLearningToggle.getToggleState();
    config.fftOrder = static_cast<int>(fftOrderSlider.getValue());
    config.fftSize = 1 << config.fftOrder;
    config.hopSize = static_cast<int>(hopSizeSlider.getValue());
    config.medianFilter = medianFilterToggle.getToggleState();
    config.medianWindowSize = static_cast<int>(medianWindowSlider.getValue());
    config.s = static_cast<float>(sSlider.getValue());
    config.similarityThreshold = static_cast<float>(thresholdSlider.getValue());
    config.chromaRes = static_cast<int>(chromaResSlider.getValue());

    if (onSaveCallback)
      onSaveCallback();
  };

  setSize(400, 500);
}

void SettingsComponent::resized() {
  int margin = 20;
  int y = margin;
  int height = 30;
  int labelWidth = 150;
  int sliderWidht = getWidth() - labelWidth - margin * 2;

  useDeepLearningToggle.setBounds(margin, y, getWidth() - (margin * 2), height);
  y += height + 10;

  auto layoutRow = [&](juce::Label &lbl, juce::Slider &sl) {
    lbl.setBounds(margin, y, labelWidth, height);
    sl.setBounds(margin + labelWidth, y, sliderWidht, height);
    y += height + 10;
  };

  layoutRow(fftOrderLabel, fftOrderSlider);
  layoutRow(hopSizeLabel, hopSizeSlider);

  medianFilterToggle.setBounds(margin, y, getWidth() - (margin * 2), height);
  y += height + 10;

  layoutRow(medianWindowLabel, medianWindowSlider);
  layoutRow(sLabel, sSlider);
  layoutRow(thresholdLabel, thresholdSlider);
  layoutRow(chromaResLabel, chromaResSlider);

  saveButton.setBounds((getWidth() / 2) - 60, y + 10, 120, 40);
}

// ==========================================================================================

SettingsWindow::SettingsWindow(const juce::String &name, AnalyzerConfig &config)
    : juce::DocumentWindow(name, juce::Colours::darkgrey,
                           DocumentWindow::allButtons),
      settingsComp(config, [this] { closeButtonPressed(); }) {
  setContentNonOwned(&settingsComp, true);
  setResizable(false, false);
  centreWithSize(getWidth(), getHeight());
  setVisible(true);
}

void SettingsWindow::closeButtonPressed() { delete this; }
