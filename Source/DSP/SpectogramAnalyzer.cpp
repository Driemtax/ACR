#include "SpectogramAnalyzer.h"
#include "AnalyzerConfig.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_dsp/juce_dsp.h"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <execution>
#include <numeric>
#include <vector>

SpectogramAnalyzer::SpectogramAnalyzer(int order)
    : fftOrder(order), fftSize(1 << order), fft(order), hopSize(512) {}

SpectogramAnalyzer::SpectogramAnalyzer(AnalyzerConfig &config)
    : fftOrder(config.fftOrder), fftSize(config.fftSize), fft(config.fftOrder),
      hopSize(config.hopSize), normalizeAudio(config.normalizeAudio),
      centerOriginPadding(config.centerOriginPadding),
      convertToDecibels(config.convertToDecibel) {}

/**
 * @brief Processes a full audio file and generates its spectrogram.
 *
 * This method normalizes the volume of the audio, mixes multi-channel audio
 * down to a single mono channel, and applies a Short-Time Fourier Transform
 * (STFT) over the entire audio buffer to generate the frequency magnitude data
 * over time.
 *
 * @param fullAudioFile A reference to the juce::AudioBuffer containing the
 * audio data to be processed.
 * @param sampleRate    The sample rate of the audio data.
 * @return              A 2D vector representing the spectrogram, where each
 * inner vector contains the magnitude bins for a single timeframe.
 */
juce::AudioBuffer<float> SpectogramAnalyzer::processFullFile(
    const juce::AudioBuffer<float> &fullAudioFile) const {
  std::vector<float> frameMagnitudes;

  int numBins = fftSize / 2;
  int numFrames = 0;

  // 1. Normalize everything between values [-1.0, 1.0]
  juce::AudioBuffer<float> workingBuffer = fullAudioFile;

  // 2. Optional amplitude normalization (HPCP only, NOT for DeepChroma).
  //    madmom's SignalProcessor uses norm=False by default. Normalizing before
  //    the FFT scales all magnitudes and breaks the model's input distribution.
  if (normalizeAudio) {
    normalizeVolume(workingBuffer);
  }

  // Mix left and right channel to one mono channel to save computing time
  // analyzing it
  if (workingBuffer.getNumChannels() > 1) {
    workingBuffer.addFrom(0, 0, workingBuffer, 1, 0,
                          workingBuffer.getNumSamples());
    // Devide the gain by 2 since 2 channels were mixed together
    workingBuffer.applyGain(0, 0, workingBuffer.getNumSamples(), 0.5f);
  }

  // 3. Optional center-origin padding (DeepChroma only, matches madmom).
  //    madmom's FramedSignalProcessor centers the first frame at sample 0,
  //    equivalent to prepending fftSize/2 zeros. We also append fftSize/2
  //    zeros so the last frame can extend cleanly beyond the signal end.
  juce::AudioBuffer<float> processBuffer;
  if (centerOriginPadding) {
    int padding = fftSize / 2;
    int paddedSamples = workingBuffer.getNumSamples() + 2 * padding;
    processBuffer.setSize(1, paddedSamples);
    processBuffer.copyFrom(0, padding, workingBuffer, 0, 0,
                           workingBuffer.getNumSamples());
  } else {
    processBuffer = workingBuffer;
  }

  const float *monoData = processBuffer.getReadPointer(0);
  const int totalSamples = processBuffer.getNumSamples();
  const int originalSamples = workingBuffer.getNumSamples();

  numFrames = 0;
  if (centerOriginPadding) {
    if (originalSamples > 0) {
      numFrames = 1 + (originalSamples - 1) / hopSize;
    }
  } else {
    if (totalSamples >= fftSize) {
      numFrames = 1 + (totalSamples - fftSize) / hopSize;
    }
  }

  juce::AudioBuffer<float> spectogram(numFrames, numBins);
  std::vector<int> frameIndices(numFrames);
  std::iota(frameIndices.begin(), frameIndices.end(), 0);

  auto startTime = std::chrono::high_resolution_clock::now();

  // 2. Short-Time Fourier Transform with Hop Size
  // Hopping prevents samples to get "lost" due to the windowing function
  // for further details see docs/DSP
  std::for_each(std::execution::par, frameIndices.begin(), frameIndices.end(),
                [&](int i) {
                  int startIdx = i * hopSize;

                  auto magnitudes = processSingleFrame(monoData + startIdx);

                  juce::FloatVectorOperations::copy(
                      spectogram.getWritePointer(i), magnitudes.data(),
                      numBins);
                });

  // old code single threaded calculation, but takes 3,56 times the duration
  // int frameIndex = 0;
  // for (int startIdx = 0;
  //      startIdx + fftSize <= totalSamples && frameIndex < numFrames;
  //      startIdx += hopSize) {
  //   // Chunk audio into frames
  //   frameMagnitudes = processSingleFrame(monoData + startIdx);
  //
  //   juce::FloatVectorOperations::copy(spectogram.getWritePointer(frameIndex),
  //                                     frameMagnitudes.data(), numBins);
  //   frameIndex++;
  // }
  auto endTime = std::chrono::high_resolution_clock::now();
  auto durationSeconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime)
          .count();

  std::cout << "Duration of spectogram calculation: " << durationSeconds
            << " ms" << std::endl;

  return spectogram;
}

/**
 * @brief Normalizes the volume of the given audio buffer.
 *
 * This method calculates the maximum magnitude across all samples in the buffer
 * and applies a corresponding gain to scale the highest magnitude to 1.0 (or
 * -1.0).
 *
 * @param buffer A reference to the juce::AudioBuffer<float> to be normalized.
 */
void SpectogramAnalyzer::normalizeVolume(
    juce::AudioBuffer<float> &buffer) const {
  float maxMagnitude = buffer.getMagnitude(0, buffer.getNumSamples());

  // This applies gain so that the value with the highest magnitude will always
  // be 1.0 (or -1.0)
  if (maxMagnitude > 0.0f) {
    buffer.applyGain(1.0f / maxMagnitude);
  }
}

/**
 * @brief Processes a single frame of audio data and calculates its frequency
 * magnitudes.
 *
 * This method applies a windowing function to the input frame to prevent
 * spectral leakage, performs a Forward Fast Fourier Transform (FFT) to convert
 * the time-domain signal into the frequency domain, and computes the magnitude
 * of each frequency bin in decibels. Due to the Nyquist theorem, only the first
 * half of the FFT bins (up to the Nyquist frequency) are included in the
 * result.
 *
 * @param frameData A pointer to the array of floating-point audio samples for
 * the frame. The array must contain at least `fftSize` elements.
 * @return          A std::vector containing the frequency magnitudes in
 * decibels for the first half of the FFT bins.
 */
std::vector<float>
SpectogramAnalyzer::processSingleFrame(const float *frameData) const {
  // Thread local instances of fft and window
  thread_local juce::dsp::FFT threadFFT(fftOrder);
  thread_local juce::dsp::WindowingFunction<float> threadWindow(
      static_cast<size_t>(fftSize), juce::dsp::WindowingFunction<float>::hann,
      false);

  // Since the result of FFT is an array of complex numbers we need fftSize * 2
  // for real part and imaginary part
  std::vector<float> fftBuffer(fftSize * 2, 0.0f);

  // 1. Copy data to fftBuffer
  std::copy(frameData, frameData + fftSize, fftBuffer.begin());

  // 2. apply hann windowing function to prevent spectral leakage (see docs/DSP
  // for further details)
  threadWindow.multiplyWithWindowingTable(fftBuffer.data(), fftSize);

  // 3. Apply Forward FFT
  threadFFT.performFrequencyOnlyForwardTransform(fftBuffer.data());

  // 4. According to Nyquist sample theorem we only need the first half of the
  // frequencies (see docs/DSP for further details)
  int numBins = fftSize / 2;
  std::vector<float> magnitudes(numBins);

  for (int i = 0; i < numBins; i++) {
    // For HPCP we use Decibels (log scale)
    if (convertToDecibels) {
      magnitudes[i] = juce::Decibels::gainToDecibels(fftBuffer[i], -100.0f);
    }
    // For DeepLearning we cannot convert to decibels
    else {
      magnitudes[i] = fftBuffer[i];
    }
  }

  return magnitudes;
}
