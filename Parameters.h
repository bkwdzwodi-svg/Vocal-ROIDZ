#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Parameter IDs. Keep these stable once you've saved presets: presets store values by ID.
namespace ids
{
    // 01 Autotune Pro
    inline constexpr const char* tuneOn       = "tune_on";
    inline constexpr const char* tuneKey      = "tune_key";
    inline constexpr const char* tuneScale    = "tune_scale";
    inline constexpr const char* tuneRetune   = "tune_retune";
    inline constexpr const char* tuneHumanize = "tune_humanize";
    inline constexpr const char* tuneAmount   = "tune_amount";
    inline constexpr const char* tuneHard     = "tune_hard";

    // 02 Analog EQ
    inline constexpr const char* eqOn         = "eq_on";
    inline constexpr const char* eqHpf        = "eq_hpf";
    inline constexpr const char* eqLpf        = "eq_lpf";
    inline constexpr const char* eqLow        = "eq_low";
    inline constexpr const char* eqLowFreq    = "eq_lowfreq";
    inline constexpr const char* eqMid        = "eq_mid";
    inline constexpr const char* eqMidFreq    = "eq_midfreq";
    inline constexpr const char* eqHigh       = "eq_high";
    inline constexpr const char* eqAir        = "eq_air";

    // 03 Compressor X
    inline constexpr const char* compOn       = "comp_on";
    inline constexpr const char* compModel    = "comp_model";
    inline constexpr const char* compThresh   = "comp_thresh";
    inline constexpr const char* compRatio    = "comp_ratio";
    inline constexpr const char* compAttack   = "comp_attack";
    inline constexpr const char* compRelease  = "comp_release";
    inline constexpr const char* compKnee     = "comp_knee";
    inline constexpr const char* compMakeup   = "comp_makeup";
    inline constexpr const char* compMix      = "comp_mix";

    // 04 Noise Gate Pro
    inline constexpr const char* gateOn       = "gate_on";
    inline constexpr const char* gateThresh   = "gate_thresh";
    inline constexpr const char* gateAttack   = "gate_attack";
    inline constexpr const char* gateHold     = "gate_hold";
    inline constexpr const char* gateRelease  = "gate_release";
    inline constexpr const char* gateRange    = "gate_range";

    // 05 Saturation Engine
    inline constexpr const char* satOn        = "sat_on";
    inline constexpr const char* satMode      = "sat_mode";
    inline constexpr const char* satDrive     = "sat_drive";
    inline constexpr const char* satWarmth    = "sat_warmth";
    inline constexpr const char* satDensity   = "sat_density";
    inline constexpr const char* satMix       = "sat_mix";

    // 06 Vocal Doubler
    inline constexpr const char* dblOn        = "dbl_on";
    inline constexpr const char* dblAmount    = "dbl_amount";
    inline constexpr const char* dblWidth     = "dbl_width";
    inline constexpr const char* dblHuman     = "dbl_human";
    inline constexpr const char* dblPitch     = "dbl_pitch";

    // 07 Harmony Engine
    inline constexpr const char* harmOn       = "harm_on";
    inline constexpr const char* harmThird    = "harm_third";
    inline constexpr const char* harmFifth    = "harm_fifth";
    inline constexpr const char* harmOctUp    = "harm_octup";
    inline constexpr const char* harmOctDown  = "harm_octdown";

    // 08 Reverb Space
    inline constexpr const char* revOn        = "rev_on";
    inline constexpr const char* revAlgo      = "rev_algo";
    inline constexpr const char* revDecay     = "rev_decay";
    inline constexpr const char* revPredelay  = "rev_predelay";
    inline constexpr const char* revWidth     = "rev_width";
    inline constexpr const char* revTone      = "rev_tone";
    inline constexpr const char* revMod       = "rev_mod";
    inline constexpr const char* revMix       = "rev_mix";

    // 09 Delay Lab
    inline constexpr const char* dlyOn        = "dly_on";
    inline constexpr const char* dlyMode      = "dly_mode";
    inline constexpr const char* dlyTime      = "dly_time";
    inline constexpr const char* dlyFb        = "dly_fb";
    inline constexpr const char* dlyTone      = "dly_tone";
    inline constexpr const char* dlyWidth     = "dly_width";
    inline constexpr const char* dlyMix       = "dly_mix";

    // 10 Hyper Stutter FX
    inline constexpr const char* stutOn       = "stut_on";
    inline constexpr const char* stutMode     = "stut_mode";
    inline constexpr const char* stutSlice    = "stut_slice";
    inline constexpr const char* stutRate     = "stut_rate";
    inline constexpr const char* stutReverse  = "stut_reverse";
    inline constexpr const char* stutRandom   = "stut_random";
    inline constexpr const char* stutPitch    = "stut_pitch";
    inline constexpr const char* stutMix      = "stut_mix";

    // 11 Auto Chop-N-Screw
    inline constexpr const char* chopOn       = "chop_on";
    inline constexpr const char* chopMode     = "chop_mode";
    inline constexpr const char* chopPitch    = "chop_pitch";
    inline constexpr const char* chopSlow     = "chop_slow";
    inline constexpr const char* chopDensity  = "chop_density";
    inline constexpr const char* chopRepeat   = "chop_repeat";
    inline constexpr const char* chopDrift    = "chop_drift";
    inline constexpr const char* chopMix      = "chop_mix";

    // 12 Stereo Imager
    inline constexpr const char* stereoOn     = "stereo_on";
    inline constexpr const char* stereoWidth  = "stereo_width";
    inline constexpr const char* stereoBass   = "stereo_bass";
    inline constexpr const char* stereoBalance = "stereo_balance";

    // 13 De-Esser
    inline constexpr const char* deessOn      = "deess_on";
    inline constexpr const char* deessFreq    = "deess_freq";
    inline constexpr const char* deessThresh  = "deess_thresh";
    inline constexpr const char* deessRatio   = "deess_ratio";

    // 14 Noise Removal
    inline constexpr const char* nrOn         = "nr_on";
    inline constexpr const char* nrHum        = "nr_hum";
    inline constexpr const char* nrBreath     = "nr_breath";
    inline constexpr const char* nrClicks     = "nr_clicks";
    inline constexpr const char* nrAmount     = "nr_amount";

    // 15 Creative FX Lab
    inline constexpr const char* fxOn         = "fx_on";
    inline constexpr const char* fxType       = "fx_type";
    inline constexpr const char* fxAmount     = "fx_amount";
    inline constexpr const char* fxTone       = "fx_tone";
    inline constexpr const char* fxMix        = "fx_mix";

    // 16 Master Finish
    inline constexpr const char* masterOn     = "master_on";
    inline constexpr const char* masterLoud   = "master_loud";
    inline constexpr const char* masterPunch  = "master_punch";
    inline constexpr const char* masterWarmth = "master_warmth";
    inline constexpr const char* masterAir    = "master_air";
    inline constexpr const char* masterGlue   = "master_glue";
    inline constexpr const char* masterCeiling = "master_ceiling";

    // Global trim (header / master page)
    inline constexpr const char* outGain      = "out_gain";
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
            NormalisableRange<float> (lo, hi, 0.001f, skew), def,
            AudioParameterFloatAttributes()
                .withLabel (unit)
                .withStringFromValueFunction ([decimals] (float v, int) { return String (v, decimals); })));
    };

    auto addChoice = [&p] (const char* id, const char* name, StringArray items, int def)
    {
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
    };

    // ------------------------------------------------------------------ 01 Autotune Pro
    addBool   (ids::tuneOn,       "Autotune On",      false);
    addChoice (ids::tuneKey,      "Key",              { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0);
    addChoice (ids::tuneScale,    "Scale",            { "Major", "Minor", "Harmonic Minor", "Melodic Minor", "Chromatic",
                                                        "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian",
                                                        "Pentatonic Major", "Pentatonic Minor" }, 0);
    addFloat  (ids::tuneRetune,   "Retune Speed",     0.f, 1.f, 0.45f, 1.f, 2, "");
    addFloat  (ids::tuneHumanize, "Humanize",         0.f, 1.f, 0.25f, 1.f, 2, "");
    addFloat  (ids::tuneAmount,   "Correction",       0.f, 1.f, 0.85f, 1.f, 2, "");
    addBool   (ids::tuneHard,     "Hard Tune",        false);

    // ------------------------------------------------------------------ 02 Analog EQ
    addBool   (ids::eqOn,      "EQ On",     true);
    addFloat  (ids::eqHpf,     "High Pass", 20.f, 400.f, 85.f, 0.45f, 0, "Hz");
    addFloat  (ids::eqLpf,     "Low Pass",  4000.f, 20000.f, 18000.f, 0.5f, 0, "Hz");
    addFloat  (ids::eqLow,     "Low Shelf", -12.f, 12.f, 1.5f, 1.f, 1, "dB");
    addFloat  (ids::eqLowFreq, "Low Freq",  40.f, 400.f, 160.f, 0.5f, 0, "Hz");
    addFloat  (ids::eqMid,     "Mid Band",  -14.f, 14.f, 0.f, 1.f, 1, "dB");
    addFloat  (ids::eqMidFreq, "Mid Freq",  250.f, 6000.f, 2200.f, 0.5f, 0, "Hz");
    addFloat  (ids::eqHigh,    "High Shelf", -12.f, 12.f, 2.f, 1.f, 1, "dB");
    addFloat  (ids::eqAir,     "Air Band",  0.f, 12.f, 2.5f, 1.f, 1, "dB");

    // ------------------------------------------------------------------ 03 Compressor X
    addBool   (ids::compOn,     "Comp On",   true);
    addChoice (ids::compModel,  "Model",     { "VCA", "FET", "Optical", "Tube" }, 0);
    addFloat  (ids::compThresh, "Threshold", -60.f, 0.f, -20.f, 1.f, 1, "dB");
    addFloat  (ids::compRatio,  "Ratio",     1.f, 20.f, 3.5f, 0.5f, 1, ":1");
    addFloat  (ids::compAttack, "Attack",    0.1f, 200.f, 12.f, 0.4f, 1, "ms");
    addFloat  (ids::compRelease,"Release",   10.f, 1200.f, 140.f, 0.4f, 0, "ms");
    addFloat  (ids::compKnee,   "Knee",      0.f, 24.f, 8.f, 1.f, 1, "dB");
    addFloat  (ids::compMakeup, "Makeup",    -12.f, 24.f, 2.f, 1.f, 1, "dB");
    addFloat  (ids::compMix,    "Parallel",  0.f, 1.f, 1.f, 1.f, 2, "");

    // ------------------------------------------------------------------ 04 Noise Gate Pro
    addBool   (ids::gateOn,      "Gate On",   false);
    addFloat  (ids::gateThresh,  "Threshold", -80.f, -10.f, -48.f, 1.f, 1, "dB");
    addFloat  (ids::gateAttack,  "Attack",    0.1f, 100.f, 2.f, 0.4f, 1, "ms");
    addFloat  (ids::gateHold,    "Hold",      0.f, 500.f, 60.f, 0.5f, 0, "ms");
    addFloat  (ids::gateRelease, "Release",   5.f, 2000.f, 250.f, 0.4f, 0, "ms");
    addFloat  (ids::gateRange,   "Range",     0.f, 80.f, 60.f, 1.f, 0, "dB");

    // ------------------------------------------------------------------ 05 Saturation Engine
    addBool   (ids::satOn,      "Sat On",    true);
    addChoice (ids::satMode,    "Mode",      { "Tube", "Tape", "Vintage", "Modern", "Warm", "Aggressive" }, 0);
    addFloat  (ids::satDrive,   "Drive",     0.f, 24.f, 4.f, 1.f, 1, "dB");
    addFloat  (ids::satWarmth,  "Warmth",    0.f, 1.f, 0.25f, 1.f, 2, "");
    addFloat  (ids::satDensity, "Density",   0.f, 1.f, 0.3f, 1.f, 2, "");
    addFloat  (ids::satMix,     "Mix",       0.f, 1.f, 0.6f, 1.f, 2, "");

    // ------------------------------------------------------------------ 06 Vocal Doubler
    addBool   (ids::dblOn,      "Doubler On", false);
    addFloat  (ids::dblAmount,  "Amount",     0.f, 1.f, 0.35f, 1.f, 2, "");
    addFloat  (ids::dblWidth,   "Width",      0.f, 1.f, 0.6f, 1.f, 2, "");
    addFloat  (ids::dblHuman,   "Humanize",   0.f, 1.f, 0.5f, 1.f, 2, "");
    addFloat  (ids::dblPitch,   "Pitch Drift", 0.f, 1.f, 0.2f, 1.f, 2, "");

    // ------------------------------------------------------------------ 07 Harmony Engine
    addBool   (ids::harmOn,      "Harmony On", false);
    addFloat  (ids::harmThird,   "3rd",         0.f, 1.f, 0.f, 1.f, 2, "");
    addFloat  (ids::harmFifth,   "5th",         0.f, 1.f, 0.f, 1.f, 2, "");
    addFloat  (ids::harmOctUp,   "Octave Up",   0.f, 1.f, 0.f, 1.f, 2, "");
    addFloat  (ids::harmOctDown, "Octave Down", 0.f, 1.f, 0.f, 1.f, 2, "");

    // ------------------------------------------------------------------ 08 Reverb Space
    addBool   (ids::revOn,       "Reverb On", false);
    addChoice (ids::revAlgo,     "Algorithm", { "Room", "Hall", "Plate", "Arena", "Ambient", "Dream", "Cathedral" }, 1);
    addFloat  (ids::revDecay,    "Decay",     0.f, 1.f, 0.55f, 1.f, 2, "");
    addFloat  (ids::revPredelay, "Pre-Delay", 0.f, 200.f, 20.f, 0.6f, 0, "ms");
    addFloat  (ids::revWidth,    "Width",     0.f, 1.f, 0.8f, 1.f, 2, "");
    addFloat  (ids::revTone,     "Tone",      800.f, 16000.f, 7000.f, 0.5f, 0, "Hz");
    addFloat  (ids::revMod,      "Modulation", 0.f, 1.f, 0.25f, 1.f, 2, "");
    addFloat  (ids::revMix,      "Mix",       0.f, 1.f, 0.2f, 1.f, 2, "");

    // ------------------------------------------------------------------ 09 Delay Lab
    addBool   (ids::dlyOn,      "Delay On", false);
    addChoice (ids::dlyMode,    "Mode",     { "Analog", "Tape", "Ping Pong", "Ducking" }, 0);
    addFloat  (ids::dlyTime,    "Time",     20.f, 1200.f, 300.f, 0.5f, 0, "ms");
    addFloat  (ids::dlyFb,      "Feedback", 0.f, 0.9f, 0.32f, 1.f, 2, "");
    addFloat  (ids::dlyTone,    "Tone",     500.f, 18000.f, 5000.f, 0.4f, 0, "Hz");
    addFloat  (ids::dlyWidth,   "Width",    0.f, 1.f, 0.6f, 1.f, 2, "");
    addFloat  (ids::dlyMix,     "Mix",      0.f, 1.f, 0.18f, 1.f, 2, "");

    // ------------------------------------------------------------------ 10 Hyper Stutter
    addBool   (ids::stutOn,      "Stutter On", false);
    addChoice (ids::stutMode,    "Mode",       { "Micro Chop", "Machine Gun", "Hyperpop", "Reverse Chaos", "Freeze", "Glitch Storm" }, 1);
    addFloat  (ids::stutSlice,   "Slice Size", 15.f, 500.f, 90.f, 0.6f, 0, "ms");
    addFloat  (ids::stutRate,    "Repeat Rate", 1.f, 32.f, 8.f, 0.7f, 1, "Hz");
    addFloat  (ids::stutReverse, "Reverse",    0.f, 1.f, 0.f, 1.f, 2, "");
    addFloat  (ids::stutRandom,  "Randomize",  0.f, 1.f, 0.15f, 1.f, 2, "");
    addFloat  (ids::stutPitch,   "Pitch Move", -12.f, 12.f, 0.f, 1.f, 0, "st");
    addFloat  (ids::stutMix,     "Wet/Dry",    0.f, 1.f, 1.f, 1.f, 2, "");

    // ------------------------------------------------------------------ 11 Auto Chop-N-Screw
    addBool   (ids::chopOn,      "Chop On",    false);
    addChoice (ids::chopMode,    "Mode",       { "Classic Screw", "Texas Heavy", "BKWDZ WODI", "Dark Atmosphere",
                                                 "Trance Screw", "Chaos Screw" }, 2);
    addFloat  (ids::chopPitch,   "Pitch Drop", -12.f, 0.f, -3.f, 1.f, 0, "st");
    addFloat  (ids::chopSlow,    "Slow Amount", 0.f, 1.f, 0.35f, 1.f, 2, "");
    addFloat  (ids::chopDensity, "Chop Density", 0.f, 1.f, 0.35f, 1.f, 2, "");
    addFloat  (ids::chopRepeat,  "Repeat Intensity", 0.f, 1.f, 0.4f, 1.f, 2, "");
    addFloat  (ids::chopDrift,   "Tape Drift", 0.f, 1.f, 0.3f, 1.f, 2, "");
    addFloat  (ids::chopMix,     "Mix",        0.f, 1.f, 0.8f, 1.f, 2, "");

    // ------------------------------------------------------------------ 12 Stereo Imager
    addBool   (ids::stereoOn,     "Stereo On", true);
    addFloat  (ids::stereoWidth,  "Width",     0.f, 2.f, 1.f, 1.f, 2, "");
    addFloat  (ids::stereoBass,   "Bass Mono", 20.f, 400.f, 110.f, 0.5f, 0, "Hz");
    addFloat  (ids::stereoBalance,"Balance",   -1.f, 1.f, 0.f, 1.f, 2, "");

    // ------------------------------------------------------------------ 13 De-Esser
    addBool   (ids::deessOn,     "De-Ess On", true);
    addFloat  (ids::deessFreq,   "Freq",      3000.f, 12000.f, 7000.f, 0.6f, 0, "Hz");
    addFloat  (ids::deessThresh, "Threshold", -60.f, 0.f, -30.f, 1.f, 1, "dB");
    addFloat  (ids::deessRatio,  "Ratio",     1.f, 10.f, 4.f, 0.6f, 1, ":1");

    // ------------------------------------------------------------------ 14 Noise Removal
    addBool   (ids::nrOn,       "Cleanup On", true);
    addFloat  (ids::nrHum,      "Hum Removal", 0.f, 1.f, 0.4f, 1.f, 2, "");
    addFloat  (ids::nrBreath,   "Breath Reduction", 0.f, 1.f, 0.25f, 1.f, 2, "");
    addFloat  (ids::nrClicks,   "Click Removal", 0.f, 1.f, 0.3f, 1.f, 2, "");
    addFloat  (ids::nrAmount,   "Intensity",   0.f, 1.f, 0.5f, 1.f, 2, "");

    // ------------------------------------------------------------------ 15 Creative FX Lab
    addBool   (ids::fxOn,       "FX On",    false);
    addChoice (ids::fxType,     "Type",     { "Reverse FX", "Tape Stop", "Vinyl Stop", "Megaphone",
                                              "Telephone Voice", "Robot Voice", "Bit Crusher", "Granular Echo" }, 0);
    addFloat  (ids::fxAmount,   "Amount",   0.f, 1.f, 0.5f, 1.f, 2, "");
    addFloat  (ids::fxTone,     "Tone",     200.f, 12000.f, 3000.f, 0.5f, 0, "Hz");
    addFloat  (ids::fxMix,      "Mix",      0.f, 1.f, 1.f, 1.f, 2, "");

    // ------------------------------------------------------------------ 16 Master Finish
    addBool   (ids::masterOn,      "Master On", true);
    addFloat  (ids::masterLoud,    "Loudness",  -12.f, 12.f, 1.5f, 1.f, 1, "dB");
    addFloat  (ids::masterPunch,   "Punch",      0.f, 1.f, 0.3f, 1.f, 2, "");
    addFloat  (ids::masterWarmth,  "Warmth",     0.f, 1.f, 0.25f, 1.f, 2, "");
    addFloat  (ids::masterAir,     "Air",        0.f, 1.f, 0.3f, 1.f, 2, "");
    addFloat  (ids::masterGlue,    "Glue",       0.f, 1.f, 0.3f, 1.f, 2, "");
    addFloat  (ids::masterCeiling, "Ceiling",   -3.f, 0.f, -0.3f, 1.f, 1, "dB");
    addFloat  (ids::outGain,       "Output Trim", -24.f, 12.f, 0.f, 1.f, 1, "dB");

    return { p.begin(), p.end() };
}
