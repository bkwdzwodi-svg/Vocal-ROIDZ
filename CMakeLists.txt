cmake_minimum_required(VERSION 3.22)

project(VocalROIDZ VERSION 2.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17")

# Use a local JUCE checkout with -DJUCE_PATH=/path/to/JUCE, otherwise download JUCE 8.
if (DEFINED JUCE_PATH)
    add_subdirectory("${JUCE_PATH}" JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.4.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(VocalROIDZ
    COMPANY_NAME "BKWDZ"
    BUNDLE_ID "com.bkwdz.vocalroidz"
    PLUGIN_MANUFACTURER_CODE Bkwd
    PLUGIN_CODE Vrdz
    PRODUCT_NAME "Vocal ROIDZ"
    DESCRIPTION "All-in-one vocal production workstation"
    FORMATS VST3 Standalone AU
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

target_sources(VocalROIDZ PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp)

target_compile_definitions(VocalROIDZ PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0)

target_link_libraries(VocalROIDZ
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
