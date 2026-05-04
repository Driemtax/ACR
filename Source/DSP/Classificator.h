#pragma once

#include <JuceHeader.h>
#include <vector>

// Classifies a musical chord based on a 12-dimensional feature vector.
// Binary Template vectors are matched using the cosine similarity
class Classificator {
    public:
    Classificator();
    ~Classificator() = default;

    // Takes a 12-dimensional vector as an input and returns the index of the chord.
    // Returns -1 if no template is similiar enough to feature vector
    int classifyFrame(const float* frame) const;

    // Returns the name of the chord corresponding to the given Index or "".
    juce::String getChordName(int index) const;

    private:
    void generateTemplates();
    float calculateCosineSimilarity(const std::vector<float>& templateFeatures, const float* frame) const;

    std::vector<std::vector<float>> templates;
    std::vector<juce::String> chordNamesMap;
    const float similarityThreshold = 0.3f;

};
