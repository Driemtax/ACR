#include "Test.h"
#include "../DSP/ChordAnalyzer.h"
#include "juce_core/juce_core.h"
#include <cstddef>
#include <iostream>
#include <vector>

void Test::runTests(Test::TestConfig &config, const juce::File &testDataDirectory,
    const juce::File &outputDirectory) const {

        juce::DynamicObject::Ptr jsonRoot = new juce::DynamicObject();
        juce::Array<juce::var> allSongsArray;

        juce::DirectoryIterator iter(testDataDirectory, false, "*.wav");

        int globalCorrectFrames = 0;
        int globalTotalFrames = 0;

        while (iter.next()) {
            juce::File wavFile = iter.getFile();
            juce::String baseName = wavFile.getFileNameWithoutExtension();

            juce::String labelFileName = baseName + "_label.txt";
            juce::File labelFile = testDataDirectory.getChildFile(labelFileName);

            if (!labelFile.existsAsFile()) {
                std::cout << "Warning: No Label File for " << baseName << " found. Skip this recording." << std::endl;
                continue;
            }

            std::cout << "Analyze recording: " << baseName << "..." << std::endl;

            auto groundTruth = parseGroundTruth(labelFile);

            ChordAnalyzer analyzer = ChordAnalyzer(config);
            auto result = analyzer.runAnalysis(wavFile);

            int trackCorrectFrames = 0;
            int trackTotalFrames = 0;

            evaluateTrackAccuracy(groundTruth, result.rawClassifications, result.sampleRate, result.hopSize, trackCorrectFrames, trackTotalFrames);

            globalCorrectFrames += trackCorrectFrames;
            globalTotalFrames += trackTotalFrames;

            float trackAccuracy = 0.0f;
            if (trackTotalFrames > 0) {
                trackAccuracy = static_cast<float>(trackCorrectFrames) / static_cast<float>(trackTotalFrames);
            }

            juce::var songJson = createJSONForTrack(baseName, groundTruth, result.rawClassifications, result.sampleRate, result.hopSize, trackAccuracy);
            allSongsArray.add(songJson);
        }
        float overallAccuracy = 0.0f;
        if (globalTotalFrames > 0) {
            overallAccuracy = static_cast<float>(globalCorrectFrames) / static_cast<float>(globalTotalFrames);
        }

        jsonRoot->setProperty("test_name", config.testName);
        jsonRoot->setProperty("overall_accuracy", overallAccuracy);
        jsonRoot->setProperty("total_frames_analyzed", globalTotalFrames);
        jsonRoot->setProperty("songs", allSongsArray);

        juce::File outputFile = outputDirectory.getChildFile("results_" + config.testName + ".json");
        saveResultsToJSON(juce::var(jsonRoot.get()), outputFile);

        std::cout << "Test-Run Finished! Results were saved: " << outputFile.getFullPathName() << std::endl;
        std::cout << "Overall Accuracy: " << (overallAccuracy * 100.0f) << " %" << std::endl;

}

std::vector<Test::GroundTruthLabel> Test::parseGroundTruth(const juce::File &labelFile) const {
    std::vector<Test::GroundTruthLabel> result;

    juce::StringArray lines;
    labelFile.readLines(lines);

    struct TempLabel {
      float time;
      juce::String label;
    };
    std::vector<TempLabel> parsedInstants;

    for (juce::String line : lines) {
        line = line.trim();
        if(line.isEmpty()) continue;

        // Seperate line with every occurence of ","
        juce::StringArray tokens = juce::StringArray::fromTokens(line, ",", "");

        if (tokens.size() >= 2) {
            float time = tokens[0].getFloatValue(); // time instance
            juce::String text = tokens[1].trim(); // Label

            parsedInstants.push_back({time, text});
        }
    }

    if (parsedInstants.empty()) return result;

    // 1. From 0.0 (start of audio file) to first timestamp there is silence
    if (parsedInstants[0].time > 0.001f) {
        result.push_back({0.0f, parsedInstants[0].time, ""});
    }

    // 2. Chords start at timestamp and end at the next timestamp
    for (size_t i = 0; i < parsedInstants.size(); i++) {
        float startTime = parsedInstants[i].time;
        float endTime = (i + 1 < parsedInstants.size()) ? parsedInstants[i+1].time : 9999.0f;

        juce::String label = parsedInstants[i].label;
        result.push_back({startTime, endTime, label});
    }

    return result;
}

juce::var Test::createJSONForTrack(const juce::String &songName,
    const std::vector<Test::GroundTruthLabel> &groundTruth,
    const std::vector<int> &predictions,
    double sampleRate, int hopSize, float trackAccuracy) const {

        Classificator classifier = Classificator(0.8f);
        juce::DynamicObject::Ptr trackObj = new juce::DynamicObject();
        trackObj->setProperty("track_name", songName);
        trackObj->setProperty("track_accuracy", trackAccuracy);
        trackObj->setProperty("sample_rate", sampleRate);
        trackObj->setProperty("hop_size", hopSize);

        juce::Array<juce::var> framesArray;
        size_t gtIndex = 0;
        int totalFrames = predictions.size(); // size_t hier nehmen

        for (int f = 0; f < totalFrames; f++) {
            double frameTimeSec = (f * hopSize) / sampleRate;

            while (gtIndex < groundTruth.size() && frameTimeSec >= groundTruth[gtIndex].endTimeSec) {
                gtIndex++;
            }

            juce::String gtLabel = "";
            if (gtIndex < groundTruth.size())  {
                gtLabel = groundTruth[gtIndex].chordName;
            }

            juce::String predLabel = classifier.getChordName(predictions[f]);

            juce::DynamicObject::Ptr frameObj = new juce::DynamicObject();
            frameObj->setProperty("frame", f);
            frameObj->setProperty("time_sec", frameTimeSec);
            frameObj->setProperty("ground_truth", gtLabel);
            frameObj->setProperty("prediction", predLabel);

            framesArray.add(juce::var(frameObj.get()));
        }

        trackObj->setProperty("frames", framesArray);

        return juce::var(trackObj.get());
}

void Test::saveResultsToJSON(const juce::var &resultsData, const juce::File &outputFile) const {
    juce::String jsonString = juce::JSON::toString(resultsData);

    bool success = outputFile.replaceWithText(jsonString);

    if (!success) {
        std::cerr << "Error: Couldn't save json file here: "
            << outputFile.getFullPathName() << std::endl;
    }
}

void Test::evaluateTrackAccuracy(const std::vector<Test::GroundTruthLabel> &groundTruth,
    const std::vector<int> &predictions,
    double sampleRate, int hopSize,
    int &outCorrectFrames, int &outTotalFrames) const {
        if (predictions.empty() || groundTruth.empty()) return;

        Classificator classifier = Classificator(0.8f);
        int totalFrames = predictions.size();

        size_t gtIndex = 0;

        for (int f = 0; f < totalFrames; f++) {
            double frameTimeSec = (f * hopSize) / sampleRate;

            while (gtIndex < groundTruth.size() && frameTimeSec >= groundTruth[gtIndex].endTimeSec) {
                gtIndex++;
            }

            if (gtIndex >= groundTruth.size()) {
                break;
            }

            juce::String gtLabel = groundTruth[gtIndex].chordName;
            juce::String predLabel = classifier.getChordName(predictions[f]);

            outTotalFrames++;
            if (gtLabel == predLabel) {
                outCorrectFrames++;
            }
        }
    }
