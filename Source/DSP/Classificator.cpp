#include "Classificator.h"
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

int Classificator::classifyFrame(const float* frame) const {
    return -1;
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

float Classificator::calculateCosineSimilarity(const std::vector<float> &templateFeatures, const float *frame) const {
    return 0.0f;
}
