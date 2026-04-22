#pragma once

#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include <JuceHeader.h>
#include <atomic>
#include <memory>

class AudioEngine : public juce::AudioIODeviceCallback
{
public:
    enum class TransportState {
        Stopped,
        Recording,
        Playing
    };

    AudioEngine();
    ~AudioEngine() override;

    // --- AudioIODeviceCallback Overrides ---
    void audioDeviceAboutToStart (juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                float* const* outputChannelData, int numOutputChannels,
                                int numSamples,
                                const juce::AudioIODeviceCallbackContext& context) override;

    // --- Recording/Playback calls for GUI ---
    void startRecording();
    void stop();
    void startPlayback();

    juce::AudioTransportSource& getTransportSource() { return transportSource; }

    TransportState getState() const { return state; }
    juce::AudioDeviceManager& getDeviceManager() { return deviceManager; }

    juce::AudioThumbnail& getThumbnail() { return thumbnail; }
    double getLengthInSeconds() const;

    juce::File getDefaultRecordingFile() const;
    juce::File getAudioFilePath() const;

    void setFilename(juce::String name);
    void setAudioFile(const juce::File& file);

private:
    juce::AudioDeviceManager deviceManager;
    std::atomic<TransportState> state { TransportState::Stopped };

    // Recording ressources
    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread backgroundThread { "Audio Recorder Thread" };

    std::unique_ptr<juce::AudioFormatWriter::ThreadedWriter> threadedWriter;
    std::atomic<juce::AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };

    // Playback ressources
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    // Audio Thumbnail
    juce::AudioThumbnailCache thumbnailCache { 1 };
    juce::AudioThumbnail thumbnail { 512, formatManager, thumbnailCache };
    juce::int64 samplesRecorded = 0;

    // File
    juce::String filename = "recording.wav";
    juce::File currentFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
};
