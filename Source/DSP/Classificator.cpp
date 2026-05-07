#include "Classificator.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <cmath>
#include <ostream>
#include <vector>

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "[";
       for (size_t i = 0; i < v.size(); ++i) {
           os << v[i];
           if (i != v.size() - 1) os << ", ";
       }
       os << "]";
       return os;
}

Classificator::Classificator() {
    chordNamesMap = std::vector<juce::String>(36);

    templates = std::vector<std::vector<float>>(chordNamesMap.capacity());
    generateTemplates();

}

void Classificator::classifyFullChroma(const juce::AudioBuffer<float> &chroma, std::vector<int> &result) const {
    for (int i = 0; i < chroma.getNumChannels(); i++) {
        const float* currentFrame = chroma.getReadPointer(i);

        int classification = classifyFrame(currentFrame);
        result[i] = classification;
    }
}

// classifies one frame comparing it to all template vectors. For comparison this function uses cosine similarity.
int Classificator::classifyFrame(const float* frame) const {
    int highestSimilarityIndex = -1;
    float highestSimilarity = 0.0f;
    float currentSimilarity = 0.0f;

    for (int i = 0; i < templates.size(); i++) {
        currentSimilarity = calculateCosineSimilarity(templates[i], frame);

        if (currentSimilarity > highestSimilarity) {
            highestSimilarity = currentSimilarity;
            highestSimilarityIndex = i;
        }
    }

    // If the nearets template vector is too far away for a classification, then we just return -1, indicating there is no template matching the current frame.
    if (highestSimilarity < similarityThreshold) {
        highestSimilarityIndex = -1;
    }

    return highestSimilarityIndex;
}

juce::String Classificator::getChordName(int index) const {
    juce::String name;
    if (index < 0) {
        name = "";
        return name;
    }

    name = chordNamesMap.at(index);

    return name;
}

/**
 * Groups consecutive frames with the same classification into chord segments.
 *
 * This function iterates through a sequence of frame classifications and merges
 * consecutive identical classifications into a single ChordSegment. To avoid
 * rapid fluctuations, a minimum segment length can be specified. A new chord
 * must be present for at least `minSegmentLength` consecutive frames before
 * it is considered a valid segment.
 *
 * @param frameResults      A vector containing the classification index for each frame.
 * @param minSegmentLength  The minimum number of consecutive frames required to form a new segment.
 * @return                  A vector of ChordSegment objects representing the grouped chords.
 */
std::vector<Classificator::ChordSegment> Classificator::getGroupedSegments(const std::vector<int> &frameResults, int minSegmentLength) const {
    std::vector<ChordSegment> segments;
    if (frameResults.empty()) return segments;

    int currentChord = frameResults[0];
    int currentStart = 0;

    int candidateChord = -1;
    int candidateStart = -1;

    for (int i = 1; i < frameResults.size(); i++) {
        if (frameResults[i] != currentChord) {
            if (candidateChord == -1 || frameResults[i] != candidateChord) {
                // new candidate
                candidateChord = frameResults[i];
                candidateStart = i;
            }

            // Check if candidate is present long enough to become current chord.
            int candidateDurateion = i - candidateStart + 1;
            if (candidateDurateion >= minSegmentLength) {
                segments.push_back({currentStart, candidateStart - 1, getChordName(currentChord)});

                currentChord = candidateChord;
                currentStart = candidateStart;
                candidateChord = -1;
            }
        } else {
            candidateChord = -1;
        }
    }

    segments.push_back({currentStart, (int)frameResults.size() - 1, getChordName(currentChord)});

    return segments;
}

// generates template vectors for major, minor and powerchords.
void Classificator::generateTemplates() {
    chordNamesMap.clear();
    templates.clear();

    chordNamesMap.reserve(36);
    templates.reserve(36);

    const char* noteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    juce::String name;
    std::vector<float> features;

    for (int root = 0; root < 12; root++) {
        // 1. Major
        features = std::vector<float>(12, 0.0f);
        name = juce::String(noteNames[root]) + " Maj";
        features[root] = 1.0f; // root
        features[(root + 4) % 12] = 1.0f; // major third
        features[(root + 7) % 12] = 1.0f; // perfect fifth

        chordNamesMap.push_back(name);
        templates.push_back(features);

        // 2. Minor
        features = std::vector<float>(12, 0.0f);
        name = juce::String(noteNames[root]) + " Min";
        features[root] = 1.0f; // root
        features[(root + 3) % 12] = 1.0f; // minor third
        features[(root + 7) % 12] = 1.0f; // perfect fifth

        chordNamesMap.push_back(name);
        templates.push_back(features);

        // 3. Powerchords
        features = std::vector<float>(12, 0.0f);
        name = juce::String(noteNames[root]) + "5";
        features[root] = 1.0f; // root
        features[(root + 7) % 12] = 1.0f; // perfect fifth

        chordNamesMap.push_back(name);
        templates.push_back(features);
    }

}

// calculates the cosine similarity for two vectors. Values will be between 0 (no similarity) and 1 (full similarity).
float Classificator::calculateCosineSimilarity(const std::vector<float> &templateFeatures, const float *frame) const {
    float sumAB = 0.0f;
    float sumASquared = 0.0f;
    float sumBSquared = 0.0f;

    int n = templateFeatures.size();

    for (int i = 0; i < n; i++) {
        sumAB += templateFeatures[i] * frame[i];
        sumASquared += templateFeatures[i] * templateFeatures[i];
        sumBSquared += frame[i] * frame[i];
    }

    // If one of the sums in the denominator is 0, then the root will be zero and the product will be zero aswell. Return 0.0f here to avoid division by zero!
    if (sumASquared == 0.0f || sumBSquared == 0.0f) {
        return 0.0f;
    }

    float sumARooted = sqrtf(sumASquared);
    float sumBRooted = sqrtf(sumBSquared);
    float ABSquared = sumARooted * sumBRooted;

    // for safety reason just return 0 if denominator is to small.
    if (ABSquared < 0.000001f) {
        return 0.0f;
    }

    float similarity = sumAB / ABSquared;

    return similarity;
}
