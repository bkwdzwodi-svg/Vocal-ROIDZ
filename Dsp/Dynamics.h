#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>

// ---------------------------------------------------------------------------------------
// VocalCompressor: soft-knee, stereo-linked, with the four classic detector feels and a
// parallel mix. Returns the peak gain reduction so the GUI meter is honest.
// ---------------------------------------------------------------------------------------
class VocalCompressor
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        reset();
    }

    void reset()
    {
        env = 0.0f;
        lastReduction = 0.0f;
        gainSmoothed = 1.0f;
    }

    void setParams (float thresholdDb, float ratio, float attackMs, float releaseMs,
                    float kneeDb, int model)
    {
        // Each model leans on the same detector with its own feel.
        switch (model)
        {
            case 1:  attackMs *= 0.45f; releaseMs *= 0.75f; kneeDb *= 0.35f; break;  // FET
            case 2:  attackMs *= 3.2f;  releaseMs *= 2.4f;  kneeDb *= 1.4f;  break;  // Optical
            case 3:  attackMs *= 1.6f;  releaseMs *= 1.5f;  kneeDb *= 2.0f;  break;  // Tube
            default: break;                                                          // VCA
        }

        threshold = thresholdDb;
        knee = juce::jmax (0.1f, kneeDb);
        ratioValue = juce::jmax (1.0f, ratio);

        const float atkSec = juce::jmax (0.00005f, attackMs * 0.001f);
        const float relSec = juce::jmax (0.0005f, releaseMs * 0.001f);
        attackCoeff  = std::exp (-1.0f / (float) (atkSec * fs));
        releaseCoeff = std::exp (-1.0f / (float) (relSec * fs));
    }

    float process (juce::AudioBuffer<float>& buffer, float mix, int model)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float m = juce::jlimit (0.0f, 1.0f, mix);
        float maxReduction = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            float peak = 0.0f;

            for (int c = 0; c < ch; ++c)
                peak = juce::jmax (peak, std::abs (buffer.getSample (c, i)));

            const float coeff = peak > env ? attackCoeff : releaseCoeff;
            env = coeff * env + (1.0f - coeff) * peak;

            const float envDb = juce::Decibels::gainToDecibels (env, -120.0f);
            const float over = envDb - threshold;
            float reductionDb = 0.0f;

            if (over > -knee * 0.5f)
            {
                if (over < knee * 0.5f)
                {
                    const float t = over + knee * 0.5f;
                    reductionDb = (1.0f - 1.0f / ratioValue) * (t * t) / (2.0f * knee);
                }
                else
                {
                    reductionDb = (1.0f - 1.0f / ratioValue) * over;
                }
            }

            maxReduction = juce::jmax (maxReduction, reductionDb);

            float gain = juce::Decibels::decibelsToGain (-reductionDb);
            gain = 1.0f + (gain - 1.0f) * m;

            for (int c = 0; c < ch; ++c)
            {
                float x = buffer.getSample (c, i) * gain;

                // A little detector colour on the aggressive models.
                if (model == 1)
                    x = std::tanh (x * 1.15f) / 1.15f;
                else if (model == 3)
                    x = std::tanh (x * 1.3f) / 1.3f;

                buffer.setSample (c, i, x);
            }
        }

        lastReduction = maxReduction;
        return maxReduction;
    }

    float getReductionDb() const { return lastReduction; }

private:
    double fs = 44100.0;
    float threshold = -20.0f, ratioValue = 3.5f, knee = 8.0f;
    float attackCoeff = 0.0f, releaseCoeff = 0.0f;
    float env = 0.0f, lastReduction = 0.0f, gainSmoothed = 1.0f;
};

// ---------------------------------------------------------------------------------------
// NoiseGate: peak detector with hold and a tunable floor.
// ---------------------------------------------------------------------------------------
class NoiseGate
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        reset();
    }

    void reset()
    {
        env = 0.0f;
        gain = 1.0f;
        holdCounter = 0;
        open = false;
        lastReduction = 0.0f;
    }

    void setParams (float thresholdDb, float attackMs, float holdMs, float releaseMs, float rangeDb)
    {
        threshold = thresholdDb;
        holdSamples = (int) (holdMs * 0.001f * fs);
        rangeGain = juce::Decibels::decibelsToGain (-juce::jlimit (0.0f, 90.0f, rangeDb));

        attackCoeff  = std::exp (-1.0f / (float) (juce::jmax (0.05f, attackMs) * 0.001f * fs));
        releaseCoeff = std::exp (-1.0f / (float) (juce::jmax (1.0f, releaseMs) * 0.001f * fs));
        detectCoeff  = std::exp (-1.0f / (float) (0.002f * fs));
    }

    float process (juce::AudioBuffer<float>& buffer)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        float maxReduction = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            float peak = 0.0f;

            for (int c = 0; c < ch; ++c)
                peak = juce::jmax (peak, std::abs (buffer.getSample (c, i)));

            env = detectCoeff * env + (1.0f - detectCoeff) * peak;

            const float envDb = juce::Decibels::gainToDecibels (env, -120.0f);

            if (envDb > threshold)
            {
                open = true;
                holdCounter = holdSamples;
            }
            else if (holdCounter > 0)
            {
                --holdCounter;
            }
            else if (envDb < threshold - 3.0f)
            {
                open = false;
            }

            const float target = open ? 1.0f : rangeGain;
            const float coeff = target > gain ? attackCoeff : releaseCoeff;
            gain = coeff * gain + (1.0f - coeff) * target;

            maxReduction = juce::jmax (maxReduction, -juce::Decibels::gainToDecibels (juce::jmax (1.0e-4f, gain)));

            for (int c = 0; c < ch; ++c)
                buffer.setSample (c, i, buffer.getSample (c, i) * gain);
        }

        lastReduction = maxReduction;
        return maxReduction;
    }

    bool isOpen() const { return open; }

private:
    double fs = 44100.0;
    float threshold = -48.0f, rangeGain = 0.001f;
    float attackCoeff = 0.0f, releaseCoeff = 0.0f, detectCoeff = 0.0f;
    float env = 0.0f, gain = 1.0f, lastReduction = 0.0f;
    int holdSamples = 0, holdCounter = 0;
    bool open = false;
};
