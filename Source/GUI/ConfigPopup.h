#pragma once

#include "../DSP/AnalyzerConfig.h"
// clang-format off
#include <JuceHeader.h>
// clang-format on
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <functional>

class SettingsComponent : public juce::Component {
public:
  SettingsComponent(AnalyzerConfig &config, std::function<void()> onSave);
  void resized() override;

private:
  AnalyzerConfig &config;
  std::function<void()> onSaveCallback;

  juce::ToggleButton useDeepLearningToggle{"Use Deep Learning (Madmom)"};

  juce::Label fftOrderLabel{{}, "FFT Order (eg. 12 = 4096:"};
  juce::Slider fftOrderSlider{juce::Slider::LinearHorizontal,
                              juce::Slider::TextBoxLeft};

  juce::Label hopSizeLabel{{}, "STFT Hop Size:"};
  juce::Slider hopSizeSlider{juce::Slider::LinearHorizontal,
                             juce::Slider::TextBoxLeft};

  juce::ToggleButton medianFilterToggle{"Enable Median Filter"};

  juce::Label medianWindowLabel{{}, "Median Window Size: "};
  juce::Slider medianWindowSlider{juce::Slider::LinearHorizontal,
                                  juce::Slider::TextBoxLeft};

  juce::Label sLabel{{}, "Harmonic Weight (s):"};
  juce::Slider sSlider{juce::Slider::LinearHorizontal,
                       juce::Slider::TextBoxLeft};

  juce::Label thresholdLabel{{}, "Similarity Threshold:"};
  juce::Slider thresholdSlider{juce::Slider::LinearHorizontal,
                               juce::Slider::TextBoxLeft};

  juce::Label chromaResLabel{{}, "Chroma Resolution:"};
  juce::Slider chromaResSlider{juce::Slider::LinearHorizontal,
                               juce::Slider::TextBoxLeft};

  juce::ToggleButton TuningShiftToggle{"Enable Tuning Shift"};

  juce::TextButton saveButton{"Save Settings"};
};

class SettingsWindow : public juce::DocumentWindow {
public:
  SettingsWindow(const juce::String &name, AnalyzerConfig &config);
  void closeButtonPressed() override;

private:
  SettingsComponent settingsComp;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsWindow);
};
