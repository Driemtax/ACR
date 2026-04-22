#include "ChordAnalyzer.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "juce_dsp/juce_dsp.h"
#include <algorithm>
#include <vector>

ChordAnalyzer::ChordAnalyzer(int order)
    : fftOrder(order),
    fftSize(1 << order),
    fft(order) {}

    std::vector<std::vector<float>> ChordAnalyzer::processFullFile(const juce::AudioBuffer<float>& fullAudioFile, double sampleRate)
    {
        std::vector<std::vector<float>> spectogram;

        // 1. Normalize everything between values [-1.0, 1.0]
        juce::AudioBuffer<float> workingBuffer = fullAudioFile;
        normalizeVolume(workingBuffer);

        // Mix left and right channel to one mono channel to save computing time analyzing it
        if (workingBuffer.getNumChannels() > 1) {
            workingBuffer.addFrom(0, 0, workingBuffer, 1, 0, workingBuffer.getNumSamples());
            // Devide the gain by 2 since 2 channels were mixed together
            workingBuffer.applyGain(0,0, workingBuffer.getNumSamples(), 0.5f);
        }

        const float* monoData = workingBuffer.getReadPointer(0);
        const int totalSamples = workingBuffer.getNumSamples();

        // 2. Short-Time Fourier Transform with Hop Size 50%
        int hopSize = fftSize / 2;

        // Hopping prevents samples to get "lost" due to the windowing function
        // for further details see docs/DSP
        for (int startIdx = 0; startIdx + fftSize <= totalSamples; startIdx += hopSize) {
            // Chunk audio into frames
            std::vector<float> frameMagnitudes = processSingleFrame(monoData + startIdx);
            spectogram.push_back(std::move(frameMagnitudes));
        }

        juce::Logger::writeToLog("Hallo!");

        return spectogram;
    }

    void ChordAnalyzer::normalizeVolume(juce::AudioBuffer<float>& buffer)
    {
        float maxMagnitude = buffer.getMagnitude(0, buffer.getNumSamples());

        // This applies gain so that the value with the highest magnitude will always be 1.0 (or -1.0)
        if (maxMagnitude > 0.0f) {
            buffer.applyGain(1.0f / maxMagnitude);
        }
    }

    std::vector<float> ChordAnalyzer::processSingleFrame(const float* frameData)
    {
        // Since the result of FFT is an array of complex numbers we need fftSize * 2 for real part and imaginary part
        std::vector<float> fftBuffer(fftSize * 2, 0.0f);

        // 1. Copy data to fftBuffer
        std::copy(frameData, frameData + fftSize, fftBuffer.begin());

        // 2. apply hann windowing function to prevent spectral leakage (see docs/DSP for further details)
       window.multiplyWithWindowingTable(fftBuffer.data(), fftSize);

       // 3. Apply Forward FFT
       fft.performFrequencyOnlyForwardTransform(fftBuffer.data());

       // 4. According to Nyquist sample theorem we only need the first half of the frequencies (see docs/DSP for further details)
       int numBins = fftSize / 2;
       std::vector<float> magnitudes(numBins);

       for (int i = 0; i < numBins; i++) {
           magnitudes[i] = juce::Decibels::gainToDecibels(fftBuffer[i], -100.0f);
       }

       return magnitudes;
    }
