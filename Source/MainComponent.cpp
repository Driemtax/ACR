#include "MainComponent.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include "juce_audio_utils/juce_audio_utils.h"
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include "juce_gui_basics/juce_gui_basics.h"
#include <memory>

const int WIDTH = 800;
const int HEIGHT = 600;

//==============================================================================
// Parameters of audioSetupComp: minInput, maxInput, minOutput, maxOutput, showMidiIn, showMidiOut, showStereo, hideAdvancedOptions
MainComponent::MainComponent() : audioSetupComp(deviceManager, 0, 2, 0, 2, false, false, true, true)
{
    // Initialize and show Audio Device Selector dropdown
    addAndMakeVisible(audioSetupComp);

    // Add Buttons for playing and recording audio
    addAndMakeVisible(recordButton);
    addAndMakeVisible(playButton);

    // click functions for buttons
    recordButton.onClick = [this] {
      if (state == TransportState::Stopped) {
          // create temp file in documents folder
          auto file = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
              .getChildFile("Test_JUCE_Aufnahme.wav");
          file.deleteFile();

          recordButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);

          // create wav writer for this file
          if (auto fileStream = std::unique_ptr<juce::FileOutputStream> (file.createOutputStream())) {
              juce::WavAudioFormat wavFormat;
              double sampleRate = deviceManager.getAudioDeviceSetup().sampleRate;

              if (auto writer = wavFormat.createWriterFor(fileStream.get(), sampleRate, 2, 16, {}, 0)) {
                  // give filestream to writer thread
                  fileStream.release();

                  threadedWriter.reset(new juce::AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));
                  activeWriter.store(threadedWriter.get());
                  state = TransportState::Recording;
                  recordButton.setButtonText("Stop");
                  recordButton.buttonColourId;
              }
          }
      }  else if (state == TransportState::Recording) {
          activeWriter.store(nullptr);
          threadedWriter.reset();

          state = TransportState::Stopped;
          recordButton.setButtonText("Record");
      }
    };

    playButton.onClick = [this] {
      if (state == TransportState::Stopped) {
          state = TransportState::Playing;
          playButton.setButtonText("Stop");
          // TODO start playback
          juce::Logger::writeToLog("Started playback...");
      } else if (state == TransportState::Playing) {
          state = TransportState::Stopped;
          playButton.setButtonText("Play");
          // TODO stop playback
          juce::Logger::writeToLog("Playback stopped.");
      }
    };


    // Make sure you set the size of the component after
    // you add any child components.
    setSize (WIDTH, HEIGHT);

    formatManager.registerBasicFormats();
    backgroundThread.startThread();

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }


}

MainComponent::~MainComponent()
{
    backgroundThread.stopThread(1000);
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto *writer = activeWriter.load();

    if (writer != nullptr) {
        writer->write(bufferToFill.buffer->getArrayOfReadPointers(), bufferToFill.numSamples);
    }

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    // Audio Device Selector on top
    audioSetupComp.setBounds(10, 10, getWidth() - 20, 200);

    // Play and Record Buttons
    recordButton.setBounds(10, 220, 120, 40);
    playButton.setBounds(140, 220, 120, 40);
}
