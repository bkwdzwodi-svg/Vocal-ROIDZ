#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "Parameters.h"

// The rack layout: one page per module, each with its own colour, graph and controls.
namespace modules
{
    enum class ScopeKind
    {
        spectrum, pitch, compressor, gate, harmonics, stereo,
        decay, echo, slice, speed, sibilance, cleanup, fx, loudness
    };

    struct Control { const char* id; const char* label; };

    struct Module
    {
        const char* name;
        const char* subtitle;
        const char* powerId;
        juce::uint32 colour;
        ScopeKind scope;
        const char* extraToggle;      // optional second switch, e.g. hard tune
        const char* extraToggleName;
        std::vector<Control> controls;
    };

    inline const std::vector<Module>& all()
    {
        static const std::vector<Module> list =
        {
            { "AUTOTUNE PRO", "Pitch tracking and scale-locked correction", ids::tuneOn,
              0xff30e6ff, ScopeKind::pitch, ids::tuneHard, "HARD TUNE",
              { { ids::tuneKey, "KEY" }, { ids::tuneScale, "SCALE" }, { ids::tuneRetune, "RETUNE" },
                { ids::tuneHumanize, "HUMANIZE" }, { ids::tuneAmount, "CORRECTION" } } },

            { "ANALOG EQ", "High pass, shelves, bell and air band", ids::eqOn,
              0xff4fd8ff, ScopeKind::spectrum, nullptr, nullptr,
              { { ids::eqHpf, "HIGH PASS" }, { ids::eqLpf, "LOW PASS" }, { ids::eqLow, "LOW SHELF" },
                { ids::eqLowFreq, "LOW FREQ" }, { ids::eqMid, "MID BAND" }, { ids::eqMidFreq, "MID FREQ" },
                { ids::eqHigh, "HIGH SHELF" }, { ids::eqAir, "AIR" } } },

            { "COMPRESSOR X", "VCA, FET, optical and tube detectors", ids::compOn,
              0xff86ff2b, ScopeKind::compressor, nullptr, nullptr,
              { { ids::compModel, "MODEL" }, { ids::compThresh, "THRESHOLD" }, { ids::compRatio, "RATIO" },
                { ids::compAttack, "ATTACK" }, { ids::compRelease, "RELEASE" }, { ids::compKnee, "KNEE" },
                { ids::compMakeup, "MAKEUP" }, { ids::compMix, "PARALLEL" } } },

            { "NOISE GATE PRO", "Hold, range and a clean close", ids::gateOn,
              0xff30e6ff, ScopeKind::gate, nullptr, nullptr,
              { { ids::gateThresh, "THRESHOLD" }, { ids::gateAttack, "ATTACK" }, { ids::gateHold, "HOLD" },
                { ids::gateRelease, "RELEASE" }, { ids::gateRange, "RANGE" } } },

            { "SATURATION ENGINE", "Six drive flavours, warmth and density", ids::satOn,
              0xffff4d5e, ScopeKind::harmonics, nullptr, nullptr,
              { { ids::satMode, "MODE" }, { ids::satDrive, "DRIVE" }, { ids::satWarmth, "WARMTH" },
                { ids::satDensity, "DENSITY" }, { ids::satMix, "MIX" } } },

            { "VOCAL DOUBLER", "Drifting doubles with human variation", ids::dblOn,
              0xffc65cff, ScopeKind::stereo, nullptr, nullptr,
              { { ids::dblAmount, "AMOUNT" }, { ids::dblWidth, "WIDTH" }, { ids::dblHuman, "HUMANIZE" },
                { ids::dblPitch, "PITCH DRIFT" } } },

            { "HARMONY ENGINE", "Stacked thirds, fifths and octaves", ids::harmOn,
              0xffc65cff, ScopeKind::pitch, nullptr, nullptr,
              { { ids::harmThird, "3RD" }, { ids::harmFifth, "5TH" },
                { ids::harmOctUp, "OCTAVE UP" }, { ids::harmOctDown, "OCTAVE DOWN" } } },

            { "REVERB SPACE", "Seven spaces from room to cathedral", ids::revOn,
              0xffc65cff, ScopeKind::decay, nullptr, nullptr,
              { { ids::revAlgo, "ALGORITHM" }, { ids::revDecay, "DECAY" }, { ids::revPredelay, "PRE-DELAY" },
                { ids::revWidth, "WIDTH" }, { ids::revTone, "TONE" }, { ids::revMod, "MODULATION" },
                { ids::revMix, "MIX" } } },

            { "DELAY LAB", "Analog, tape, ping-pong and ducking", ids::dlyOn,
              0xffc65cff, ScopeKind::echo, nullptr, nullptr,
              { { ids::dlyMode, "MODE" }, { ids::dlyTime, "TIME" }, { ids::dlyFb, "FEEDBACK" },
                { ids::dlyTone, "TONE" }, { ids::dlyWidth, "WIDTH" }, { ids::dlyMix, "MIX" } } },

            { "HYPER STUTTER FX", "Slices, repeats, reverse and freeze", ids::stutOn,
              0xffff8d1f, ScopeKind::slice, nullptr, nullptr,
              { { ids::stutMode, "MODE" }, { ids::stutSlice, "SLICE SIZE" }, { ids::stutRate, "REPEAT RATE" },
                { ids::stutReverse, "REVERSE" }, { ids::stutRandom, "RANDOMIZE" }, { ids::stutPitch, "PITCH MOVE" },
                { ids::stutMix, "WET/DRY" } } },

            { "AUTO CHOP-N-SCREW", "Phrase repeats, slowdowns and tape drift", ids::chopOn,
              0xffff8d1f, ScopeKind::speed, nullptr, nullptr,
              { { ids::chopMode, "MODE" }, { ids::chopPitch, "PITCH DROP" }, { ids::chopSlow, "SLOW AMOUNT" },
                { ids::chopDensity, "CHOP DENSITY" }, { ids::chopRepeat, "REPEAT" },
                { ids::chopDrift, "TAPE DRIFT" }, { ids::chopMix, "MIX" } } },

            { "STEREO IMAGER", "Width, mono bass and balance", ids::stereoOn,
              0xff30e6ff, ScopeKind::stereo, nullptr, nullptr,
              { { ids::stereoWidth, "WIDTH" }, { ids::stereoBass, "BASS MONO" }, { ids::stereoBalance, "BALANCE" } } },

            { "DE-ESSER", "Split-band sibilance control", ids::deessOn,
              0xffff8d1f, ScopeKind::sibilance, nullptr, nullptr,
              { { ids::deessFreq, "FREQ" }, { ids::deessThresh, "THRESHOLD" }, { ids::deessRatio, "RATIO" } } },

            { "NOISE REMOVAL", "Hum, breath and click cleanup", ids::nrOn,
              0xff30e6ff, ScopeKind::cleanup, nullptr, nullptr,
              { { ids::nrHum, "HUM" }, { ids::nrBreath, "BREATH" }, { ids::nrClicks, "CLICKS" },
                { ids::nrAmount, "INTENSITY" } } },

            { "CREATIVE FX LAB", "Eight signature processors", ids::fxOn,
              0xffc65cff, ScopeKind::fx, nullptr, nullptr,
              { { ids::fxType, "TYPE" }, { ids::fxAmount, "AMOUNT" }, { ids::fxTone, "TONE" }, { ids::fxMix, "MIX" } } },

            { "MASTER FINISH", "Punch, warmth, air, glue and limiting", ids::masterOn,
              0xffffc93c, ScopeKind::loudness, nullptr, nullptr,
              { { ids::masterLoud, "LOUDNESS" }, { ids::masterPunch, "PUNCH" }, { ids::masterWarmth, "WARMTH" },
                { ids::masterAir, "AIR" }, { ids::masterGlue, "GLUE" }, { ids::masterCeiling, "CEILING" },
                { ids::outGain, "OUT TRIM" } } },
        };

        return list;
    }
}
