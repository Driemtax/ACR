#include "KeyEstimator.h"
#include "../Util/FretboardMapper.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include <algorithm>
#include <array>
#include <iterator>
#include <string>

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::array<T, 24> &v) {
  os << "[";
  for (size_t i = 0; i < v.size(); ++i) {
    std::cout << v[i];
    if (i != v.size() - 1)
      os << ", ";
  }
  os << "]";
  return os;
}

KeyEstimator::KeyEstimator(ProfileType type) {
  if (type == ProfileType::Temperley) {
    majorProfile = {5.0f, 2.0f, 3.5f, 2.0f, 4.5f, 4.0f,
                    2.0f, 4.5f, 2.0f, 3.5f, 1.5f, 4.0f};

    minorProfile = {5.0f, 2.0f, 3.5f, 4.5f, 2.0f, 4.0f,
                    2.0f, 4.5f, 3.5f, 2.0f, 1.5f, 4.0f};
  } else {
    // Krumhansl-Kessler profiles
    majorProfile = {6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
                    2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f};

    minorProfile = {6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
                    2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f};
  }
}

/**
 * @brief Applies key-specific weights to the given chroma features.
 *
 * This method modifies the input chroma buffer in-place by multiplying the
 * chroma bins with the profile weights corresponding to the specified key.
 * After applying the weights, the chroma features are normalized using the
 * global maximum value across all frames to ensure the values remain within
 * a valid range.
 *
 * @param chroma A juce::AudioBuffer containing the chroma features to be
 * weighted. The buffer is modified in-place. The number of samples must be
 * exactly 12.
 * @param key The musical key (root note index and mode) used to determine
 *            the appropriate major or minor profile weights to apply.
 */
void KeyEstimator::applyKeyWeights(juce::AudioBuffer<float> &chroma,
                                   const Key &key) const {
  const int frameCount = chroma.getNumChannels();
  if (frameCount == 0)
    return;

  const int numBins = chroma.getNumSamples();
  jassert(numBins == 12);

  // choose correct profile
  const auto &profile = (key.mode == Mode::Major) ? majorProfile : minorProfile;

  // keep track of global max value of the chroma for normalization later on.
  float globalMax = 0.0f;

  // apply weights of the profile
  for (int f = 0; f < frameCount; f++) {
    float *frame = chroma.getWritePointer(f);

    for (int b = 0; b < numBins; b++) {
      int profileIndex = (b - key.rootIndex + 12) % 12;

      frame[b] = frame[b] * profile[profileIndex];

      if (frame[b] > globalMax) {
        globalMax = frame[b];
      }
    }
  }

  // Normalize chroma again
  if (globalMax > 0.0f) {
    for (int f = 0; f < frameCount; f++) {
      float *frame = chroma.getWritePointer(f);
      for (int b = 0; b < numBins; b++) {
        frame[b] /= globalMax;
      }
    }
  }
}

/**
 * @brief Estimates the musical key from a given chroma feature matrix.
 *
 * This method calculates the average harmonic profile from the input chroma
 * buffer, computes its cross-correlation with predefined major and minor key
 * profiles, and determines the most likely key based on the highest
 * correlation score.
 *
 * @param chroma A juce::AudioBuffer containing the chroma features. The number
 *               of channels represents the frames, and the number of samples
 *               must be exactly 12 (corresponding to the 12 pitch classes).
 * @return KeyEstimator::Key A structure containing the estimated root note
 * index (0-11) and the musical mode (Major or Minor).
 */
KeyEstimator::Key
KeyEstimator::estimateKey(const juce::AudioBuffer<float> &chroma) const {
  std::array<float, 12> harmonicProfile = calculateHarmonicProfile(chroma);

  std::cout << "Harmonic Profile Bins: " << std::endl;

  for (int i = 0; i < 12; i++) {
    std::cout << "Bin " << i << " (" << FretboardMapper::getNoteName(i)
              << "): " << harmonicProfile[i] << std::endl;
  }

  std::array<float, 24> correlations =
      calculateCrossCorrelation(harmonicProfile);

  std::cout << "Correlations: " << correlations << std::endl;

  // 1. Find maximum Value of correlations
  int maxValueIndex =
      std::distance(correlations.begin(),
                    std::max_element(correlations.begin(), correlations.end()));

  // 2. Find root
  int rootIndex = maxValueIndex % 12;

  // 3. Find mode
  Mode mode = maxValueIndex < 12 ? Mode::Major : Mode::Minor;
  juce::String modeString = mode == Mode::Major ? "Maj" : "Min";

  std::cout << "Key: " << FretboardMapper::getNoteName(rootIndex) << " "
            << modeString << std::endl;

  Key key = {rootIndex, mode};
  return key;
}

/**
 * @brief Calculates the harmonic profile from a given chroma matrix.
 *
 * This method computes the average magnitude for each of the 12 pitch classes
 * (chroma bins) across all frames provided in the input buffer.
 *
 * @param chroma A juce::AudioBuffer containing the chroma features. The number
 *               of channels represents the frames, and the number of samples
 *               must be exactly 12 (corresponding to the 12 pitch classes).
 * @return std::array<float, 12> An array containing the averaged harmonic
 * profile.
 */
std::array<float, 12> KeyEstimator::calculateHarmonicProfile(
    const juce::AudioBuffer<float> &chroma) const {
  std::array<float, 12> binSums = {0.0f};
  const int frameCount = chroma.getNumChannels();

  if (frameCount == 0)
    return binSums;

  jassert(chroma.getNumSamples() == 12);

  // cumulate all bins over every frame
  for (int f = 0; f < frameCount; f++) {
    const float *frame = chroma.getReadPointer(f);

    for (int b = 0; b < chroma.getNumSamples(); b++) {
      binSums[b] += frame[b];
    }
  }

  // divide by frame count to calculate average of every bin over every frame
  for (int i = 0; i < binSums.size(); i++) {
    binSums[i] = binSums[i] / static_cast<float>(frameCount);
  }

  return binSums;
}

/**
 * @brief Calculates the cross-correlation between a harmonic profile and key
 * profiles.
 *
 * This method computes the cross-correlation of the provided 12-bin harmonic
 * profile against predefined major and minor key profiles across all 12
 * possible pitch shifts. The resulting array contains the correlation scores
 * for all 24 possible keys, which indicate the likelihood of each key being the
 * correct one.
 *
 * @param harmonicProfile A 12-element array representing the averaged harmonic
 * profile.
 * @return std::array<float, 24> An array containing the correlation scores,
 * where the first 12 elements correspond to major keys and the next 12 to minor
 * keys.
 */
std::array<float, 24> KeyEstimator::calculateCrossCorrelation(
    const std::array<float, 12> &harmonicProfile) const {
  std::array<float, 24> correlations = {0.0f};

  // Check Major and Minor Profiles
  for (int shift = 0; shift < 12; shift++) {
    float majorSum = 0.0f;
    float minorSum = 0.0f;

    for (int bin = 0; bin < harmonicProfile.size(); bin++) {
      int profileIndex = (bin - shift + 12) % 12;

      majorSum += harmonicProfile[bin] * majorProfile[profileIndex];
      minorSum += harmonicProfile[bin] * minorProfile[profileIndex];
    }

    correlations[shift] = majorSum;      // Index 0-11: C-Maj - B-Maj
    correlations[shift + 12] = minorSum; // Index 11-23: C-Min - B-Min
  }

  return correlations;
}
