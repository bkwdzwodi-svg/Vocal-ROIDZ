#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>
#include <vector>
#include "Creative.h"

// ---------------------------------------------------------------------------------------
// ConvolverSpace
//
// A real convolution reverb. Four synthetic impulse responses (room, plate, hall, church)
// are generated at prepare time and loaded into JUCE's partitioned convolver, then filtered,
// pre-delayed and mixed like a hardware IR unit.
// ---------------------------------------------------------------------------------------
class ConvolverSpace
{
public:
    static constexpr int numSpaces = 4;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;

        juce::dsp::ProcessSpec stereo = spec;
        stereo.numChannels = 2;

        for (int i = 0; i < numSpaces; ++i)
        {
            conv[(size_t) i].prepare (stereo);
            conv[(size_t) i].loadImpulseResponse (makeIr (i), fs,
                                                  juce::dsp::Convolution::Stereo::yes,
                                                  juce::dsp::Convolution::Trim::yes,
                                                  juce::dsp::Convolution::Normalise::yes);
        }

        preDelay.setMaximumDelayInSamples ((int) std::ceil (fs * 0.2) + 4);
        preDelay.prepare (stereo);

        tone.prepare (stereo);
        tone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        tone.setResonance (0.707f);

        wet.setSize (2, juce::jmax (32, (int) spec.maximumBlockSize));
        activeSpace = -1;
        reset();
    }

    void reset()
    {
        for (auto& c : conv)
            c.reset();

        preDelay.reset();
        tone.reset();
        wet.clear();
    }

    void process (juce::AudioBuffer<float>& buffer, int space, float predelayMs,
                  float toneHz, float width, float mix)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2, wet.getNumChannels());

        if (n <= 0 || ch <= 0 || n > wet.getNumSamples())
            return;

        space = juce::jlimit (0, numSpaces - 1, space);

        for (int c = 0; c < ch; ++c)
            wet.copyFrom (c, 0, buffer, c, 0, n);

        juce::dsp::AudioBlock<float> block (wet.getArrayOfWritePointers(), (size_t) ch, 0, (size_t) n);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        conv[(size_t) space].process (ctx);

        preDelay.setDelay (juce::jlimit (0.0f, 150.0f, predelayMs) * 0.001f * (float) fs);
        tone.setCutoffFrequency (juce::jlimit (800.0f, (float) fs * 0.45f, toneHz));

        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float w = juce::jlimit (0.0f, 1.0f, width);
        activeSpace = space;

        for (int i = 0; i < n; ++i)
        {
            float w0 = 0.0f, w1 = 0.0f;

            for (int c = 0; c < ch; ++c)
            {
                preDelay.pushSample (c, wet.getSample (c, i));
                const float delayed = preDelay.popSample (c);
                const float shaped = tone.processSample (c, delayed);

                if (c == 0)
                    w0 = shaped;
                else
                    w1 = shaped;
            }

            if (ch == 2)
            {
                const float mid = 0.5f * (w0 + w1);
                const float side = 0.5f * (w0 - w1) * (0.35f + w * 1.65f);
                w0 = mid + side;
                w1 = mid - side;
            }

            for (int c = 0; c < ch; ++c)
            {
                const float dry = buffer.getSample (c, i);
                const float wetSample = c == 0 ? w0 : w1;
                buffer.setSample (c, i, dry + wetSample * m);
            }
        }
    }

private:
    juce::AudioBuffer<float> makeIr (int space) const
    {
        static const float seconds[numSpaces] = { 0.9f, 1.7f, 2.7f, 3.6f };
        static const float dampHz[numSpaces]  = { 9000.0f, 7200.0f, 5200.0f, 3800.0f };

        const int len = juce::jmax (64, (int) (fs * (double) seconds[juce::jlimit (0, numSpaces - 1, space)]));
        juce::AudioBuffer<float> ir (2, len);
        const float lpCoeff = std::exp (-juce::MathConstants<float>::twoPi
                                        * dampHz[juce::jlimit (0, numSpaces - 1, space)] / (float) fs);

        for (int c = 0; c < 2; ++c)
        {
            juce::Random rng (0x51afb125u + (juce::uint32) space * 977u + (juce::uint32) c * 31u);
            float lp = 0.0f;
            auto* out = ir.getWritePointer (c);

            for (int i = 0; i < len; ++i)
            {
                const float t = (float) i / (float) len;
                const float decay = std::exp (-t * (5.0f + (float) space * 0.9f));
                const float noise = rng.nextFloat() * 2.0f - 1.0f;
                lp = lpCoeff * lp + (1.0f - lpCoeff) * noise;
                out[i] = (noise * 0.32f + lp * 0.68f) * decay;
            }

            // A handful of early reflections gives the tail a room around it.
            const int early = (int) juce::jmin ((float) len - 8, (float) fs * 0.09f);

            for (int k = 0; k < 16; ++k)
            {
                const int pos = 8 + (int) (rng.nextFloat() * (float) juce::jmax (1, early));
                const float gain = 0.55f * std::exp (-(float) pos / (float) (fs * 0.035));
                out[juce::jmin (len - 1, pos)] += (k % 2 == 0 ? gain : -gain);
            }

            float peak = 0.0f;

            for (int i = 0; i < len; ++i)
                peak = juce::jmax (peak, std::abs (out[i]));

            if (peak > 1.0e-6f)
            {
                const float s = 0.85f / peak;

                for (int i = 0; i < len; ++i)
                    out[i] *= s;
            }
        }

        return ir;
    }

    std::array<juce::dsp::Convolution, numSpaces> conv;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::None> preDelay;
    juce::dsp::StateVariableTPTFilter<float> tone;
    juce::AudioBuffer<float> wet;
    double fs = 44100.0;
    int activeSpace = -1;
};

// ---------------------------------------------------------------------------------------
// CassetteTape
//
// The sound of a cassette: wow and flutter wobble the read head, the drive stage rounds the
// peaks, a low pass rolls the top off and a little hiss sits under everything.
// ---------------------------------------------------------------------------------------
class CassetteTape
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;

        juce::dsp::ProcessSpec stereo = spec;
        stereo.numChannels = 2;

        line.setMaximumDelayInSamples ((int) std::ceil (fs * 0.12) + 8);
        line.prepare (stereo);

        tone.prepare (stereo);
        tone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        tone.setResonance (0.707f);

        rng.setSeed (0x0badc0de);
        wowPhase = 0.0f;
        flutterPhase = 0.0f;
        reset();
    }

    void reset()
    {
        line.reset();
        tone.reset();
    }

    void process (juce::AudioBuffer<float>& buffer, float drive, float wow, float toneHz,
                  float hiss, float mix)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float d = juce::jlimit (0.0f, 1.0f, drive);
        const float w = juce::jlimit (0.0f, 1.0f, wow);
        const float h = juce::jlimit (0.0f, 1.0f, hiss);
        const float m = juce::jlimit (0.0f, 1.0f, mix);

        const float baseMs = 9.0f;
        const float wowMs = w * 3.2f;
        const float flutterMs = w * 0.35f;
        const float wowInc = (float) (juce::MathConstants<double>::twoPi * 0.55 / fs);
        const float flutterInc = (float) (juce::MathConstants<double>::twoPi * 6.1 / fs);
        const float driveGain = 1.0f + d * 5.0f;
        const float driveComp = 1.0f / std::tanh (driveGain);

        tone.setCutoffFrequency (juce::jlimit (1000.0f, (float) fs * 0.45f, toneHz));

        for (int i = 0; i < n; ++i)
        {
            const float wobble = wowMs * std::sin (wowPhase) + flutterMs * std::sin (flutterPhase);
            wowPhase += wowInc;
            flutterPhase += flutterInc;

            if (wowPhase > juce::MathConstants<float>::twoPi) wowPhase -= juce::MathConstants<float>::twoPi;
            if (flutterPhase > juce::MathConstants<float>::twoPi) flutterPhase -= juce::MathConstants<float>::twoPi;

            const float delaySamples = (baseMs + wobble) * 0.001f * (float) fs;

            for (int c = 0; c < ch; ++c)
            {
                const float dry = buffer.getSample (c, i);
                const float read = line.popSample (c, delaySamples, true);
                line.pushSample (c, dry);

                const float driven = std::tanh (read * driveGain) * driveComp;
                const float filtered = tone.processSample (c, driven);
                const float noise = (rng.nextFloat() * 2.0f - 1.0f) * 0.0035f * h;
                const float wetSample = filtered + noise;

                buffer.setSample (c, i, dry * (1.0f - m) + wetSample * m);
            }
        }
    }

private:
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> line;
    juce::dsp::StateVariableTPTFilter<float> tone;
    juce::Random rng;
    double fs = 44100.0;
    float wowPhase = 0.0f, flutterPhase = 0.0f;
};

// ---------------------------------------------------------------------------------------
// DJSpin
//
// A timed backspin on the vocal: the read head brakes, drags backwards and catches up again,
// like a DJ throwing a record back on the one.
// ---------------------------------------------------------------------------------------
class DJSpin
{
public:
    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        ring.prepare (sampleRate, 3.0, numChannels);
        reset();
    }

    void reset()
    {
        ring.clear();
        readPos = -1.0f;
        countdown = 0;
        spinLeft = 0;
        spinSamples = 1;
    }

    void process (juce::AudioBuffer<float>& buffer, float everySeconds, float lengthMs,
                  float depth, float mix)
    {
        if (! ring.ready())
            return;

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float amount = juce::jlimit (0.0f, 1.0f, depth);

        if (m < 0.001f || amount < 0.001f)
            return;

        const int rateSamples = juce::jmax (64, (int) (juce::jlimit (2.0f, 30.0f, everySeconds) * fs));
        spinSamples = juce::jmax (64, (int) (juce::jlimit (120.0f, 1500.0f, lengthMs) * 0.001f * fs));

        if (readPos < 0.0f)
            readPos = (float) ring.now() - 128.0f;

        if (countdown <= 0)
            countdown = rateSamples;

        for (int i = 0; i < n; ++i)
        {
            float dry[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
            {
                dry[c] = buffer.getSample (c, i);
                ring.write (c, dry[c]);
            }

            const float nowF = (float) ring.now();
            float speed = 1.0f;

            if (--countdown <= 0)
            {
                countdown = rateSamples;
                spinLeft = spinSamples;
            }

            if (spinLeft > 0)
            {
                const float t = 1.0f - (float) spinLeft / (float) spinSamples;
                speed = 1.0f - 1.75f * amount * std::sin (juce::MathConstants<float>::pi * t);
                --spinLeft;
            }
            else
            {
                const float lag = nowF - readPos;

                if (lag > 0.03f * (float) fs)
                    speed = 1.0f + juce::jmin (0.85f, (lag - 0.03f * (float) fs) * 0.0004f);
                else if (lag < 32.0f)
                    speed = 0.0f;
            }

            readPos += speed;

            if (readPos > nowF - 2.0f)
                readPos = nowF - 2.0f;

            if (readPos < nowF - (float) ring.mask * 0.7f)
                readPos = nowF - (float) ring.mask * 0.7f;

            float wet[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
                wet[c] = ring.read (c, readPos);

            ring.advance();

            for (int c = 0; c < ch; ++c)
                buffer.setSample (c, i, dry[c] * (1.0f - m) + wet[c] * m);
        }
    }

private:
    StereoRing ring;
    double fs = 44100.0;
    float readPos = -1.0f;
    int countdown = 0, spinLeft = 0, spinSamples = 1;
};
