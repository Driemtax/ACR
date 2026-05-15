#include "ChromaAnalyzer.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

ChromaAnalyzer::ChromaAnalyzer(float sampleRate, float fftSize, float s, int chromaRes, int medianWindow, bool medianFilter)
    : fftBinFrequencies(static_cast<int>(fftSize / 2)),
      binMids(chromaSize * resolution),
      harmonicWeights(8),
      resolution(chromaRes),
      s(s),
      medianWindow(medianWindow),
      medianFilter(medianFilter){
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

/**
 * @brief Processes every frame of a given spectrogram and calculates the HPCP vector.
 *
 * This method iterates over all frames of the input spectrogram, computes the Harmonic
 * Pitch Class Profile (HPCP) for each frame, normalizes the resulting vectors to the
 * range [0, 1], and applies a median filter to smooth the output chromagram.
 *
 * @param spectogram A constant reference to the input audio buffer containing the spectrogram data.
 * @param outChromagram A reference to the output audio buffer where the computed and filtered chromagram will be stored.
 */
void ChromaAnalyzer::processFullSpectogram(const juce::AudioBuffer<float> &spectogram, juce::AudioBuffer<float> &outChromagram) {
    const float* currentFrame;

    // Iterate over every frame and calculate all chroma bins
    for (int i = 0; i < spectogram.getNumChannels(); i++) {
        currentFrame = spectogram.getReadPointer(i);
        processFrame(currentFrame, i, outChromagram);
    }

    // Normalize every frame
    normalizeBins(outChromagram);

    // Apply median filter
    if (medianFilter) {
        applyMedianFilter(outChromagram);
    }
}

/**
 * @brief Performs a full Harmonic Pitch Class Profile (HPCP) calculation on a single frame of the spectrogram.
 *
 * For detailed information regarding the underlying HPCP algorithm, please refer to docs/DSP.
 *
 * @param currentFrame A pointer to the array containing the magnitude values of the current spectrogram frame.
 * @param frameNum The index of the frame currently being processed.
 * @param outChroma A reference to the output audio buffer where the computed chroma values will be written.
 */
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

/**
 * @brief Extracts the local maxima (peaks) from a given spectrogram frame.
 *
 * This method iterates over the frequency bins of the provided frame and identifies
 * local maxima by comparing each bin with its immediate left and right neighbors.
 * It only considers peaks within a specific frequency range (approximately 100 Hz to 5000 Hz)
 * and those that exceed a calculated magnitude threshold to ignore noise in quiet frames.
 * The identified peaks are stored internally for further processing.
 *
 * @param currentFrame A pointer to the array containing the magnitude values of the current spectrogram frame.
 */
void ChromaAnalyzer::extractPeaks(const float* currentFrame) {
    // reset currentPeaks every Frame
    currentPeaks.clear();

    int numFrequencies = fftBinFrequencies.size();
    float binWidth = fftBinFrequencies[1];
    int startIndex = std::max(2, static_cast<int>(100.f / binWidth));
    int endIndex = std::min(numFrequencies - 1, static_cast<int>(5000.0f / binWidth) + 1);
    Peak peak(0.0f, 0.0f);;

    float maxValInFrame = *std::max_element(currentFrame + startIndex, currentFrame + endIndex);
    // the thresholds needs to be at a minimum of 0.001 so that a silent frame does not get peaks due to noise
    float threshold = std::max(maxValInFrame * 0.1f, 0.001f);


    // We iterate over all Frequency Bins of this frame and search for a local maxima considering the left and right neighbour frame
    // we start at Index 1, cause index 0 has no left neighbour. We also end one bin early, cause last bin has no right neighbour.
    for (int i = startIndex; i < endIndex; i++) {
        if (currentFrame[i] > currentFrame[i-1] &&
            currentFrame[i] > currentFrame[i+1] &&
            currentFrame[i] > threshold) {
            peak = Peak(fftBinFrequencies[i], currentFrame[i]);
            currentPeaks.push_back(peak);
        }
    }
}

/**
 * @brief Calculates and initializes the center frequencies for all chroma bins.
 *
 * This method computes the center frequency for each Harmonic Pitch Class Profile
 * (HPCP) bin based on a reference frequency. The calculated frequencies are
 * spaced logarithmically and stored in the internal binMids array.
 */
void ChromaAnalyzer::calculateBinMids() {
  int numBins = binMids.size();
  for (int i = 0; i < numBins; i++) {
    binMids[i] = f_ref * pow(2, (float)i / numBins);
  }
}

/**
 * @brief Calculates the shortest pitch class distance between two frequencies.
 *
 * This method computes the distance in semitones between a given frequency and a reference
 * bin frequency. The resulting distance is normalized to be within the range [-6, 6] to
 * represent the shortest path in the circular pitch class space.
 *
 * @param f_i The input frequency to calculate the distance for.
 * @param f_n The reference frequency.
 * @return The shortest distance in semitones between the two frequencies. Returns INFINITY if f_n is less than 0.1.
 */
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

/**
 * @brief Calculates the weight of a given frequency in the n-th HPCP bin.
 *
 * This method computes the weighting factor for a frequency based on its distance
 * to the center frequency of the specified Harmonic Pitch Class Profile (HPCP) bin.
 * It uses a cosine-squared window, ensuring that only frequencies within a certain
 * distance contribute to the bin's energy. For further mathematical details, please
 * refer to docs/DSP.
 *
 * @param n The index of the HPCP bin.
 * @param dist The distance between the frequency and the center frequency of the bin.
 * @return The computed weight for the frequency.
 */
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

/**
 * @brief Calculates and initializes the weights for each harmonic.
 *
 * This method computes the weighting factor for each harmonic based on the
 * spectral shape parameter 's'. The weights are calculated as s raised to the
 * power of the harmonic index, resulting in an exponential decay. For further
 * mathematical details, please refer to docs/DSP.
 */
void ChromaAnalyzer::calculateHarmonicWeights() {
    for (int i = 0; i < harmonicWeights.size(); i++) {
        harmonicWeights[i] = pow(s, i);
    }
}

/**
 * @brief Normalizes all bins of every frame to the range [0, 1].
 *
 * This method ensures that the overall volume of the audio does not affect
 * the resulting chroma values. It scales the bin energies of each frame by
 * dividing them by the maximum bin value within that frame. Frames with a
 * maximum energy below a specified noise gate threshold are zeroed out to
 * prevent noise amplification in silent passages.
 *
 * @param outChroma A reference to the audio buffer containing the chromagram data to be normalized in-place.
 */
void ChromaAnalyzer::normalizeBins(juce::AudioBuffer<float> &outChroma) {
  int frameCount = outChroma.getNumChannels();
  int binCount = outChroma.getNumSamples();

  float globalMax = 0.0f;
  for (int i = 0; i < frameCount; i++) {
      float frameMax = outChroma.getMagnitude(i, 0, binCount);
      if (frameMax > globalMax) globalMax = frameMax;
  }

  float maxBinVal = 0.0;
  float noiseGate = globalMax * 0.1f;

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

/**
 * @brief Applies a median filter across the frames of the chromagram to smooth the data.
 *
 * This method iterates over each chroma bin and applies a moving median filter across
 * consecutive frames. This smoothing process reduces noise and isolated spurious peaks
 * in the chroma features, improving the temporal stability of the output chromagram.
 * The filtering is performed using an internal copy of the buffer to prevent the
 * processed frames from recursively affecting the subsequent frames.
 *
 * @param chroma A reference to the audio buffer containing the chromagram data to be filtered in-place.
 */
void ChromaAnalyzer::applyMedianFilter(juce::AudioBuffer<float> &chroma) {
    // It's just me and the OG Boys. We cant write directly in-place, because then the results of one frame would influence the next frames result.
    juce::AudioBuffer<float> ogChroma;
    ogChroma.makeCopyOf(chroma);

    const int medianWindowSize = medianWindow;
    const int medianMid = medianWindowSize / 2;
    const int maxFrames = chroma.getNumChannels();
    int currentIndex = 0;
    float binMedian = 0.0f;

    std::vector<float> currentValues(medianWindowSize, 0.0f);

    for (int frame = 0; frame < maxFrames; frame++) {
        float* outputFrame = chroma.getWritePointer(frame);
        for (int bin = 0; bin < chroma.getNumSamples(); bin++) {
            // We iterate over the bin of the left and right neighbors and calculate the median.
            for (int i = -medianMid; i <= medianMid; i++) {
                // This ensures we dont acces out of bounds elements!
                currentIndex = std::max(0, std::min(maxFrames - 1, frame + i));
                const float* neighborFrame = ogChroma.getReadPointer(currentIndex);
                currentValues[i + medianMid] = neighborFrame[bin];
            }
            auto first = currentValues.begin();
            auto nth = first + medianMid;
            auto last = currentValues.end();
            std::nth_element(first, nth, last);

            // The result of nth_element is garanteed to be at index medianMid.
            outputFrame[bin] = currentValues[medianMid];
        }
    }


}

int ChromaAnalyzer::getChromaBinSize() const {
    return chromaSize * resolution;
}
