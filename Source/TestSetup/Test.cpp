#include "Test.h"
#include "../DSP/ChordAnalyzer.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

void Test::runAllTests() const {
  // 1. TestCase: HPCP base with no median Filter
  juce::String testName = "HPCP_noGain_noMedian";
  AnalyzerConfig config = AnalyzerConfig();
  config.medianFilter = false;
  runTests(config, const_cast<juce::String &>(testName), true);

  // 2. TestCase: HPCP base with median Filter Window Size = 5
  testName = "HPCP_noGain_median19";
  config.medianFilter = true;
  config.medianWindowSize = 19;
  runTests(config, const_cast<juce::String &>(testName), true);

  // 3. TestCase: Use ML modell of madmom
  testName = "DeepChroma_noGain";
  config.setToDeepLearningDefaults();
  runTests(config, const_cast<juce::String &>(testName), true);

  // TODO: run tests on every config
  // TODO: Output every result json with appropriate name
}

void Test::findMaxima() const {
  // findMaximaMedianWindowSize();
  // findMaximaFloatParameters();

  findMaximaThresholdDeepLearning();
}

void Test::findMaximaMedianWindowSize() const {
  AnalyzerConfig config;
  config.s = 0.5;
  config.similarityThreshold = 0.3;
  const juce::String testfileName = "MedianMaximum";

  int bestWindowSize = 1;
  float highestAccuracy = 0.0f;
  float currentAccuracy = 0.0f;

  for (int i = 1; i <= 60; i++) {
    config.medianWindowSize = i;

    currentAccuracy =
        runTests(config, testfileName + "_" + std::to_string(i), false);

    std::cout << "Median Size: " << i
              << ", Accuracy: " << currentAccuracy * 100.0f << std::endl;

    if (currentAccuracy > highestAccuracy) {
      highestAccuracy = currentAccuracy;
      bestWindowSize = i;
    }
  }

  std::cout << "Highest Accuracy: " << highestAccuracy * 100.0f
            << " with Median Window Size: " << bestWindowSize << std::endl;
}

void Test::findMaximaThresholdDeepLearning() const {
  AnalyzerConfig config;
  config.setToDeepLearningDefaults();

  const juce::String testfileName = "ThresholdMaximumDeepLearning";

  float bestThreshold = 0.0f;
  float bestAccuracy = -1.0f;
  float thresholdMin = 0.1f;
  float thresholdMax = 0.9f;
  float thresholdStep = 0.1f;

  std::cout << "=== STARTE GROBE RASTERSUCHE ===" << std::endl;

  for (float t = thresholdMin; t < thresholdMax + 0.0001f; t += thresholdStep) {
    config.similarityThreshold = t;

    float currentAccuracy =
        runTests(config, testfileName + "_deepT_" + std::to_string(t), false);

    std::cout << "Test t=" << t << " -> Accuracy: " << currentAccuracy * 100.0f
              << "%" << std::endl;

    if (currentAccuracy > bestAccuracy) {
      bestAccuracy = currentAccuracy;
      bestThreshold = t;
    }
  }

  std::cout << "Grobes Maximum gefunden bei t = " << bestThreshold
            << " (Accuracy: " << bestAccuracy << "%)" << std::endl;
  std::cout << "\n=== STARTE FEINE RASTERSUCHE ===" << std::endl;
  thresholdMin = std::max(0.0f, bestThreshold - thresholdStep);
  thresholdMax = std::min(1.0f, bestThreshold + thresholdStep);

  thresholdStep = 0.02f;

  for (float t = thresholdMin; t <= thresholdMax + 0.0001f;
       t += thresholdStep) {
    config.similarityThreshold = t;

    float currentAccuracy =
        runTests(config, testfileName + "_deepT_" + std::to_string(t), false);

    std::cout << "Test t=" << t << " -> Accuracy: " << currentAccuracy * 100.0f
              << "%" << std::endl;

    if (currentAccuracy > bestAccuracy) {
      bestAccuracy = currentAccuracy;
      bestThreshold = t;
    }
  }

  std::cout << "\n=== OPTIMIERUNG ABGESCHLOSSEN ===" << std::endl;
  std::cout << "Beste Parameter:" << std::endl;
  std::cout << "similarityThreshold = " << bestThreshold << std::endl;
  std::cout << "Finale Accuracy = " << bestAccuracy << "%" << std::endl;
}

void Test::findMaximaFloatParameters() const {
  AnalyzerConfig config;
  // Accpording to first tests the best value for accuracy.
  config.medianWindowSize = 60;
  const juce::String testfileName = "FloatParametersMaximum";

  float bestS = 0.0f;
  float bestThreshold = 0.0f;
  float bestAccuracy = -1.0f;
  float sMin = 0.1f;
  float sMax = 1.0f;
  float sStep = 0.1f;
  float thresholdMin = 0.3f;
  float thresholdMax = 0.9f;
  float thresholdStep = 0.1f;

  std::cout << "=== STARTE GROBE RASTERSUCHE ===" << std::endl;

  for (float s = sMin; s < sMax + 0.0001f; s += sStep) {
    for (float t = thresholdMin; t < thresholdMax + 0.0001f;
         t += thresholdStep) {
      config.s = s;
      config.similarityThreshold = t;

      float currentAccuracy = runTests(config,
                                       testfileName + "_s" + std::to_string(s) +
                                           "_t" + std::to_string(t),
                                       false);

      std::cout << "Test s=" << s << ", treshold=" << t
                << " -> Accuracy: " << currentAccuracy * 100.0f << "%"
                << std::endl;

      if (currentAccuracy > bestAccuracy) {
        bestAccuracy = currentAccuracy;
        bestS = s;
        bestThreshold = t;
      }

      std::cout << "\n=== OPTIMIERUNG ABGESCHLOSSEN ===" << std::endl;
      std::cout << "Beste Parameter:" << std::endl;
      std::cout << "similarityThreshold = " << bestThreshold << std::endl;
      std::cout << "Finale Accuracy = " << bestAccuracy << "%" << std::endl;
    }
  }

  std::cout << "Grobes Maximum gefunden bei s=" << bestS
            << ", threshold=" << bestThreshold << " (Accuracy: " << bestAccuracy
            << "%)" << std::endl;

  std::cout << "\n=== STARTE FEINE RASTERSUCHE ===" << std::endl;
  sMin = std::max(0.0f, bestS - sStep);
  sMax = std::min(1.0f, bestS + sStep);

  thresholdMin = std::max(0.0f, bestThreshold - thresholdStep);
  thresholdMax = std::min(1.0f, bestThreshold + thresholdStep);

  sStep = 0.02f;
  thresholdStep = 0.02f;

  for (float s = sMin; s <= sMax + 0.0001f; s += sStep) {
    for (float t = thresholdMin; t <= thresholdMax + 0.0001f;
         t += thresholdStep) {
      config.s = s;
      config.similarityThreshold = t;

      float currentAccuracy = runTests(config,
                                       testfileName + "_s" + std::to_string(s) +
                                           "_t" + std::to_string(t),
                                       false);

      std::cout << "Test s=" << s << ", treshold=" << t
                << " -> Accuracy: " << currentAccuracy * 100.0f << "%"
                << std::endl;

      if (currentAccuracy > bestAccuracy) {
        bestAccuracy = currentAccuracy;
        bestS = s;
        bestThreshold = t;
      }
    }
  }

  std::cout << "\n=== OPTIMIERUNG ABGESCHLOSSEN ===" << std::endl;
  std::cout << "Beste Parameter:" << std::endl;
  std::cout << "s = " << bestS << std::endl;
  std::cout << "similarityThreshold = " << bestThreshold << std::endl;
  std::cout << "Finale Accuracy = " << bestAccuracy << "%" << std::endl;
}

float Test::runTests(AnalyzerConfig &config, const juce::String testFileName,
                     bool logToConsole) const {

  juce::DynamicObject::Ptr jsonRoot = new juce::DynamicObject();
  juce::Array<juce::var> allSongsArray;

  juce::RangedDirectoryIterator iter(testDataDir, false, "*.wav");

  int globalCorrectFrames = 0;
  int globalTotalFrames = 0;

  for (const juce::DirectoryEntry &entry : iter) {
    juce::File wavFile = entry.getFile();
    juce::String baseName = wavFile.getFileNameWithoutExtension();

    juce::String labelFileName = baseName + "_label.txt";
    juce::File labelFile = testDataDir.getChildFile(labelFileName);

    if (!labelFile.existsAsFile()) {
      std::cout << "Warning: No Label File for " << baseName
                << " found. Skip this recording." << std::endl;
      continue;
    }

    if (logToConsole) {
      std::cout << "Analyze recording: " << baseName << "..." << std::endl;
    }

    auto groundTruth = parseGroundTruth(labelFile);

    ChordAnalyzer analyzer = ChordAnalyzer(config);
    auto result = analyzer.runAnalysis(wavFile);

    int trackCorrectFrames = 0;
    int trackTotalFrames = 0;

    evaluateTrackAccuracy(groundTruth, result.rawClassifications,
                          result.sampleRate, result.hopSize, trackCorrectFrames,
                          trackTotalFrames);

    globalCorrectFrames += trackCorrectFrames;
    globalTotalFrames += trackTotalFrames;

    float trackAccuracy = 0.0f;
    if (trackTotalFrames > 0) {
      trackAccuracy = static_cast<float>(trackCorrectFrames) /
                      static_cast<float>(trackTotalFrames);
    }

    juce::var songJson =
        createJSONForTrack(baseName, groundTruth, result.rawClassifications,
                           result.sampleRate, result.hopSize, trackAccuracy);
    allSongsArray.add(songJson);
  }
  float overallAccuracy = 0.0f;
  if (globalTotalFrames > 0) {
    overallAccuracy = static_cast<float>(globalCorrectFrames) /
                      static_cast<float>(globalTotalFrames);
  }

  jsonRoot->setProperty("test_name", testFileName);
  jsonRoot->setProperty("overall_accuracy", overallAccuracy);
  jsonRoot->setProperty("total_frames_analyzed", globalTotalFrames);
  jsonRoot->setProperty("songs", allSongsArray);

  juce::File outputFile =
      outputDir.getChildFile("results_" + testFileName + ".json");
  saveResultsToJSON(juce::var(jsonRoot.get()), outputFile);

  if (logToConsole) {
    std::cout << "Test-Run Finished! Results were saved: "
              << outputFile.getFullPathName() << std::endl;
    std::cout << "Overall Accuracy: " << (overallAccuracy * 100.0f) << " %"
              << std::endl;
  }

  return overallAccuracy;
}

std::vector<Test::GroundTruthLabel>
Test::parseGroundTruth(const juce::File &labelFile) const {
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
    if (line.isEmpty())
      continue;

    // Seperate line with every occurence of ","
    juce::StringArray tokens = juce::StringArray::fromTokens(line, ",", "");

    if (tokens.size() >= 2) {
      float time = tokens[0].getFloatValue(); // time instance
      juce::String text = tokens[1].trim();   // Label

      parsedInstants.push_back({time, text});
    }
  }

  if (parsedInstants.empty())
    return result;

  // 1. From 0.0 (start of audio file) to first timestamp there is silence
  if (parsedInstants[0].time > 0.001f) {
    result.push_back({0.0f, parsedInstants[0].time, ""});
  }

  // 2. Chords start at timestamp and end at the next timestamp
  for (size_t i = 0; i < parsedInstants.size(); i++) {
    float startTime = parsedInstants[i].time;
    float endTime =
        (i + 1 < parsedInstants.size()) ? parsedInstants[i + 1].time : 9999.0f;

    juce::String label = parsedInstants[i].label;
    result.push_back({startTime, endTime, label});
  }

  return result;
}

juce::var
Test::createJSONForTrack(const juce::String &songName,
                         const std::vector<Test::GroundTruthLabel> &groundTruth,
                         const std::vector<int> &predictions, double sampleRate,
                         int hopSize, float trackAccuracy) const {

  Classificator classifier = Classificator(0.8f);
  juce::DynamicObject::Ptr trackObj = new juce::DynamicObject();
  trackObj->setProperty("track_name", songName);
  trackObj->setProperty("track_accuracy", trackAccuracy);
  trackObj->setProperty("sample_rate", sampleRate);
  trackObj->setProperty("hop_size", hopSize);

  juce::Array<juce::var> framesArray;
  size_t gtIndex = 0;
  size_t totalFrames = predictions.size(); // size_t hier nehmen

  for (size_t f = 0; f < totalFrames; f++) {
    double frameTimeSec = (f * hopSize) / sampleRate;

    while (gtIndex < groundTruth.size() &&
           frameTimeSec >= groundTruth[gtIndex].endTimeSec) {
      gtIndex++;
    }

    juce::String gtLabel = "";
    if (gtIndex < groundTruth.size()) {
      gtLabel = groundTruth[gtIndex].chordName;
    }

    juce::String predLabel = classifier.getChordName(predictions[f]);

    juce::DynamicObject::Ptr frameObj = new juce::DynamicObject();
    frameObj->setProperty("frame", static_cast<int>(f));
    frameObj->setProperty("time_sec", frameTimeSec);
    frameObj->setProperty("ground_truth", gtLabel);
    frameObj->setProperty("prediction", predLabel);

    framesArray.add(juce::var(frameObj.get()));
  }

  trackObj->setProperty("frames", framesArray);

  return juce::var(trackObj.get());
}

void Test::saveResultsToJSON(const juce::var &resultsData,
                             const juce::File &outputFile) const {
  juce::String jsonString = juce::JSON::toString(resultsData);

  bool success = outputFile.replaceWithText(jsonString);

  if (!success) {
    std::cerr << "Error: Couldn't save json file here: "
              << outputFile.getFullPathName() << std::endl;
  }
}

void Test::evaluateTrackAccuracy(
    const std::vector<Test::GroundTruthLabel> &groundTruth,
    const std::vector<int> &predictions, double sampleRate, int hopSize,
    int &outCorrectFrames, int &outTotalFrames) const {
  if (predictions.empty() || groundTruth.empty())
    return;

  Classificator classifier = Classificator(0.8f);
  size_t totalFrames = predictions.size();

  size_t gtIndex = 0;

  for (size_t f = 0; f < totalFrames; f++) {
    double frameTimeSec = (f * hopSize) / sampleRate;

    while (gtIndex < groundTruth.size() &&
           frameTimeSec >= groundTruth[gtIndex].endTimeSec) {
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
