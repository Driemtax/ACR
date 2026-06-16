#pragma once

#include "Audio/AudioEngine.h"
#include "DSP/AnalyzerConfig.h"
#include "GUI/InstrumentComponent.h"
#include "GUI/ScienceView.h"
#include "GUI/WaveformDisplay.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "juce_events/juce_events.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <JuceHeader.h>
#include <memory>

class MainComponent : public juce::Component,
                      private juce::AsyncUpdater,
                      private juce::ChangeListener {

public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics &g) override;
  void resized() override;
  void handleAsyncUpdate() override;
  void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:
  AudioEngine audioEngine;
  AnalyzerConfig config;

  // --- Global Transport Bar ---
  juce::TextButton recordButton{"Record"};
  juce::TextButton playButton{"Play"};
  juce::TextButton stopButton{"Stop"};
  juce::Label fileToAnalyze;
  juce::TextButton fileButton{"Open File"};
  juce::TextButton settingsButton{"Settings"};
  std::unique_ptr<juce::FileChooser> fileChooser;

  // Audio device settings window
  juce::Component::SafePointer<juce::DocumentWindow> audioSettingsWindow;

  // Waveform (always visible)
  WaveformDisplay waveformDisplay{audioEngine};

  // --- View Switching ---
  juce::TextButton instrumentTab{"Instrument"};
  juce::TextButton scienceTab{"Analyze"};

  // --- Views ---
  InstrumentComponent instrumentView{audioEngine};
  ScienceView scienceView{audioEngine, config};

  enum class ActiveView { Instrument, Science };
  ActiveView activeView = ActiveView::Instrument;

  void switchToView(ActiveView view);
  void updateTransportState();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
