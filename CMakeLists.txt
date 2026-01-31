cmake_minimum_required(VERSION 3.15)

project(Aether VERSION 3.0.0)

# Add JUCE using FetchContent
include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(juce)

# Define the plugin
juce_add_plugin(Aether
    COMPANY_NAME "Antigravity"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Juce
    PLUGIN_CODE Aeth
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "AETHER 3.0"
)

# Add source files
target_sources(Aether PUBLIC
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
)

# Link standard libraries and JUCE modules
target_compile_definitions(Aether PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Aether PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_graphics
)

# Manual PDF build (run: cmake --build build --target manual_pdf)
set(MANUAL_MD "${CMAKE_CURRENT_SOURCE_DIR}/docs/AETHER_3.0_User_Manual.md")
set(MANUAL_PDF "${CMAKE_CURRENT_SOURCE_DIR}/docs/AETHER_3.0_User_Manual.pdf")
set(MANUAL_CSS "${CMAKE_CURRENT_SOURCE_DIR}/docs/aether-manual.css")
add_custom_target(manual_pdf
  COMMAND npx --yes md-to-pdf "${MANUAL_MD}" --stylesheet "${MANUAL_CSS}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs"
  COMMENT "Building AETHER_3.0_User_Manual.pdf from Markdown"
  BYPRODUCTS "${MANUAL_PDF}"
)
