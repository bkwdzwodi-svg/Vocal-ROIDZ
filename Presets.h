#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <initializer_list>
#include <vector>
#include "Parameters.h"

namespace presets
{
    struct Param   { const char* id; float value; };
    struct Factory { const char* category; const char* name; std::vector<Param> params; };

    // Every preset starts from this flat chain: no reverb, no delay, no gate, neutral EQ.
    // Presets then only add the moves that give them their character.
    inline std::vector<Param> base()
    {
        return {
            { ids::tuneOn, 0.f }, { ids::pitchFxOn, 0.f }, { ids::harmOn, 0.f },
            { ids::gateOn, 0.f }, { ids::revOn, 0.f }, { ids::dlyOn, 0.f },
            { ids::dblOn, 0.f }, { ids::stutOn, 0.f }, { ids::chopOn, 0.f },
            { ids::rstutOn, 0.f }, { ids::convOn, 0.f }, { ids::casOn, 0.f }, { ids::spinOn, 0.f },
            { ids::nrOn, 1.f }, { ids::deessOn, 1.f }, { ids::eqOn, 1.f }, { ids::compOn, 1.f },
            { ids::satOn, 1.f }, { ids::stereoOn, 1.f }, { ids::masterOn, 1.f },

            { ids::eqHpf, 90.f }, { ids::eqLpf, 20000.f },
            { ids::eqLow, 0.f }, { ids::eqLowFreq, 150.f },
            { ids::eqMid, 0.f }, { ids::eqMidFreq, 2000.f }, { ids::eqMidQ, 0.8f },
            { ids::eqMid2, 0.f }, { ids::eqMid2Freq, 3200.f }, { ids::eqMid2Q, 1.0f },
            { ids::eqHigh, 0.f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 0.f },

            { ids::compModel, 0.f }, { ids::compThresh, -20.f }, { ids::compRatio, 3.f },
            { ids::compAttack, 10.f }, { ids::compRelease, 120.f }, { ids::compKnee, 8.f },
            { ids::compMakeup, 3.f }, { ids::compMix, 1.f },

            { ids::satMode, 0.f }, { ids::satDrive, 3.f }, { ids::satWarmth, 0.2f },
            { ids::satDensity, 0.25f }, { ids::satMix, 0.25f },

            { ids::deessFreq, 7200.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 4.f },

            { ids::nrHum, 0.3f }, { ids::nrBreath, 0.15f }, { ids::nrClicks, 0.2f }, { ids::nrAmount, 0.35f },

            { ids::stereoWidth, 1.f }, { ids::stereoBass, 110.f }, { ids::stereoBalance, 0.f },

            { ids::masterLoud, 1.f }, { ids::masterPunch, 0.25f }, { ids::masterWarmth, 0.2f },
            { ids::masterAir, 0.3f }, { ids::masterGlue, 0.3f }, { ids::masterCeiling, -1.f },
        };
    }

    inline Factory make (const char* category, const char* name, std::initializer_list<Param> extra)
    {
        Factory f { category, name, base() };

        for (const auto& p : extra)
            f.params.push_back (p);

        return f;
    }

    inline const std::vector<Factory>& factory()
    {
        static const std::vector<Factory> list =
        {
            // ============================================================ genre presets
            make ("Genre Presets", "Melodic Trap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.05f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 105.f }, { ids::eqLow, 1.5f }, { ids::eqLowFreq, 130.f },
                { ids::eqMid, -1.5f }, { ids::eqMidFreq, 450.f }, { ids::eqMidQ, 1.1f },
                { ids::eqMid2, 1.5f }, { ids::eqMid2Freq, 3200.f }, { ids::eqMid2Q, 1.0f },
                { ids::eqHigh, 1.f }, { ids::eqHighFreq, 9500.f }, { ids::eqAir, 2.5f },
                { ids::compThresh, -21.f }, { ids::compRatio, 2.8f }, { ids::compAttack, 6.f },
                { ids::compRelease, 110.f }, { ids::compKnee, 10.f }, { ids::compMakeup, 3.5f },
                { ids::satMode, 0.f }, { ids::satDrive, 2.5f }, { ids::satWarmth, 0.2f },
                { ids::satDensity, 0.2f }, { ids::satMix, 0.2f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.18f }, { ids::dblWidth, 0.7f },
                { ids::dblHuman, 0.4f }, { ids::dblPitch, 0.15f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 0.f }, { ids::dlyTime, 300.f },
                { ids::dlyFb, 0.22f }, { ids::dlyTone, 4800.f }, { ids::dlyMix, 0.10f },
                { ids::revOn, 1.f }, { ids::revAlgo, 2.f }, { ids::revDecay, 0.45f },
                { ids::revPredelay, 25.f }, { ids::revWidth, 0.9f }, { ids::revTone, 6500.f },
                { ids::revMod, 0.2f }, { ids::revMix, 0.14f },
                { ids::stereoWidth, 1.12f }, { ids::stereoBass, 120.f },
                { ids::masterLoud, 1.2f }, { ids::masterPunch, 0.25f }, { ids::masterAir, 0.35f },
                { ids::masterGlue, 0.3f },
            }),

            make ("Genre Presets", "Trap Tune", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 3.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 120.f }, { ids::eqLow, 1.f }, { ids::eqLowFreq, 150.f },
                { ids::eqMid, -2.f }, { ids::eqMidFreq, 500.f }, { ids::eqMidQ, 1.2f },
                { ids::eqMid2, 2.f }, { ids::eqMid2Freq, 3000.f }, { ids::eqMid2Q, 1.1f },
                { ids::eqHigh, 1.5f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 3.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 3.2f }, { ids::compAttack, 4.f },
                { ids::compRelease, 95.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 3.f }, { ids::satDrive, 4.f }, { ids::satWarmth, 0.15f },
                { ids::satDensity, 0.3f }, { ids::satMix, 0.22f },
                { ids::deessFreq, 7600.f }, { ids::deessThresh, -30.f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 250.f },
                { ids::dlyFb, 0.25f }, { ids::dlyTone, 6000.f }, { ids::dlyMix, 0.10f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.4f },
                { ids::revPredelay, 20.f }, { ids::revMix, 0.13f },
                { ids::masterLoud, 1.8f }, { ids::masterPunch, 0.35f }, { ids::masterAir, 0.4f },
            }),

            make ("Genre Presets", "Dark Trap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 8.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 75.f }, { ids::eqLpf, 16000.f }, { ids::eqLow, 2.5f }, { ids::eqLowFreq, 140.f },
                { ids::eqMid, -2.f }, { ids::eqMidFreq, 700.f }, { ids::eqMidQ, 0.9f },
                { ids::eqMid2, -1.f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, -1.5f }, { ids::eqHighFreq, 7000.f }, { ids::eqAir, 0.5f },
                { ids::compModel, 1.f }, { ids::compThresh, -24.f }, { ids::compRatio, 3.5f },
                { ids::compAttack, 8.f }, { ids::compRelease, 120.f }, { ids::compKnee, 6.f },
                { ids::compMakeup, 3.5f },
                { ids::satMode, 2.f }, { ids::satDrive, 5.f }, { ids::satWarmth, 0.4f },
                { ids::satDensity, 0.35f }, { ids::satMix, 0.35f },
                { ids::deessFreq, 7500.f }, { ids::deessThresh, -34.f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 375.f },
                { ids::dlyFb, 0.3f }, { ids::dlyTone, 3000.f }, { ids::dlyMix, 0.14f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.6f },
                { ids::revPredelay, 30.f }, { ids::revTone, 3400.f }, { ids::revMix, 0.2f },
                { ids::masterLoud, 0.8f }, { ids::masterWarmth, 0.45f }, { ids::masterAir, 0.1f },
            }),

            make ("Genre Presets", "Emo Rap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 4.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.95f }, { ids::tuneHumanize, 0.1f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 110.f }, { ids::eqLow, 1.f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, 1.f }, { ids::eqMidFreq, 2800.f }, { ids::eqMidQ, 1.f },
                { ids::eqMid2, 0.5f }, { ids::eqMid2Freq, 6000.f },
                { ids::eqHigh, 0.f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 2.f },
                { ids::compModel, 2.f }, { ids::compThresh, -20.f }, { ids::compRatio, 2.5f },
                { ids::compAttack, 18.f }, { ids::compRelease, 150.f }, { ids::compKnee, 12.f },
                { ids::compMakeup, 3.f },
                { ids::satMode, 1.f }, { ids::satDrive, 3.f }, { ids::satWarmth, 0.3f },
                { ids::satDensity, 0.2f }, { ids::satMix, 0.25f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.25f }, { ids::dblWidth, 0.8f },
                { ids::dblHuman, 0.55f }, { ids::dblPitch, 0.2f },
                { ids::revOn, 1.f }, { ids::revAlgo, 2.f }, { ids::revDecay, 0.5f },
                { ids::revPredelay, 20.f }, { ids::revTone, 6000.f }, { ids::revMix, 0.18f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 320.f }, { ids::dlyFb, 0.24f },
                { ids::dlyTone, 4500.f }, { ids::dlyMix, 0.12f },
                { ids::masterLoud, 1.f }, { ids::masterPunch, 0.25f }, { ids::masterAir, 0.35f },
            }),

            make ("Genre Presets", "Rage", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 2.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 120.f }, { ids::eqLow, 2.f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, -1.f }, { ids::eqMidFreq, 900.f },
                { ids::eqMid2, 2.f }, { ids::eqMid2Freq, 3500.f },
                { ids::eqHigh, 2.f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 4.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.f }, { ids::compAttack, 3.f },
                { ids::compRelease, 80.f }, { ids::compKnee, 4.f }, { ids::compMakeup, 4.5f },
                { ids::satMode, 5.f }, { ids::satDrive, 6.f }, { ids::satWarmth, 0.2f },
                { ids::satDensity, 0.35f }, { ids::satMix, 0.3f },
                { ids::deessFreq, 7200.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 5.f },
                { ids::revOn, 1.f }, { ids::revAlgo, 3.f }, { ids::revDecay, 0.55f },
                { ids::revPredelay, 25.f }, { ids::revWidth, 1.f }, { ids::revMix, 0.16f },
                { ids::stereoWidth, 1.2f },
                { ids::masterLoud, 1.8f }, { ids::masterPunch, 0.4f }, { ids::masterAir, 0.4f },
            }),

            make ("Genre Presets", "Hyperpop", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 130.f }, { ids::eqLow, -1.f }, { ids::eqLowFreq, 200.f },
                { ids::eqMid, 1.5f }, { ids::eqMidFreq, 4000.f }, { ids::eqMidQ, 1.2f },
                { ids::eqMid2, 1.f }, { ids::eqMid2Freq, 8000.f },
                { ids::eqHigh, 2.f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 5.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 5.f }, { ids::compAttack, 2.f },
                { ids::compRelease, 70.f }, { ids::compKnee, 4.f }, { ids::compMakeup, 5.f },
                { ids::satMode, 3.f }, { ids::satDrive, 4.f }, { ids::satWarmth, 0.15f },
                { ids::satDensity, 0.3f }, { ids::satMix, 0.25f },
                { ids::deessFreq, 7500.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 5.f },
                { ids::stutOn, 1.f }, { ids::stutMode, 2.f }, { ids::stutSlice, 70.f },
                { ids::stutRate, 11.f }, { ids::stutRandom, 0.3f }, { ids::stutMix, 0.3f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 190.f },
                { ids::dlyFb, 0.3f }, { ids::dlyTone, 6500.f }, { ids::dlyMix, 0.15f },
                { ids::revOn, 1.f }, { ids::revAlgo, 5.f }, { ids::revDecay, 0.55f },
                { ids::revTone, 8000.f }, { ids::revMix, 0.2f },
                { ids::stereoWidth, 1.25f },
                { ids::masterLoud, 1.5f }, { ids::masterPunch, 0.4f }, { ids::masterAir, 0.5f },
            }),

            make ("Genre Presets", "Drill", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 3.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.05f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 100.f }, { ids::eqLow, 1.5f }, { ids::eqLowFreq, 150.f },
                { ids::eqMid, -1.f }, { ids::eqMidFreq, 800.f },
                { ids::eqMid2, 1.f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, 0.5f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 2.5f },
                { ids::compModel, 1.f }, { ids::compThresh, -25.f }, { ids::compRatio, 4.f },
                { ids::compAttack, 5.f }, { ids::compRelease, 85.f }, { ids::compKnee, 5.f },
                { ids::compMakeup, 4.f },
                { ids::satMode, 0.f }, { ids::satDrive, 4.f }, { ids::satWarmth, 0.25f },
                { ids::satDensity, 0.3f }, { ids::satMix, 0.25f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -34.f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.2f }, { ids::dblWidth, 0.6f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.3f },
                { ids::revPredelay, 15.f }, { ids::revMix, 0.12f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 220.f },
                { ids::dlyFb, 0.24f }, { ids::dlyTone, 5000.f }, { ids::dlyMix, 0.11f },
                { ids::masterLoud, 1.2f }, { ids::masterPunch, 0.4f }, { ids::masterAir, 0.3f },
            }),

            make ("Genre Presets", "Alternative Hip-Hop", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 9.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.9f }, { ids::tuneHumanize, 0.15f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 90.f }, { ids::eqLow, 1.f }, { ids::eqLowFreq, 150.f },
                { ids::eqMid, -0.5f }, { ids::eqMidFreq, 900.f },
                { ids::eqMid2, 0.5f }, { ids::eqMid2Freq, 2500.f },
                { ids::eqHigh, -1.f }, { ids::eqHighFreq, 7500.f }, { ids::eqAir, 1.f },
                { ids::compThresh, -18.f }, { ids::compRatio, 2.2f }, { ids::compAttack, 20.f },
                { ids::compRelease, 170.f }, { ids::compKnee, 12.f }, { ids::compMakeup, 2.5f },
                { ids::satMode, 1.f }, { ids::satDrive, 4.f }, { ids::satWarmth, 0.35f },
                { ids::satDensity, 0.25f }, { ids::satMix, 0.3f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -28.f }, { ids::deessRatio, 2.5f },
                { ids::nrHum, 0.45f }, { ids::nrBreath, 0.3f }, { ids::nrClicks, 0.3f },
                { ids::revOn, 1.f }, { ids::revAlgo, 4.f }, { ids::revDecay, 0.5f },
                { ids::revPredelay, 25.f }, { ids::revTone, 4200.f }, { ids::revMix, 0.18f },
                { ids::masterLoud, 1.f }, { ids::masterWarmth, 0.35f }, { ids::masterAir, 0.2f },
            }),

            make ("Genre Presets", "Singing Vocals", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 0.9f }, { ids::tuneHumanize, 0.2f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 90.f }, { ids::eqLow, 1.f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, 0.5f }, { ids::eqMidFreq, 2000.f },
                { ids::eqMid2, 0.5f }, { ids::eqMid2Freq, 4000.f },
                { ids::eqHigh, 1.f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 2.f },
                { ids::compModel, 2.f }, { ids::compThresh, -20.f }, { ids::compRatio, 2.8f },
                { ids::compAttack, 16.f }, { ids::compRelease, 140.f }, { ids::compKnee, 10.f },
                { ids::compMakeup, 3.f },
                { ids::satMode, 0.f }, { ids::satDrive, 2.f }, { ids::satWarmth, 0.25f },
                { ids::satDensity, 0.2f }, { ids::satMix, 0.18f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -28.f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.28f }, { ids::dblWidth, 0.75f },
                { ids::dblHuman, 0.55f }, { ids::dblPitch, 0.2f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.5f },
                { ids::revPredelay, 30.f }, { ids::revTone, 6500.f }, { ids::revMix, 0.16f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 300.f }, { ids::dlyFb, 0.24f }, { ids::dlyMix, 0.10f },
                { ids::masterLoud, 1.f }, { ids::masterPunch, 0.2f }, { ids::masterAir, 0.3f },
            }),

            make ("Genre Presets", "Pop Vocals", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 7.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.08f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 100.f }, { ids::eqLow, 0.5f }, { ids::eqLowFreq, 180.f },
                { ids::eqMid, 1.f }, { ids::eqMidFreq, 3500.f }, { ids::eqMidQ, 1.f },
                { ids::eqMid2, 1.f }, { ids::eqMid2Freq, 6000.f },
                { ids::eqHigh, 1.5f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 3.5f },
                { ids::compThresh, -22.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 5.f },
                { ids::compRelease, 100.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 3.f }, { ids::satDrive, 2.5f }, { ids::satWarmth, 0.15f },
                { ids::satMix, 0.18f },
                { ids::deessFreq, 7800.f }, { ids::deessThresh, -34.f }, { ids::deessRatio, 5.f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.3f }, { ids::dblWidth, 0.9f },
                { ids::dblHuman, 0.5f }, { ids::dblPitch, 0.2f },
                { ids::revOn, 1.f }, { ids::revAlgo, 2.f }, { ids::revDecay, 0.42f },
                { ids::revPredelay, 25.f }, { ids::revTone, 7500.f }, { ids::revMix, 0.14f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 260.f },
                { ids::dlyFb, 0.26f }, { ids::dlyTone, 6000.f }, { ids::dlyMix, 0.12f },
                { ids::masterLoud, 1.3f }, { ids::masterPunch, 0.3f }, { ids::masterAir, 0.4f },
            }),

            make ("Genre Presets", "R&B Smooth", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 5.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.85f }, { ids::tuneHumanize, 0.15f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 85.f }, { ids::eqLow, 1.5f }, { ids::eqLowFreq, 160.f },
                { ids::eqMid, 0.5f }, { ids::eqMidFreq, 1500.f },
                { ids::eqMid2, 0.5f }, { ids::eqMid2Freq, 4000.f },
                { ids::eqHigh, 1.f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 2.5f },
                { ids::compModel, 2.f }, { ids::compThresh, -19.f }, { ids::compRatio, 2.5f },
                { ids::compAttack, 14.f }, { ids::compRelease, 150.f }, { ids::compKnee, 14.f },
                { ids::compMakeup, 3.f },
                { ids::satMode, 4.f }, { ids::satDrive, 3.5f }, { ids::satWarmth, 0.4f },
                { ids::satDensity, 0.2f }, { ids::satMix, 0.25f },
                { ids::deessFreq, 7200.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.5f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.35f }, { ids::dblWidth, 0.85f },
                { ids::dblHuman, 0.6f }, { ids::dblPitch, 0.25f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.55f },
                { ids::revPredelay, 30.f }, { ids::revTone, 6000.f }, { ids::revMix, 0.18f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 340.f },
                { ids::dlyFb, 0.28f }, { ids::dlyTone, 4500.f }, { ids::dlyMix, 0.12f },
                { ids::masterLoud, 1.f }, { ids::masterWarmth, 0.35f }, { ids::masterAir, 0.3f },
            }),

            make ("Genre Presets", "Podcast Voice", {
                { ids::eqHpf, 100.f }, { ids::eqLpf, 16000.f }, { ids::eqLow, -1.f }, { ids::eqLowFreq, 200.f },
                { ids::eqMid, 1.5f }, { ids::eqMidFreq, 2600.f }, { ids::eqMidQ, 1.1f },
                { ids::eqHigh, 0.5f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 1.5f },
                { ids::compThresh, -26.f }, { ids::compRatio, 4.f }, { ids::compAttack, 3.f },
                { ids::compRelease, 120.f }, { ids::compKnee, 6.f }, { ids::compMakeup, 6.f },
                { ids::satMode, 4.f }, { ids::satDrive, 2.f }, { ids::satWarmth, 0.2f }, { ids::satMix, 0.15f },
                { ids::deessFreq, 8000.f }, { ids::deessThresh, -34.f }, { ids::deessRatio, 5.f },
                { ids::gateOn, 1.f }, { ids::gateThresh, -50.f }, { ids::gateAttack, 1.f },
                { ids::gateHold, 80.f }, { ids::gateRelease, 200.f },
                { ids::nrHum, 0.5f }, { ids::nrBreath, 0.4f }, { ids::nrClicks, 0.4f }, { ids::nrAmount, 0.5f },
                { ids::masterLoud, 1.8f }, { ids::masterPunch, 0.2f }, { ids::masterGlue, 0.4f },
            }),

            make ("Genre Presets", "Broadcast Voice", {
                { ids::eqHpf, 90.f }, { ids::eqLow, 0.5f }, { ids::eqLowFreq, 180.f },
                { ids::eqMid, 1.2f }, { ids::eqMidFreq, 3000.f },
                { ids::eqHigh, 1.f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 2.5f },
                { ids::compThresh, -30.f }, { ids::compRatio, 5.f }, { ids::compAttack, 2.f },
                { ids::compRelease, 90.f }, { ids::compKnee, 4.f }, { ids::compMakeup, 8.f },
                { ids::satMode, 0.f }, { ids::satDrive, 2.5f }, { ids::satWarmth, 0.2f }, { ids::satMix, 0.15f },
                { ids::deessFreq, 8500.f }, { ids::deessThresh, -36.f }, { ids::deessRatio, 6.f },
                { ids::gateOn, 1.f }, { ids::gateThresh, -52.f }, { ids::gateAttack, 1.f },
                { ids::gateHold, 100.f }, { ids::gateRelease, 250.f },
                { ids::nrHum, 0.6f }, { ids::nrBreath, 0.5f }, { ids::nrClicks, 0.5f }, { ids::nrAmount, 0.55f },
                { ids::masterLoud, 2.5f }, { ids::masterGlue, 0.5f }, { ids::masterPunch, 0.2f },
            }),

            make ("Genre Presets", "Voiceover Clean", {
                { ids::eqHpf, 120.f }, { ids::eqLow, 0.f }, { ids::eqLowFreq, 200.f },
                { ids::eqMid, 0.8f }, { ids::eqMidFreq, 2000.f },
                { ids::eqHigh, 0.5f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 0.5f },
                { ids::compModel, 2.f }, { ids::compThresh, -24.f }, { ids::compRatio, 2.5f },
                { ids::compAttack, 10.f }, { ids::compRelease, 150.f }, { ids::compKnee, 10.f },
                { ids::compMakeup, 5.f },
                { ids::satMode, 4.f }, { ids::satDrive, 1.5f }, { ids::satMix, 0.12f },
                { ids::deessFreq, 7500.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 4.f },
                { ids::gateOn, 1.f }, { ids::gateThresh, -55.f }, { ids::gateAttack, 2.f },
                { ids::gateHold, 120.f }, { ids::gateRelease, 300.f },
                { ids::nrHum, 0.4f }, { ids::nrBreath, 0.4f }, { ids::nrClicks, 0.35f }, { ids::nrAmount, 0.45f },
                { ids::masterLoud, 1.3f }, { ids::masterPunch, 0.2f }, { ids::masterGlue, 0.3f },
            }),

            // ========================================================= signature presets
            make ("Signature Presets", "BKWDZ Signature", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.05f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 95.f }, { ids::eqLow, 1.5f }, { ids::eqLowFreq, 150.f },
                { ids::eqMid, -1.f }, { ids::eqMidFreq, 500.f },
                { ids::eqMid2, 1.5f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, 1.f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 3.f },
                { ids::compModel, 1.f }, { ids::compThresh, -23.f }, { ids::compRatio, 3.5f },
                { ids::compAttack, 5.f }, { ids::compRelease, 95.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 0.f }, { ids::satDrive, 4.5f }, { ids::satWarmth, 0.35f },
                { ids::satDensity, 0.3f }, { ids::satMix, 0.3f },
                { ids::deessFreq, 7200.f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.3f }, { ids::dblWidth, 0.8f },
                { ids::dblHuman, 0.5f }, { ids::dblPitch, 0.2f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 290.f }, { ids::dlyFb, 0.28f },
                { ids::dlyTone, 5000.f }, { ids::dlyMix, 0.14f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.5f },
                { ids::revPredelay, 25.f }, { ids::revTone, 6500.f }, { ids::revMix, 0.18f },
                { ids::stereoWidth, 1.1f },
                { ids::masterLoud, 1.5f }, { ids::masterPunch, 0.35f }, { ids::masterWarmth, 0.25f },
                { ids::masterAir, 0.4f },
            }),

            make ("Signature Presets", "Dream Vocal", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.8f }, { ids::tuneHumanize, 0.2f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 120.f }, { ids::eqLow, -1.f }, { ids::eqLowFreq, 220.f },
                { ids::eqMid2, 1.f }, { ids::eqMid2Freq, 5000.f },
                { ids::eqHigh, 2.f }, { ids::eqHighFreq, 8000.f }, { ids::eqAir, 5.f },
                { ids::compModel, 2.f }, { ids::compThresh, -18.f }, { ids::compRatio, 2.2f },
                { ids::compAttack, 20.f }, { ids::compRelease, 200.f }, { ids::compKnee, 14.f },
                { ids::compMakeup, 2.f },
                { ids::satMode, 4.f }, { ids::satDrive, 3.f }, { ids::satWarmth, 0.45f },
                { ids::satDensity, 0.25f }, { ids::satMix, 0.2f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.5f }, { ids::dblWidth, 1.f },
                { ids::dblHuman, 0.7f }, { ids::dblPitch, 0.3f },
                { ids::convOn, 1.f }, { ids::convAlgo, 2.f }, { ids::convPredelay, 35.f },
                { ids::convTone, 5200.f }, { ids::convWidth, 1.f }, { ids::convMix, 0.26f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 420.f },
                { ids::dlyFb, 0.35f }, { ids::dlyTone, 3500.f }, { ids::dlyMix, 0.15f },
                { ids::stereoWidth, 1.3f },
                { ids::masterLoud, 0.f }, { ids::masterWarmth, 0.25f }, { ids::masterAir, 0.5f },
                { ids::masterGlue, 0.35f },
            }),

            make ("Signature Presets", "Dark Atmosphere", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 10.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.9f }, { ids::tuneHumanize, 0.1f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 70.f }, { ids::eqLpf, 9000.f }, { ids::eqLow, 2.5f }, { ids::eqLowFreq, 130.f },
                { ids::eqMid, -2.5f }, { ids::eqMidFreq, 700.f },
                { ids::eqMid2, -1.f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, -2.f }, { ids::eqHighFreq, 6500.f }, { ids::eqAir, 0.f },
                { ids::compModel, 2.f }, { ids::compThresh, -20.f }, { ids::compRatio, 2.5f },
                { ids::compAttack, 18.f }, { ids::compRelease, 180.f }, { ids::compKnee, 12.f },
                { ids::compMakeup, 3.f },
                { ids::satMode, 2.f }, { ids::satDrive, 6.f }, { ids::satWarmth, 0.5f },
                { ids::satDensity, 0.4f }, { ids::satMix, 0.45f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -36.f }, { ids::deessRatio, 3.f },
                { ids::chopOn, 1.f }, { ids::chopMode, 3.f }, { ids::chopPitch, -4.f },
                { ids::chopSlow, 0.45f }, { ids::chopDensity, 0.4f }, { ids::chopRepeat, 0.5f },
                { ids::chopDrift, 0.5f }, { ids::chopMix, 0.45f },
                { ids::convOn, 1.f }, { ids::convAlgo, 3.f }, { ids::convPredelay, 40.f },
                { ids::convTone, 3200.f }, { ids::convWidth, 1.f }, { ids::convMix, 0.3f },
                { ids::masterLoud, 0.5f }, { ids::masterWarmth, 0.5f }, { ids::masterAir, 0.1f },
            }),

            make ("Signature Presets", "Arena Vocal", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.1f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 110.f }, { ids::eqLow, 1.f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, 0.5f }, { ids::eqMidFreq, 2400.f },
                { ids::eqMid2, 0.5f }, { ids::eqMid2Freq, 5000.f },
                { ids::eqHigh, 1.5f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 3.5f },
                { ids::compThresh, -24.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 6.f },
                { ids::compRelease, 110.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 0.f }, { ids::satDrive, 3.f }, { ids::satWarmth, 0.25f }, { ids::satMix, 0.25f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.4f }, { ids::dblWidth, 1.f },
                { ids::dblHuman, 0.6f }, { ids::dblPitch, 0.25f },
                { ids::convOn, 1.f }, { ids::convAlgo, 2.f }, { ids::convPredelay, 45.f },
                { ids::convTone, 6000.f }, { ids::convMix, 0.24f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 480.f },
                { ids::dlyFb, 0.3f }, { ids::dlyTone, 5000.f }, { ids::dlyMix, 0.16f },
                { ids::stereoWidth, 1.2f },
                { ids::masterLoud, 1.5f }, { ids::masterPunch, 0.4f }, { ids::masterAir, 0.45f },
            }),

            make ("Signature Presets", "Hard Tune", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 110.f }, { ids::eqLow, 1.5f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, -1.f }, { ids::eqMidFreq, 600.f },
                { ids::eqMid2, 1.5f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, 1.5f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 3.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.f }, { ids::compAttack, 3.f },
                { ids::compRelease, 90.f }, { ids::compKnee, 4.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 3.f }, { ids::satDrive, 3.5f }, { ids::satMix, 0.22f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 250.f },
                { ids::dlyFb, 0.24f }, { ids::dlyTone, 6000.f }, { ids::dlyMix, 0.12f },
                { ids::revOn, 1.f }, { ids::revAlgo, 1.f }, { ids::revDecay, 0.45f }, { ids::revMix, 0.12f },
                { ids::masterLoud, 1.5f }, { ids::masterPunch, 0.35f }, { ids::masterAir, 0.3f },
            }),

            make ("Signature Presets", "Natural Correction", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 0.7f }, { ids::tuneHumanize, 0.35f }, { ids::tuneAmount, 0.8f },
                { ids::eqHpf, 85.f }, { ids::eqLow, 0.5f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, 0.5f }, { ids::eqMidFreq, 2000.f },
                { ids::eqHigh, 0.5f }, { ids::eqHighFreq, 9000.f }, { ids::eqAir, 1.5f },
                { ids::compModel, 2.f }, { ids::compThresh, -16.f }, { ids::compRatio, 2.f },
                { ids::compAttack, 25.f }, { ids::compRelease, 180.f }, { ids::compKnee, 16.f },
                { ids::compMakeup, 2.f },
                { ids::satMode, 4.f }, { ids::satDrive, 1.5f }, { ids::satMix, 0.12f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -28.f }, { ids::deessRatio, 2.5f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.25f },
                { ids::revPredelay, 15.f }, { ids::revMix, 0.08f },
                { ids::masterLoud, 0.7f }, { ids::masterGlue, 0.25f },
            }),

            make ("Signature Presets", "Screwed & Chopped", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 5.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.9f }, { ids::tuneHumanize, 0.05f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 70.f }, { ids::eqLow, 2.5f }, { ids::eqLowFreq, 140.f },
                { ids::eqMid, -1.f }, { ids::eqMidFreq, 900.f },
                { ids::eqHigh, -0.5f }, { ids::eqHighFreq, 7000.f }, { ids::eqAir, 0.5f },
                { ids::compModel, 1.f }, { ids::compThresh, -26.f }, { ids::compRatio, 4.f },
                { ids::compAttack, 8.f }, { ids::compRelease, 140.f }, { ids::compKnee, 6.f },
                { ids::compMakeup, 4.f },
                { ids::satMode, 1.f }, { ids::satDrive, 6.f }, { ids::satWarmth, 0.45f },
                { ids::satDensity, 0.35f }, { ids::satMix, 0.4f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -34.f },
                { ids::chopOn, 1.f }, { ids::chopMode, 1.f }, { ids::chopPitch, -3.f },
                { ids::chopSlow, 0.5f }, { ids::chopDensity, 0.5f }, { ids::chopRepeat, 0.6f },
                { ids::chopDrift, 0.6f }, { ids::chopMix, 0.8f },
                { ids::stereoWidth, 1.05f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 1.f }, { ids::dlyTime, 400.f },
                { ids::dlyFb, 0.32f }, { ids::dlyTone, 2600.f }, { ids::dlyMix, 0.12f },
                { ids::revOn, 1.f }, { ids::revAlgo, 4.f }, { ids::revDecay, 0.5f },
                { ids::revPredelay, 20.f }, { ids::revMix, 0.12f },
                { ids::masterLoud, 1.f }, { ids::masterWarmth, 0.5f }, { ids::masterAir, 0.1f },
            }),

            make ("Signature Presets", "Doubled Stack", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 0.f },
                { ids::tuneRetune, 0.95f }, { ids::tuneHumanize, 0.1f }, { ids::tuneAmount, 0.95f },
                { ids::eqHpf, 90.f }, { ids::eqLow, 1.5f }, { ids::eqLowFreq, 160.f },
                { ids::eqMid, 0.5f }, { ids::eqMidFreq, 2600.f },
                { ids::eqMid2, 0.5f }, { ids::eqMid2Freq, 5000.f },
                { ids::eqHigh, 1.5f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 2.5f },
                { ids::compThresh, -22.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 8.f },
                { ids::compRelease, 110.f }, { ids::compMakeup, 4.f },
                { ids::satMode, 0.f }, { ids::satDrive, 3.5f }, { ids::satWarmth, 0.3f }, { ids::satMix, 0.25f },
                { ids::dblOn, 1.f }, { ids::dblAmount, 0.6f }, { ids::dblWidth, 1.f },
                { ids::dblHuman, 0.65f }, { ids::dblPitch, 0.3f },
                { ids::harmOn, 1.f }, { ids::harmThird, 0.22f },
                { ids::stereoWidth, 1.25f },
                { ids::revOn, 1.f }, { ids::revAlgo, 2.f }, { ids::revDecay, 0.45f },
                { ids::revPredelay, 25.f }, { ids::revMix, 0.16f },
                { ids::dlyOn, 1.f }, { ids::dlyTime, 300.f }, { ids::dlyFb, 0.26f }, { ids::dlyMix, 0.12f },
                { ids::masterLoud, 1.2f }, { ids::masterPunch, 0.3f }, { ids::masterGlue, 0.4f },
            }),

            make ("Signature Presets", "Stutter Trap", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 3.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 1.f }, { ids::tuneHumanize, 0.05f }, { ids::tuneAmount, 1.f },
                { ids::eqHpf, 120.f }, { ids::eqLow, 1.f }, { ids::eqLowFreq, 170.f },
                { ids::eqMid, -1.f }, { ids::eqMidFreq, 900.f },
                { ids::eqMid2, 2.f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, 1.f }, { ids::eqHighFreq, 8500.f }, { ids::eqAir, 3.f },
                { ids::compThresh, -25.f }, { ids::compRatio, 4.f }, { ids::compAttack, 4.f },
                { ids::compRelease, 90.f }, { ids::compKnee, 4.f }, { ids::compMakeup, 4.5f },
                { ids::satMode, 5.f }, { ids::satDrive, 4.f }, { ids::satWarmth, 0.2f }, { ids::satMix, 0.2f },
                { ids::stutOn, 1.f }, { ids::stutMode, 1.f }, { ids::stutSlice, 60.f },
                { ids::stutRate, 14.f }, { ids::stutRandom, 0.4f }, { ids::stutMix, 0.4f },
                { ids::stereoWidth, 1.15f },
                { ids::dlyOn, 1.f }, { ids::dlyMode, 2.f }, { ids::dlyTime, 180.f },
                { ids::dlyFb, 0.34f }, { ids::dlyTone, 6000.f }, { ids::dlyMix, 0.16f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.3f }, { ids::revMix, 0.1f },
                { ids::masterLoud, 1.6f }, { ids::masterPunch, 0.4f }, { ids::masterAir, 0.4f },
            }),

            make ("Signature Presets", "Lo-Fi Bedroom", {
                { ids::tuneOn, 1.f }, { ids::tuneKey, 0.f }, { ids::tuneScale, 1.f },
                { ids::tuneRetune, 0.9f }, { ids::tuneHumanize, 0.2f }, { ids::tuneAmount, 0.9f },
                { ids::eqHpf, 80.f }, { ids::eqLpf, 12000.f }, { ids::eqLow, 2.f }, { ids::eqLowFreq, 130.f },
                { ids::eqMid2, -1.f }, { ids::eqMid2Freq, 3000.f },
                { ids::eqHigh, -2.f }, { ids::eqHighFreq, 6500.f }, { ids::eqAir, 0.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.f }, { ids::compAttack, 10.f },
                { ids::compRelease, 150.f }, { ids::compKnee, 10.f }, { ids::compMakeup, 3.f },
                { ids::satMode, 1.f }, { ids::satDrive, 6.f }, { ids::satWarmth, 0.35f },
                { ids::satDensity, 0.4f }, { ids::satMix, 0.35f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.f },
                { ids::nrClicks, 0.5f }, { ids::nrAmount, 0.45f },
                { ids::casOn, 1.f }, { ids::casDrive, 0.35f }, { ids::casWow, 0.35f },
                { ids::casTone, 4200.f }, { ids::casHiss, 0.2f }, { ids::casMix, 0.3f },
                { ids::revOn, 1.f }, { ids::revAlgo, 0.f }, { ids::revDecay, 0.35f },
                { ids::revPredelay, 15.f }, { ids::revMix, 0.12f },
                { ids::masterLoud, 0.7f }, { ids::masterWarmth, 0.35f }, { ids::masterAir, 0.1f },
            }),
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
