#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "Parameters.h"

// The workstation is nine sections: eight focused pages (autotune, pitch, EQ, compressor,
// saturation, reverb, limit, gate) and an FX rack that holds everything else.
namespace modules
{
    enum class ScopeKind
    {
        spectrum, pitch, compressor, gate, harmonics, stereo,
        decay, echo, slice, speed, sibilance, cleanup, fx, loudness,
        pitchfx, stutter, convolution, cassette, spin
    };

    struct Control { const char* id; const char* label; };

    struct Module
    {
        const char* name;
        const char* subtitle;
        const char* tag;              // section chip: PITCH / TONE / DYNAMICS / SPACE / MASTER / FX
        const char* powerId;
        juce::uint32 colour;
        ScopeKind scope;
        std::vector<Control> switches;  // extra on/off switches beside POWER
        std::vector<Control> controls;
    };

    // The eight section pages, in signal order, numbered 01..08.
    inline const std::vector<Module>& all()
    {
        static const std::vector<Module> list =
        {
            { "AUTOTUNE", "Classic hard tune, always snapped, tone untouched", "PITCH", ids::tuneOn,
              0xff30e6ff, ScopeKind::pitch,
              {},
              { { ids::tuneKey, "KEY" }, { ids::tuneScale, "SCALE" }, { ids::tuneRetune, "RETUNE" },
                { ids::tuneHumanize, "HUMANIZE" }, { ids::tuneAmount, "CORRECTION" } } },

            { "PITCH", "Creative transpose, formants and octaves", "PITCH FX", ids::pitchFxOn,
              0xff2ec4ff, ScopeKind::pitchfx,
              {},
              { { ids::pitchFxSemis, "PITCH" }, { ids::pitchFxFormant, "FORMANT SHIFT" },
                { ids::pitchFxMix, "MIX" } } },

            { "EQ", "Seven bands, dual bells, live spectrum and precise drag", "TONE", ids::eqOn,
              0xff4fd8ff, ScopeKind::spectrum,
              {},
              { { ids::eqHpf, "HIGH PASS" }, { ids::eqLpf, "LOW PASS" }, { ids::eqLow, "LOW SHELF" },
                { ids::eqLowFreq, "LOW FREQ" }, { ids::eqMid, "MID LOW" }, { ids::eqMidFreq, "MID LOW FREQ" },
                { ids::eqMidQ, "MID LOW Q" }, { ids::eqMid2, "MID HIGH" }, { ids::eqMid2Freq, "MID HIGH FREQ" },
                { ids::eqMid2Q, "MID HIGH Q" }, { ids::eqHigh, "HIGH SHELF" },
                { ids::eqHighFreq, "HIGH FREQ" }, { ids::eqAir, "AIR" } } },

            { "COMPRESSOR", "VCA, FET, optical and tube detectors", "DYNAMICS", ids::compOn,
              0xff86ff2b, ScopeKind::compressor,
              {},
              { { ids::compModel, "MODEL" }, { ids::compThresh, "THRESHOLD" }, { ids::compRatio, "RATIO" },
                { ids::compAttack, "ATTACK" }, { ids::compRelease, "RELEASE" }, { ids::compKnee, "KNEE" },
                { ids::compMakeup, "MAKEUP" }, { ids::compMix, "PARALLEL" } } },

            { "SATURATION", "Six drive flavours, warmth and density", "TONE", ids::satOn,
              0xffff4d5e, ScopeKind::harmonics,
              {},
              { { ids::satMode, "MODE" }, { ids::satDrive, "DRIVE" }, { ids::satWarmth, "WARMTH" },
                { ids::satDensity, "DENSITY" }, { ids::satMix, "MIX" } } },

            { "REVERB", "Seven spaces from room to cathedral", "SPACE", ids::revOn,
              0xffc65cff, ScopeKind::decay,
              {},
              { { ids::revAlgo, "ALGORITHM" }, { ids::revDecay, "DECAY" }, { ids::revPredelay, "PRE-DELAY" },
                { ids::revWidth, "WIDTH" }, { ids::revTone, "TONE" }, { ids::revMod, "MODULATION" },
                { ids::revMix, "MIX" } } },

            { "LIMIT", "Punch, warmth, air, glue and ceiling", "MASTER", ids::masterOn,
              0xffffc93c, ScopeKind::loudness,
              {},
              { { ids::masterLoud, "LOUDNESS" }, { ids::masterPunch, "PUNCH" }, { ids::masterWarmth, "WARMTH" },
                { ids::masterAir, "AIR" }, { ids::masterGlue, "GLUE" }, { ids::masterCeiling, "CEILING" },
                { ids::outGain, "OUT TRIM" } } },

            { "GATE", "Hold, range and a clean close", "DYNAMICS", ids::gateOn,
              0xff30e6ff, ScopeKind::gate,
              {},
              { { ids::gateThresh, "THRESHOLD" }, { ids::gateAttack, "ATTACK" }, { ids::gateHold, "HOLD" },
                { ids::gateRelease, "RELEASE" }, { ids::gateRange, "RANGE" } } },
        };

        return list;
    }

    // Everything that is not one of the eight sections lives in the FX rack, in chain order.
    inline const std::vector<Module>& fx()
    {
        static const std::vector<Module> list =
        {
            { "HARMONY", "Stacked thirds, fifths and octaves", "FX", ids::harmOn,
              0xffc65cff, ScopeKind::pitch,
              {},
              { { ids::harmThird, "3RD" }, { ids::harmFifth, "5TH" },
                { ids::harmOctUp, "OCTAVE UP" }, { ids::harmOctDown, "OCTAVE DOWN" } } },

            { "NOISE REMOVAL", "Hum, breath and click cleanup", "FX", ids::nrOn,
              0xff30e6ff, ScopeKind::cleanup,
              {},
              { { ids::nrHum, "HUM" }, { ids::nrBreath, "BREATH" }, { ids::nrClicks, "CLICKS" },
                { ids::nrAmount, "INTENSITY" } } },

            { "DE-ESSER", "Split-band sibilance control", "FX", ids::deessOn,
              0xffff8d1f, ScopeKind::sibilance,
              {},
              { { ids::deessFreq, "FREQ" }, { ids::deessThresh, "THRESHOLD" }, { ids::deessRatio, "RATIO" } } },

            { "REGULAR STUTTER", "Clean slices repeated on a timer", "FX", ids::rstutOn,
              0xffff8d1f, ScopeKind::stutter,
              {},
              { { ids::rstutPeriod, "TRIGGER EVERY" }, { ids::rstutSlice, "SLICE" },
                { ids::rstutReps, "REPEATS" }, { ids::rstutMix, "MIX" } } },

            { "CHOP-N-SCREW", "Slowdowns, phrase repeats, tape drift", "FX", ids::chopOn,
              0xffff8d1f, ScopeKind::speed,
              {},
              { { ids::chopMode, "MODE" }, { ids::chopPitch, "PITCH DROP" }, { ids::chopSlow, "SLOW AMOUNT" },
                { ids::chopDensity, "CHOP DENSITY" }, { ids::chopRepeat, "REPEAT" },
                { ids::chopDrift, "TAPE DRIFT" }, { ids::chopMix, "MIX" } } },

            { "HYPER STUTTER", "Slices, repeats, reverse and freeze", "FX", ids::stutOn,
              0xffff8d1f, ScopeKind::slice,
              {},
              { { ids::stutMode, "MODE" }, { ids::stutSlice, "SLICE SIZE" }, { ids::stutRate, "REPEAT RATE" },
                { ids::stutReverse, "REVERSE" }, { ids::stutRandom, "RANDOMIZE" }, { ids::stutPitch, "PITCH MOVE" },
                { ids::stutMix, "WET/DRY" } } },

            { "DOUBLER", "Drifting doubles with human movement", "FX", ids::dblOn,
              0xffc65cff, ScopeKind::stereo,
              {},
              { { ids::dblAmount, "AMOUNT" }, { ids::dblWidth, "WIDTH" }, { ids::dblHuman, "HUMANIZE" },
                { ids::dblPitch, "PITCH DRIFT" } } },

            { "CONVOLVER", "Real impulse responses, four spaces", "FX", ids::convOn,
              0xff30e6ff, ScopeKind::convolution,
              {},
              { { ids::convAlgo, "SPACE" }, { ids::convPredelay, "PRE-DELAY" }, { ids::convTone, "TONE" },
                { ids::convWidth, "WIDTH" }, { ids::convMix, "MIX" } } },

            { "CASSETTE", "Wow, flutter, grit and tape hiss", "FX", ids::casOn,
              0xffff8d1f, ScopeKind::cassette,
              {},
              { { ids::casDrive, "DRIVE" }, { ids::casWow, "WOW" }, { ids::casTone, "TONE" },
                { ids::casHiss, "HISS" }, { ids::casMix, "MIX" } } },

            { "DJ SPIN", "Backspins and brake stops on a timer", "FX", ids::spinOn,
              0xffffc93c, ScopeKind::spin,
              {},
              { { ids::spinRate, "EVERY" }, { ids::spinLength, "LENGTH" }, { ids::spinDepth, "BACKSPIN" },
                { ids::spinMix, "MIX" } } },

            { "STEREO", "Width, mono bass and balance", "FX", ids::stereoOn,
              0xff30e6ff, ScopeKind::stereo,
              {},
              { { ids::stereoWidth, "WIDTH" }, { ids::stereoBass, "BASS MONO" }, { ids::stereoBalance, "BALANCE" } } },

            { "DELAY", "Analog, tape, ping-pong and ducking", "FX", ids::dlyOn,
              0xffc65cff, ScopeKind::echo,
              {},
              { { ids::dlyMode, "MODE" }, { ids::dlyTime, "TIME" }, { ids::dlyFb, "FEEDBACK" },
                { ids::dlyTone, "TONE" }, { ids::dlyWidth, "WIDTH" }, { ids::dlyMix, "MIX" } } },
        };

        return list;
    }
}
