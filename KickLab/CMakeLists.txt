cmake_minimum_required(VERSION 3.22)
project(KickLab VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# JUCE: uses ./JUCE if present, otherwise downloads JUCE 8.0.4 automatically
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt)
  add_subdirectory(JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.4 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

set(KL_FORMATS VST3 Standalone CACHE STRING "Plugin formats to build")

juce_add_plugin(KickLab
  COMPANY_NAME "KickLab"
  PLUGIN_MANUFACTURER_CODE Kklb
  PLUGIN_CODE Kkl1
  FORMATS ${KL_FORMATS}
  PRODUCT_NAME "Kick Lab"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Instrument Drum Synth)

target_sources(KickLab PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)

target_compile_definitions(KickLab PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(KickLab
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

option(KL_BUILD_TESTS "Build the headless engine test" OFF)
if(KL_BUILD_TESTS)
  add_executable(KickLabTest Tests/EngineTest.cpp)
  target_link_libraries(KickLabTest PRIVATE KickLab juce::juce_audio_utils juce::juce_dsp)
  target_compile_definitions(KickLabTest PRIVATE JUCE_STANDALONE_APPLICATION=1)
endif()
