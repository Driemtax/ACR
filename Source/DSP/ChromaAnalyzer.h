#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>
#include <vector>

class ChromaAnalyzer {
    public:
    ChromaAnalyzer(float sampleRate, float fftSize);
    ~ChromaAnalyzer();

    void processFullSpectogram(const juce::AudioBuffer<float> &spectogram, juce::AudioBuffer<float> &outChromagram);

    private:
    struct Peak {
      float frequency;
      float magnitude;
    };

    void processFrame(const float* currentFrame, int frameNum);
    void extractPeaks(const float* currentFrame, int frameNum);
    void calculateBinMids();
    float calculateDistance(float f_i, float f_n) const;
    float calculateWeightFreq(float f_i, int n, float dist) const;
    void normalizeBins();

    juce::AudioBuffer<float> &outChroma;
    std::vector<Peak> currentPeaks;
    std::vector<float> currentBins;

    // Lookup table for Frequencies of FFT-Bins
    std::vector<float> fftBinFrequencies;
    // Lookup table for bin mids for HPCP
    std::vector<float> binMids;

    // HPCP Parameters
    // l is the window size of the weighting function
    float l = 1.3333f; // set to 4/3 semitones regarding Gomez, 2006
    // s is the weighting parameter for harmonics
    float s = 0.6; // value regarding Gomez 2006

    // Resolution of Chromagram
    int resolution = 1;
    int chromaSize = 12;

    // Reference Frequenz
    float f_ref = 440.0f;

};
