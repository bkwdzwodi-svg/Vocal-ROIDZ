#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>

// ---------------------------------------------------------------------------------------
// DelayLab: analog / tape / ping-pong / ducking delay with tone shaping and width.
// ---------------------------------------------------------------------------------------
class DelayLab
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        const auto maxSamples = (int) std::ceil (spec.sampleRate * 1.4) + 8;

        line.setMaximumDelayInSamples (maxSamples);
        line.prepare (spec);

        tone.prepare (spec);
        tone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        tone.setResonance (0.707f);

        wow.prepare (spec);
        wow.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

        delaySmoothed.reset (spec.sampleRate, 0.05);
        delaySmoothed.setCurrentAndTargetValue (0.3f * (float) spec.sampleRate);

        lfoPhase = 0.0f;
        duckEnv = 0.0f;
        reset();
    }

    void reset()
    {
        line.reset();
        tone.reset();
        wow.reset();
        duckEnv = 0.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, int mode, float timeMs, float feedback,
                  float toneHz, float width, float mix)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float fb = juce::jlimit (0.0f, 0.92f, feedback);
        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float w = juce::jlimit (0.0f, 1.0f, width);

        delaySmoothed.setTargetValue (juce::jlimit (20.0f, 1200.0f, timeMs) * 0.001f * (float) fs);
        tone.setCutoffFrequency (juce::jlimit (300.0f, (float) fs * 0.45f, toneHz));

        const float lfoInc = (float) (juce::MathConstants<double>::twoPi * (mode == 1 ? 0.55 : 0.25) / fs);

        for (int i = 0; i < n; ++i)
        {
            const float baseDelay = delaySmoothed.getNextValue();
            const float wowDepth = mode == 1 ? baseDelay * 0.012f : 0.0f;
            const float d = baseDelay + wowDepth * std::sin (lfoPhase);

            lfoPhase += lfoInc;
            if (lfoPhase > juce::MathConstants<float>::twoPi)
                lfoPhase -= juce::MathConstants<float>::twoPi;

            float wet[2] = { 0.0f, 0.0f };
            float dry[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
            {
                dry[c] = buffer.getSample (c, i);
                wet[c] = line.popSample (c, d, true);
                wet[c] = tone.processSample (c, wet[c]);

                if (mode == 1)                                   // tape: soften the repeats
                    wet[c] = std::tanh (wet[c] * 1.1f) / 1.1f;
                else if (mode == 3)                              // ducking
                {
                    const float level = juce::jmax (std::abs (dry[0]), std::abs (dry[1]));
                    duckEnv = level > duckEnv ? level : duckEnv * 0.9995f;
                    const float duck = juce::jlimit (0.15f, 1.0f, 1.0f - duckEnv * 2.5f);
                    wet[c] *= duck;
                }
            }

            if (mode == 2 && ch == 2)                            // ping pong
            {
                line.pushSample (0, dry[0] + wet[1] * fb);
                line.pushSample (1, dry[1] + wet[0] * fb);
            }
            else
            {
                for (int c = 0; c < ch; ++c)
                    line.pushSample (c, dry[c] + wet[c] * fb);
            }

            // Width on the wet signal only, so the dry vocal stays put.
            if (ch == 2)
            {
                const float mid = 0.5f * (wet[0] + wet[1]);
                const float side = 0.5f * (wet[0] - wet[1]) * (0.4f + w * 1.6f);
                wet[0] = mid + side;
                wet[1] = mid - side;
            }

            for (int c = 0; c < ch; ++c)
                buffer.setSample (c, i, dry[c] + wet[c] * m);
        }
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> line;
    juce::dsp::StateVariableTPTFilter<float> tone, wow;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delaySmoothed;
    double fs = 44100.0;
    float lfoPhase = 0.0f, duckEnv = 0.0f;
};

// ---------------------------------------------------------------------------------------
// ReverbSpace: seven spaces on top of JUCE's reverb, with pre-delay, tone and modulation.
// ---------------------------------------------------------------------------------------
class ReverbSpace
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        reverb.prepare (spec);

        preDelay.setMaximumDelayInSamples ((int) std::ceil (spec.sampleRate * 0.25) + 4);
        preDelay.prepare (spec);

        tone.prepare (spec);
        tone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        tone.setResonance (0.707f);

        wet.setSize (juce::jlimit (1, 2, (int) spec.numChannels), juce::jmax (32, (int) spec.maximumBlockSize));
        lfoPhase = 0.0f;
        reset();
    }

    void reset()
    {
        reverb.reset();
        preDelay.reset();
        tone.reset();
    }

    void process (juce::AudioBuffer<float>& buffer, int algo, float decay, float predelayMs,
                  float width, float toneHz, float mod, float mix)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2, wet.getNumChannels());

        if (n > wet.getNumSamples() || ch <= 0)
            return;

        for (int c = 0; c < ch; ++c)
            wet.copyFrom (c, 0, buffer, c, 0, n);

        preDelay.setDelay (juce::jlimit (0.0f, 200.0f, predelayMs) * 0.001f * (float) fs);

        // The delay line has to be fed as well as tapped: without the push every tap reads
        // silence and the whole wet path disappears.
        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                preDelay.pushSample (c, buffer.getSample (c, i));
                wet.setSample (c, i, preDelay.popSample (c));
            }
        }

        juce::dsp::Reverb::Parameters params;
        params.roomSize = 0.35f;
        params.damping = 0.5f;

        switch (algo)
        {
            case 0: params.roomSize = 0.32f; params.damping = 0.65f; break;   // Room
            case 1: params.roomSize = 0.62f; params.damping = 0.42f; break;   // Hall
            case 2: params.roomSize = 0.55f; params.damping = 0.28f; break;   // Plate
            case 3: params.roomSize = 0.80f; params.damping = 0.35f; break;   // Arena
            case 4: params.roomSize = 0.70f; params.damping = 0.55f; break;   // Ambient
            case 5: params.roomSize = 0.78f; params.damping = 0.20f; break;   // Dream
            default: params.roomSize = 0.92f; params.damping = 0.30f; break;  // Cathedral
        }

        // The decay control stretches each space from intimate to endless.
        params.roomSize = juce::jlimit (0.05f, 0.99f, params.roomSize * (0.55f + decay * 0.75f));
        params.damping = juce::jlimit (0.0f, 1.0f, params.damping * (1.0f - mod * 0.25f));
        params.width = juce::jlimit (0.0f, 1.0f, width);
        params.wetLevel = 1.0f;
        params.dryLevel = 0.0f;
        params.freezeMode = 0.0f;

        reverb.setParameters (params);

        juce::dsp::AudioBlock<float> block (wet.getArrayOfWritePointers(), (size_t) ch, 0, (size_t) n);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        reverb.process (ctx);

        tone.setCutoffFrequency (juce::jlimit (800.0f, (float) fs * 0.45f, toneHz));

        // Slight modulation on the tail keeps it from sounding static.
        lfoPhase += juce::MathConstants<float>::twoPi * 0.13f * (float) n / (float) fs;
        if (lfoPhase > juce::MathConstants<float>::twoPi)
            lfoPhase -= juce::MathConstants<float>::twoPi;

        const float shimmer = 1.0f + 0.010f * mod * std::sin (lfoPhase);
        const float m = juce::jlimit (0.0f, 1.0f, mix);

        for (int c = 0; c < ch; ++c)
        {
            auto* out = buffer.getWritePointer (c);
            auto* w = wet.getReadPointer (c);

            for (int i = 0; i < n; ++i)
            {
                const float sm = tone.processSample (c, w[i]) * shimmer;
                out[i] = out[i] * (1.0f - m) + sm * m;
            }
        }
    }

private:
    juce::dsp::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> preDelay;
    juce::dsp::StateVariableTPTFilter<float> tone;
    juce::AudioBuffer<float> wet;
    double fs = 44100.0;
    float lfoPhase = 0.0f;
};

// ---------------------------------------------------------------------------------------
// VocalDoubler: two detuned, drifting short delays, spread hard left and right.
// ---------------------------------------------------------------------------------------
class VocalDoubler
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        line.setMaximumDelayInSamples ((int) std::ceil (spec.sampleRate * 0.12) + 8);
        line.prepare (spec);
        phase[0] = 0.0f;
        phase[1] = 1.7f;
    }

    void reset() { line.reset(); }

    void process (juce::AudioBuffer<float>& buffer, float amount, float width, float human, float pitchDrift)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float amt = juce::jlimit (0.0f, 1.0f, amount);

        if (amt < 0.001f)
            return;

        const float w = juce::jlimit (0.0f, 1.0f, width);
        const float depthMs = 1.2f + human * 6.0f;
        const float driftMs = pitchDrift * 2.5f;

        for (int i = 0; i < n; ++i)
        {
            float dry[2] = { 0.0f, 0.0f };
            float wet[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
                dry[c] = buffer.getSample (c, i);

            const float mod0 = std::sin (phase[0]) * depthMs * 0.001f * (float) fs;
            const float mod1 = std::sin (phase[1]) * (depthMs + driftMs) * 0.001f * (float) fs;

            phase[0] += (float) (juce::MathConstants<double>::twoPi * 0.37 / fs);
            phase[1] += (float) (juce::MathConstants<double>::twoPi * 0.53 / fs);

            if (phase[0] > juce::MathConstants<float>::twoPi) phase[0] -= juce::MathConstants<float>::twoPi;
            if (phase[1] > juce::MathConstants<float>::twoPi) phase[1] -= juce::MathConstants<float>::twoPi;

            const float base0 = 0.012f * (float) fs;
            const float base1 = 0.019f * (float) fs;

            for (int c = 0; c < ch; ++c)
            {
                const float d0 = base0 + mod0;
                const float d1 = base1 + mod1;

                wet[c] = 0.5f * (line.popSample (c, d0, true) + line.popSample (c, d1, false));
                line.pushSample (c, dry[c]);
            }

            if (ch == 2)
            {
                // Spread the double away from the centre.
                const float side = (wet[0] - wet[1]) * 0.5f * w;
                const float mid = (wet[0] + wet[1]) * 0.5f;
                wet[0] = mid + side * 1.6f;
                wet[1] = mid - side * 1.6f;
            }

            for (int c = 0; c < ch; ++c)
                buffer.setSample (c, i, dry[c] + wet[c] * amt * 0.55f);
        }
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> line;
    double fs = 44100.0;
    float phase[2] = { 0.0f, 0.0f };
};
