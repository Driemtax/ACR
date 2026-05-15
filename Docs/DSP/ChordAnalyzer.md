# ChordAnalyzer
## Goal
This class acts as the central orchestrator for the entire chord recognition pipeline using the facade pattern. It manages the data flow between reading the audio file, generating the spectrogram, extracting the chromagram, and ultimately performing the chord classification. 

## runAnalysis
This function executes the complete processing pipeline sequentially. It handles the loading of the audio file using JUCE's `AudioFormatManager` and coordinates the individual DSP modules. 

The pipeline consists of the following steps:
1. **Audio Decoding**: The audio file is loaded and decoded into an internal floating-point buffer.
2. **Spectrogram Generation**: The audio buffer is passed to a `SpectrogramAnalyzer` (not shown in this file, but managed here) which computes the Short-Time Fourier Transform (STFT), resulting in a matrix of magnitude frames.
3. **Data Conversion**: The 2D vector array from the spectrogram is converted into a `juce::AudioBuffer` for more efficient memory access during the chroma phase.
4. **Chroma Extraction**: The spectrogram buffer is fed into the `ChromaAnalyzer`, which applies HPCP logic, normalization, and temporal median filtering to produce the chromagram.
5. **Classification**: The resulting chromagram is passed to the `Classificator`, which runs the cosine similarity pattern matching to output both raw frame-by-frame classifications and grouped chord segments.
6. **Result Aggregation**: All intermediate datasets (Spectrogram, Chromagram, Raw Classifications, Segmented Chords) along with vital metadata (Sample Rate, Hop Size) are packaged into an `AnalysisResult` struct. This allows the UI or testing frameworks to inspect any step of the pipeline.
