#pragma once
#include <juce_dsp/juce_dsp.h>
#include <algorithm>
#include <cmath>

// Split-band de-esser: a band-pass around the sibilance range drives a detector,
// and only that band is turned down when it gets too loud. Works for mono/stereo (linked).
class DeEsser
{
public:
    void prepare (double sr)
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) 512, (juce::uint32) 2 };
        band.prepare (spec);
        band.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        band.setResonance (1.5f);

        attackCoeff  = std::exp (-1.0f / (float) (0.001 * sr)); //  1 ms
        releaseCoeff = std::exp (-1.0f / (float) (0.040 * sr)); // 40 ms
        reset();
    }

    void reset()
    {
        band.reset();
        env = 0.0f;
        lastReductionDb = 0.0f;
    }

    void setParams (float freqHz, float threshDb, float newRatio)
    {
        band.setCutoffFrequency (freqHz);
        thresholdDb = threshDb;
        ratio = std::max (1.0f, newRatio);
    }

    // Biggest gain reduction (in dB, positive) seen in the most recent block. Used by the meter.
    float getReductionDb() const { return lastReductionDb; }

    void process (juce::AudioBuffer<float>& buffer)
    {
        const int numCh = juce::jmin (buffer.getNumChannels(), 2);
        const int n = buffer.getNumSamples();
        float maxReduction = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            float bands[2] = { 0.0f, 0.0f };
            float peak = 0.0f;

            for (int c = 0; c < numCh; ++c)
            {
                bands[c] = band.processSample (c, buffer.getSample (c, i));
                peak = std::max (peak, std::abs (bands[c]));
            }

            const float coeff = peak > env ? attackCoeff : releaseCoeff;
            env = coeff * env + (1.0f - coeff) * peak;

            const float envDb = juce::Decibels::gainToDecibels (env, -100.0f);
            float gain = 1.0f;

            if (envDb > thresholdDb)
            {
                const float reductionDb = (envDb - thresholdDb) * (1.0f - 1.0f / ratio);
                gain = juce::Decibels::decibelsToGain (-reductionDb);
                maxReduction = std::max (maxReduction, reductionDb);
            }

            // out = input - (1 - gain) * band  -> only the sibilant band is attenuated
            for (int c = 0; c < numCh; ++c)
                buffer.setSample (c, i, buffer.getSample (c, i) - (1.0f - gain) * bands[c]);
        }

        lastReductionDb = maxReduction;
    }

private:
    juce::dsp::StateVariableTPTFilter<float> band;
    float attackCoeff = 0.0f, releaseCoeff = 0.0f, env = 0.0f;
    float thresholdDb = -28.0f, ratio = 4.0f;
    float lastReductionDb = 0.0f;
};
