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

Classificator::Classificator(float similarityThreshold) : similarityThreshold(similarityThreshold) {
    chordNamesMap = std::vector<juce::String>(templateCount);

    templates = std::vector<std::vector<float>>(chordNamesMap.capacity());
    generateTemplates();

}

/**
 * Classifies an entire sequence of chroma frames.
 *
 * This function iterates over all frames in the provided chroma buffer,
 * classifies each frame individually using the internal template vectors, and stores
 * the resulting classification indices in the provided result vector.
 *
 * @param chroma  A buffer containing the chroma features to classify. Each channel
 *                represents a single frame containing chroma data.
 * @param result  A vector where the classification indices for each frame will be stored.
 *                The size of this vector should be at least equal to the number of
 *                channels in the chroma buffer.
 */
void Classificator::classifyFullChroma(const juce::AudioBuffer<float> &chroma, std::vector<int> &result) const {
    for (int i = 0; i < chroma.getNumChannels(); i++) {
        const float* currentFrame = chroma.getReadPointer(i);

        int classification = classifyFrame(currentFrame);
        result[i] = classification;
    }
}

/**
 * Classifies a single chroma frame by comparing it to all template vectors.
 *
 * This function uses cosine similarity to compare the provided frame
 * against all generated chord templates. It returns the index of the template
 * with the highest similarity score. If the highest similarity score is below
 * the configured similarity threshold, the classification is considered invalid.
 * For more details on cosine similarity please refer to Docs/
 *
 * @param frame  A pointer to a float array representing the chroma features of a single frame.
 *               The array size must match the number of features in the template vectors (typically 12).
 * @return       The index of the highest matching template vector, or -1 if the best match
 *               falls below the similarity threshold.
 */
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

/**
 * Generates the template vectors for major, minor, and power chords.
 *
 * This function initializes and populates the internal template vectors and
 * chord names map. It creates 12-dimensional chroma feature vectors for
 * all 12 root notes, producing templates for Major, Minor, and Power chords.
 * The generated templates are used later for frame classification.
 */
void Classificator::generateTemplates() {
    chordNamesMap.clear();
    templates.clear();

    chordNamesMap.reserve(templateCount);
    templates.reserve(templateCount);

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

/**
 * Calculates the cosine similarity between a template vector and a chroma frame.
 *
 * This function computes the cosine similarity score, which is a measure of similarity
 * between two non-zero vectors of an inner product space. The resulting value ranges
 * from 0.0 (no similarity) to 1.0 (identical direction).
 *
 * @param templateFeatures  A vector containing the features of the chord template.
 * @param frame             A pointer to a float array representing the features of a single chroma frame.
 * @return                  The cosine similarity score between the two input vectors, ranging from 0.0 to 1.0.
 */
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
