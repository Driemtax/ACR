#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>
#include <array>

class KeyEstimator {
public:
  enum class ProfileType { KrumhanslKessler, Temperley };
  KeyEstimator(ProfileType type = ProfileType::KrumhanslKessler);
  ~KeyEstimator() = default;

  enum class Mode { Major, Minor };

  struct Key {
    int rootIndex;
    Mode mode;
  };

  Key estimateKey(const juce::AudioBuffer<float> &chroma) const;
  void applyKeyWeights(juce::AudioBuffer<float> &chroma, const Key &key) const;

private:
  std::array<float, 12>
  calculateHarmonicProfile(const juce::AudioBuffer<float> &chroma) const;
  std::array<float, 24>
  calculateCrossCorrelation(const std::array<float, 12> &harmonicProfile) const;

  // Those weights are provided by Krumhansl and Kessler
  std::array<float, 12> majorProfile;
  std::array<float, 12> minorProfile;

  // static constexpr std::array<float, 12> majorProfile = {
  //     6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
  //     2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f};
  // static constexpr std::array<float, 12> minorProfile = {
  //     6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
  //     2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f};

  // Temperly Weights
  // static constexpr std::array<float, 12> majorProfile = {
  // 5.0f,
  // 2.0f, 3.5f, 2.0f, 4.5f, 4.0f, 2.0f, 4.5f, 2.0f, 3.5f, 1.5f, 4.0f
  //};

  // static constexpr std::array<float, 12> minorProfile = {
  // 5.0f, 2.0f, 3.5f, 4.5f, 2.0f, 4.0f, 2.0f, 4.5f, 3.5f, 2.0f, 1.5f, 4.0f
  //}
  //;
};
