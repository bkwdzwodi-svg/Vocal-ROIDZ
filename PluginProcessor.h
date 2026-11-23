#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "Parameters.h"
#include "Presets.h"
#include "DeEsser.h"

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
    double getTailLengthSeconds() const override { return 2.0; }

    // The factory style presets are exposed as host programs so they show up in the DAW too.
    int getNumPrograms() override { return (int) presets::factory().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int) override;
    const juce::String getProgramName (int) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Meter feeds for the syringes (written on the audio thread, read by the GUI).
    // meterIn/meterOut are linear peaks; meterComp/meterEss are gain reduction in dB (positive).
    std::atomic<float> meterIn { 0.0f }, meterOut { 0.0f }, meterComp { 0.0f }, meterEss { 0.0f };

private:
    float param (const char* id) const;
    void processDelay (juce::AudioBuffer<float>& buffer);

    // Signal chain: HPF -> Compressor -> De-esser -> Delay -> Output gain
    juce::dsp::StateVariableTPTFilter<float> hpf;
    juce::dsp::Compressor<float> comp;
    juce::dsp::Gain<float> makeup, outGain;
    DeEsser deesser;

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delay { 48000 };
    juce::dsp::StateVariableTPTFilter<float> delayTone;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delaySamples, delayMix;
    float delayFeedback = 0.3f;

    double currentSampleRate = 44100.0;
    int currentProgram = 0;
    static constexpr const char* currentProgramProperty = "currentProgram";

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalROIDZProcessor)
};
