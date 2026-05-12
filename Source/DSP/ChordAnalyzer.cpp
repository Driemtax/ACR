#include "ChordAnalyzer.h"
#include "Classificator.h"
#include "juce_audio_formats/juce_audio_formats.h"
#include <memory>

ChordAnalyzer::ChordAnalyzer() : classifier(0.8f) {}

ChordAnalyzer::ChordAnalyzer(Test::TestConfig &config) : classifier(config.similarityThreshold) {
  if (!config.medianFilter) {
    medianFilter = false;
  }

  medianWindowSize = config.medianWindowSize;
  s = config.s;
  similarityThreshold = config.similarityThreshold;
  chromaRes = config.chromaRes;
}


/**
 * Runs the complete chord analysis process on a given audio file.
 *
 * This method reads the audio file, computes its spectrogram, extracts
 * the chromagram, and classifies the resulting data into chord segments.
 *
 * @param audioFile The audio file to be analyzed.
 * @return An AnalysisResult object containing the computed spectrogram, chromagram, and chord segments.
 */
ChordAnalyzer::AnalysisResult
ChordAnalyzer::runAnalysis(const juce::File &audioFile) {
  AnalysisResult result;

  juce::AudioFormatManager formatManager;
  formatManager.registerBasicFormats();

  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(audioFile));
  if (reader == nullptr)
    return result;

  juce::AudioBuffer<float> buffer((int)reader->numChannels,
                                  (int)reader->lengthInSamples);
  reader->read(&buffer, 0, (int)reader->lengthInSamples, 0, true, true);

  auto spectogramData =
      spectoAnalyzer.processFullFile(buffer, reader->sampleRate);

  std::cout << "=== SPEKTOGRAMM BERECHNET ===" << std::endl;
  std::cout << "Anzahl Frames (Zeit): " << spectogramData.size() << std::endl;
  if (!spectogramData.empty()) {
    std::cout << "Anzahl Bins (Frequenz): " << spectogramData[0].size()
              << std::endl;
  }

  int numFrames = (int)spectogramData.size();
  int numBins = spectogramData.empty() ? 0 : (int)spectogramData[0].size();

  juce::AudioBuffer<float> spectoBuffer(numFrames, numBins);

  for (int i = 0; i < numFrames; i++) {
    juce::FloatVectorOperations::copy(spectoBuffer.getWritePointer(i),
                                      spectogramData[i].data(), numBins);
  }

  float fftSize = (float)(numBins * 2);
  ChromaAnalyzer chromaAnalyzer = ChromaAnalyzer(reader->sampleRate, fftSize, s, chromaRes);
  int chromaBins = chromaAnalyzer.getChromaBinSize();

  auto chromagram = juce::AudioBuffer<float>(numFrames, chromaBins);
  chromaAnalyzer.processFullSpectogram(spectoBuffer, chromagram);

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
  result.sampleRate = spectoAnalyzer.getSampleRate();
  result.hopSize = spectoAnalyzer.getHopSize();

  return result;
}
