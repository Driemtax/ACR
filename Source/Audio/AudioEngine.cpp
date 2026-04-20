#include "AudioEngine.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_core/juce_core.h"
#include <memory>

AudioEngine::AudioEngine()
{
    formatManager.registerBasicFormats();
    backgroundThread.startThread();

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) {
                                               if (granted)
                                                   deviceManager.initialiseWithDefaultDevices (2, 2);
                                           });
    }
    else
    {
        deviceManager.initialiseWithDefaultDevices (2, 2);
    }

    // register callback for audio data
    deviceManager.addAudioCallback (this);

    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Test_JUCE_Aufnahme.wav");

    if (file.existsAsFile()) {
        thumbnail.setSource(new juce::FileInputSource(file));
    }
}

AudioEngine::~AudioEngine()
{
    // free all ressources
    deviceManager.removeAudioCallback (this);
    backgroundThread.stopThread(1000);
}

void AudioEngine::audioDeviceAboutToStart (juce::AudioIODevice* device) {
    // set sample rate and buffer size
    transportSource.prepareToPlay(device->getCurrentBufferSizeSamples(), device->getCurrentSampleRate());
}
void AudioEngine::audioDeviceStopped() {
    // free all ressources
    transportSource.releaseResources();
}

void AudioEngine::audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                            float* const* outputChannelData, int numOutputChannels,
                            int numSamples,
                            const juce::AudioIODeviceCallbackContext& context)
{
    auto writer = activeWriter.load();

    // --- RECORDING ---
    // If there is an input channel and we have a writer save the data to a wav file
    if (writer != nullptr && numInputChannels > 0) {
        writer->write (inputChannelData, numSamples);

        juce::AudioBuffer<float> inputBuffer(const_cast<float**>(inputChannelData), numInputChannels, numSamples);
        thumbnail.addBlock(samplesRecorded, inputBuffer, 0, numSamples);
        samplesRecorded += numSamples;
    }

    // --- PLAYBACK ---
    juce::AudioBuffer<float> outputBuffer (const_cast<float**>(outputChannelData), numOutputChannels, numSamples);

    // clear the outputBuffer first to remove all noise
    outputBuffer.clear();

    // if we want to play audio, the buffer gets filled with data from the transportSource
    if (state == TransportState::Playing) {
        juce::AudioSourceChannelInfo info (&outputBuffer, 0, numSamples);
        transportSource.getNextAudioBlock(info);
    }
}

void AudioEngine::startRecording()
{
    if (state != TransportState::Stopped) return;

    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                    .getChildFile("Test_JUCE_Aufnahme.wav");
    file.deleteFile();

    if (auto fileStream = std::unique_ptr<juce::FileOutputStream> (file.createOutputStream())) {
        juce::WavAudioFormat wavFormat;
        if (auto* device = deviceManager.getCurrentAudioDevice()) {
            double sampleRate = device->getCurrentSampleRate();
            if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 2, 16, {}, 0)) {
                fileStream.release();
                threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));
                activeWriter.store(threadedWriter.get());

                // reset AudioThumbnail
                samplesRecorded = 0;
                thumbnail.reset(2, sampleRate, 0);

                // update state of audioEngine
                state = TransportState::Recording;
            }
        }
    }
}

void AudioEngine::stop()
{
    activeWriter.store(nullptr);
    threadedWriter.reset();

   {
       juce::AudioIODevice *currentDevice = deviceManager.getCurrentAudioDevice();
       juce::ScopedLock audioLock (deviceManager.getAudioCallbackLock());

        if (state.load() == TransportState::Playing) {
            transportSource.stop();
            transportSource.setSource(nullptr);
        }

        state.store(TransportState::Stopped);
   }

}

void AudioEngine::startPlayback()
{
    if (state != TransportState::Stopped) return;

    auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Test_JUCE_Aufnahme.wav");

    if (file.existsAsFile()) {
        auto* reader = formatManager.createReaderFor(file);

        if (reader != nullptr) {
            // set file to thumbnail
            thumbnail.setSource(new juce::FileInputSource(file));


            // readerSource frees the reader at end of reading
            readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);

            // stream data to transportSource
            transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
            transportSource.setPosition(0.0); // start from beginning!
            transportSource.start();

            state.store(TransportState::Playing);
        } else {
            juce::Logger::writeToLog("File does not exist!");
        }
    }
}

double AudioEngine::getLengthInSeconds() const {
    return thumbnail.getTotalLength();
}
