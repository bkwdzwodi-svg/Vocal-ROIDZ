#pragma once
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <vector>

// A small shared helper: power-of-two ring buffer with linear interpolation.
struct StereoRing
{
    void prepare (double sampleRate, double seconds, int numChannels)
    {
        channels = juce::jlimit (1, 2, numChannels);
        int size = 1;
        const int wanted = (int) std::ceil (sampleRate * seconds) + 8;

        while (size < wanted)
            size <<= 1;

        mask = size - 1;
        data.assign ((size_t) channels, std::vector<float> ((size_t) size, 0.0f));
        writePos = 0;
        writeCount = 0;
    }

    void clear()
    {
        for (auto& d : data)
            std::fill (d.begin(), d.end(), 0.0f);

        writePos = 0;
        writeCount = 0;
    }

    void write (int ch, float value) { data[(size_t) ch][(size_t) writePos] = value; }

    void advance()
    {
        writePos = (writePos + 1) & mask;
        ++writeCount;
    }

    // Absolute sample position of the next write slot. Using an absolute clock keeps
    // every read head correct across buffer wraps.
    double now() const { return (double) writeCount; }

    float read (int ch, float position) const
    {
        const auto& d = data[(size_t) ch];
        int i0 = (int) std::floor (position);
        const float frac = position - (float) i0;
        const float a = d[(size_t) (i0 & mask)];
        const float b = d[(size_t) ((i0 + 1) & mask)];
        return a * (1.0f - frac) + b * frac;
    }

    bool ready() const { return ! data.empty(); }

    std::vector<std::vector<float>> data;
    int mask = 0, writePos = 0, channels = 2;
    long long writeCount = 0;
};

// ---------------------------------------------------------------------------------------
// HyperStutter: captures a slice of audio and repeats it, with reverse, randomness and
// pitch movement. The six modes are presets over those controls.
// ---------------------------------------------------------------------------------------
class HyperStutter
{
public:
    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        ring.prepare (sampleRate, 2.5, numChannels);
        reset();
    }

    void reset()
    {
        ring.clear();
        counter = 0;
        playPos = 0.0f;
        sliceStart = 0;
        sliceLen = 0;
        speed = 1.0f;
        reversed = false;
        rng.setSeed (0x5f3759df);
    }

    void process (juce::AudioBuffer<float>& buffer, int mode, float sliceMs, float rateHz,
                  float reverse, float randomAmount, float pitchSemitones, float mix)
    {
        if (! ring.ready())
            return;

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);

        // Mode presets sit on top of the controls.
        float sliceScale = 1.0f, rateScale = 1.0f, pitch = pitchSemitones;
        bool forceReverse = false;

        switch (mode)
        {
            case 0: sliceScale = 0.55f; rateScale = 1.3f; break;                    // Micro Chop
            case 1: sliceScale = 0.35f; rateScale = 1.8f; break;                    // Machine Gun
            case 2: sliceScale = 0.7f;  rateScale = 1.15f; pitch += 5.0f; break;    // Hyperpop
            case 3: forceReverse = true; sliceScale = 0.8f; break;                  // Reverse Chaos
            case 4: sliceScale = 1.0f;  rateScale = 1.0f; break;                    // Freeze
            case 5: sliceScale = 0.4f;  rateScale = 1.6f; break;                    // Glitch Storm
            default: break;
        }

        const float sliceSamples = juce::jlimit (64.0f, (float) (ring.mask / 3),
                                                 sliceMs * sliceScale * 0.001f * (float) fs);
        const float periodSamples = juce::jmax (256.0f, (float) fs / juce::jmax (0.5f, rateHz * rateScale));
        speed = (mode == 4) ? 0.0f : std::pow (2.0f, pitch / 12.0f);
        reversed = forceReverse || reverse > 0.5f;

        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float randomise = juce::jlimit (0.0f, 1.0f, mode == 5 ? 1.0f : randomAmount);

        for (int i = 0; i < n; ++i)
        {
            float dry[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
            {
                dry[c] = buffer.getSample (c, i);
                ring.write (c, dry[c]);
            }

            if (counter <= 0)
            {
                float offset = 0.0f;

                if (randomise > 0.001f)
                    offset = (rng.nextFloat() * 2.0f - 1.0f) * randomise * sliceSamples * 0.75f;

                sliceStart = (int) ((float) ring.now() - sliceSamples + offset);
                sliceLen = (int) sliceSamples;
                playPos = reversed ? (float) (sliceStart + sliceLen - 1) : (float) sliceStart;
                counter = (int) periodSamples;
            }

            --counter;
            ring.advance();

            for (int c = 0; c < ch; ++c)
            {
                const float wet = ring.read (c, playPos);
                buffer.setSample (c, i, dry[c] * (1.0f - m) + wet * m);
            }

            playPos += reversed ? -speed : speed;

            if (playPos >= (float) (sliceStart + sliceLen))
                playPos -= (float) sliceLen;
            else if (playPos < (float) sliceStart)
                playPos += (float) sliceLen;
        }
    }

private:
    StereoRing ring;
    double fs = 44100.0;
    int counter = 0, sliceStart = 0, sliceLen = 0;
    float playPos = 0.0f, speed = 1.0f;
    bool reversed = false;
    juce::Random rng;
};

// ---------------------------------------------------------------------------------------
// ChopNScrew: varispeed playback with phrase repeats and tape drift. The read head runs
// slower than the write head, and jumps back on the chop grid to repeat a phrase.
// ---------------------------------------------------------------------------------------
class ChopNScrew
{
public:
    void prepare (double sampleRate, int numChannels)
    {
        fs = sampleRate;
        ring.prepare (sampleRate, 6.0, numChannels);
        tone.prepare ({ sampleRate, 512, (juce::uint32) juce::jlimit (1, 2, numChannels) });
        tone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        reset();
    }

    void reset()
    {
        ring.clear();
        readPos = 0.0f;
        chopCounter = 0;
        lfo = 0.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, int mode, float pitchSemitones, float slowAmount,
                  float density, float repeat, float drift, float mix)
    {
        if (! ring.ready())
            return;

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);

        float slow = juce::jlimit (0.0f, 1.0f, slowAmount);
        float repeatAmt = juce::jlimit (0.0f, 1.0f, repeat);
        float driftAmt = juce::jlimit (0.0f, 1.0f, drift);
        float dens = juce::jlimit (0.0f, 1.0f, density);
        float pitch = pitchSemitones;
        float toneHz = 12000.0f;

        switch (mode)
        {
            case 0: break;                                                              // Classic Screw
            case 1: slow *= 1.15f; pitch -= 1.5f; repeatAmt *= 1.2f; break;             // Texas Heavy
            case 2: slow *= 1.05f; dens *= 1.35f; repeatAmt *= 1.25f; toneHz = 9000.f; break; // BKWDZ WODI
            case 3: toneHz = 2800.0f; slow *= 1.1f; break;                              // Dark Atmosphere
            case 4: driftAmt = juce::jmax (driftAmt, 0.55f); toneHz = 7000.f; break;     // Trance Screw
            default: dens = 1.0f; repeatAmt = 1.0f; driftAmt = 1.0f; break;             // Chaos Screw
        }

        const float rate = juce::jlimit (0.15f, 1.0f, (1.0f - slow * 0.55f) * std::pow (2.0f, pitch / 12.0f));
        const float chopLen = juce::jmax (512.0f, (float) fs * (0.35f - dens * 0.28f));
        const float maxLag = (float) (ring.mask / 3);

        tone.setCutoffFrequency (juce::jlimit (500.0f, (float) fs * 0.45f, toneHz));

        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float lfoInc = (float) (juce::MathConstants<double>::twoPi * 0.7 / fs);

        if (readPos <= 0.0f)
            readPos = (float) ring.now() - 2048.0f;

        for (int i = 0; i < n; ++i)
        {
            float dry[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
            {
                dry[c] = buffer.getSample (c, i);
                ring.write (c, dry[c]);
            }

            float rateNow = rate;

            if (driftAmt > 0.001f)
            {
                lfo += lfoInc;
                if (lfo > juce::MathConstants<float>::twoPi)
                    lfo -= juce::MathConstants<float>::twoPi;

                rateNow *= 1.0f + 0.012f * driftAmt * std::sin (lfo);
            }

            readPos += rateNow;

            // Keep the head inside the buffer; jumping forward is itself a chop.
            const float lag = (float) ring.now() - readPos;

            if (lag < 32.0f)
                readPos -= (float) ring.mask * 0.25f;
            else if (lag > maxLag)
                readPos = (float) ring.now() - 2048.0f;

            if (--chopCounter <= 0)
            {
                chopCounter = (int) chopLen;

                if (repeatAmt > 0.001f)
                    readPos -= chopLen * repeatAmt * 0.85f;
            }

            ring.advance();

            for (int c = 0; c < ch; ++c)
            {
                float wet = ring.read (c, readPos);
                wet = tone.processSample (c, wet);

                if (mode == 1 || mode == 4)
                    wet = std::tanh (wet * 1.2f) / 1.2f;   // tape-ish glue

                buffer.setSample (c, i, dry[c] * (1.0f - m) + wet * m);
            }
        }
    }

private:
    StereoRing ring;
    juce::dsp::StateVariableTPTFilter<float> tone;
    double fs = 44100.0;
    float readPos = 0.0f, lfo = 0.0f;
    int chopCounter = 0;
};

// ---------------------------------------------------------------------------------------
// CreativeFxLab: eight creative processors behind one selector.
// ---------------------------------------------------------------------------------------
class CreativeFxLab
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        ring.prepare (spec.sampleRate, 1.5, (int) spec.numChannels);

        band.prepare (spec);
        band.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        band.setResonance (0.9f);

        cut.prepare (spec);
        cut.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        cut.setResonance (0.707f);

        reset();
    }

    void reset()
    {
        ring.clear();
        band.reset();
        cut.reset();
        phase = 0.0f;
        playPos = 0.0f;
        reversePos = 0.0f;
        holdCounter = 0;
        retrigger = 0;
        holdValue[0] = holdValue[1] = 0.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, int type, float amount, float toneHz, float mix)
    {
        if (! ring.ready())
            return;

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), 2);
        const float amt = juce::jlimit (0.0f, 1.0f, amount);
        const float m = juce::jlimit (0.0f, 1.0f, mix);
        const float tone = juce::jlimit (300.0f, (float) fs * 0.45f, toneHz);

        // Filter settings are block-rate: cheap, and stable.
        if (type == 3)
        {
            band.setCutoffFrequency (1100.0f);
            band.setResonance (0.6f + amt * 1.8f);
        }
        else if (type == 4)
        {
            band.setCutoffFrequency (1300.0f);
            band.setResonance (1.5f);
            cut.setCutoffFrequency (juce::jmin (3400.0f, (float) fs * 0.45f));
        }
        else if (type == 7)
        {
            cut.setCutoffFrequency (tone);
        }

        const int chunk = (int) juce::jmax (1024.0f, (float) fs * (0.12f + amt * 0.28f));
        const float stopPeriod = (float) fs * 3.0f;

        for (int i = 0; i < n; ++i)
        {
            float dry[2] = { 0.0f, 0.0f };

            for (int c = 0; c < ch; ++c)
            {
                dry[c] = buffer.getSample (c, i);
                ring.write (c, dry[c]);
            }

            const float nowPos = (float) ring.now() - 1.0f;
            ring.advance();

            float wet[2] = { dry[0], dry[1] };

            switch (type)
            {
                case 0:   // Reverse FX
                {
                    if (retrigger <= 0)
                    {
                        reversePos = nowPos;
                        retrigger = chunk;
                    }

                    --retrigger;

                    for (int c = 0; c < ch; ++c)
                        wet[c] = ring.read (c, reversePos);

                    reversePos -= 1.0f;

                    if (reversePos < nowPos - (float) chunk)
                        reversePos = nowPos;

                    break;
                }

                case 1:   // Tape stop
                case 2:   // Vinyl stop
                {
                    if (retrigger <= 0)
                    {
                        playPos = nowPos;
                        retrigger = (int) stopPeriod;
                    }

                    --retrigger;

                    const float t = 1.0f - (float) retrigger / stopPeriod;
                    const float rate = juce::jlimit (0.03f, 1.0f, 1.0f - amt * 0.95f * t * t);
                    playPos += rate;

                    if (playPos > nowPos)
                        playPos = nowPos;

                    for (int c = 0; c < ch; ++c)
                        wet[c] = ring.read (c, playPos);

                    if (type == 2)
                    {
                        for (int c = 0; c < ch; ++c)
                            wet[c] += (rng.nextFloat() * 2.0f - 1.0f) * 0.01f * amt;
                    }

                    break;
                }

                case 3:   // Megaphone
                    for (int c = 0; c < ch; ++c)
                    {
                        const float bp = band.processSample (c, dry[c]);
                        wet[c] = std::tanh ((bp * 3.0f + dry[c] * 0.3f) * (1.0f + amt * 5.0f));
                    }
                    break;

                case 4:   // Telephone voice
                    for (int c = 0; c < ch; ++c)
                        wet[c] = cut.processSample (c, band.processSample (c, dry[c])) * 1.6f;
                    break;

                case 5:   // Robot voice
                {
                    phase += juce::MathConstants<float>::twoPi * (28.0f + amt * 90.0f) / (float) fs;

                    if (phase > juce::MathConstants<float>::twoPi)
                        phase -= juce::MathConstants<float>::twoPi;

                    const float mod = std::sin (phase);

                    for (int c = 0; c < ch; ++c)
                        wet[c] = std::tanh (dry[c] * mod * 2.5f) * 0.75f;

                    break;
                }

                case 6:   // Bit crusher
                {
                    const int holdSamples = 1 + (int) (amt * 40.0f);

                    if (--holdCounter <= 0)
                    {
                        holdCounter = holdSamples;

                        const float levels = std::pow (2.0f, 9.0f - amt * 7.0f);

                        for (int c = 0; c < ch; ++c)
                            holdValue[c] = std::round (dry[c] * levels) / levels;
                    }

                    for (int c = 0; c < ch; ++c)
                        wet[c] = holdValue[c];

                    break;
                }

                default:  // Granular echo
                {
                    if (--holdCounter <= 0)
                    {
                        holdCounter = (int) juce::jmax (64.0f, (float) fs * (0.02f + amt * 0.08f));
                        phase = nowPos - 32.0f - rng.nextFloat() * (float) (ring.mask / 8);
                    }

                    for (int c = 0; c < ch; ++c)
                        wet[c] = cut.processSample (c, ring.read (c, phase));

                    phase += 1.0f;

                    if (phase > nowPos - 4.0f)
                        phase = nowPos - (float) (ring.mask / 8);

                    break;
                }
            }

            for (int c = 0; c < ch; ++c)
                buffer.setSample (c, i, dry[c] * (1.0f - m) + wet[c] * m);
        }
    }

private:
    StereoRing ring;
    juce::dsp::StateVariableTPTFilter<float> band, cut;
    juce::Random rng;
    double fs = 44100.0;
    float phase = 0.0f, playPos = 0.0f, reversePos = 0.0f;
    int holdCounter = 0, retrigger = 0;
    float holdValue[2] = { 0.0f, 0.0f };
};
