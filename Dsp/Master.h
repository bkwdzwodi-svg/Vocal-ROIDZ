#pragma once
#include <juce_dsp/juce_dsp.h>
#include "Dynamics.h"

// ---------------------------------------------------------------------------------------
// MasterFinish: the last stop. Punch, warmth, air, glue compression, loudness trim and a
// ceiling limiter, plus the loudness and correlation readings the master page draws.
// ---------------------------------------------------------------------------------------
class MasterFinish
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;

        limiter.prepare (spec);
        limiter.setRelease (60.0f);

        glue.prepare (spec.sampleRate);

        using Coeffs = juce::dsp::IIR::Coefficients<float>;

        // State must exist before prepare(): ProcessorDuplicator wires its channel filters
        // to the pointer it sees at prepare time.
        warmthFilter.state = Coeffs::makeLowShelf (spec.sampleRate, 140.0f, 0.707f, 1.0f);
        airFilter.state = Coeffs::makeHighShelf (spec.sampleRate,
                                                 juce::jmin (9500.0f, (float) spec.sampleRate * 0.4f),
                                                 0.707f, 1.0f);

        warmthFilter.prepare (spec);
        airFilter.prepare (spec);

        fastEnv = slowEnv = 0.0f;
        lufsSmoothed = -70.0f;
        correlation = 1.0f;
    }

    void reset()
    {
        limiter.reset();
        glue.reset();
        warmthFilter.reset();
        airFilter.reset();
        fastEnv = slowEnv = 0.0f;
        lufsSmoothed = -70.0f;
        correlation = 1.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, bool enabled, float loudnessDb, float punch,
                  float warmth, float air, float glueAmount, float ceilingDb)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);

        if (enabled)
        {
            const float w = juce::jlimit (0.0f, 1.0f, warmth);
            const float a = juce::jlimit (0.0f, 1.0f, air);
            const float p = juce::jlimit (0.0f, 1.0f, punch);
            const float g = juce::jlimit (0.0f, 1.0f, glueAmount);

            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);

            if (w > 0.001f)
            {
                *warmthFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (
                    fs, 140.0f, 0.707f, juce::Decibels::decibelsToGain (w * 4.0f));
                warmthFilter.process (ctx);
            }

            if (a > 0.001f)
            {
                *airFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                    fs, juce::jmin (9500.0f, (float) fs * 0.4f), 0.707f,
                    juce::Decibels::decibelsToGain (a * 5.0f));
                airFilter.process (ctx);
            }

            if (p > 0.001f)
            {
                const float fastCoeff = (float) std::exp (-1.0 / (fs * 0.002));
                const float slowCoeff = (float) std::exp (-1.0 / (fs * 0.120));

                for (int i = 0; i < n; ++i)
                {
                    float peak = 0.0f;

                    for (int c = 0; c < ch; ++c)
                        peak = juce::jmax (peak, std::abs (buffer.getSample (c, i)));

                    fastEnv = fastCoeff * fastEnv + (1.0f - fastCoeff) * peak;
                    slowEnv = slowCoeff * slowEnv + (1.0f - slowCoeff) * peak;

                    const float transient = juce::jlimit (0.0f, 1.5f, (fastEnv - slowEnv) * 3.0f);
                    const float gain = 1.0f + transient * p * 0.9f;

                    for (int c = 0; c < ch; ++c)
                        buffer.setSample (c, i, buffer.getSample (c, i) * gain);
                }
            }

            if (g > 0.001f)
            {
                glue.setParams (-22.0f, 2.0f, 30.0f, 320.0f, 12.0f, 2);
                glue.process (buffer, g, 2);

                // Make up what the glue took away, roughly.
                const float makeup = juce::Decibels::decibelsToGain (glue.getReductionDb() * 0.55f * g);

                for (int c = 0; c < ch; ++c)
                    buffer.applyGain (c, 0, n, makeup);
            }

            if (w > 0.001f)
            {
                const float drive = 1.0f + w * 0.6f;

                for (int c = 0; c < ch; ++c)
                {
                    auto* d = buffer.getWritePointer (c);

                    for (int i = 0; i < n; ++i)
                        d[i] = std::tanh (d[i] * drive) / drive;
                }
            }

            const float gain = juce::Decibels::decibelsToGain (loudnessDb);

            for (int c = 0; c < ch; ++c)
                buffer.applyGain (c, 0, n, gain);

            limiter.setThreshold (juce::jlimit (-24.0f, 0.0f, ceilingDb));
            limiter.process (ctx);
        }

        updateMeters (buffer);
    }

    float getLufs() const { return lufsSmoothed; }
    float getCorrelation() const { return correlation; }
    float getPeak() const { return peakOut; }

private:
    void updateMeters (const juce::AudioBuffer<float>& buffer)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);

        if (n <= 0)
            return;

        double sumL = 0.0, sumR = 0.0, sumLR = 0.0;
        float peak = 0.0f;

        for (int i = 0; i < n; ++i)
        {
            const float l = buffer.getSample (0, i);
            const float r = ch > 1 ? buffer.getSample (1, i) : l;
            sumL += (double) l * l;
            sumR += (double) r * r;
            sumLR += (double) l * r;
            peak = juce::jmax (peak, juce::jmax (std::abs (l), std::abs (r)));
        }

        const double mean = (sumL + sumR) / (double) (n * juce::jmax (1, ch));
        const float lufs = (float) (-0.691 + 10.0 * std::log10 (juce::jmax (1.0e-12, mean)));
        lufsSmoothed = lufsSmoothed < -60.0f ? lufs : lufsSmoothed + (lufs - lufsSmoothed) * 0.15f;

        const double denom = std::sqrt (juce::jmax (1.0e-12, sumL * sumR));
        const float corr = (float) juce::jlimit (-1.0, 1.0, sumLR / denom);
        correlation += (corr - correlation) * 0.2f;

        peakOut = peak;
    }

    using Duplicator = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                      juce::dsp::IIR::Coefficients<float>>;

    juce::dsp::Limiter<float> limiter;
    VocalCompressor glue;
    Duplicator warmthFilter, airFilter;
    double fs = 44100.0;
    float fastEnv = 0.0f, slowEnv = 0.0f, lufsSmoothed = -70.0f, correlation = 1.0f, peakOut = 0.0f;
};
