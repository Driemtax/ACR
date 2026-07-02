#include "Test.h"
#include "../DSP/ChordAnalyzer.h"
#include "juce_core/juce_core.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

void Test::runAllTests() const {
  // 1. TestCase: HPCP base with no gain, no powerchords
  juce::String testName = "HPCP_Base_noGain_noPowerchords";
  AnalyzerConfig config;
  config.setToDefaults();
  config.medianFilter = false;
  config.useKeyEstimator = false;
  config.tuningShift = false;
  config.useDeepLearning = false;
  // runTests(config, const_cast<juce::String &>(testName),
  // "noGain_noPowerchords",
  //          true, true);

  // 2. TestCase: HPCP base with powerchords
  // testName = "HPCP_Base_noGain";
  // runTests(config, const_cast<juce::String &>(testName), "noGain", true,
  // true);

  // 3. TestCase: HPCP base with gain
  // testName = "HPCP_Base";
  // runTests(config, const_cast<juce::String &>(testName), "", true, true);

  // 4. TestCase: HPCP similarityThreshold sweep
  // testName = "HPCP_thresholdSweep";
  // TODO: Do i use the optimum of every other parameter or do i build up test
  // after test?
  // findMaximaThreshold(config, testName);

  // 5. TestCase: HPCP s parameter sweep
  // findMaximaSParameter();
  findMaximaFloatParameters("");

  // 6. TestCase: HPCP median Filter sweep
  // findMaximaMedianWindowSize();

  // 7. TestCase: HPCP keyEstimator
  // Already tested.
  //
  // 7.2 TestCase: HPCP Tuning Shift ChromaRes = 3
  // config.tuningShift = true;
  // config.chromaRes = 3;
  // testName = "HPCP_tuning_shift";
  // runTests(config, testName, "", true, true);

  // 7.3 TestCase: HPCP new Base with optimized parameters
  // config.setToDefaults();
  // config.medianFilter = true;
  // config.medianWindowSize = 223;
  // config.s = 0.6;
  // config.similarityThreshold = 0.3;
  // config.useDeepLearning = false;
  // config.useKeyEstimator = false;
  // config.tuningShift = false;
  // testName = "HPCP_newBase";
  // runTests(config, testName, "", true, true);

  // 8. TestCase: deep learning base with no gain, no powerchords
  // config.setToDeepLearningDefaults();
  // config.useKeyEstimator = false;
  // testName = "ML_Base_noGain_noPowerchords";
  // runTests(config, testName, "noGain_noPowerchords", true, true);

  // 9. TestCase: deep learning base with powerchords
  // testName = "ML_Base_noGain";
  // runTests(config, testName, "noGain", true, true);

  // 10. TestCase: deep learning base with gain
  // testName = "ML_Base";
  // runTests(config, testName, "", true, true);

  // 11. TestCase: deep learning similarityThreshold sweep
  // testName = "ML_thresholdSweep";
  // findMaximaThreshold(config, testName);

  // 12. TestCase: deep learning key estimator
  // Already tested.
}

void Test::keyEstimatorTest() const {
  std::cout << "\n=== Start KeyEstimator Vergleichstest ===" << std::endl;

  AnalyzerConfig config;

  // --- 1. HPCP configuration  ---
  std::cout << "\n[1/4] Teste HPCP (OHNE Smoothing)..." << std::endl;
  config.useDeepLearning = false;
  config.medianFilter = true;
  config.medianWindowSize = 19;

  config.useKeyEstimator = false;
  float hpcpNoKey = runTests(config, "HPCP_NoKeySmoothing");

  std::cout << "[2/4] Teste HPCP (MIT Smoothing)..." << std::endl;
  config.useKeyEstimator = true;
  float hpcpWithKey = runTests(config, "HPCP_WithKeySmoothing");

  // --- 2. Deep Learning Konfiguration ---
  std::cout << "\n[3/4] Teste Deep Learning (OHNE Smoothing)..." << std::endl;
  config.setToDeepLearningDefaults();

  config.useKeyEstimator = false;
  float deepNoKey = runTests(config, "DeepChroma_NoKeySmoothing");

  std::cout << "[4/4] Teste Deep Learning (MIT Smoothing)..." << std::endl;
  config.useKeyEstimator = true;
  float deepWithKey = runTests(config, "DeepChroma_WithKeySmoothing");

  // --- 3. Ergebnisse berechnen und übersichtlich ausgeben ---
  float hpcpDiff = (hpcpWithKey - hpcpNoKey) * 100.0f;
  float deepDiff = (deepWithKey - deepNoKey) * 100.0f;

  std::cout << "\n=========================================" << std::endl;
  std::cout << "               ERGEBNISSE                " << std::endl;
  std::cout << "=========================================\n" << std::endl;

  // HPCP Block
  std::cout << "--- HPCP (Traditionell) ---" << std::endl;
  std::cout << "Ohne KeyEstimator: " << hpcpNoKey * 100.0f << " %" << std::endl;
  std::cout << "Mit KeyEstimator:  " << hpcpWithKey * 100.0f << " %"
            << std::endl;
  std::cout << "Differenz:         " << (hpcpDiff > 0 ? "+" : "") << hpcpDiff
            << " Prozentpunkte\n"
            << std::endl;

  // Deep Learning Block
  std::cout << "--- Deep Learning ---" << std::endl;
  std::cout << "Ohne KeyEstimator: " << deepNoKey * 100.0f << " %" << std::endl;
  std::cout << "Mit KeyEstimator:  " << deepWithKey * 100.0f << " %"
            << std::endl;
  std::cout << "Differenz:         " << (deepDiff > 0 ? "+" : "") << deepDiff
            << " Prozentpunkte\n"
            << std::endl;

  std::cout << "=========================================" << std::endl;
}

void Test::findMaxima() const {
  // findMaximaMedianWindowSize();
  // findMaximaFloatParameters();

  findMaximaThresholdDeepLearning();
}

void Test::findMaximaMedianWindowSize() const {
  AnalyzerConfig config;
  config.s = 0.6;
  config.similarityThreshold = 0.3;
  config.medianFilter = true;
  config.useKeyEstimator = false;

  int bestWindowSize = 1;
  float highestAccuracy = 0.0f;
  float currentAccuracy = 0.0f;

  juce::Array<juce::var> sweepResultsArray;

  std::cout << "\n=== STARTE MEDIAN WINDOW SWEEP ===" << std::endl;

  for (int i = 121; i <= 231; i += 2) {
    config.medianWindowSize = i;

    currentAccuracy = runTests(config, "", "", false, false);

    std::cout << "Median Size: " << i
              << ", Accuracy: " << currentAccuracy * 100.0f << std::endl;

    juce::DynamicObject::Ptr sweepEntry = new juce::DynamicObject();
    sweepEntry->setProperty("median_window_size", i);
    sweepEntry->setProperty("accuracy", currentAccuracy);
    sweepResultsArray.add(juce::var(sweepEntry.get()));

    if (currentAccuracy > highestAccuracy) {
      highestAccuracy = currentAccuracy;
      bestWindowSize = i;
    }
  }

  juce::File outputFile =
      outputDir.getChildFile("results_HPCP_MedianSweep.json");
  saveResultsToJSON(juce::var(sweepResultsArray), outputFile);

  std::cout << "\n=== SWEEP ABGESCHLOSSEN ===" << std::endl;
  std::cout << "Highest Accuracy: " << highestAccuracy * 100.0f
            << " with Median Window Size: " << bestWindowSize << std::endl;
  std::cout << "Saves sweep results to: " << outputFile.getFullPathName()
            << std::endl;
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

    float currentAccuracy = runTests(
        config, testfileName + "_deepT_" + std::to_string(t), "", false, false);

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

    float currentAccuracy = runTests(
        config, testfileName + "_deepT_" + std::to_string(t), "", false, false);

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

void Test::findMaximaFloatParameters(const juce::String &testDir) const {
  AnalyzerConfig config;
  config.medianFilter = false;
  const juce::String testfileName = "FloatParametersMaximum";

  juce::Array<juce::var> gridResultsArray;

  float bestS = 0.0f;
  float bestThreshold = 0.0f;
  float bestAccuracy = -1.0f;
  float sMin = 0.1f;
  float sMax = 1.0f;
  float sStep = 0.1f;
  float thresholdMin = 0.3f;
  float thresholdMax = 0.9f;
  float thresholdStep = 0.1f;

  std::cout << "=== FLOAT SWEEP STARTED ===" << std::endl;
  std::cout << "=== STARTE GROBE RASTERSUCHE ===" << std::endl;

  for (float s = sMin; s < sMax + 0.0001f; s += sStep) {
    for (float t = thresholdMin; t < thresholdMax + 0.0001f;
         t += thresholdStep) {
      config.s = s;
      config.similarityThreshold = t;

      float currentAccuracy = runTests(config, "", testDir, false, false);

      juce::DynamicObject::Ptr sweepEntry = new juce::DynamicObject();
      sweepEntry->setProperty("s", static_cast<double>(s));
      sweepEntry->setProperty("threshold", static_cast<double>(t));
      sweepEntry->setProperty("accuracy", static_cast<double>(currentAccuracy));
      gridResultsArray.add(juce::var(sweepEntry.get()));

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

      float currentAccuracy = runTests(config, "", testDir, false, false);

      juce::DynamicObject::Ptr sweepEntry = new juce::DynamicObject();
      sweepEntry->setProperty("s", static_cast<double>(s));
      sweepEntry->setProperty("threshold", static_cast<double>(t));
      sweepEntry->setProperty("accuracy", static_cast<double>(currentAccuracy));
      gridResultsArray.add(juce::var(sweepEntry.get()));

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

  juce::File outputFile =
      outputDir.getChildFile("results_FLoatParameters_Heatmap.json");
  saveResultsToJSON(juce::var(gridResultsArray), outputFile);

  std::cout << "\n=== OPTIMIERUNG ABGESCHLOSSEN ===" << std::endl;
  std::cout << "Beste Parameter:" << std::endl;
  std::cout << "s = " << bestS << std::endl;
  std::cout << "similarityThreshold = " << bestThreshold << std::endl;
  std::cout << "Finale Accuracy = " << bestAccuracy << "%" << std::endl;
  std::cout << "Heatmap Daten gespeichert in: " << outputFile.getFullPathName()
            << std::endl;
  std::cout << "=== FLOAT SWEEP ENDED ===" << std::endl;
}

float Test::runTests(AnalyzerConfig &config, const juce::String testFileName,
                     juce::String inputDir, bool logToConsole,
                     bool saveFullJson) const {

  juce::DynamicObject::Ptr jsonRoot = new juce::DynamicObject();
  juce::Array<juce::var> allSongsArray;

  juce::File inputFileDir = testDataDir;

  if (testFileName.isNotEmpty()) {
    inputFileDir = inputFileDir.getChildFile(inputDir);
  }

  if (!inputFileDir.exists() || !inputFileDir.isDirectory()) {
    std::cout << "Error: Directory " << inputFileDir.getFullPathName()
              << " does not exist!" << std::endl;
    return 0.0f;
  }

  juce::RangedDirectoryIterator iter(inputFileDir, false, "*.wav");

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

    if (saveFullJson) {
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
  }

  float overallAccuracy = 0.0f;
  if (globalTotalFrames > 0) {
    overallAccuracy = static_cast<float>(globalCorrectFrames) /
                      static_cast<float>(globalTotalFrames);
  }

  if (saveFullJson) {
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
    }
  }

  if (logToConsole) {
    std::cout << "Overall Accuracy: " << (overallAccuracy * 100.0f) << " %"
              << std::endl;
  }

  return overallAccuracy;
}

void Test::findMaximaThreshold(AnalyzerConfig config,
                               const juce::String &testName) const {
  float thresholdMin = 0.1f;
  float thresholdMax = 0.9f;
  float thresholdStep = 0.05f;

  float bestThreshold = 0.0f;
  float highestAccuracy = 0.0f;
  float currentAccuracy = 0.0f;

  juce::Array<juce::var> sweepResultsArray;

  std::cout << "\n=== STARTE THRESHOLD SWEEP (" << testName
            << ") ===" << std::endl;

  // +0.0001f prevents floating point errors to due to rounding.
  for (float t = thresholdMin; t <= thresholdMax + 0.0001f;
       t += thresholdStep) {
    config.similarityThreshold = t;

    currentAccuracy = runTests(config, "", "", false, false);

    std::cout << "Threshold: " << t
              << " -> Accuracy: " << currentAccuracy * 100.0f << " %"
              << std::endl;

    juce::DynamicObject::Ptr sweepEntry = new juce::DynamicObject();
    sweepEntry->setProperty("similarity_threshold", static_cast<double>(t));
    sweepEntry->setProperty("accuracy", static_cast<double>(currentAccuracy));
    sweepResultsArray.add(juce::var(sweepEntry.get()));

    if (currentAccuracy > highestAccuracy) {
      highestAccuracy = currentAccuracy;
      bestThreshold = t;
    }
  }

  // Save everything into one json file
  juce::File outputFile =
      outputDir.getChildFile("results_" + testName + ".json");
  saveResultsToJSON(juce::var(sweepResultsArray), outputFile);

  std::cout << "Highest Accuracy: " << highestAccuracy * 100.0f
            << " with Threshold: " << bestThreshold << std::endl;
  std::cout << "Saved sweep results to: " << outputFile.getFullPathName()
            << std::endl;
}

void Test::findMaximaSParameter() const {
  AnalyzerConfig config;
  config.useDeepLearning = false;
  config.medianFilter = true;
  config.medianWindowSize = 19; // Default, that was used in Base tests

  float sMin = 0.1f;
  float sMax = 1.0f;
  float sStep = 0.05f;

  int bestS = 0;
  float highestAccuracy = 0.0f;
  float currentAccuracy = 0.0f;

  juce::Array<juce::var> sweepResultsArray;

  std::cout << "\n=== STARTE S-PARAMETER SWEEP (HPCP) ===" << std::endl;

  for (float s = sMin; s <= sMax + 0.0001f; s += sStep) {
    config.s = s;

    currentAccuracy = runTests(config, "", "", false, false);

    std::cout << "S-Parameter: " << s
              << " -> Accuracy: " << currentAccuracy * 100.0f << " %"
              << std::endl;

    juce::DynamicObject::Ptr sweepEntry = new juce::DynamicObject();
    sweepEntry->setProperty("s_parameter", static_cast<double>(s));
    sweepEntry->setProperty("accuracy", static_cast<double>(currentAccuracy));
    sweepResultsArray.add(juce::var(sweepEntry.get()));

    if (currentAccuracy > highestAccuracy) {
      highestAccuracy = currentAccuracy;
      bestS = s;
    }
  }

  juce::File outputFile =
      outputDir.getChildFile("results_HPCP_SParameterSweep.json");
  saveResultsToJSON(juce::var(sweepResultsArray), outputFile);

  std::cout << "Highest Accuracy: " << highestAccuracy * 100.0f
            << " with S-Parameter: " << bestS << std::endl;
  std::cout << "Saved sweep results to: " << outputFile.getFullPathName()
            << std::endl;
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
