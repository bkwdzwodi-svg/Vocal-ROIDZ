#pragma once
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <cmath>
#include <vector>

// ---------------------------------------------------------------------------------------
// GranularPitchShifter
//
// Two Hann-windowed read heads half a grain apart, both sliding through a circular buffer.
// Their windows sum to exactly 1, so the crossfade is click-free while the delay ramps,
// which is what bends the pitch. Used by the Pitch FX module, the harmony engine and the
// creative pitch effects.
// ---------------------------------------------------------------------------------------
class GranularPitchShifter
{
public:
    void prepare (double sampleRate, int maxBlock, int numChannels)
    {
        fs = sampleRate;
        channels = juce::jlimit (1, 2, numChannels);
        grain = juce::jmax (128, (int) std::lround (sampleRate * 0.05));

        int size = 1;
        while (size < grain * 3 + juce::jmax (16, maxBlock) + 8)
            size <<= 1;

        ringSize = size;
        mask = size - 1;
        rings.assign ((size_t) channels, std::vector<float> ((size_t) size, 0.0f));
        writePos = 0;
        phase = 0.0f;
        prepared = true;
    }

    void reset()
    {
        for (auto& r : rings)
            std::fill (r.begin(), r.end(), 0.0f);

        writePos = 0;
        phase = 0.0f;
    }

    bool isPrepared() const { return prepared; }

    void process (juce::AudioBuffer<float>& buffer, float ratio)
    {
        if (! prepared)
            return;

        ratio = juce::jlimit (0.25f, 4.0f, ratio);

        const int n = buffer.getNumSamples();
        const int numCh = juce::jmin (buffer.getNumChannels(), channels);
        const float inc = (1.0f - ratio) / (float) grain;

        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < numCh; ++c)
                rings[(size_t) c][(size_t) (writePos & mask)] = buffer.getSample (c, i);

            const float p1 = phase;
            float p2 = phase + 0.5f;
            if (p2 >= 1.0f)
                p2 -= 1.0f;

            const float d1 = p1 * (float) grain;
            const float d2 = p2 * (float) grain;
            const float g1 = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * p1);
            const float g2 = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::twoPi * p2);

            for (int c = 0; c < numCh; ++c)
            {
                const float s1 = read (c, d1);
                const float s2 = read (c, d2);
                buffer.setSample (c, i, s1 * g1 + s2 * g2);
            }

            phase += inc;
            phase -= std::floor (phase);
            writePos = (writePos + 1) & mask;
        }
    }

private:
    float read (int c, float delaySamples) const
    {
        float pos = (float) writePos - delaySamples;

        while (pos < 0.0f)
            pos += (float) ringSize;

        const int i0 = (int) pos;
        const float frac = pos - (float) i0;
        const int i1 = (i0 + 1) & mask;
        const auto& r = rings[(size_t) c];

        return r[(size_t) (i0 & mask)] * (1.0f - frac) + r[(size_t) i1] * frac;
    }

    double fs = 44100.0;
    int channels = 2, grain = 2048, ringSize = 8192, mask = 8191, writePos = 0;
    float phase = 0.0f;
    bool prepared = false;
    std::vector<std::vector<float>> rings;
};

// ---------------------------------------------------------------------------------------
// PsolaShifter
//
// Pitch-synchronous overlap-add. Grains are two pitch periods long, cut from the input on
// a pitch-period grid and re-emitted at the corrected period, so the harmonics move but
// the spectral envelope (the formants, and with them the singer's tone) stays put. A small
// fixed delay (two of the longest tracked periods) gives every grain its samples without
// reading into the future.
// ---------------------------------------------------------------------------------------
class PsolaShifter
{
public:
    void prepare (double sampleRate, int maxBlock, int numChannels)
    {
        fs = sampleRate;
        channels = juce::jlimit (1, 2, numChannels);
        maxHalf = (int) std::ceil (sampleRate / 90.0) + 2;   // lowest tracked voice ~90 Hz
        delay = maxHalf * 2 + 8;

        int size = 1;
        while (size < delay + juce::jmax (16, maxBlock) + maxHalf * 2 + 16)
            size <<= 1;

        mask = size - 1;
        rings.assign ((size_t) channels, std::vector<float> ((size_t) size, 0.0f));

        halfPeriod = (float) (sampleRate / 220.0);
        writePos = 0;
        count = 0;
        numGrains = 0;
        nextGrain = 0.0;
        prepared = true;
    }

    void reset()
    {
        for (auto& r : rings)
            std::fill (r.begin(), r.end(), 0.0f);

        writePos = 0;
        count = 0;
        numGrains = 0;
        nextGrain = 0.0;
    }

    bool isPrepared() const { return prepared; }
    int latencySamples() const { return delay; }

    void process (juce::AudioBuffer<float>& buffer, float ratio, float detectedHz)
    {
        if (! prepared)
            return;

        ratio = juce::jlimit (0.5f, 2.0f, ratio);

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), channels);

        const float targetHalf = (detectedHz > 45.0f && detectedHz < (float) (fs * 0.45))
                                     ? (float) (fs / (double) detectedHz)
                                     : halfPeriod;

        halfPeriod += (juce::jlimit (24.0f, (float) maxHalf, targetHalf) - halfPeriod) * 0.35f;

        const double h = (double) halfPeriod;
        const double hop = juce::jlimit (h * 0.5, h * 1.98, h / (double) ratio);

        for (int i = 0; i < n; ++i)
        {
            for (int c = 0; c < ch; ++c)
                rings[(size_t) c][(size_t) writePos] = buffer.getSample (c, i);

            const double m = (double) count - (double) delay;

            while (nextGrain <= (double) count - h)
            {
                if (numGrains < (int) grains.size())
                {
                    Grain g;
                    g.h = h;
                    g.oc = nextGrain;
                    g.ic = std::floor (nextGrain / h) * h;
                    grains[(size_t) numGrains++] = g;
                }

                nextGrain += hop;
            }

            // Drop grains that can no longer touch this sample.
            int live = 0;

            for (int gi = 0; gi < numGrains; ++gi)
            {
                const auto& g = grains[(size_t) gi];

                if (m < g.oc + g.h)
                {
                    grains[(size_t) live] = g;
                    ++live;
                }
            }

            numGrains = live;

            float acc[2] = { 0.0f, 0.0f };
            float norm = 0.0f;

            for (int gi = 0; gi < numGrains; ++gi)
            {
                const auto& g = grains[(size_t) gi];
                const double tau = m - g.oc;

                if (tau <= -g.h || tau >= g.h)
                    continue;

                const float w = (float) (0.5 + 0.5 * std::cos (juce::MathConstants<double>::pi * tau / g.h));

                for (int c = 0; c < ch; ++c)
                    acc[c] += w * read (c, (float) (g.ic + tau));

                norm += w;
            }

            const float inv = norm > 1.0e-4f ? 1.0f / norm : 0.0f;

            for (int c = 0; c < ch; ++c)
                buffer.setSample (c, i, acc[c] * inv);

            writePos = (writePos + 1) & mask;
            ++count;
        }
    }

private:
    struct Grain { double oc = 0.0, ic = 0.0, h = 0.0; };

    // Read the input at an absolute sample index. `count` is the index of the sample just
    // written at `writePos`, and every grain read stays at or behind it.
    float read (int c, float absPos) const
    {
        if (absPos < 0.0f)
            return 0.0f;

        int i0 = (int) std::floor (absPos);
        float frac = absPos - (float) i0;
        int back0 = (int) count - i0;

        if (back0 < 0)
        {
            back0 = 0;
            frac = 0.0f;
        }

        int back1 = back0 - 1;

        if (back1 < 0)
            back1 = 0;

        const auto& r = rings[(size_t) c];
        const float a = r[(size_t) ((writePos - back0) & mask)];
        const float b = r[(size_t) ((writePos - back1) & mask)];

        return a + (b - a) * frac;
    }

    double fs = 44100.0;
    int channels = 2, maxHalf = 551, delay = 1102, mask = 8191, writePos = 0;
    long long count = 0;
    float halfPeriod = 200.0f;
    double nextGrain = 0.0;
    std::array<Grain, 16> grains {};
    int numGrains = 0;
    bool prepared = false;
    std::vector<std::vector<float>> rings;
};

// ---------------------------------------------------------------------------------------
// AutoTune
//
// Monophonic pitch tracking (normalised autocorrelation on a decimated window), snapped to
// the nearest note of the chosen key and scale in the singer's own octave, then corrected
// by at most a semitone. Correction always uses the classic granular path (no formant
// preservation, no mode switches) and always snaps hard, so changing key or scale can never
// move the voice's register: creative pitch lives in the Pitch FX module.
// ---------------------------------------------------------------------------------------
class AutoTune
{
public:
    struct Settings
    {
        int key = 0, scale = 0;
        float retune = 1.0f, humanize = 0.0f, amount = 1.0f;
    };

    static juce::StringArray keyNames()
    {
        return { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    }

    static juce::StringArray scaleNames()
    {
        return { "Major", "Minor", "Harmonic Minor", "Melodic Minor", "Chromatic",
                 "Dorian", "Phrygian", "Lydian", "Mixolydian", "Locrian",
                 "Pentatonic Major", "Pentatonic Minor" };
    }

    void prepare (double sampleRate, int maxBlock, int numChannels)
    {
        fs = sampleRate;
        shifter.prepare (sampleRate, maxBlock, numChannels);

        const int hopSamples = juce::jmax (64, (int) std::lround (sampleRate * 0.010));  // 10 ms
        hop = hopSamples;
        history.assign (4096, 0.0f);
        writeIdx = 0;
        hopCounter = 0;
        ratioSmoothed = 1.0f;
        targetRatio = 1.0f;
        voicedHz = 220.0f;
        detectedHz.store (0.0f);
        targetHz.store (0.0f);
    }

    void reset()
    {
        shifter.reset();
        std::fill (history.begin(), history.end(), 0.0f);
        writeIdx = 0;
        hopCounter = 0;
        ratioSmoothed = 1.0f;
        targetRatio = 1.0f;
        detectedHz.store (0.0f);
        targetHz.store (0.0f);
    }

    // Pitch the tracker currently hears, and the note it is tuning towards, for the graph.
    std::atomic<float> detectedHz { 0.0f }, targetHz { 0.0f };

    void process (juce::AudioBuffer<float>& buffer, const Settings& s)
    {
        if (! shifter.isPrepared() || history.empty())
            return;

        const int n = buffer.getNumSamples();
        const int numCh = juce::jlimit (1, 2, buffer.getNumChannels());

        for (int i = 0; i < n; ++i)
        {
            const float mono = numCh > 1 ? 0.5f * (buffer.getSample (0, i) + buffer.getSample (1, i))
                                         : buffer.getSample (0, i);
            history[(size_t) writeIdx] = mono;
            writeIdx = (writeIdx + 1) % (int) history.size();

            if (++hopCounter >= hop)
            {
                hopCounter = 0;
                updateTarget (s);
            }
        }

        // Classic hard tune: the snap lands in 2-45 ms, always on the note. The control only
        // decides how much of that tiny window the correction takes.
        const float timeSec = juce::jmap (juce::jlimit (0.0f, 1.0f, s.retune), 0.045f, 0.002f);
        const float coeff = 1.0f - std::exp (-(float) n / (float) (timeSec * fs));
        ratioSmoothed += (targetRatio - ratioSmoothed) * juce::jlimit (0.0f, 1.0f, coeff);

        shifter.process (buffer, ratioSmoothed);
    }

private:
    static const std::vector<int>& scaleTable (int index)
    {
        static const std::vector<std::vector<int>> tables =
        {
            { 0, 2, 4, 5, 7, 9, 11 },              // Major
            { 0, 2, 3, 5, 7, 8, 10 },              // Minor
            { 0, 2, 3, 5, 7, 8, 11 },              // Harmonic Minor
            { 0, 2, 3, 5, 7, 9, 11 },              // Melodic Minor
            { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 }, // Chromatic
            { 0, 2, 3, 5, 7, 9, 10 },              // Dorian
            { 0, 1, 3, 5, 7, 8, 10 },              // Phrygian
            { 0, 2, 4, 6, 7, 9, 11 },              // Lydian
            { 0, 2, 4, 5, 7, 9, 10 },              // Mixolydian
            { 0, 1, 3, 5, 6, 8, 10 },              // Locrian
            { 0, 2, 4, 7, 9 },                     // Pentatonic Major
            { 0, 3, 5, 7, 10 }                     // Pentatonic Minor
        };

        return tables[(size_t) juce::jlimit (0, (int) tables.size() - 1, index)];
    }

    static float midiToHz (float note)
    {
        return 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
    }

    static float hzToMidi (float hz)
    {
        return 69.0f + 12.0f * std::log2 (hz / 440.0f);
    }

    // Nearest note in the selected key/scale *to the note being sung*. It searches the
    // singer's own octave plus one either side, so the nearest note is at most a semitone
    // away and a key change can never transpose the voice.
    static float snapToScale (float midi, int key, int scale)
    {
        const auto& table = scaleTable (scale);
        const int base = juce::jlimit (0, 11, key);
        const int octave = (int) std::floor (midi / 12.0f) * 12;

        float best = midi;
        float bestDist = 1.0e9f;

        for (int oct = -1; oct <= 1; ++oct)
        {
            for (int deg : table)
            {
                const float candidate = (float) (octave + oct * 12 + base + deg);
                const float dist = std::abs (candidate - midi);

                if (dist < bestDist)
                {
                    bestDist = dist;
                    best = candidate;
                }
            }
        }

        return best;
    }

    void updateTarget (const Settings& s)
    {
        const float hz = detectPitch();

        if (hz <= 0.0f)
        {
            detectedHz.store (0.0f);
            targetHz.store (0.0f);
            targetRatio = 1.0f;
            return;
        }

        voicedHz = hz;

        const float midi = hzToMidi (hz);
        const float target = snapToScale (midi, s.key, s.scale);
        const float centsError = (target - midi) * 100.0f;

        float depth = juce::jlimit (0.0f, 1.0f, s.amount);

        // Humanize sets a very small deadband: the note is still snapped, but a few cents of
        // natural movement survive so the take keeps its life.
        if (s.humanize > 0.001f)
        {
            const float deadband = s.humanize * 12.0f;
            depth *= juce::jlimit (0.0f, 1.0f, std::abs (centsError) / juce::jmax (1.0f, deadband));
        }

        // Hard guard: the tuner only ever nudges the note. Anything bigger is a Pitch FX job.
        const float semitones = juce::jlimit (-1.0f, 1.0f, centsError * depth / 100.0f);
        targetRatio = juce::jlimit (0.5f, 2.0f, std::pow (2.0f, semitones / 12.0f));

        detectedHz.store (hz);
        targetHz.store (midiToHz (target));
    }

    float detectPitch() const
    {
        constexpr int win = 512;          // decimated window
        constexpr int decimate = 2;

        if ((int) history.size() < win * decimate + 4)
            return 0.0f;

        float x[win];
        int idx = writeIdx - win * decimate;

        if (idx < 0)
            idx += (int) history.size();

        float energy = 0.0f;

        for (int i = 0; i < win; ++i)
        {
            const int a = (idx + i * decimate) % (int) history.size();
            const int b = (idx + i * decimate + 1) % (int) history.size();
            x[i] = 0.5f * (history[(size_t) a] + history[(size_t) b]);
            energy += x[i] * x[i];
        }

        if (energy / (float) win < 1.0e-7f)      // silence / noise floor
            return 0.0f;

        const double fsEff = fs / (double) decimate;
        const int minLag = juce::jmax (2, (int) std::floor (fsEff / 1100.0));
        const int maxLag = juce::jmin (win - 8, (int) std::ceil (fsEff / 65.0));

        if (maxLag <= minLag + 2)
            return 0.0f;

        float bestScore = 0.0f;
        int bestLag = 0;
        float scores[512] = {};

        const int inner = win - maxLag;

        for (int lag = minLag; lag <= maxLag; ++lag)
        {
            float corr = 0.0f, e1 = 0.0f, e2 = 0.0f;

            for (int i = 0; i < inner; ++i)
            {
                corr += x[i] * x[i + lag];
                e1 += x[i] * x[i];
                e2 += x[i + lag] * x[i + lag];
            }

            const float score = corr / std::sqrt (e1 * e2 + 1.0e-12f);
            scores[lag] = score;

            if (score > bestScore)
            {
                bestScore = score;
                bestLag = lag;
            }
        }

        if (bestScore < 0.55f || bestLag <= 0)
            return 0.0f;

        // Parabolic interpolation around the winning lag for sub-sample accuracy.
        float refined = (float) bestLag;

        if (bestLag > minLag && bestLag < maxLag)
        {
            const float a = scores[bestLag - 1], b = scores[bestLag], c = scores[bestLag + 1];
            const float denom = a - 2.0f * b + c;

            if (std::abs (denom) > 1.0e-9f)
                refined += 0.5f * (a - c) / denom;
        }

        if (refined < 1.0f)
            return 0.0f;

        return (float) (fsEff / (double) refined);
    }

    GranularPitchShifter shifter;
    std::vector<float> history;
    int writeIdx = 0, hop = 441, hopCounter = 0;
    float ratioSmoothed = 1.0f, targetRatio = 1.0f, voicedHz = 220.0f;
    double fs = 44100.0;
};

// ---------------------------------------------------------------------------------------
// HarmonyEngine: stacked pitch-shifted copies at a third, fifth and the octaves.
// ---------------------------------------------------------------------------------------
class HarmonyEngine
{
public:
    void prepare (double sampleRate, int maxBlock, int numChannels)
    {
        for (auto& s : shifters)
            s.prepare (sampleRate, maxBlock, numChannels);

        scratch.setSize (juce::jlimit (1, 2, numChannels), juce::jmax (32, maxBlock));
    }

    void reset()
    {
        for (auto& s : shifters)
            s.reset();
    }

    void process (juce::AudioBuffer<float>& buffer, float third, float fifth, float octUp, float octDown)
    {
        const float levels[4] = { juce::jlimit (0.0f, 1.0f, third), juce::jlimit (0.0f, 1.0f, fifth),
                                  juce::jlimit (0.0f, 1.0f, octUp), juce::jlimit (0.0f, 1.0f, octDown) };
        static const float ratios[4] = { 1.259921f, 1.498307f, 2.0f, 0.5f };

        float sum = 0.0f;
        for (float l : levels)
            sum += l;

        if (sum < 0.001f || scratch.getNumSamples() < buffer.getNumSamples())
            return;

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), scratch.getNumChannels());

        for (int v = 0; v < 4; ++v)
        {
            if (levels[v] < 0.001f)
                continue;

            for (int c = 0; c < ch; ++c)
                scratch.copyFrom (c, 0, buffer, c, 0, n);

            shifters[(size_t) v].process (scratch, ratios[v]);

            for (int c = 0; c < ch; ++c)
                buffer.addFrom (c, 0, scratch, c, 0, n, levels[v] / (1.0f + sum));
        }

        const float dryGain = 1.0f / (1.0f + sum);

        for (int c = 0; c < ch; ++c)
            buffer.applyGain (c, 0, n, dryGain);
    }

private:
    std::array<GranularPitchShifter, 4> shifters;
    juce::AudioBuffer<float> scratch;
};

// ---------------------------------------------------------------------------------------
// PitchFx: the creative half of the pitch story. Pitch up/down (±2 octaves), a formant
// shifter that moves the vocal tract resonances without touching the fundamental, and a
// dry/wet mix. Completely independent of the autotune.
// ---------------------------------------------------------------------------------------
class PitchFx
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        fs = spec.sampleRate;
        shifter.prepare (spec.sampleRate, (int) spec.maximumBlockSize,
                         (int) juce::jlimit (1u, 2u, spec.numChannels));

        scratch.setSize (juce::jlimit (1, 2, (int) spec.numChannels),
                         juce::jmax (32, (int) spec.maximumBlockSize));

        using Coeffs = juce::dsp::IIR::Coefficients<float>;

        // The duplicator binds its coefficient pointer at prepare time, so seed real
        // coefficients first and mutate them in place later.
        band0.state = Coeffs::makePeakFilter (spec.sampleRate, 550.0f, 1.1f, 1.0f);
        band1.state = Coeffs::makePeakFilter (spec.sampleRate, 1400.0f, 1.1f, 1.0f);
        band2.state = Coeffs::makePeakFilter (spec.sampleRate, 2800.0f, 1.1f, 1.0f);

        for (auto* f : { &band0, &band1, &band2 })
            f->prepare (spec);

        lastFormant = 9999.0f;
        reset();
    }

    void reset()
    {
        shifter.reset();

        for (auto* f : { &band0, &band1, &band2 })
            f->reset();

        lastFormant = 9999.0f;
    }

    void process (juce::AudioBuffer<float>& buffer, float semitones, float formantSemitones, float mix)
    {
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (buffer.getNumChannels(), scratch.getNumChannels());

        if (n <= 0 || ch <= 0 || scratch.getNumSamples() < n)
            return;

        semitones = juce::jlimit (-24.0f, 24.0f, semitones);
        formantSemitones = juce::jlimit (-12.0f, 12.0f, formantSemitones);

        const bool shifting = std::abs (semitones) > 0.01f;
        const bool forming = std::abs (formantSemitones) > 0.01f;

        if (! shifting && ! forming)
            return;

        for (int c = 0; c < ch; ++c)
            scratch.copyFrom (c, 0, buffer, c, 0, n);

        if (shifting)
            shifter.process (scratch, std::pow (2.0f, semitones / 12.0f));

        if (forming)
        {
            if (std::abs (formantSemitones - lastFormant) > 0.01f)
            {
                using Coeffs = juce::dsp::IIR::Coefficients<float>;
                const float ratio = std::pow (2.0f, formantSemitones / 12.0f);
                const float gain = juce::Decibels::decibelsToGain (9.0f * juce::jmin (1.0f, std::abs (formantSemitones) / 12.0f));
                const float nyq = (float) (fs * 0.5);

                *band0.state = *Coeffs::makePeakFilter (fs, juce::jlimit (80.0f, nyq * 0.8f, 550.0f * ratio), 1.1f, gain);
                *band1.state = *Coeffs::makePeakFilter (fs, juce::jlimit (80.0f, nyq * 0.8f, 1400.0f * ratio), 1.1f, gain);
                *band2.state = *Coeffs::makePeakFilter (fs, juce::jlimit (80.0f, nyq * 0.8f, 2800.0f * ratio), 1.1f, gain);
                lastFormant = formantSemitones;
            }

            juce::dsp::AudioBlock<float> block (scratch);
            juce::dsp::ProcessContextReplacing<float> ctx (block);

            band0.process (ctx);
            band1.process (ctx);
            band2.process (ctx);
        }

        const float m = juce::jlimit (0.0f, 1.0f, mix);

        for (int c = 0; c < ch; ++c)
        {
            auto* out = buffer.getWritePointer (c);
            const auto* wet = scratch.getReadPointer (c);

            for (int i = 0; i < n; ++i)
                out[i] += (wet[i] - out[i]) * m;
        }
    }

private:
    using Duplicator = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                      juce::dsp::IIR::Coefficients<float>>;

    GranularPitchShifter shifter;
    Duplicator band0, band1, band2;
    juce::AudioBuffer<float> scratch;
    double fs = 44100.0;
    float lastFormant = 9999.0f;
};
