#include "ChromaAnalyzer.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include <cmath>
#include <vector>

ChromaAnalyzer::ChromaAnalyzer(float sampleRate, float fftSize) {
    int numBins = static_cast<int>(fftSize/2);

    fftBinFrequencies.resize(numBins);
    for (int i = 0; i < numBins; i++) {
        fftBinFrequencies[i] = i * (sampleRate / fftSize);
    }

    currentPeaks.reserve(numBins);
    binMids.resize(chromaSize * resolution);
    currentBins = std::vector<float>(resolution);

    // Initialize binMids
    calculateBinMids();
}

ChromaAnalyzer::~ChromaAnalyzer() {
}

// Calculates the mid frequencies for every bin of the hpcp. This only needs to be called once to initialize it, those frequencies never change.
void ChromaAnalyzer::calculateBinMids() {
    int numBins = binMids.size();
   for (int i = 0; i < numBins; i++) {
       binMids[i] = f_ref * pow(2, (float)i/numBins);
   }
}

// Calculates the distance between to frequencies, not regarding octave information. It is used to calculate the distance between a given frequency f_i
// and the mid frequency of a hpcp bin.
float ChromaAnalyzer::calculateDistance(float f_i, float f_n) const {
    if (f_n < 0.1) {
        return INFINITY;
    }
    float rawDist = 12 * log2f(f_i / f_n);
    float dist = fmod(rawDist, 12);

    // To find the shortest distance, we need to normalize it from [-6,6] since 0 is the middle
    if (dist > 6.0f) {
        dist -= 12.0f;
    } else if (dist < -6.0f) {
        dist += 12.0f;
    }

    return dist;
}

// Calculates the weight of a given frequency f_i in the n-th hpcp bin
// see docs/DSP for further explanations.
float ChromaAnalyzer::calculateWeightFreq(float f_i, int n, float dist) const {
    float weight = 0.0;
    float halfPi = juce::MathConstants<float>::halfPi;

    if (dist <= 0.5 * l) {
        float rootedWeight = cos(halfPi * (dist / (0.5*l)));
        weight = rootedWeight * rootedWeight;
    } else {
        weight = 0.0f;
    }

    return weight;
}

// Normalizes all Bins of every Frame to [0,1]. This ensures that volume does not has any effects.
void ChromaAnalyzer::normalizeBins() {
    int frameCount = outChroma.getNumChannels();
    int binCount = outChroma.getNumSamples();

    float maxBinVal = 0.0;

    // Divide every bin by the highestValue
    // All Frames
    for (int i = 0; i < frameCount; i++) {
        // find the highest value per Frame
        maxBinVal = outChroma.getMagnitude(i, 0, binCount);
        float* const framePointer = outChroma.getWritePointer(i);
        // All 12 Bins of one frame
        for (int j = 0; j < binCount; j++) {
            float rawVal = framePointer[j];

            if (maxBinVal > 0.0f) {
                framePointer[j] = rawVal / maxBinVal;
            }
        }
    }
}
