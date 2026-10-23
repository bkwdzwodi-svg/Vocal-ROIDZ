#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Parameter IDs. Keep these stable once you've saved presets: presets store values by ID.
namespace ids
{
    inline constexpr const char* hpfOn       = "hpf_on";
    inline constexpr const char* hpfFreq     = "hpf_freq";

    inline constexpr const char* compOn      = "comp_on";
    inline constexpr const char* compThresh  = "comp_thresh";
    inline constexpr const char* compRatio   = "comp_ratio";
    inline constexpr const char* compAttack  = "comp_attack";
    inline constexpr const char* compRelease = "comp_release";
    inline constexpr const char* compMakeup  = "comp_makeup";

    inline constexpr const char* deessOn     = "deess_on";
    inline constexpr const char* deessFreq   = "deess_freq";
    inline constexpr const char* deessThresh = "deess_thresh";
    inline constexpr const char* deessRatio  = "deess_ratio";

    inline constexpr const char* delayOn     = "delay_on";
    inline constexpr const char* delayTime   = "delay_time";
    inline constexpr const char* delayFb     = "delay_fb";
    inline constexpr const char* delayTone   = "delay_tone";
    inline constexpr const char* delayMix    = "delay_mix";

    inline constexpr const char* outGain     = "out_gain";
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    auto addBool = [&p] (const char* id, const char* name, bool def)
    {
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    };

    auto addFloat = [&p] (const char* id, const char* name, float lo, float hi, float def,
                          float skew, int decimals, const char* unit)
    {
        p.push_back (std::make_unique<AudioParameterFloat> (
            ParameterID { id, 1 }, name,
            NormalisableRange<float> (lo, hi, 0.01f, skew), def,
            AudioParameterFloatAttributes()
                .withLabel (unit)
                .withStringFromValueFunction ([decimals] (float v, int) { return String (v, decimals); })));
    };

    // High-pass
    addBool  (ids::hpfOn,       "HPF On",        true);
    addFloat (ids::hpfFreq,     "HPF Freq",      20.f, 400.f, 90.f, 0.4f, 0, "Hz");

    // Compressor
    addBool  (ids::compOn,      "Comp On",       true);
    addFloat (ids::compThresh,  "Comp Thresh",   -60.f, 0.f, -18.f, 1.0f, 1, "dB");
    addFloat (ids::compRatio,   "Comp Ratio",    1.f, 20.f, 3.5f, 0.5f, 1, ":1");
    addFloat (ids::compAttack,  "Comp Attack",   0.1f, 100.f, 12.f, 0.4f, 1, "ms");
    addFloat (ids::compRelease, "Comp Release",  10.f, 1000.f, 120.f, 0.4f, 0, "ms");
    addFloat (ids::compMakeup,  "Comp Makeup",   -12.f, 24.f, 2.f, 1.0f, 1, "dB");

    // De-esser
    addBool  (ids::deessOn,     "De-Ess On",     true);
    addFloat (ids::deessFreq,   "De-Ess Freq",   3000.f, 12000.f, 7000.f, 0.6f, 0, "Hz");
    addFloat (ids::deessThresh, "De-Ess Thresh", -60.f, 0.f, -28.f, 1.0f, 1, "dB");
    addFloat (ids::deessRatio,  "De-Ess Ratio",  1.f, 10.f, 4.f, 0.6f, 1, ":1");

    // Delay
    addBool  (ids::delayOn,     "Delay On",      false);
    addFloat (ids::delayTime,   "Delay Time",    20.f, 1000.f, 250.f, 0.5f, 0, "ms");
    addFloat (ids::delayFb,     "Delay Feedback", 0.f, 0.9f, 0.3f, 1.0f, 2, "");
    addFloat (ids::delayTone,   "Delay Tone",    500.f, 16000.f, 5000.f, 0.4f, 0, "Hz");
    addFloat (ids::delayMix,    "Delay Mix",     0.f, 1.f, 0.15f, 1.0f, 2, "");

    // Output
    addFloat (ids::outGain,     "Output Gain",   -24.f, 12.f, 0.f, 1.0f, 1, "dB");

    return { p.begin(), p.end() };
}
