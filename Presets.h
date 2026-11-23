#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include "Parameters.h"

namespace presets
{
    struct Param   { const char* id; float value; };
    struct Factory { const char* category; const char* name; std::vector<Param> params; };

    // Starting points, not gospel: every voice and mic is different, so tweak to taste.
    // apply() resets everything to defaults first, so only changed params are listed.
    inline const std::vector<Factory>& factory()
    {
        static const std::vector<Factory> list =
        {
            // ============================================================ genre presets
            { "Genre Presets", "Melodic Trap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.55f }, { ids::tuneHumanize, 0.35f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 95.f }, { ids::eqLow, 2.f }, { ids::eqMid, 1.f }, { ids::eqMidFreq, 2500.f },
                { ids::eqHigh, 2.5f }, { ids::eqAir, 4.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 8.f },
                { ids::compRelease, 110.f }, { ids::compMakeup, 4.f },
                { ids::satDrive, 5.f }, { ids::satWarmth, 0.3f }, { ids::satMix, 0.45f },
                { ids::deessThresh, -32.f }, { ids::deessRatio, 4.f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.3f }, { ids::dblWidth, 0.7f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.5f },
                { ids::revPredelay, 25.f }, { ids::revWidth, 0.9f }, { ids::revTone, 6500.f }, { ids::revMix, 0.18f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 0.f }, { ids::dlyTime, 280.f },
                { ids::dlyFb, 0.3f }, { ids::dlyTone, 5000.f }, { ids::dlyMix, 0.15f },
                { ids::stereoWidth, 1.15f },
                { ids::masterLoud, 1.5f }, { ids::masterPunch, 0.35f }, { ids::masterAir, 0.4f } } },

            { "Genre Presets", "Dark Trap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 8.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.35f }, { ids::tuneHumanize, 0.2f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 80.f }, { ids::eqLow, 2.5f }, { ids::eqMid, -2.f }, { ids::eqMidFreq, 650.f },
                { ids::eqHigh, -1.f }, { ids::eqAir, 1.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.f }, { ids::compAttack, 6.f }, { ids::compMakeup, 4.5f },
                { ids::satMode, 2.f }, { ids::satDrive, 7.f }, { ids::satWarmth, 0.5f }, { ids::satMix, 0.5f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.65f }, { ids::revTone, 3400.f }, { ids::revMix, 0.24f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 375.f }, { ids::dlyFb, 0.4f },
                { ids::dlyTone, 3000.f }, { ids::dlyMix, 0.2f },
                { ids::masterWarmth, 0.5f }, { ids::masterLoud, 1.f }, { ids::masterPunch, 0.3f } } },

            { "Genre Presets", "Emo Rap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 4.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.3f }, { ids::tuneHumanize, 0.5f }, { ids::tuneAmount, 0.7f },
                { ids::eqHpf, 110.f }, { ids::eqLow, 1.f }, { ids::eqMid, 2.f }, { ids::eqMidFreq, 3000.f }, { ids::eqAir, 3.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.f }, { ids::compAttack, 15.f }, { ids::compMakeup, 3.f },
                { ids::satMode, 1.f }, { ids::satDrive, 4.f }, { ids::satWarmth, 0.35f }, { ids::satMix, 0.4f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.4f }, { ids::dblWidth, 0.8f }, { ids::dblHuman, 0.6f },
                { ids::revOn, 1.f }, { ids::revAlgo, 2.f }, { ids::revDecay, 0.6f }, { ids::revMix, 0.22f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 320.f }, { ids::dlyFb, 0.32f }, { ids::dlyMix, 0.18f },
                { ids::masterPunch, 0.3f }, { ids::masterAir, 0.35f } } },

            { "Genre Presets", "Rage", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 2.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.8f }, { ids::tuneHumanize, 0.1f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 120.f }, { ids::eqLow, 3.f }, { ids::eqMid, 1.5f }, { ids::eqHigh, 3.f }, { ids::eqAir, 5.f },
                { ids::compThresh, -26.f }, { ids::compRatio, 5.f }, { ids::compAttack, 3.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 5.f }, { ids::satDrive, 9.f }, { ids::satMix, 0.45f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.35f }, { ids::dblWidth, 0.9f },
                { ids::revOn, 1.f }, { ids::revAlgo, 3.f }, { ids::revDecay, 0.6f }, { ids::revMix, 0.2f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 250.f }, { ids::dlyMix, 0.16f },
                { ids::masterLoud, 2.f }, { ids::masterPunch, 0.45f }, { ids::masterAir, 0.4f } } },

            { "Genre Presets", "Hyperpop", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 0.85f }, { ids::tuneHumanize, 0.1f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 130.f }, { ids::eqLow, -1.f }, { ids::eqMid, 2.f }, { ids::eqMidFreq, 4000.f },
                { ids::eqHigh, 3.f }, { ids::eqAir, 6.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 5.f }, { ids::compAttack, 2.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 3.f }, { ids::satDrive, 6.f }, { ids::satMix, 0.35f },
                { ids::stutOn, 1.f }, { ids::stutMode, 2.f }, { ids::stutSlice, 75.f }, { ids::stutRate, 10.f },
                { ids::stutRandom, 0.3f }, { ids::stutMix, 0.4f },
                { ids::fxOn, 1.f }, { ids::fxType, 6.f }, { ids::fxAmount, 0.3f }, { ids::fxMix, 0.25f },
                { ids::revOn, 1.f }, { ids::revAlgo, 5.f }, { ids::revDecay, 0.7f }, { ids::revMix, 0.25f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 190.f }, { ids::dlyFb, 0.4f }, { ids::dlyMix, 0.22f },
                { ids::masterLoud, 2.f }, { ids::masterAir, 0.6f }, { ids::masterPunch, 0.5f } } },

            { "Genre Presets", "Drill", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 3.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.6f }, { ids::tuneHumanize, 0.2f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 100.f }, { ids::eqLow, 2.f }, { ids::eqMid, -1.f }, { ids::eqMidFreq, 800.f }, { ids::eqAir, 3.f },
                { ids::compThresh, -26.f }, { ids::compRatio, 5.f }, { ids::compAttack, 4.f }, { ids::compRelease, 80.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 0.f }, { ids::satDrive, 6.f }, { ids::satMix, 0.45f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.25f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.35f }, { ids::revMix, 0.15f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 220.f }, { ids::dlyFb, 0.28f }, { ids::dlyMix, 0.14f },
                { ids::masterPunch, 0.5f }, { ids::masterLoud, 1.5f } } },

            { "Genre Presets", "Alternative Hip-Hop", {
                { ids::tuneOn, 1.f }, { ids::tuneScale, 1.f }, { ids::tuneRetune, 0.25f },
                { ids::tuneHumanize, 0.65f }, { ids::tuneAmount, 0.5f },
                { ids::eqHpf, 90.f }, { ids::eqLow, 2.f }, { ids::eqMid, 1.f }, { ids::eqMidFreq, 1200.f }, { ids::eqHigh, -2.f }, { ids::eqAir, 1.f },
                { ids::compThresh, -18.f }, { ids::compRatio, 2.5f }, { ids::compAttack, 25.f }, { ids::compMakeup, 2.f },
                { ids::satMode, 1.f }, { ids::satDrive, 5.f }, { ids::satWarmth, 0.4f }, { ids::satMix, 0.5f },
                { ids::fxOn, 1.f }, { ids::fxType, 2.f }, { ids::fxAmount, 0.25f }, { ids::fxMix, 0.25f },
                { ids::revOn, 1.f }, { ids::revAlgo, 4.f }, { ids::revDecay, 0.55f }, { ids::revTone, 4200.f }, { ids::revMix, 0.2f },
                { ids::masterWarmth, 0.45f }, { ids::masterPunch, 0.25f } } },

            { "Genre Presets", "Singing Vocals", {
                { ids::tuneOn, 1.f }, { ids::tuneScale, 0.f }, { ids::tuneRetune, 0.3f },
                { ids::tuneHumanize, 0.6f }, { ids::tuneAmount, 0.6f },
                { ids::eqHpf, 85.f }, { ids::eqLow, 1.5f }, { ids::eqMid, 0.5f }, { ids::eqHigh, 2.f }, { ids::eqAir, 3.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.f }, { ids::compAttack, 18.f }, { ids::compRelease, 140.f }, { ids::compMakeup, 3.f },
                { ids::satDrive, 3.f }, { ids::satMix, 0.3f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.3f }, { ids::dblWidth, 0.75f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.55f }, { ids::revPredelay, 30.f }, { ids::revMix, 0.2f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 300.f }, { ids::dlyFb, 0.28f }, { ids::dlyMix, 0.12f },
                { ids::masterPunch, 0.25f }, { ids::masterAir, 0.3f } } },

            { "Genre Presets", "Pop Vocals", {
                { ids::tuneOn, 1.f }, { ids::tuneRetune, 0.45f }, { ids::tuneHumanize, 0.4f }, { ids::tuneAmount, 0.8f },
                { ids::eqHpf, 100.f }, { ids::eqLow, 0.5f }, { ids::eqMid, 1.5f }, { ids::eqMidFreq, 3500.f }, { ids::eqHigh, 3.f }, { ids::eqAir, 5.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 4.f }, { ids::compAttack, 6.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 3.f }, { ids::satDrive, 3.f }, { ids::satMix, 0.25f },
                { ids::deessThresh, -34.f }, { ids::deessRatio, 5.f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.35f }, { ids::dblWidth, 0.9f },
                { ids::revOn, 1.f }, { ids::revAlgo, 2.f }, { ids::revDecay, 0.45f }, { ids::revMix, 0.16f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 260.f }, { ids::dlyMix, 0.14f },
                { ids::masterLoud, 1.5f }, { ids::masterAir, 0.5f }, { ids::masterPunch, 0.35f } } },

            { "Genre Presets", "Podcast Voice", {
                { ids::tuneOn, 0.f },
                { ids::eqHpf, 100.f }, { ids::eqLpf, 16000.f }, { ids::eqLow, -1.f }, { ids::eqMid, 2.f },
                { ids::eqMidFreq, 2600.f }, { ids::eqHigh, 1.f }, { ids::eqAir, 2.f },
                { ids::compThresh, -26.f }, { ids::compRatio, 4.f }, { ids::compAttack, 3.f }, { ids::compRelease, 120.f }, { ids::compMakeup, 6.f },
                { ids::deessFreq, 8000.f }, { ids::deessThresh, -34.f }, { ids::deessRatio, 5.f },
                { ids::gateOn, 1.f }, { ids::gateThresh, -50.f },
                { ids::nrHum, 0.5f }, { ids::nrBreath, 0.4f }, { ids::nrClicks, 0.4f },
                { ids::masterLoud, 2.f }, { ids::masterPunch, 0.2f }, { ids::masterGlue, 0.4f } } },

            { "Genre Presets", "Broadcast Voice", {
                { ids::tuneOn, 0.f },
                { ids::eqHpf, 90.f }, { ids::eqLow, 0.5f }, { ids::eqMid, 1.5f }, { ids::eqMidFreq, 3000.f },
                { ids::eqHigh, 1.5f }, { ids::eqAir, 3.f },
                { ids::compThresh, -30.f }, { ids::compRatio, 6.f }, { ids::compAttack, 2.f }, { ids::compRelease, 90.f }, { ids::compMakeup, 8.f },
                { ids::deessFreq, 8500.f }, { ids::deessThresh, -36.f }, { ids::deessRatio, 6.f },
                { ids::gateOn, 1.f }, { ids::gateThresh, -52.f },
                { ids::nrHum, 0.6f }, { ids::nrBreath, 0.5f }, { ids::nrClicks, 0.5f },
                { ids::satWarmth, 0.25f }, { ids::satMix, 0.2f },
                { ids::masterLoud, 3.f }, { ids::masterGlue, 0.5f }, { ids::masterCeiling, -1.f } } },

            // ========================================================= signature presets
            { "Signature Presets", "BKWDZ Signature", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.5f }, { ids::tuneHumanize, 0.3f }, { ids::tuneAmount, 0.85f },
                { ids::eqHpf, 95.f }, { ids::eqLow, 2.f }, { ids::eqMid, 1.f }, { ids::eqMidFreq, 2800.f },
                { ids::eqHigh, 2.f }, { ids::eqAir, 4.f },
                { ids::compModel, 1.f }, { ids::compThresh, -24.f }, { ids::compRatio, 4.f },
                { ids::compAttack, 5.f }, { ids::compRelease, 90.f }, { ids::compMakeup, 4.5f },
                { ids::satMode, 0.f }, { ids::satDrive, 6.f }, { ids::satWarmth, 0.4f }, { ids::satMix, 0.5f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.35f }, { ids::dblWidth, 0.8f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 290.f }, { ids::dlyFb, 0.32f }, { ids::dlyMix, 0.16f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.55f }, { ids::revMix, 0.2f },
                { ids::masterLoud, 2.f }, { ids::masterPunch, 0.4f }, { ids::masterWarmth, 0.35f },
                { ids::masterAir, 0.4f }, { ids::masterGlue, 0.35f } } },

            { "Signature Presets", "Dream Vocal", {
                { ids::tuneOn, 1.f }, { ids::tuneScale, 1.f }, { ids::tuneRetune, 0.25f },
                { ids::tuneHumanize, 0.55f }, { ids::tuneAmount, 0.6f },
                { ids::eqHpf, 120.f }, { ids::eqLow, -1.f }, { ids::eqHigh, 3.f }, { ids::eqAir, 6.f },
                { ids::compThresh, -18.f }, { ids::compRatio, 2.5f }, { ids::compAttack, 22.f }, { ids::compMakeup, 2.5f },
                { ids::satMode, 4.f }, { ids::satWarmth, 0.5f }, { ids::satMix, 0.35f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.55f }, { ids::dblWidth, 1.f }, { ids::dblHuman, 0.7f },
                { ids::revOn, 1.f }, { ids::revAlgo, 5.f }, { ids::revDecay, 0.85f }, { ids::revPredelay, 35.f },
                { ids::revWidth, 1.f }, { ids::revTone, 5200.f }, { ids::revMod, 0.5f }, { ids::revMix, 0.37f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 420.f }, { ids::dlyFb, 0.42f },
                { ids::dlyTone, 3500.f }, { ids::dlyMix, 0.2f },
                { ids::masterAir, 0.55f }, { ids::masterWarmth, 0.3f }, { ids::masterLoud, 1.f } } },

            { "Signature Presets", "Dark Atmosphere", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 10.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.2f }, { ids::tuneHumanize, 0.4f }, { ids::tuneAmount, 0.8f },
                { ids::eqHpf, 70.f }, { ids::eqLpf, 9000.f }, { ids::eqLow, 3.f }, { ids::eqMid, -3.f },
                { ids::eqMidFreq, 700.f }, { ids::eqHigh, -3.f }, { ids::eqAir, 0.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.f }, { ids::compAttack, 20.f }, { ids::compMakeup, 3.f },
                { ids::satMode, 2.f }, { ids::satDrive, 8.f }, { ids::satWarmth, 0.6f }, { ids::satMix, 0.55f },
                { ids::chopOn, 1.f }, { ids::chopMode, 3.f }, { ids::chopPitch, -4.f }, { ids::chopSlow, 0.45f },
                { ids::chopMix, 0.55f },
                { ids::fxOn, 1.f }, { ids::fxType, 2.f }, { ids::fxAmount, 0.2f }, { ids::fxMix, 0.2f },
                { ids::revOn, 1.f }, { ids::revAlgo, 6.f }, { ids::revDecay, 0.9f }, { ids::revTone, 3200.f }, { ids::revMix, 0.32f },
                { ids::masterWarmth, 0.5f }, { ids::masterAir, 0.1f }, { ids::masterLoud, 0.5f } } },

            { "Signature Presets", "Arena Vocal", {
                { ids::tuneOn, 1.f }, { ids::tuneRetune, 0.5f }, { ids::tuneHumanize, 0.35f }, { ids::tuneAmount, 0.85f },
                { ids::eqHpf, 110.f }, { ids::eqLow, 1.f }, { ids::eqMid, 1.f }, { ids::eqHigh, 3.f }, { ids::eqAir, 5.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.f }, { ids::compAttack, 6.f }, { ids::compMakeup, 4.f },
                { ids::satDrive, 4.f }, { ids::satMix, 0.35f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.45f }, { ids::dblWidth, 1.f },
                { ids::revOn, 1.f }, { ids::revAlgo, 3.f }, { ids::revDecay, 0.75f }, { ids::revPredelay, 45.f },
                { ids::revWidth, 1.f }, { ids::revMix, 0.3f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 480.f }, { ids::dlyFb, 0.38f }, { ids::dlyMix, 0.2f },
                { ids::masterLoud, 2.f }, { ids::masterPunch, 0.45f }, { ids::masterAir, 0.5f } } },

            { "Signature Presets", "Heavy Auto-Tune", {
                { ids::tuneOn, 1.f }, { ids::tuneScale, 1.f }, { ids::tuneRetune, 1.f },
                { ids::tuneHumanize, 0.f }, { ids::tuneAmount, 1.f }, { ids::tuneHard, 1.f },
                { ids::eqHpf, 110.f }, { ids::eqLow, 2.f }, { ids::eqMid, 2.f }, { ids::eqHigh, 3.f }, { ids::eqAir, 4.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 5.f }, { ids::compAttack, 3.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 3.f }, { ids::satDrive, 5.f }, { ids::satMix, 0.35f },
                { ids::deessThresh, -34.f }, { ids::deessRatio, 5.f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.5f }, { ids::revMix, 0.16f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 240.f }, { ids::dlyMix, 0.14f },
                { ids::masterLoud, 2.f }, { ids::masterPunch, 0.4f } } },

            { "Signature Presets", "Natural Auto-Tune", {
                { ids::tuneOn, 1.f }, { ids::tuneScale, 0.f }, { ids::tuneRetune, 0.25f },
                { ids::tuneHumanize, 0.75f }, { ids::tuneAmount, 0.45f },
                { ids::eqHpf, 85.f }, { ids::eqLow, 1.f }, { ids::eqHigh, 1.5f }, { ids::eqAir, 2.f },
                { ids::compThresh, -16.f }, { ids::compRatio, 2.f }, { ids::compAttack, 25.f }, { ids::compMakeup, 2.f },
                { ids::deessThresh, -28.f }, { ids::deessRatio, 2.5f },
                { ids::satWarmth, 0.2f }, { ids::satMix, 0.2f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.3f }, { ids::revMix, 0.1f },
                { ids::masterLoud, 0.5f }, { ids::masterGlue, 0.25f } } },

            { "Signature Presets", "Texas Screw Vocal", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 5.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.35f }, { ids::tuneHumanize, 0.3f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 70.f }, { ids::eqLow, 3.f }, { ids::eqMid, -1.f }, { ids::eqMidFreq, 900.f }, { ids::eqAir, 0.5f },
                { ids::compThresh, -26.f }, { ids::compRatio, 5.f }, { ids::compAttack, 10.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 1.f }, { ids::satDrive, 7.f }, { ids::satWarmth, 0.55f }, { ids::satMix, 0.55f },
                { ids::chopOn, 1.f }, { ids::chopMode, 1.f }, { ids::chopPitch, -3.f }, { ids::chopSlow, 0.5f },
                { ids::chopDensity, 0.5f }, { ids::chopRepeat, 0.6f }, { ids::chopDrift, 0.6f }, { ids::chopMix, 0.85f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 400.f }, { ids::dlyFb, 0.35f },
                { ids::dlyTone, 2600.f }, { ids::dlyMix, 0.18f },
                { ids::masterWarmth, 0.55f }, { ids::masterLoud, 1.f } } },

            { "Signature Presets", "Hyper Stutter Trap", {
                { ids::tuneOn, 1.f }, { ids::tuneRetune, 0.75f }, { ids::tuneHumanize, 0.15f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 120.f }, { ids::eqLow, 1.f }, { ids::eqMid, 1.5f }, { ids::eqAir, 5.f },
                { ids::compThresh, -26.f }, { ids::compRatio, 5.f }, { ids::compAttack, 4.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 5.f }, { ids::satDrive, 6.f }, { ids::satMix, 0.35f },
                { ids::stutOn, 1.f }, { ids::stutMode, 1.f }, { ids::stutSlice, 70.f }, { ids::stutRate, 12.f },
                { ids::stutRandom, 0.4f }, { ids::stutMix, 0.55f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 180.f }, { ids::dlyFb, 0.42f }, { ids::dlyMix, 0.2f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.3f }, { ids::revMix, 0.12f },
                { ids::masterLoud, 2.f }, { ids::masterPunch, 0.5f }, { ids::masterAir, 0.45f } } },
        };
        return list;
    }

    inline void apply (juce::AudioProcessorValueTreeState& apvts, const std::vector<Param>& values)
    {
        for (auto* p : apvts.processor.getParameters())
            p->setValueNotifyingHost (p->getDefaultValue());

        for (auto& v : values)
            if (auto* param = apvts.getParameter (v.id))
                param->setValueNotifyingHost (param->convertTo0to1 (v.value));
    }

    // ---- your own presets, saved as .vrpreset files ----

    inline juce::File userFolder()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("VocalROIDZ").getChildFile ("Presets");
        dir.createDirectory();
        return dir;
    }

    inline juce::Array<juce::File> listUser()
    {
        auto files = userFolder().findChildFiles (juce::File::findFiles, false, "*.vrpreset");
        std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
                   { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
        return files;
    }

    inline bool saveUser (juce::AudioProcessorValueTreeState& apvts, const juce::String& name)
    {
        auto xml = apvts.copyState().createXml();
        if (xml == nullptr)
            return false;

        return xml->writeTo (userFolder().getChildFile (juce::File::createLegalFileName (name) + ".vrpreset"));
    }

    inline bool loadUser (juce::AudioProcessorValueTreeState& apvts, const juce::File& file)
    {
        auto xml = juce::XmlDocument::parse (file);
        if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
            return false;

        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        return true;
    }
}
