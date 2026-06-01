#pragma once

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
  int medianWindowSize = 5;
  float s = 0.6f;

  // 4. Classification
  float similarityThreshold = 0.8f;

  // Helper to safely set ML defaults
  void setToDeepLearningDefaults() {
    useDeepLearning = true;
    fftOrder = 13;              // 2^13 = 8192
    fftSize = 8192;             // Required by madmon model
    hopSize = 4410;             // 10 FPS at 44.1 kHz
    normalizeAudio = false;     // madmom does not normalize
    centerOriginPadding = true; // madmom uses cetner-origin framing
    convertToDecibel = false;   // madmom uses linear scaled fft magnitudes
  }
};
