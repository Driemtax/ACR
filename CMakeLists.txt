cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Name deines Projekts
project(ACR VERSION 0.0.1)

# ==============================================================================
# 1. JUCE einbinden
# Hier musst du den Pfad zu deinem JUCE-Ordner angeben.
# Falls JUCE im selben Ordner wie Test_BA liegt (z.B. C:/Users/a930084/dev/JUCE),
# dann wäre der Pfad "../JUCE".
# ==============================================================================
#add_subdirectory("C:/dev/ACR/JUCE" JUCE)
include(FetchContent)

FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.12
)

FetchContent_MakeAvailable(juce)

# ==============================================================================
# 2. Die ausführbare GUI-Anwendung erstellen
# ==============================================================================
juce_add_gui_app(ACR
    # PRODUCT_NAME "Test_BA"
    # BUNDLE_ID    com.deinname.test_ba
    # COMPANY_NAME "Dein Name"
)


# ==============================================================================
# 2. ONNX Integration
# ==============================================================================
target_include_directories(ACR PRIVATE
  "C:/Test/ML/onnxruntime-win-x64-1.26.0/include"
)

target_link_directories(ACR PRIVATE
  "C:/Test/ML/onnxruntime-win-x64-1.26.0/lib"
)

# ==============================================================================
# 3. Quellcode-Dateien hinzufügen
# ==============================================================================
target_sources(ACR PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/Audio/AudioEngine.h
    Source/Audio/AudioEngine.cpp
    Source/GUI/WaveformDisplay.h
    Source/GUI/WaveformDisplay.cpp
    Source/GUI/SpectogramDisplay.h
    Source/GUI/SpectogramDisplay.cpp
    Source/GUI/ChromaDisplay.h
    Source/GUI/ChromaDisplay.cpp
    Source/DSP/SpectogramAnalyzer.h
    Source/DSP/SpectogramAnalyzer.cpp
    Source/DSP/ChromaAnalyzer.h
    Source/DSP/ChromaAnalyzer.cpp
    Source/DSP/Classificator.h
    Source/DSP/Classificator.cpp
    Source/DSP/ChordAnalyzer.h
    Source/DSP/ChordAnalyzer.cpp
    Source/DSP/ChromaExtractorInterface.h
    Source/TestSetup/Test.h
    Source/TestSetup/Test.cpp
    Source/ML/DeepChromaExtractor.h
    Source/ML/DeepChromaExtractor.cpp
)

# ==============================================================================
# 4. Compiler-Definitionen & Flags
# ==============================================================================
target_compile_definitions(ACR PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    # Hier kannst du weitere JUCE-Flags aktivieren/deaktivieren (0 oder 1)
)

# ==============================================================================
# 5. Benötigte JUCE-Module linken
# Für ein Audio/GUI Projekt reicht das meistens aus:
# ==============================================================================
target_link_libraries(ACR PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
    onnxruntime.lib
)

# Hiermit wird der JUCEHeader.h generiert (optional, aber Projucer nutzt ihn oft)
juce_generate_juce_header(ACR)
