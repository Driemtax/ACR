#include "ChordAnalyzer.h"
#include "Classificator.h"
#include "SpectogramAnalyzer.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include <algorithm>
#include <memory>

ChordAnalyzer::ChordAnalyzer() : classifier(0.8f) {}

ChordAnalyzer::ChordAnalyzer(Test::TestConfig &config)
    : classifier(config.similarityThreshold),
    spectoAnalyzer(config),
    fftOrder(config.fftOrder),
    fftSize(config.fftSize),
    hopSize(config.hopSize),
    medianFilter(config.medianFilter),
    medianWindowSize(config.medianWindowSize),
    s(config.s),
    similarityThreshold(config.similarityThreshold),
    chromaRes(config.chromaRes)
    {}

/**
 * Runs the complete chord analysis process on a given audio file.
 *
 * This method reads the audio file, computes its spectrogram, extracts
 * the chromagram, and classifies the resulting data into chord segments.
 *
 * @param audioFile The audio file to be analyzed.
 * @return An AnalysisResult object containing the computed spectrogram,
 * chromagram, and chord segments.
 */
ChordAnalyzer::AnalysisResult
ChordAnalyzer::runAnalysis(const juce::File &audioFile) {
  AnalysisResult result;

  // 1. Read in audio file
  juce::AudioFormatManager formatManager;
  formatManager.registerBasicFormats();

  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(audioFile));
  if (reader == nullptr)
    return result;

  juce::AudioBuffer<float> buffer((int)reader->numChannels,
                                  (int)reader->lengthInSamples);
  reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);

  // 2. Create Spectogram
  auto spectogramData =
      spectoAnalyzer.processFullFile(buffer, reader->sampleRate);

  int numFrames = (int)spectogramData.getNumChannels();
  int numBins =
      spectogramData.hasBeenCleared() ? 0 : (int)spectogramData.getNumSamples();

  std::cout << "=== SPEKTOGRAMM BERECHNET ===" << std::endl;
  std::cout << "Anzahl Frames (Zeit): " << numFrames
            << std::endl;
  if (!spectogramData.hasBeenCleared()) {
    std::cout << "Anzahl Bins (Frequenz): " << numBins
              << std::endl;
  }


  // Create Chromagram
  ChromaAnalyzer chromaAnalyzer =
      ChromaAnalyzer(reader->sampleRate, fftSize, s, chromaRes,
                     medianWindowSize, medianFilter);
  int chromaBins = chromaAnalyzer.getChromaBinSize();

  auto chromagram = juce::AudioBuffer<float>(numFrames, chromaBins);
  chromaAnalyzer.extractChroma(spectogramData, chromagram);

  // classify
  Classificator classifier = Classificator(similarityThreshold);
  std::vector<int> classifiedFrames;
  classifiedFrames.resize(chromagram.getNumChannels());
  classifier.classifyFullChroma(chromagram, classifiedFrames);
  std::vector<Classificator::ChordSegment> chordSegments =
      classifier.getGroupedSegments(classifiedFrames);

  // save all results
  result.spectogramData = std::move(spectogramData);
  result.chromagramData = std::move(chromagram);
  result.chordSegments = std::move(chordSegments);
  result.rawClassifications = std::move(classifiedFrames);
  result.sampleRate = spectoAnalyzer.getSampleRate();
  result.hopSize = spectoAnalyzer.getHopSize();

  return result;
}
