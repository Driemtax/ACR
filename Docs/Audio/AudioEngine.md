# AudioEngine
## Goal
The Purpose of this Class is to handle Audio input and output, therefore recording and playback of audio data. For now this contains recording Audio and saving it as a wav file. They player can only play wav files for now. The AudioEngine also handles communication with audio devices and the sound driver of the OS.

## Members
The members of this class mostly encapsulate information necessary for IO handling and device communicaten. The device manager, for example, uses the functionallity of the JUCE Framework to manage the state of the audio IO devices. The purpose is to not worry about how the communication between device, operating system and application works. We can use the JUCE Framework to do the work for us so we can concentrate on process the audio data.

## TransportState
This enum reflects the state of the AudioEngine. It is needed in a number of places of this application in order to decide, what to do. There are three states neccessary:
- Stopped: The application does not record or play audio and is therefore ready to do so
- Recording: The application is currently recording audio, which disables the play button
- Playing: The application is currently playing audio, which disables the record button

## audioDeviceIOCallbackWithContext
This Method is a Callback, which will be called by the operating system very frequently. The operating system gives an area of the ram to the application for writing data directly to the soundcard of the pc and read from it. The `inputChannelData` represents the incoming stream of data from the soundcard, the `outputChannelData` represents the area of ram we can write raw audio data to. So this callback is the direct communication point between application and operating system. 

The documentation of the JUCE Framework states that the `outputChannelData` values will be undefined. It needs to be filled with zeros in order to represent silence. If this is not done by us, then there will be a very unpleasant noise. Most importantly this function needs to return quickly so no blocking operations as gui calls, networking calls or string manipulation should take place in this function.

The `transportSource` is a manager holding audio data, which should be played. Everytime the OS calls the callback the `transportSource` decides wether the soundcard should be filled with zeros or the audio data it holds.
