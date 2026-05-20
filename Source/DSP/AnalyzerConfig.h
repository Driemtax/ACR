#pragma once

struct AnalyzerConfig {
  // 1. Spectogram
  int fftOrder = 12;
  int fftSize = 4096; // 2^12 = 4096
  int hopSize = 512;

  // 2. Algorithm selection
  bool useDeepLearning = false;

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
      fftOrder = 13; // 2^13 = 8192
      fftSize = 8192; // Required by madmon model
      hopSize = 4410; // 10 FPS at 44.1 kHz
  }
};
