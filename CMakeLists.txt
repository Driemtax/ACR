cmake_minimum_required(VERSION 3.22)

# Name deines Projekts
project(ACR VERSION 0.0.1)

# ==============================================================================
# 1. JUCE einbinden
# Hier musst du den Pfad zu deinem JUCE-Ordner angeben.
# Falls JUCE im selben Ordner wie Test_BA liegt (z.B. C:/Users/a930084/dev/JUCE),
# dann wäre der Pfad "../JUCE".
# ==============================================================================
add_subdirectory("C:/dev/ACR/JUCE" JUCE)

# ==============================================================================
# 2. Die ausführbare GUI-Anwendung erstellen
# ==============================================================================
juce_add_gui_app(ACR
    # PRODUCT_NAME "Test_BA"
    # BUNDLE_ID    com.deinname.test_ba
    # COMPANY_NAME "Dein Name"
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
    Source/DSP/SpectogramAnalyzer.h
    Source/DSP/SpectogramAnalyzer.cpp
    Source/DSP/ChromaAnalyzer.h
    Source/DSP/ChromaAnalyzer.cpp
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
)

# Hiermit wird der JUCEHeader.h generiert (optional, aber Projucer nutzt ihn oft)
juce_generate_juce_header(ACR)
