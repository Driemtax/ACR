#pragma once

#include "AnalyzerConfig.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include <JuceHeader.h>
#include <cstddef>
#include <juce_dsp/juce_dsp.h>
#include <vector>

class SpectogramAnalyzer
{
  public:
  SpectogramAnalyzer(int fftOrder = 12);
  SpectogramAnalyzer(AnalyzerConfig &config);
  ~SpectogramAnalyzer() = default;

  // 2D-Array with spectogram for full audio data
  juce::AudioBuffer<float> processFullFile(const juce::AudioBuffer<float>& fullAudioFile, double sampleRate);

  double getSampleRate() {
      return 44100.0;
  }

  int getHopSize() {
      return hopSize;
  }

  private:
  int fftOrder;
  int fftSize;
  int hopSize;

  juce::dsp::FFT fft;
  juce::dsp::WindowingFunction<float> window { static_cast<size_t>(fftSize), juce::dsp::WindowingFunction<float>::hann };

  // Utility
  void normalizeVolume(juce::AudioBuffer<float>& bufferToNormalize);
  std::vector<float> processSingleFrame(const float* frameData);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectogramAnalyzer);
};
