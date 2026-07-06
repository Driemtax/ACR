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
      setDeepLearning();

    } else {
      setHPCP();
    }
  };

  addAndMakeVisible(TuningShiftToggle);
  TuningShiftToggle.setToggleState(config.tuningShift,
                                   juce::dontSendNotification);
  TuningShiftToggle.onClick = [this] {
    chromaResSlider.setEnabled(TuningShiftToggle.getToggleState());
  };

  addAndMakeVisible(keyEstimationToggle);
  keyEstimationToggle.setToggleState(config.useKeyEstimator,
                                     juce::dontSendNotification);
  keyEstimationToggle.onClick = [this] {
    profileSelector.setEnabled(keyEstimationToggle.getToggleState());
  };

  addAndMakeVisible(profileLabel);
  addAndMakeVisible(profileSelector);
  profileSelector.addItem("Krumhansl-Kessler", 1);
  profileSelector.addItem("Temperley", 2);
  profileSelector.setSelectedId(
      config.profileType == KeyEstimator::ProfileType::KrumhanslKessler ? 1 : 2,
      juce::dontSendNotification);

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
  setupSlider(medianWindowSlider, medianWindowLabel, 1, 60, 1,
              config.medianWindowSize);
  setupSlider(sSlider, sLabel, 0.1, 1.0, 0.05, config.s);
  setupSlider(thresholdSlider, thresholdLabel, 0.0, 1.0, 0.05,
              config.similarityThreshold);
  setupSlider(chromaResSlider, chromaResLabel, 1, 3, 1, config.chromaRes);

  addAndMakeVisible(medianFilterToggle);
  medianFilterToggle.setToggleState(config.medianFilter,
                                    juce::dontSendNotification);
  medianFilterToggle.onClick = [this] {
    medianWindowSlider.setEnabled(medianFilterToggle.getToggleState());
  };

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
    config.tuningShift = TuningShiftToggle.getToggleState();
    config.useKeyEstimator = keyEstimationToggle.getToggleState();
    config.profileType = profileSelector.getSelectedId() == 1
                             ? KeyEstimator::ProfileType::KrumhanslKessler
                             : KeyEstimator::ProfileType::Temperley;

    if (onSaveCallback)
      onSaveCallback();
  };

  if (config.useDeepLearning) {
    setDeepLearning();
  }

  if (!config.tuningShift) {
    chromaResSlider.setEnabled(false);
  }

  if (!config.medianFilter) {
    medianWindowSlider.setEnabled(false);
  }

  if (!config.useKeyEstimator) {
    profileSelector.setEnabled(false);
  }

  setSize(400, 550);
}

void SettingsComponent::setDeepLearning() {
  // auto-set madmom defaults
  fftOrderSlider.setValue(13);
  hopSizeSlider.setValue(4410);

  // Disable all settings
  fftOrderSlider.setEnabled(false);
  hopSizeSlider.setEnabled(false);
  medianFilterToggle.setEnabled(false);
  medianWindowSlider.setEnabled(false);
  sSlider.setEnabled(false);
  thresholdSlider.setEnabled(false);
  chromaResSlider.setEnabled(false);
  TuningShiftToggle.setEnabled(false);
}

void SettingsComponent::setHPCP() {
  // Enable all settings an reset to defaults
  config.setToDefaults();

  fftOrderSlider.setEnabled(true);
  hopSizeSlider.setEnabled(true);
  medianFilterToggle.setEnabled(true);
  if (config.medianFilter) {
    medianWindowSlider.setEnabled(true);
  }
  sSlider.setEnabled(true);
  thresholdSlider.setEnabled(true);
  TuningShiftToggle.setEnabled(true);
  if (config.tuningShift) {
    chromaResSlider.setEnabled(true);
  }

  if (config.useKeyEstimator) {
    profileSelector.setEnabled(true);
  }

  // update UI
  setUIDefaults();
}

void SettingsComponent::setUIDefaults() {
  fftOrderSlider.setValue(config.fftOrder);
  hopSizeSlider.setValue(config.hopSize);
  medianFilterToggle.setToggleState(config.medianFilter,
                                    juce::dontSendNotification);
  medianWindowSlider.setValue(config.medianWindowSize);
  sSlider.setValue(config.s);
  thresholdSlider.setValue(config.similarityThreshold);
  chromaResSlider.setValue(config.chromaRes);
  TuningShiftToggle.setToggleState(config.tuningShift,
                                   juce::dontSendNotification);
  keyEstimationToggle.setToggleState(config.useKeyEstimator,
                                     juce::dontSendNotification);
  profileSelector.setSelectedId(
      config.profileType == KeyEstimator::ProfileType::KrumhanslKessler ? 1 : 2,
      juce::dontSendNotification);
  profileSelector.setEnabled(config.useKeyEstimator);
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

  TuningShiftToggle.setBounds(margin, y, getWidth() - (margin * 2), height);
  y += height + 10;
  layoutRow(chromaResLabel, chromaResSlider);

  keyEstimationToggle.setBounds(margin, y, getWidth() - (margin * 2), height);
  y += height + 10;
  profileLabel.setBounds(margin, y, labelWidth, height);
  profileSelector.setBounds(margin + labelWidth, y, sliderWidht, height);
  y += height + 10;

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
