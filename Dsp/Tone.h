#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>

// ---------------------------------------------------------------------------------------
// AnalogEQ: high pass, low pass, low/high shelves, two parametric bells and an air shelf.
// Coefficients are rebuilt only when a control actually moves.
// ---------------------------------------------------------------------------------------
class AnalogEQ
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        using Coeffs = juce::dsp::IIR::Coefficients<float>;
        fs = spec.sampleRate;
        const auto nyq = (float) (fs * 0.5);

        // ProcessorDuplicator builds its per-channel filters around the state pointer passed
        // at prepare time, so every band needs real coefficients before prepare().
        hpf.state      = Coeffs::makeHighPass (fs, 85.0f, 0.707f);
        lpf.state      = Coeffs::makeLowPass (fs, juce::jmin (18000.0f, nyq * 0.9f), 0.707f);
        lowShelf.state = Coeffs::makeLowShelf (fs, 160.0f, 0.707f, 1.0f);
        highShelf.state = Coeffs::makeHighShelf (fs, juce::jmin (8000.0f, nyq * 0.7f), 0.707f, 1.0f);
        peak1.state    = Coeffs::makePeakFilter (fs, 2200.0f, 0.9f, 1.0f);
        peak2.state    = Coeffs::makePeakFilter (fs, 2200.0f, 0.9f, 1.0f);
        air.state      = Coeffs::makeHighShelf (fs, juce::jmin (12000.0f, nyq * 0.85f), 0.707f, 1.0f);

        for (auto* f : { &hpf, &lpf, &lowShelf, &highShelf, &peak1, &peak2, &air })
            f->prepare (spec);

        lastHpf = lastLpf = lastLow = lastLowFreq = lastMid = lastMidFreq = lastMidQ = -9999.0f;
        lastHigh = lastHighFreq = lastAir = -9999.0f;
        reset();
    }

    void reset()
    {
        for (auto* f : { &hpf, &lpf, &lowShelf, &highShelf, &peak1, &peak2, &air })
            f->reset();
    }

    void process (juce::AudioBuffer<float>& buffer,
                  float hpfHz, float lpfHz,
                  float lowGain, float lowFreq,
                  float midGain, float midFreq, float midQ,
                  float highGain, float highFreq, float airGain)
    {
        using Coeffs = juce::dsp::IIR::Coefficients<float>;
        const auto nyq = (float) (fs * 0.5);

        hpfHz  = juce::jlimit (20.0f, nyq * 0.9f, hpfHz);
        lpfHz  = juce::jlimit (500.0f, nyq * 0.9f, lpfHz);
        lowFreq = juce::jlimit (30.0f, nyq * 0.5f, lowFreq);
        midFreq = juce::jlimit (120.0f, nyq * 0.8f, midFreq);
        midQ = juce::jlimit (0.2f, 10.0f, midQ);
        highFreq = juce::jlimit (1000.0f, nyq * 0.85f, highFreq);

        if (std::abs (hpfHz - lastHpf) > 0.01f)
        {
            *hpf.state = *Coeffs::makeHighPass (fs, hpfHz, 0.707f);
            lastHpf = hpfHz;
        }

        if (std::abs (lpfHz - lastLpf) > 0.01f)
        {
            *lpf.state = *Coeffs::makeLowPass (fs, lpfHz, 0.707f);
            lastLpf = lpfHz;
        }

        if (std::abs (lowGain - lastLow) > 0.001f || std::abs (lowFreq - lastLowFreq) > 0.01f)
        {
            *lowShelf.state = *Coeffs::makeLowShelf (fs, lowFreq, 0.707f, juce::Decibels::decibelsToGain (lowGain));
            lastLow = lowGain;
            lastLowFreq = lowFreq;
        }

        if (std::abs (midGain - lastMid) > 0.001f || std::abs (midFreq - lastMidFreq) > 0.01f
            || std::abs (midQ - lastMidQ) > 0.001f)
        {
            *peak1.state = *Coeffs::makePeakFilter (fs, midFreq, midQ, juce::Decibels::decibelsToGain (midGain));
            lastMid = midGain;
            lastMidFreq = midFreq;
            lastMidQ = midQ;
        }

        if (std::abs (highGain - lastHigh) > 0.001f || std::abs (highFreq - lastHighFreq) > 0.01f)
        {
            *highShelf.state = *Coeffs::makeHighShelf (fs, highFreq, 0.707f,
                                                      juce::Decibels::decibelsToGain (highGain));
            lastHigh = highGain;
            lastHighFreq = highFreq;
        }

        if (std::abs (airGain - lastAir) > 0.001f)
        {
            *air.state = *Coeffs::makeHighShelf (fs, juce::jmin (12000.0f, nyq * 0.85f), 0.707f,
                                                juce::Decibels::decibelsToGain (airGain));
            lastAir = airGain;
        }

        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);

        hpf.process (ctx);
        lpf.process (ctx);
        lowShelf.process (ctx);
        peak1.process (ctx);
        highShelf.process (ctx);
        air.process (ctx);
    }

private:
    using Duplicator = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                      juce::dsp::IIR::Coefficients<float>>;

    Duplicator hpf, lpf, lowShelf, highShelf, peak1, peak2, air;
    double fs = 44100.0;
    float lastHpf = -9999.f, lastLpf = -9999.f, lastLow = -9999.f, lastLowFreq = -9999.f;
    float lastMid = -9999.f, lastMidFreq = -9999.f, lastMidQ = -9999.f;
    float lastHigh = -9999.f, lastHighFreq = -9999.f, lastAir = -9999.f;
};

// ---------------------------------------------------------------------------------------
// SaturationEngine: six flavours of drive, with a warmth stage and a parallel mix.
// ---------------------------------------------------------------------------------------
class SaturationEngine
{
public:
    void prepare (double sampleRate)
    {
        fs = sampleRate;
        tapeTone.reset();
        reset();
    }

    void reset()
    {
        lpState[0] = lpState[1] = 0.0f;
        dcState[0] = dcState[1] = 0.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, int mode, float driveDb, float warmth,
                  float density, float mix)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float drive = juce::Decibels::decibelsToGain (juce::jlimit (0.0f, 30.0f, driveDb));
        const float w = juce::jlimit (0.0f, 1.0f, warmth);
        const float d = juce::jlimit (0.0f, 1.0f, density);
        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float comp = 1.0f / (1.0f + (drive - 1.0f) * 0.55f);
        const float lpCoeff = (float) std::exp (-juce::MathConstants<double>::twoPi * 11000.0 / fs);

        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < ch; ++c)
            {
                const float dry = buffer.getSample (c, i);
                float x = dry * drive;

                // Density biases the stage so the curve gets less symmetrical.
                x += d * 0.18f * x * x;

                float y = 0.0f;

                switch (mode)
                {
                    case 0:  y = std::tanh (x + 0.12f * d); break;                                  // Tube
                    case 1:  y = std::tanh (x * 0.8f) * 1.05f; break;                               // Tape
                    case 2:  y = std::tanh (x * 1.4f) / 1.4f; break;                                // Vintage
                    case 3:  y = juce::jlimit (-1.0f, 1.0f, x * 0.85f); break;                      // Modern
                    case 4:  y = std::tanh (x * 0.6f) + 0.25f * w * (x * x - 0.3f); break;           // Warm
                    default:                                                                        // Aggressive
                    {
                        // Triangle fold that passes through zero with the same polarity as the
                        // input; inverting here would cancel the dry signal in the parallel mix.
                        y = 1.0f - std::abs (std::fmod (x + 1.0f, 4.0f) - 2.0f);
                        break;
                    }
                }

                // Second-harmonic warmth.
                y += w * 0.22f * (y * y - 0.2f);

                if (mode == 1)
                {
                    lpState[c] = lpCoeff * lpState[c] + (1.0f - lpCoeff) * y;
                    y = lpState[c];
                }

                // Keep the stage from wandering off with DC.
                dcState[c] += 0.002f * (y - dcState[c]);
                y -= dcState[c];

                y *= comp;

                buffer.setSample (c, i, dry * (1.0f - m) + y * m);
            }
        }
    }

private:
    double fs = 44100.0;
    float lpState[2] = { 0.0f, 0.0f };
    float dcState[2] = { 0.0f, 0.0f };
    struct Empty { void reset() {} } tapeTone;
};

// ---------------------------------------------------------------------------------------
// NoiseCleanup: hum notches, breath-band taming and click repair.
// ---------------------------------------------------------------------------------------
class NoiseCleanup
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        scratch.setSize (juce::jlimit (1, 2, (int) spec.numChannels), juce::jmax (32, (int) spec.maximumBlockSize));

        using Coeffs = juce::dsp::IIR::Coefficients<float>;
        hum50.state  = Coeffs::makeNotch (spec.sampleRate, 50.0f, 30.0f);
        hum100.state = Coeffs::makeNotch (spec.sampleRate, 100.0f, 30.0f);
        hum60.state  = Coeffs::makeNotch (spec.sampleRate, 60.0f, 30.0f);
        hum120.state = Coeffs::makeNotch (spec.sampleRate, 120.0f, 30.0f);

        for (auto* f : { &hum50, &hum100, &hum60, &hum120 })
        {
            f->prepare (spec);
            f->reset();
        }

        breathBand.prepare (spec);
        breathBand.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        breathBand.setResonance (0.9f);
        breathBand.setCutoffFrequency (4500.0f);
        breathBand.reset();

        env = 0.0f;
        clickHold = 0;
    }

    void reset()
    {
        for (auto* f : { &hum50, &hum100, &hum60, &hum120 })
            f->reset();

        breathBand.reset();
        env = 0.0f;
        clickHold = 0;
        prevSample[0] = prevSample[1] = 0.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, float hum, float breath, float clicks, float amount)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float amt = juce::jlimit (0.0f, 1.0f, amount);

        if (hum > 0.001f && scratch.getNumSamples() >= n && scratch.getNumChannels() >= ch)
        {
            using Coeffs = juce::dsp::IIR::Coefficients<float>;

            *hum50.state  = *Coeffs::makeNotch (fs, 50.0f, 30.0f);
            *hum100.state = *Coeffs::makeNotch (fs, 100.0f, 30.0f);
            *hum60.state  = *Coeffs::makeNotch (fs, 60.0f, 30.0f);
            *hum120.state = *Coeffs::makeNotch (fs, 120.0f, 30.0f);

            for (int c = 0; c < ch; ++c)
                scratch.copyFrom (c, 0, buffer, c, 0, n);

            juce::dsp::AudioBlock<float> block (scratch);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            hum50.process (ctx);
            hum100.process (ctx);
            hum60.process (ctx);
            hum120.process (ctx);

            const float depth = hum * amt;

            for (int c = 0; c < ch; ++c)
            {
                auto* out = buffer.getWritePointer (c);
                const auto* filtered = scratch.getReadPointer (c);

                for (int i = 0; i < n; ++i)
                    out[i] += (filtered[i] - out[i]) * depth;
            }
        }

        if (breath > 0.001f)
        {
            const float thresh = -44.0f - (1.0f - amt) * 6.0f;
            const float depth = breath * amt;

            for (int i = 0; i < n; ++i)
            {
                float bandPeak = 0.0f;

                for (int c = 0; c < ch; ++c)
                    bandPeak = juce::jmax (bandPeak, std::abs (breathBand.processSample (c, buffer.getSample (c, i))));

                env = 0.85f * env + 0.15f * bandPeak;
                const float envDb = juce::Decibels::gainToDecibels (env, -100.0f);

                if (envDb < thresh)
                {
                    const float g = juce::Decibels::decibelsToGain (-9.0f * depth);
                    for (int c = 0; c < ch; ++c)
                        buffer.setSample (c, i, buffer.getSample (c, i) * g);
                }
            }
        }

        if (clicks > 0.001f)
        {
            const float limit = 0.35f + (1.0f - juce::jlimit (0.0f, 1.0f, clicks * amt)) * 1.2f;

            for (int i = 0; i < n; ++i)
            {
                bool repaired = false;

                for (int c = 0; c < ch; ++c)
                {
                    const float x = buffer.getSample (c, i);
                    const float delta = std::abs (x - prevSample[c]);

                    if (clickHold > 0 || delta > limit)
                    {
                        buffer.setSample (c, i, prevSample[c] * 0.6f + x * 0.4f);
                        repaired = true;
                    }

                    prevSample[c] = buffer.getSample (c, i);
                }

                if (repaired)
                    clickHold = 6;
                else if (clickHold > 0)
                    --clickHold;
            }
        }
    }

private:
    using Duplicator = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                      juce::dsp::IIR::Coefficients<float>>;

    double fs = 44100.0;
    Duplicator hum50, hum100, hum60, hum120;
    juce::dsp::StateVariableTPTFilter<float> breathBand;
    juce::AudioBuffer<float> scratch;
    float env = 0.0f, prevSample[2] = { 0.0f, 0.0f };
    int clickHold = 0;
};

// ---------------------------------------------------------------------------------------
// StereoImager: mid/side width, mono bass and balance.
// ---------------------------------------------------------------------------------------
class StereoImager
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        sideLow.prepare (spec);
        sideLow.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        sideLow.setResonance (0.707f);
        sideLow.reset();
    }

    void reset() { sideLow.reset(); }

    void process (juce::AudioBuffer<float>& buffer, float width, float bassMonoHz, float balance)
    {
        if (buffer.getNumChannels() < 2)
            return;

        sideLow.setCutoffFrequency (juce::jlimit (20.0f, 400.0f, bassMonoHz));

        const int n = buffer.getNumSamples();
        const float w = juce::jlimit (0.0f, 2.0f, width);
        const float bal = juce::jlimit (-1.0f, 1.0f, balance);
        const float lg = bal < 0.0f ? 1.0f + bal : 1.0f;
        const float rg = bal > 0.0f ? 1.0f - bal : 1.0f;

        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);

        for (int i = 0; i < n; ++i)
        {
            const float mid = 0.5f * (l[i] + r[i]);
            const float side = 0.5f * (l[i] - r[i]);

            const float low = sideLow.processSample (0, side);
            const float high = side - low;

            const float outSide = high * w + low;
            l[i] = (mid + outSide) * lg;
            r[i] = (mid - outSide) * rg;
        }
    }

private:
    juce::dsp::StateVariableTPTFilter<float> sideLow;
    double fs = 44100.0;
};
