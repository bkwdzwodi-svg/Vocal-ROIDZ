#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include "Parameters.h"
#include "Presets.h"
#include "DeEsser.h"
#include "Dsp/Pitch.h"
#include "Dsp/Dynamics.h"
#include "Dsp/Tone.h"
#include "Dsp/Space.h"
#include "Dsp/Creative.h"
#include "Dsp/Master.h"

class VocalROIDZProcessor : public juce::AudioProcessor
{
public:
    VocalROIDZProcessor();
    ~VocalROIDZProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Vocal ROIDZ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    // The factory presets are exposed as host programs so they show up in the DAW too.
    int getNumPrograms() override { return (int) presets::factory().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // ---------------------------------------------------------------- scope + meter feeds
    // Written on the audio thread, read by the GUI. The index is the only shared cursor;
    // a torn frame in a graph is invisible, and it keeps the audio thread allocation-free.
    struct Scope
    {
        static constexpr int waveSize = 256;
        static constexpr int spectrumSize = 256;
        static constexpr int pitchSize = 96;

        std::array<float, waveSize> wave {};
        int waveWrite = 0;

        std::array<float, spectrumSize> spectrum {};

        std::array<float, pitchSize> pitchDetected {};
        std::array<float, pitchSize> pitchTarget {};
        int pitchWrite = 0;

        std::atomic<float> inputLevel { 0.0f }, outputLevel { 0.0f };
        std::atomic<float> compReduction { 0.0f }, gateReduction { 0.0f }, essReduction { 0.0f };
        std::atomic<float> lufs { -70.0f }, correlation { 1.0f };
        std::atomic<float> detectedMidi { 0.0f }, targetMidi { 0.0f };
    } scope;

    std::atomic<float> meterIn { 0.0f }, meterOut { 0.0f }, meterComp { 0.0f }, meterEss { 0.0f };

private:
    float param (const char* id) const;
    bool isOn (const char* id) const { return param (id) > 0.5f; }
    void runSpectrum (const juce::AudioBuffer<float>& buffer);
    void pushPitch (float detected, float target);

    // ---------------------------------------------------------------- modules 01 - 16
    AutoTune          tuner;
    PitchFx           pitchFx;
    HarmonyEngine     harmony;
    NoiseCleanup      cleanup;
    NoiseGate         gate;
    AnalogEQ          eq;
    VocalCompressor   compressor;
    juce::dsp::Gain<float> makeup, outGain;
    SaturationEngine  saturation;
    DeEsser           deesser;
    RegularStutter    regularStutter;
    ChopNScrew        chopScrew;
    HyperStutter      stutter;
    VocalDoubler      doubler;
    StereoImager      imager;
    CreativeFxLab     creative;
    DelayLab          delayLab;
    ReverbSpace       reverbSpace;
    MasterFinish      masterFinish;

    // ---------------------------------------------------------------- analysis
    juce::dsp::FFT fft { 10 };
    juce::dsp::WindowingFunction<float> fftWindow { 1024, juce::dsp::WindowingFunction<float>::hann };
    std::array<float, 2048> fftData {};
    std::array<float, 1024> fftFifo {};
    int fftFifoIndex = 0;
    int fftBlockCounter = 0;

    double currentSampleRate = 44100.0;
    int currentProgram = 0;
    static constexpr const char* currentProgramProperty = "currentProgram";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalROIDZProcessor)
};
