#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "Parameters.h"

// The rack layout: one page per module, each with its own colour, family, graph and controls.
namespace modules
{
    enum class ScopeKind
    {
        spectrum, pitch, compressor, gate, harmonics, stereo,
        decay, echo, slice, speed, sibilance, cleanup, fx, loudness,
        pitchfx, stutter
    };

    struct Control { const char* id; const char* label; };

    struct Module
    {
        const char* name;
        const char* subtitle;
        const char* family;           // section chip: PITCH / TONE / DYNAMICS / SPACE / CREATIVE / MASTER
        const char* powerId;
        juce::uint32 colour;
        ScopeKind scope;
        std::vector<Control> switches;  // extra on/off switches, e.g. hard tune, formant preserve
        std::vector<Control> controls;
    };

    inline const std::vector<Module>& all()
    {
        static const std::vector<Module> list =
        {
            { "AUTOTUNE PRO", "Scale-locked correction, tone untouched", "PITCH", ids::tuneOn,
              0xff30e6ff, ScopeKind::pitch,
              { { ids::tuneHard, "HARD TUNE" }, { ids::tuneFormant, "FORMANT PRESERVE" } },
              { { ids::tuneKey, "KEY" }, { ids::tuneScale, "SCALE" }, { ids::tuneRetune, "RETUNE" },
                { ids::tuneHumanize, "HUMANIZE" }, { ids::tuneAmount, "CORRECTION" } } },

            { "PITCH FX", "Creative pitch, formant and octave moves", "PITCH", ids::pitchFxOn,
              0xff2ec4ff, ScopeKind::pitchfx,
              {},
              { { ids::pitchFxSemis, "PITCH" }, { ids::pitchFxFormant, "FORMANT SHIFT" },
                { ids::pitchFxMix, "MIX" } } },

            { "ANALOG EQ", "Draggable bands, bell Q and air shelf", "TONE", ids::eqOn,
              0xff4fd8ff, ScopeKind::spectrum,
              {},
              { { ids::eqHpf, "HIGH PASS" }, { ids::eqLpf, "LOW PASS" }, { ids::eqLow, "LOW SHELF" },
                { ids::eqLowFreq, "LOW FREQ" }, { ids::eqMid, "MID BAND" }, { ids::eqMidFreq, "MID FREQ" },
                { ids::eqMidQ, "MID Q" }, { ids::eqHigh, "HIGH SHELF" }, { ids::eqHighFreq, "HIGH FREQ" },
                { ids::eqAir, "AIR" } } },

            { "COMPRESSOR X", "VCA, FET, optical and tube detectors", "DYNAMICS", ids::compOn,
              0xff86ff2b, ScopeKind::compressor,
              {},
              { { ids::compModel, "MODEL" }, { ids::compThresh, "THRESHOLD" }, { ids::compRatio, "RATIO" },
                { ids::compAttack, "ATTACK" }, { ids::compRelease, "RELEASE" }, { ids::compKnee, "KNEE" },
                { ids::compMakeup, "MAKEUP" }, { ids::compMix, "PARALLEL" } } },

            { "NOISE GATE PRO", "Hold, range and a clean close", "DYNAMICS", ids::gateOn,
              0xff30e6ff, ScopeKind::gate,
              {},
              { { ids::gateThresh, "THRESHOLD" }, { ids::gateAttack, "ATTACK" }, { ids::gateHold, "HOLD" },
                { ids::gateRelease, "RELEASE" }, { ids::gateRange, "RANGE" } } },

            { "SATURATION ENGINE", "Six drive flavours, warmth and density", "TONE", ids::satOn,
              0xffff4d5e, ScopeKind::harmonics,
              {},
              { { ids::satMode, "MODE" }, { ids::satDrive, "DRIVE" }, { ids::satWarmth, "WARMTH" },
                { ids::satDensity, "DENSITY" }, { ids::satMix, "MIX" } } },

            { "VOCAL DOUBLER", "Drifting doubles with human variation", "SPACE", ids::dblOn,
              0xffc65cff, ScopeKind::stereo,
              {},
              { { ids::dblAmount, "AMOUNT" }, { ids::dblWidth, "WIDTH" }, { ids::dblHuman, "HUMANIZE" },
                { ids::dblPitch, "PITCH DRIFT" } } },

            { "HARMONY ENGINE", "Stacked thirds, fifths and octaves", "PITCH", ids::harmOn,
              0xffc65cff, ScopeKind::pitch,
              {},
              { { ids::harmThird, "3RD" }, { ids::harmFifth, "5TH" },
                { ids::harmOctUp, "OCTAVE UP" }, { ids::harmOctDown, "OCTAVE DOWN" } } },

            { "REVERB SPACE", "Seven spaces from room to cathedral", "SPACE", ids::revOn,
              0xffc65cff, ScopeKind::decay,
              {},
              { { ids::revAlgo, "ALGORITHM" }, { ids::revDecay, "DECAY" }, { ids::revPredelay, "PRE-DELAY" },
                { ids::revWidth, "WIDTH" }, { ids::revTone, "TONE" }, { ids::revMod, "MODULATION" },
                { ids::revMix, "MIX" } } },

            { "DELAY LAB", "Analog, tape, ping-pong and ducking", "SPACE", ids::dlyOn,
              0xffc65cff, ScopeKind::echo,
              {},
              { { ids::dlyMode, "MODE" }, { ids::dlyTime, "TIME" }, { ids::dlyFb, "FEEDBACK" },
                { ids::dlyTone, "TONE" }, { ids::dlyWidth, "WIDTH" }, { ids::dlyMix, "MIX" } } },

            { "REGULAR STUTTER", "Clean, repeating slices on a timer", "CREATIVE", ids::rstutOn,
              0xffff8d1f, ScopeKind::stutter,
              {},
              { { ids::rstutPeriod, "TRIGGER EVERY" }, { ids::rstutSlice, "SLICE" },
                { ids::rstutReps, "REPEATS" }, { ids::rstutMix, "MIX" } } },

            { "HYPER STUTTER FX", "Slices, repeats, reverse and freeze", "CREATIVE", ids::stutOn,
              0xffff8d1f, ScopeKind::slice,
              {},
              { { ids::stutMode, "MODE" }, { ids::stutSlice, "SLICE SIZE" }, { ids::stutRate, "REPEAT RATE" },
                { ids::stutReverse, "REVERSE" }, { ids::stutRandom, "RANDOMIZE" }, { ids::stutPitch, "PITCH MOVE" },
                { ids::stutMix, "WET/DRY" } } },

            { "AUTO CHOP-N-SCREW", "Phrase repeats, slowdowns and tape drift", "CREATIVE", ids::chopOn,
              0xffff8d1f, ScopeKind::speed,
              {},
              { { ids::chopMode, "MODE" }, { ids::chopPitch, "PITCH DROP" }, { ids::chopSlow, "SLOW AMOUNT" },
                { ids::chopDensity, "CHOP DENSITY" }, { ids::chopRepeat, "REPEAT" },
                { ids::chopDrift, "TAPE DRIFT" }, { ids::chopMix, "MIX" } } },

            { "STEREO IMAGER", "Width, mono bass and balance", "SPACE", ids::stereoOn,
              0xff30e6ff, ScopeKind::stereo,
              {},
              { { ids::stereoWidth, "WIDTH" }, { ids::stereoBass, "BASS MONO" }, { ids::stereoBalance, "BALANCE" } } },

            { "DE-ESSER", "Split-band sibilance control", "DYNAMICS", ids::deessOn,
              0xffff8d1f, ScopeKind::sibilance,
              {},
              { { ids::deessFreq, "FREQ" }, { ids::deessThresh, "THRESHOLD" }, { ids::deessRatio, "RATIO" } } },

            { "NOISE REMOVAL", "Hum, breath and click cleanup", "DYNAMICS", ids::nrOn,
              0xff30e6ff, ScopeKind::cleanup,
              {},
              { { ids::nrHum, "HUM" }, { ids::nrBreath, "BREATH" }, { ids::nrClicks, "CLICKS" },
                { ids::nrAmount, "INTENSITY" } } },

            { "CREATIVE FX LAB", "Eight signature processors", "CREATIVE", ids::fxOn,
              0xffc65cff, ScopeKind::fx,
              {},
              { { ids::fxType, "TYPE" }, { ids::fxAmount, "AMOUNT" }, { ids::fxTone, "TONE" }, { ids::fxMix, "MIX" } } },

            { "MASTER FINISH", "Punch, warmth, air, glue and limiting", "MASTER", ids::masterOn,
              0xffffc93c, ScopeKind::loudness,
              {},
              { { ids::masterLoud, "LOUDNESS" }, { ids::masterPunch, "PUNCH" }, { ids::masterWarmth, "WARMTH" },
                { ids::masterAir, "AIR" }, { ids::masterGlue, "GLUE" }, { ids::masterCeiling, "CEILING" },
                { ids::outGain, "OUT TRIM" } } },
        };

        return list;
    }

    // Index of the page that owns a module, used by the FX rack to jump to a page.
    inline int indexOf (const char* powerId)
    {
        const auto& list = all();

        for (int i = 0; i < (int) list.size(); ++i)
            if (juce::String (list[(size_t) i].powerId) == juce::String (powerId))
                return i;

        return 0;
    }
}
