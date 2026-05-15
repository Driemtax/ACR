# ChromaAnalyzer
## Goal
This class holds functions for processing spectogram data. It produces a chromagram representing the musical notes played in the audio frames.

## processFullSpectogram
This function orchestrates the complete HPCP (Harmonic Pitch Class Profile) calculation pipeline. It processes the input spectrogram frame by frame to calculate the raw chroma values, normalizes them globally to account for varying audio volumes, and optionally applies temporal smoothing. 

## processFrame
Calculates the HPCP for a single audio frame based on its spectral peaks. Instead of using all FFT bins, the algorithm relies on previously extracted local maxima. The energy of these peaks is distributed across the chroma bins. To account for the overtone structures of real instruments, the algorithm not only considers the fundamental frequency of a peak but also up to 8 of its harmonics, mapping their energies into the circular pitch space.

## extractPeaks
Extracts local maxima from the spectrogram frame to isolate meaningful pitch information and reduce noise. Only frequencies roughly between 100 Hz and 5000 Hz are considered. This range avoids low-frequency rumble and high-frequency noise, which typically do not contribute meaningfully to pitch or chord classification. A dynamic magnitude threshold ensures that insignificant noise in otherwise quiet frames is ignored.

## calculateBinMids
Calculates the center frequencies for all HPCP bins based on a reference frequency (`f_ref`). Since human pitch perception is logarithmic, the center frequencies are spaced logarithmically (using powers of 2) across the spectrum.

## calculateDistance
Computes the shortest distance in semitones between a given frequency and a bin's center frequency. Because the chroma space is circular (due to octave equivalence), distances are wrapped to a range of $[-6, 6]$ semitones. This ensures that a frequency slightly above a note in one octave and slightly below the same note in the next octave are mapped correctly.

## calculateWeightFreq
Calculates how much of a frequency's energy should be mapped to a specific bin based on their distance in semitones. It uses a cosine-squared window, meaning that frequencies exactly at the bin center receive the highest weight, while the weight smoothly tapers off to zero as the distance increases. 

## calculateHarmonicWeights
Computes an exponential decay weight for each harmonic based on a spectral shape parameter `s`. This models the natural behavior of acoustic instruments, where higher harmonics generally have less energy than the fundamental frequency, preventing higher overtones from skewing the chromagram.

## normalizeBins
This function normalizes the values of the HPCP bins to the range $[0,1]$. For this, the loudest value per frame is determined and all other values are divided by it.

Additionally, a `noiseGate` is defined, which is a threshold that the maximum value of the frame must exceed. If the maximum falls below this threshold, all bins in this frame are set to $0.0$, as it is assumed to be noise or other interference. The `noiseGate` is calculated dynamically and amounts to 10% of the global maximum of the chromagram.

## applyMedianFilter
This function applies temporal smoothing to the chromagram by applying a so-called median filter to it.

The goal is to remove noise, interference, etc., from the chromagram to make the subsequent classification more robust. This works well because the median inherently ignores statistical outliers. Interference noise usually consists of frequencies that are only present in the audio signal for a short timeframe, frequently during a chord change (due to fingering noise or the striking of a string).

It is important not to modify the chromagram in-place initially, otherwise the calculation of frame $x$ would influence the calculation of frame $x+1$ and so on, which is not desired. To apply the median filter, the left and right neighbors are examined for each frame and each bin, and the median of these elements is calculated to produce the new value for the current frame. How many frames to the left and right are considered is determined by the window size, which can be configured in the application.

The `std::nth_element()` function offers an optimization here, as it doesn't sort all elements in the window array; instead, it only partially sorts them to ensure the median element is in the correct position. Since this function is called per frame, this is advantageous even for relatively small arrays.
