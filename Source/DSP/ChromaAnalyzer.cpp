#include "ChromaAnalyzer.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

ChromaAnalyzer::ChromaAnalyzer(float sampleRate, float fftSize)
    : fftBinFrequencies(static_cast<int>(fftSize / 2)),
      binMids(chromaSize * resolution),
      harmonicWeights(8) {
  int numBins = static_cast<int>(fftSize / 2);

  for (int i = 0; i < numBins; i++) {
    fftBinFrequencies[i] = i * (sampleRate / fftSize);
  }

  currentPeaks.reserve(numBins);

  // Initialize binMids
  calculateBinMids();

  // Initialize Harmonic weights
  calculateHarmonicWeights();
}

// This function processes every frame of a given spectogram and calculates the HPCP vector for every frame normalized in range [0,1].
void ChromaAnalyzer::processFullSpectogram(const juce::AudioBuffer<float> &spectogram, juce::AudioBuffer<float> &outChromagram) {
    const float* currentFrame;

    // Iterate over every frame and calculate all chroma bins
    for (int i = 0; i < spectogram.getNumChannels(); i++) {
        currentFrame = spectogram.getReadPointer(i);
        processFrame(currentFrame, i, outChromagram);
    }

    // Normalize every frame
    normalizeBins(outChromagram);
}

// This function performs a full HPCP on a frame of the spectogram. For details of the HPCP algorithm see docs/DSP
void ChromaAnalyzer::processFrame(const float* currentFrame, int frameNum, juce::AudioBuffer<float> &outChroma) {
    float sumBinEnergy = 0.0f;
    float freqWeight = 0.0f;
    float dist = 0.0f;
    float harmonicWeight = 0.0f;
    float squaredMag = 0.0f;

    extractPeaks(currentFrame);

    // Iterate over all Bins
    for (int n = 0; n < chromaSize * resolution; n++) {
        sumBinEnergy = 0.0f;
        // Iterate over every peak and calculate HPCP
        for (const auto& p : currentPeaks) {
            squaredMag = p.magnitude * p.magnitude;
            // Iterate over 8 harmonics
            for (int h = 0; h < 8; h++) {
                dist = calculateDistance(p.frequency / h+1, binMids[n]);
                freqWeight = calculateWeightFreq(n, dist);
                harmonicWeight = harmonicWeights[h];

                sumBinEnergy += freqWeight * harmonicWeight * squaredMag;
            }
        }

        outChroma.getWritePointer(frameNum)[n] = sumBinEnergy;
    }
}

// Extracts all relevant peaks of one spectogram frame and fills the currentPeaks Buffer.
void ChromaAnalyzer::extractPeaks(const float* currentFrame) {
    // reset currentPeaks every Frame
    currentPeaks.clear();

    int numFrequencies = fftBinFrequencies.size();
    Peak peak(0.0f, 0.0f);;

    float maxValInFrame = *std::max_element(currentFrame, currentFrame + numFrequencies);
    // the thresholds needs to be at a minimum of 0.001 so that a silent frame does not get peaks due to noise
    float threshold = std::max(maxValInFrame * 0.1f, 0.001f);


    // We iterate over all Frequency Bins of this frame and search for a local maxima considering the left and right neighbour frame
    // we start at Index 1, cause index 0 has no left neighbour. We also end one bin early, cause last bin has no right neighbour.
    for (int i = 1; i < numFrequencies-1; i++) {
        if (currentFrame[i] > currentFrame[i-1] &&
            currentFrame[i] > currentFrame[i+1] &&
            currentFrame[i] > threshold) {
            peak = Peak(fftBinFrequencies[i], currentFrame[i]);
            currentPeaks.push_back(peak);
        }
    }
}

// Calculates the mid frequencies for every bin of the hpcp. This only needs to
// be called once to initialize it, those frequencies never change.
void ChromaAnalyzer::calculateBinMids() {
  int numBins = binMids.size();
  for (int i = 0; i < numBins; i++) {
    binMids[i] = f_ref * pow(2, (float)i / numBins);
  }
}

// Calculates the distance between to frequencies, not regarding octave
// information. It is used to calculate the distance between a given frequency
// f_i and the mid frequency of a hpcp bin.
float ChromaAnalyzer::calculateDistance(float f_i, float f_n) const {
  if (f_n < 0.1) {
    return INFINITY;
  }
  float rawDist = 12 * log2f(f_i / f_n);
  float dist = fmod(rawDist, 12);

  // To find the shortest distance, we need to normalize it from [-6,6] since 0
  // is the middle
  if (dist > 6.0f) {
    dist -= 12.0f;
  } else if (dist < -6.0f) {
    dist += 12.0f;
  }

  return dist;
}

// Calculates the weight of a given frequency f_i in the n-th hpcp bin
// see docs/DSP for further explanations.
float ChromaAnalyzer::calculateWeightFreq(int n, float dist) const {
  float weight = 0.0;
  float halfPi = juce::MathConstants<float>::halfPi;

  if (std::abs(dist) <= 0.5 * l) {
    float rootedWeight = cos(halfPi * (dist / (0.5 * l)));
    weight = rootedWeight * rootedWeight;
  } else {
    weight = 0.0f;
  }

  return weight;
}

// Calculates the weight of a harmonic. See docs/DSP for further details.
void ChromaAnalyzer::calculateHarmonicWeights() {
    for (int i = 0; i < harmonicWeights.size(); i++) {
        harmonicWeights[i] = pow(s, i);
    }
}

// Normalizes all Bins of every Frame to [0,1]. This ensures that volume does
// not has any effects.
void ChromaAnalyzer::normalizeBins(juce::AudioBuffer<float> &outChroma) {
  int frameCount = outChroma.getNumChannels();
  int binCount = outChroma.getNumSamples();

  float maxBinVal = 0.0;
  float noiseGate = 700.0f;

  // Divide every bin by the highestValue
  // All Frames
  for (int i = 0; i < frameCount; i++) {
    // find the highest value per Frame
    maxBinVal = outChroma.getMagnitude(i, 0, binCount);
    float *const framePointer = outChroma.getWritePointer(i);
    if (maxBinVal <= noiseGate) {
        for (int n = 0; n < 12; n++) {
            framePointer[n] = 0.0f;
        }

        continue;
    }


    // All 12 Bins of one frame
    for (int j = 0; j < binCount; j++) {
      float rawVal = framePointer[j];

      if (maxBinVal > 0.0f) {
        framePointer[j] = rawVal / maxBinVal;
        }

      //std::cout << "Frame: " << i << ", Bin:" << j << ", Energy: " << framePointer[j] << std::endl;
    }
  }
}

int ChromaAnalyzer::getChromaBinSize() const {
    return chromaSize * resolution;
}
