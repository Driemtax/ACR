#include "InstrumentComponent.h"
#include "../Util/FretboardMapper.h"
#include "GuitarView.h"
#include "juce_graphics/juce_graphics.h"
#include <vector>

InstrumentComponent::InstrumentComponent(AudioEngine &engine)
    : audioEngine(engine), guitarView(engine) {
  addAndMakeVisible(guitarView);
  addAndMakeVisible(chordInfoPanel);

  chordInfoPanel.clearChords();

  // callback for ScaleSelector to call if user changes selected scale
  chordInfoPanel.getScaleSelector().onScaleChanged =
      [this](int root, ScaleDatabase::ScaleType type) {
        activeScaleLabels = FretboardMapper::getLabelsForScale(
            root, type, juce::Colours::steelblue);

        auto state = audioEngine.getState();
        if (state == AudioEngine::TransportState::Playing ||
            state == AudioEngine::TransportState::Paused) {
          guitarView.setScaleLabels(activeScaleLabels);
        }
      };

  // Initialize A Minor Pentatonic
  // chordInfoPanel.getScaleSelector().onScaleChanged(
  //    9, ScaleDatabase::ScaleType::MinorPantatonic);

  //  start timer to check playback position
  startTimerHz(30);
}

InstrumentComponent::~InstrumentComponent() { stopTimer(); }

void InstrumentComponent::setTimeline(
    const std::vector<Classificator::ChordSegment> &segments, double sampleRate,
    int hopSize) {

  timeline = segments;
  analysisSampleRate = sampleRate;
  analysisHopSize = hopSize;

  // Reset state
  currentSegmentIndex = -1;
  chordInfoPanel.clearChords();
  guitarView.clearLabels();

  if (!timeline.empty()) {
    timerCallback();
  }
}

void InstrumentComponent::timerCallback() {
  if (timeline.empty())
    return;

  // Only update if playing or paused (so we can seek while paused)
  auto state = audioEngine.getState();
  if (state == AudioEngine::TransportState::Stopped) {
    if (currentSegmentIndex != -1) {
      currentSegmentIndex = -1;
      chordInfoPanel.clearChords();
      guitarView.clearLabels();
      guitarView.clearScaleLabels();
    }

    return;
  }

  // 1. Calculate current Frame
  double positionSec = audioEngine.getTransportSource().getCurrentPosition();
  double framesPerSecond = analysisSampleRate / analysisHopSize;
  int currentFrame = static_cast<int>(positionSec * framesPerSecond);

  // 2. Find chord segment for current frame
  int foundSegmentIndex = -1;
  int searchStart = std::max(0, currentSegmentIndex);

  // Quick check if we are still in the same segment
  if (currentSegmentIndex >= 0 && currentSegmentIndex < timeline.size()) {
    auto &seg = timeline[currentSegmentIndex];
    if (currentFrame >= seg.startFrame && currentFrame <= seg.endFrame) {
      foundSegmentIndex = currentSegmentIndex;
    }
  }

  // if not current segment, search the timeline
  if (foundSegmentIndex == -1) {
    for (int i = 0; i < timeline.size(); i++) {
      if (currentFrame >= timeline[i].startFrame &&
          currentFrame <= timeline[i].endFrame) {
        foundSegmentIndex = i;
        break;
      }
    }
  }

  // if segment changed, update the UI
  if (foundSegmentIndex != currentSegmentIndex) {
    bool wasStopped = (currentSegmentIndex == -1);

    currentSegmentIndex = foundSegmentIndex;
    updateUIForSegment(currentSegmentIndex);

    if (wasStopped) {
      guitarView.setScaleLabels(activeScaleLabels);
    }
  }
}

void InstrumentComponent::updateUIForSegment(int segmentIndex) {
  if (segmentIndex < 0 || segmentIndex >= timeline.size()) {
    chordInfoPanel.clearChords();
    guitarView.clearLabels();
    return;
  }

  // Current chord
  juce::String currentChord = timeline[segmentIndex].chordName;
  chordInfoPanel.setCurrentChord(currentChord);

  // Map chord to guitar labels
  if (currentChord.isNotEmpty() && currentChord != "N") {
    auto labels = FretboardMapper::getLabelsForChord(currentChord,
                                                     juce::Colours::green, 22);
    guitarView.setCurrentLabels(labels);
  } else {
    guitarView.clearLabels();
    chordInfoPanel.setCurrentChord("");
  }

  // Next chord (look ahead for the next DIFFERENT chord)
  juce::String nextChord = "";
  for (int i = segmentIndex + 1; i < timeline.size(); i++) {
    if (timeline[i].chordName != currentChord) {
      nextChord = timeline[i].chordName;
      break;
    }
  }

  auto nextLabels =
      FretboardMapper::getLabelsForChord(nextChord, juce::Colours::orange, 22);
  guitarView.setNextLabels(nextLabels);
  chordInfoPanel.setNextChord(nextChord);

  // TODO: get current Scale and set it as active? Dont need this every frame
}

void InstrumentComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colour(0xFF1A1A2E));
}

void InstrumentComponent::resized() {
  auto area = getLocalBounds();

  chordInfoPanel.setBounds(
      area.removeFromTop(static_cast<int>(getHeight() * 0.4f)));

  guitarView.setBounds(area);
}
