# SpectogramAnalyzer
## Goal

## ProcessFullFile
This function has the goal of producing a spectogram for a given audiofile. It abstracs everything under the hood and therefore makes it easy to use the ChordAnalyser class in the application.

A spectogram is a 2d-vector of floating values. The first layer holds the information about the time (as a single frame of audio, a chunk of the original audio) and therefore will be displayed on the x-Axis of the visualization. The second layer, the inner vector, holds the frequencies. The value of the frequencies represent the magnitude in decibels, with which this frequency is present in the current audio frame.

Firstly we copy the audio data into a `workingBuffer`. This is simply to prevent modifying the original buffer, which we still want to be able to play to the user. Then the whole buffer gets normalized in volume, therefore in magnitude. We then mix together both the left and the right channel into one `monoChannel`. We don't need the information of both channels and can safe us 50% of the computing time. Mixing the channels simply means adding box channels together and then dividing by 2 to get an average of both. This happens for every sample in the buffer. This step is important to prevent the volume to have an effect of the chord classification. It must not matter if a C-Major chord is played loudly or not. 

Further we only use a read-only pointer to the `workingBuffer`. We then use the concept of Short-Time Fourier Transform (STFT). Since the windowing function smoothes out the edges of the frame we analyze, there is a risk of loosing some important samples, that hold valuable information. The hopping has the effect, that frame overlap each other preventing that loss of information. The first frame will e.g. hold samples 0 to 2047, the second frame holds samples 1024 up to 3071 etc. For every frame we call the `processSingleFrame()` function, which returns the frequency bins with the magnitudes of this frame. This gets added to the spectogram, which eventually will hold this information for the whole audio file, that was passed into this function.

The spectogram can be used to visualize it to the user or further process the data.

## ProcessSingleFrame
This function processes a single audio frame of audioData. It returns a vector of floating values holding the magintudes of every frequency bin. Its argument is a pointer to the first value of the frame, therefore an array of floats. 

The FFT returns complex numbers, therefore holding 2 values per bin. Therefore we need to create a buffer that holds space double the size of the fft itself. One value represents the real part of the complex number, the oder the imaginary part of the number. 

We then apply the window function to our buffer, holding our frame of audio data. The windowing function smoothes out the edges of the frame to insure a periodic behavior, if we repeat the frame indefinitly. Why this is necessary is a fact only known to those dark wizards fully understanding the Fourier Transform, which is obsiously not myself. After applying the window function we use the build in functions of JUCE to perform the FFT. Forwards means, that we translate the signal from time-domain to frequency-domain. JUCE removes the phase for us, which we do not need and only returns the magnitudes of the frequency bins to us. 

Lastly we convert those magnitudes to decibles, since those are logharithmiclly scaled. This insures, that the overtones are clearly visible in the spectogram.

## NormalizeVolume
This function simply scales the signal so, that the sample with the highest magnitude will be 1.0 or -1.0. Everything else will be scaled in this range.
