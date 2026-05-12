#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include <JuceHeader.h>
#include <vector>

class ChromaAnalyzer {
    public:
    ChromaAnalyzer(float sampleRate, float fftSize, float s, int chromaRes);
    ~ChromaAnalyzer() = default;

    void processFullSpectogram(const juce::AudioBuffer<float> &spectogram, juce::AudioBuffer<float> &outChromagram);
    int getChromaBinSize() const;

    private:
    // HPCP Parameters
    // l is the window size of the weighting function
    float l = 1.3333f; // set to 4/3 semitones regarding Gomez, 2006
    // s is the weighting parameter for harmonics
    float s = 0.6; // value regarding Gomez 2006

    // Resolution of Chromagram
    int resolution = 1;
    int chromaSize = 12;

    // Reference Frequenz
    float f_ref = 261.626f; // C4

    struct Peak {
      float frequency;
      float magnitude;

      Peak(float freq, float mag) : frequency(freq), magnitude(mag){}
    };

    void processFrame(const float* currentFrame, int frameNum, juce::AudioBuffer<float> &outChroma);
    void extractPeaks(const float* currentFrame);
    void calculateBinMids();
    float calculateDistance(float f_i, float f_n) const;
    float calculateWeightFreq(int n, float dist) const;
    void calculateHarmonicWeights();
    void normalizeBins(juce::AudioBuffer<float> &outChroma);
    void applyMedianFilter(juce::AudioBuffer<float> &chroma);

    std::vector<Peak> currentPeaks;

    // Lookup table for Frequencies of FFT-Bins
    std::vector<float> fftBinFrequencies;
    // Lookup table for bin mids for HPCP
    std::vector<float> binMids;
    // Lookup table for harmonic weights since those are independent of frequencies
    std::vector<float> harmonicWeights;


};
