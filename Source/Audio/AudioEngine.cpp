#include "AudioEngine.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_core/juce_core.h"
#include "juce_core/system/juce_PlatformDefs.h"
#include "onnxruntime_c_api.h"
#include "onnxruntime_cxx_api.h"
#include <cstddef>
#include <memory>

const String appDirName = "ACR_App";

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

    currentFile = getDefaultRecordingFile();

    if (currentFile.existsAsFile()) {
        thumbnail.setSource(new juce::FileInputSource(currentFile));
    }

    // ML Testing
    loadModel();
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

/**
 * @brief Callback method for processing audio input and output data.
 *
 * This method is called repeatedly by the audio device to process a block of audio data.
 * If a recording is currently active, it writes the incoming audio data to a file and updates
 * the audio thumbnail. If playback is active, it fills the output buffers with audio data
 * from the transport source; otherwise, it clears the output buffers to prevent noise.
 *
 * CARE: No blocking operation should take place in this function. This is a callback, which is called by the operation system repeatetly.
 *
 * @param inputChannelData  An array of pointers to the incoming audio data for each input channel.
 * @param numInputChannels  The number of available input channels.
 * @param outputChannelData An array of pointers to the outgoing audio data for each output channel.
 * @param numOutputChannels The number of available output channels.
 * @param numSamples        The number of samples to process in this block.
 * @param context           Additional context information regarding the callback.
 */
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

/**
 * @brief Starts the audio recording process.
 *
 * This method initializes the recording by creating a new audio file and setting up
 * the necessary audio format writers. It prepares the audio thumbnail to visualize
 * the incoming audio data and updates the internal transport state to indicate that
 * a recording is in progress. If the transport state is not currently stopped,
 * this method returns immediately without doing anything.
 */
void AudioEngine::startRecording()
{
    if (state != TransportState::Stopped) return;

    currentFile = getDefaultRecordingFile();
    currentFile.deleteFile();

    if (auto fileStream = std::unique_ptr<juce::FileOutputStream> (currentFile.createOutputStream())) {
        juce::WavAudioFormat wavFormat;
        if (auto* device = deviceManager.getCurrentAudioDevice()) {
            double sampleRate = device->getCurrentSampleRate();
            if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 2, 16, {}, 0)) {
                fileStream.release();
                threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));
                activeWriter.store(threadedWriter.get());

                // reset AudioThumbnail
                thumbnail.setSource(nullptr); // removes references to files
                thumbnail.clear(); // clears all pixels in cache
                thumbnail.reset(2, sampleRate, 0); // prepares for new recording
                samplesRecorded = 0;

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

/**
 * @brief Starts the audio playback process.
 *
 * This method initiates playback by reading the current audio file and configuring
 * the necessary audio format reader. It also updates the audio thumbnail to visualize
 * the file being played and sets the internal transport state to indicate that
 * playback is active. If the file being played is the default recording, some gain
 * is applied to compensate for lower recording levels. If the transport state is
 * not currently stopped, this method returns immediately without doing anything.
 */
void AudioEngine::startPlayback()
{
    if (state != TransportState::Stopped) return;

    auto file = getAudioFilePath();

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

            // recordings are quit quiet so we add some gain
            if (file == getDefaultRecordingFile()) {
                transportSource.setGain(6.0f);
            }

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


// Gets the file for recording and playback. The filepath is defined beforehand using constants
juce::File AudioEngine::getDefaultRecordingFile() const {
    auto docsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto appDir = docsDir.getChildFile(appDirName);

    if (!appDir.exists()) {
        appDir.createDirectory();
    }


    return appDir.getChildFile("recording.wav");
}

juce::File AudioEngine::getAudioFilePath() const {
    return currentFile;
}

void AudioEngine::setFilename(juce::String name) {
    filename = name;
}

void AudioEngine::setAudioFile(const juce::File& file) {
    if (file.existsAsFile()) {
        currentFile = file;
        thumbnail.setSource(new juce::FileInputSource(currentFile));
    }
}

void AudioEngine::loadModel() {
    try {
    Ort::Env ortEnv;
    std::unique_ptr<Ort::Session> ortSession;

    ortEnv = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "DeepChromaEnv");

    // options
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    const wchar_t* modelPath = L"C:\\Test\\ML\\deep_chroma.onnx";

    // import model to ram
    ortSession = std::make_unique<Ort::Session>(ortEnv, modelPath, sessionOptions);

    std::cout << "ERFOLG: ONNX Model loaded successfully into ram." << std::endl;

    // Debug verification
    size_t numInputNodes = ortSession->GetInputCount();
    std::cout << "Count of input nodes: " << numInputNodes << std::endl;

    // Wir holen uns die Typ-Informationen des ersten (und einzigen) Inputs (Index 0)
    Ort::TypeInfo inputTypeInfo = ortSession->GetInputTypeInfo(0);
    auto tensorInfo = inputTypeInfo.GetTensorTypeAndShapeInfo();

    // Wir fragen das Array nach seinen Dimensionen
    std::vector<int64_t> inputDims = tensorInfo.GetShape();

    std::cout << "Shape der Input Node 0: " << std::endl;
    for (size_t i = 0; i < inputDims.size(); i++)
    {
        std::cout << "Dimension " << i << ": " << inputDims[i] << std::endl;
    }
    } catch (const Ort::Exception &e) {
        std::cout << "ERROR loading the ONNX model: " << e.what();
    }
}
