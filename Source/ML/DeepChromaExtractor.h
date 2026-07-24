#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>
#include <memory>
#include <onnxruntime_cxx_api.h>

#include "../DSP/ChromaExtractorInterface.h"

class DeepChromaExtractor : public ChromaExtractorInterface {
public:
  DeepChromaExtractor(int binSize, int medianSize);
  ~DeepChromaExtractor() = default;

  void extractChroma(const juce::AudioBuffer<float> &spectogram,
                     juce::AudioBuffer<float> &chroma) override;
  int getChromaBinSize() const override;

private:
  void loadModel();
  void applyMedianFilter(juce::AudioBuffer<float> &chroma) const;

  const wchar_t *modelPath = L"C:\\Test\\ML\\deep_chroma.onnx";

  // onnx config
  Ort::Env ortEnv;
  std::unique_ptr<Ort::Session> ortSession;
  Ort::SessionOptions sessionOptions;

  // chroma size
  int chromaBinSize;
  int medianSize = 19;
};
