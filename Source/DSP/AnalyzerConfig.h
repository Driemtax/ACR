#pragma once

#include "KeyEstimator.h"

struct AnalyzerConfig {
  // 1. Spectogram
  int fftOrder = 12;
  int fftSize = 4096; // 2^12 = 4096
  int hopSize = 512;

  // 2. Algorithm selection
  bool useDeepLearning = false;
  // normalizeAudio: normalize audio to max amplitude 1.0 before FFT.
  //   Must be false for DeepChroma: the model was trained on raw
  //   (un-normalized) audio magnitudes. Normalization breaks the input
  //   distribution.
  bool normalizeAudio = true;
  // centerOriginPadding: pad fftSize/2 zeros at both ends before framing.
  //   Must be true for DeepChroma to match madmom's FramedSignalProcessor
  //   which centers the first frame at sample 0 (origin='center').
  bool centerOriginPadding = false;
  bool convertToDecibel = true;

  // 3. HPCP specific parameters
  int chromaRes = 1;
  bool medianFilter = true;
  int medianWindowSize = 19;
  float s = 0.6f;
  bool tuningShift = false;
  float sptRatio = 3.8f;

  // 4. Classification
  float similarityThreshold = 0.3f;
  bool useKeyEstimator = false;
  KeyEstimator::ProfileType profileType =
      KeyEstimator::ProfileType::KrumhanslKessler;

  // Helper to safely set ML defaults
  void setToDeepLearningDefaults() {
    useDeepLearning = true;
    fftOrder = 13;              // 2^13 = 8192
    fftSize = 8192;             // Required by madmon model
    hopSize = 4410;             // 10 FPS at 44.1 kHz
    normalizeAudio = false;     // madmom does not normalize
    centerOriginPadding = true; // madmom uses cetner-origin framing
    convertToDecibel = false;   // madmom uses linear scaled fft magnitudes

    similarityThreshold = 0.7f; // ML Vectors seem to work better with a higher
                                // similarityThreshold!
    useKeyEstimator = true;
    profileType = KeyEstimator::ProfileType::KrumhanslKessler;
  }

  void setToDefaults() {
    useDeepLearning = false;
    fftOrder = 12;
    fftSize = 4096;
    hopSize = 512;
    medianWindowSize = 223;
    s = 0.6f;
    similarityThreshold = 0.3f;
    centerOriginPadding = false;
    convertToDecibel = true;
    tuningShift = false;
    chromaRes = 1;
    medianFilter = true;
    normalizeAudio = true;
    useKeyEstimator = false;
    sptRatio = 3.8f;
  }
};
