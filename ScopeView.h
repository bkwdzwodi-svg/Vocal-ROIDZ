#pragma once
#include "PluginProcessor.h"
#include "Modules.h"
#include "UiStyle.h"

// One graph per module page. Every kind reads the live scope data the processor publishes.
class ScopeView : public juce::Component, private juce::Timer
{
public:
    ScopeView (VocalROIDZProcessor& p, modules::ScopeKind k, juce::Colour c)
        : proc (p), kind (k), accent (c)
    {
        setInterceptsMouseClicks (false, false);
    }

    ~ScopeView() override { stopTimer(); }

    void visibilityChanged() override
    {
        if (isVisible())
            startTimerHz (30);
        else
            stopTimer();
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();

        g.setColour (ui::panel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (accent.withAlpha (0.22f));
        g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

        auto area = b.reduced (14.0f, 12.0f);
        drawGrid (g, area);

        switch (kind)
        {
            case modules::ScopeKind::spectrum:   drawSpectrum (g, area); break;
            case modules::ScopeKind::pitch:      drawPitch (g, area); break;
            case modules::ScopeKind::compressor: drawCompressor (g, area); break;
            case modules::ScopeKind::gate:       drawGate (g, area); break;
            case modules::ScopeKind::harmonics:  drawHarmonics (g, area); break;
            case modules::ScopeKind::stereo:     drawStereo (g, area); break;
            case modules::ScopeKind::decay:      drawDecay (g, area); break;
            case modules::ScopeKind::echo:       drawEcho (g, area); break;
            case modules::ScopeKind::slice:      drawSlice (g, area); break;
            case modules::ScopeKind::speed:      drawSpeed (g, area); break;
            case modules::ScopeKind::sibilance:  drawSibilance (g, area); break;
            case modules::ScopeKind::cleanup:    drawCleanup (g, area); break;
            case modules::ScopeKind::fx:         drawFx (g, area); break;
            case modules::ScopeKind::pitchfx:    drawPitchFx (g, area); break;
            case modules::ScopeKind::stutter:    drawRegularStutter (g, area); break;
            case modules::ScopeKind::convolution: drawConvolution (g, area); break;
            case modules::ScopeKind::cassette:   drawCassette (g, area); break;
            case modules::ScopeKind::spin:       drawSpin (g, area); break;
            default:                             drawLoudness (g, area); break;
        }
    }

private:
    void timerCallback() override { repaint(); }

    float pv (const char* id) const
    {
        if (auto* v = proc.apvts.getRawParameterValue (id))
            return v->load();

        return 0.0f;
    }

    double nowMs() const { return juce::Time::getMillisecondCounterHiRes(); }

    void drawGrid (juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setColour (juce::Colours::white.withAlpha (0.05f));

        for (int i = 0; i <= 4; ++i)
        {
            const float y = area.getY() + area.getHeight() * (float) i / 4.0f;
            g.drawLine (area.getX(), y, area.getRight(), y, 1.0f);
        }

        for (int i = 0; i <= 6; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / 6.0f;
            g.drawLine (x, area.getY(), x, area.getBottom(), 1.0f);
        }
    }

    void label (juce::Graphics& g, juce::Rectangle<float> area, const juce::String& text, bool right = false)
    {
        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText (text, area.toNearestInt(),
                    right ? juce::Justification::topRight : juce::Justification::topLeft, false);
    }

    static juce::String noteName (float midi)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        const int n = (int) std::lround (midi);

        if (n <= 0)
            return "--";

        return juce::String (names[((n % 12) + 12) % 12]) + juce::String (n / 12 - 1);
    }

    // ------------------------------------------------------------------ graphs
    void drawSpectrum (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const auto spec = proc.scope.spectrum;
        juce::Path path;
        path.startNewSubPath (area.getX(), area.getBottom());

        for (int i = 0; i < (int) spec.size(); ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / (float) (spec.size() - 1);
            const float y = area.getBottom() - juce::jlimit (0.0f, 1.0f, spec[(size_t) i]) * area.getHeight();
            path.lineTo (x, y);
        }

        path.lineTo (area.getRight(), area.getBottom());
        path.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.55f), area.getX(), area.getY(),
                                                 accent.withAlpha (0.03f), area.getX(), area.getBottom(), false));
        g.fillPath (path);
        g.setColour (accent.withAlpha (0.9f));
        g.strokePath (path, juce::PathStrokeType (1.2f));

        // Filter markers.
        const float hpf = pv (ids::eqHpf), lpf = pv (ids::eqLpf);
        auto freqToX = [&] (float f)
        {
            const float t = std::log (juce::jlimit (30.0f, 18000.0f, f) / 30.0f) / std::log (18000.0f / 30.0f);
            return area.getX() + t * area.getWidth();
        };

        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.drawVerticalLine ((int) freqToX (hpf), area.getY(), area.getBottom());
        g.drawVerticalLine ((int) freqToX (lpf), area.getY(), area.getBottom());

        label (g, area, "30", false);
        label (g, area, "18k", true);
    }

    void drawPitch (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const auto detected = proc.scope.pitchDetected;
        const auto target = proc.scope.pitchTarget;
        const int size = (int) detected.size();
        const int start = proc.scope.pitchWrite;

        auto midiToY = [&] (float m)
        {
            const float t = juce::jlimit (0.0f, 1.0f, (m - 36.0f) / 48.0f);
            return area.getBottom() - t * area.getHeight();
        };

        g.setColour (juce::Colours::white.withAlpha (0.06f));

        for (int n = 36; n <= 84; n += 12)
            g.drawHorizontalLine ((int) midiToY ((float) n), area.getX(), area.getRight());

        juce::Path detPath, targetPath;
        bool started = false;

        for (int i = 0; i < size; ++i)
        {
            const int idx = (start + i) % size;
            const float x = area.getX() + area.getWidth() * (float) i / (float) (size - 1);
            const float d = detected[(size_t) idx];

            if (d > 0.0f)
            {
                if (! started)
                {
                    detPath.startNewSubPath (x, midiToY (d));
                    targetPath.startNewSubPath (x, midiToY (target[(size_t) idx]));
                    started = true;
                }
                else
                {
                    detPath.lineTo (x, midiToY (d));
                    targetPath.lineTo (x, midiToY (target[(size_t) idx]));
                }
            }
            else
            {
                started = false;
            }
        }

        g.setColour (juce::Colours::white.withAlpha (0.35f));
        g.strokePath (detPath, juce::PathStrokeType (1.4f));

        g.setColour (accent);
        g.strokePath (targetPath, juce::PathStrokeType (1.8f));

        const float t = proc.scope.targetMidi.load();
        g.setFont (ui::font (12.0f, true));
        g.setColour (accent);
        g.drawText (t > 0.0f ? noteName (t) : "LISTENING",
                    area.removeFromTop (18.0f).toNearestInt(), juce::Justification::centredRight, false);
    }

    static float compCurve (float inDb, float thresh, float ratio, float knee)
    {
        const float over = inDb - thresh;

        if (over <= -knee * 0.5f)
            return inDb;

        if (over < knee * 0.5f)
        {
            const float t = over + knee * 0.5f;
            return inDb - (1.0f - 1.0f / ratio) * (t * t) / (2.0f * knee);
        }

        return inDb - (1.0f - 1.0f / ratio) * over;
    }

    void drawCompressor (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float thresh = pv (ids::compThresh);
        const float ratio = pv (ids::compRatio);
        const float knee = pv (ids::compKnee);

        auto xFor = [&] (float db) { return area.getX() + juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f) * area.getWidth(); };
        auto yFor = [&] (float db) { return area.getBottom() - juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f) * area.getHeight(); };

        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawLine (xFor (-60.0f), yFor (-60.0f), xFor (0.0f), yFor (0.0f), 1.0f);

        juce::Path curve;
        curve.startNewSubPath (xFor (-60.0f), yFor (-60.0f));

        for (int i = 1; i <= 40; ++i)
        {
            const float db = -60.0f + 60.0f * (float) i / 40.0f;
            curve.lineTo (xFor (db), yFor (compCurve (db, thresh, ratio, knee)));
        }

        g.setColour (accent);
        g.strokePath (curve, juce::PathStrokeType (1.8f));

        const float levelDb = juce::Decibels::gainToDecibels (proc.scope.inputLevel.load(), -60.0f);
        const float outDb = compCurve (levelDb, thresh, ratio, knee);

        g.setColour (accent.withAlpha (0.9f));
        g.fillEllipse (xFor (levelDb) - 3.5f, yFor (outDb) - 3.5f, 7.0f, 7.0f);

        // Gain reduction bar.
        auto bar = area.removeFromRight (16.0f).reduced (5.0f, 0.0f);
        const float gr = juce::jlimit (0.0f, 24.0f, proc.scope.compReduction.load());
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (bar, 3.0f);
        g.setColour (accent);
        const float h = bar.getHeight() * juce::jlimit (0.0f, 1.0f, gr / 24.0f);
        g.fillRoundedRectangle (bar.withTop (bar.getBottom() - h), 3.0f);

        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText ("-" + juce::String (gr, 1) + " dB GR", area.toNearestInt().withTrimmedRight (20),
                    juce::Justification::topRight, false);
    }

    void drawGate (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float thresh = pv (ids::gateThresh);
        const float y = area.getBottom() - juce::jlimit (0.0f, 1.0f, (thresh + 80.0f) / 70.0f) * area.getHeight();

        g.setColour (accent.withAlpha (0.7f));
        g.drawHorizontalLine ((int) y, area.getX(), area.getRight());

        const float level = juce::Decibels::gainToDecibels (proc.scope.inputLevel.load(), -80.0f);
        const float ly = area.getBottom() - juce::jlimit (0.0f, 1.0f, (level + 80.0f) / 70.0f) * area.getHeight();

        g.setColour (accent.withAlpha (0.25f));
        g.fillRect (area.getX(), ly, area.getWidth(), area.getBottom() - ly);
        g.setColour (accent);
        g.drawHorizontalLine ((int) ly, area.getX(), area.getRight());

        const bool open = proc.scope.gateReduction.load() < 0.5f;

        g.setFont (ui::font (13.0f, true));
        g.setColour (open ? accent : ui::dim);
        g.drawText (open ? "GATE OPEN" : "GATE CLOSED", area.toNearestInt().removeFromBottom (22),
                    juce::Justification::centredLeft, false);

        label (g, area, juce::String (thresh, 0) + " dB", true);
    }

    void drawHarmonics (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const int mode = (int) pv (ids::satMode);
        const float drive = juce::Decibels::decibelsToGain (pv (ids::satDrive));

        auto xFor = [&] (float v) { return area.getX() + (v + 1.0f) * 0.5f * area.getWidth(); };
        auto yFor = [&] (float v) { return area.getCentreY() - v * 0.5f * area.getHeight(); };

        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawLine (xFor (-1.0f), yFor (-1.0f), xFor (1.0f), yFor (1.0f), 1.0f);

        juce::Path curve;
        bool first = true;

        for (int i = 0; i <= 64; ++i)
        {
            const float x = -1.0f + 2.0f * (float) i / 64.0f;
            float v = x * drive;
            float y = 0.0f;

            switch (mode)
            {
                case 0: y = std::tanh (v); break;
                case 1: y = std::tanh (v * 0.8f) * 1.05f; break;
                case 2: y = std::tanh (v * 1.4f) / 1.4f; break;
                case 3: y = juce::jlimit (-1.0f, 1.0f, v * 0.85f); break;
                case 4: y = std::tanh (v * 0.6f); break;
                default: y = std::abs (std::fmod (v + 1.0f, 4.0f) - 2.0f) - 1.0f; break;
            }

            const float px = xFor (x), py = yFor (juce::jlimit (-1.2f, 1.2f, y));

            if (first)
            {
                curve.startNewSubPath (px, py);
                first = false;
            }
            else
            {
                curve.lineTo (px, py);
            }
        }

        g.setColour (accent);
        g.strokePath (curve, juce::PathStrokeType (2.0f));

        static const char* names[] = { "TUBE", "TAPE", "VINTAGE", "MODERN", "WARM", "AGGRESSIVE" };
        label (g, area, names[juce::jlimit (0, 5, mode)], true);
        label (g, area, "TRANSFER CURVE", false);
    }

    void drawStereo (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float corr = proc.scope.correlation.load();
        const float width = pv (ids::stereoWidth);

        auto centre = area.getCentre();
        const float radius = juce::jmin (area.getWidth(), area.getHeight()) * 0.36f;

        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawEllipse ({ centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f }, 1.0f);

        // Correlation as the opening of the goniometer.
        const float spread = juce::jmap (corr, -1.0f, 1.0f, 0.12f, 1.0f);
        juce::Path shape;
        shape.addEllipse (centre.x - radius * spread, centre.y - radius, radius * spread * 2.0f, radius * 2.0f);
        g.setColour (accent.withAlpha (0.18f));
        g.fillPath (shape);
        g.setColour (accent.withAlpha (0.65f));
        g.strokePath (shape, juce::PathStrokeType (1.4f));

        const float t = (float) nowMs() * 0.001f;
        for (int i = 0; i < 5; ++i)
        {
            const float ang = t * (0.6f + 0.1f * (float) i) + (float) i;
            const float x = centre.x + std::sin (ang) * radius * 0.6f * spread;
            const float y = centre.y + std::cos (ang * 1.3f) * radius * 0.6f;
            g.setColour (accent.withAlpha (0.85f));
            g.fillEllipse (x - 2.0f, y - 2.0f, 4.0f, 4.0f);
        }

        g.setFont (ui::font (12.0f, true));
        g.setColour (ui::text);
        g.drawText ("CORR " + juce::String (corr, 2) + "   WIDTH " + juce::String (width, 2),
                    area.toNearestInt().removeFromBottom (20), juce::Justification::centredLeft, false);
    }

    void drawDecay (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float decay = juce::jlimit (0.0f, 1.0f, pv (ids::revDecay));
        const int algo = (int) pv (ids::revAlgo);

        juce::Path path;
        path.startNewSubPath (area.getX(), area.getY() + 6.0f);

        const float len = 0.25f + decay * 0.75f;

        for (int i = 1; i <= 64; ++i)
        {
            const float t = (float) i / 64.0f;
            const float env = std::exp (-t / juce::jmax (0.08f, len * 0.45f));
            path.lineTo (area.getX() + t * area.getWidth(),
                         area.getBottom() - env * (area.getHeight() - 12.0f) - 4.0f);
        }

        g.setColour (accent.withAlpha (0.35f));
        g.strokePath (path, juce::PathStrokeType (2.0f));

        // Travelling tail dots.
        const float speed = 0.12f;
        for (int i = 0; i < 12; ++i)
        {
            const float t = std::fmod ((float) nowMs() * 0.001f * speed + (float) i / 12.0f, 1.0f);
            const float env = std::exp (-t / juce::jmax (0.08f, len * 0.45f));
            const float x = area.getX() + t * area.getWidth();
            const float y = area.getBottom() - env * (area.getHeight() - 12.0f) - 4.0f;
            g.setColour (accent.withAlpha (0.15f + env * 0.75f));
            g.fillEllipse (x - 2.0f, y - 2.0f, 4.0f, 4.0f);
        }

        static const char* names[] = { "ROOM", "HALL", "PLATE", "ARENA", "AMBIENT", "DREAM", "CATHEDRAL" };
        g.setFont (ui::font (11.0f, true));
        g.setColour (accent);
        g.drawText (names[juce::jlimit (0, 6, algo)], area.toNearestInt().removeFromBottom (18),
                    juce::Justification::centredLeft, false);
    }

    void drawConvolution (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const int space = juce::jlimit (0, 3, (int) pv (ids::convAlgo));
        const float mix = juce::jlimit (0.0f, 1.0f, pv (ids::convMix));
        const float predelay = pv (ids::convPredelay);
        static const float seconds[] = { 0.9f, 1.7f, 2.7f, 3.6f };
        static const char* names[] = { "ROOM", "PLATE", "HALL", "CHURCH" };

        const float len = seconds[space];
        juce::Path path;
        path.startNewSubPath (area.getX(), area.getY() + 6.0f);

        for (int i = 1; i <= 96; ++i)
        {
            const float t = (float) i / 96.0f;
            const float env = std::exp (-t * (4.0f + (float) space * 0.9f));
            path.lineTo (area.getX() + t * area.getWidth(),
                         area.getBottom() - env * (area.getHeight() - 18.0f) - 6.0f);
        }

        g.setColour (accent.withAlpha (0.30f + mix * 0.3f));
        g.strokePath (path, juce::PathStrokeType (1.8f));

        // Early-reflection cluster at the start of the impulse.
        for (int k = 0; k < 12; ++k)
        {
            const float x = area.getX() + (0.012f + 0.055f * (float) k / 12.0f) * area.getWidth();
            const float h = (0.25f - 0.017f * (float) k) * area.getHeight();
            g.setColour (accent.withAlpha (0.20f + 0.04f * (float) (12 - k)));
            g.fillRoundedRectangle (x, area.getCentreY() - h * 0.5f, 2.0f, h, 1.0f);
        }

        g.setFont (ui::font (11.0f, true));
        g.setColour (accent);
        g.drawText (juce::String (names[space]) + "  " + juce::String (len, 1) + " s",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredLeft, false);

        g.setFont (ui::font (10.0f));
        g.setColour (ui::dim);
        g.drawText (juce::String (predelay, 0) + " ms pre  /  " + juce::String (mix * 100.0f, 0) + "% wet",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredRight, false);
    }

    void drawCassette (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float wow = juce::jlimit (0.0f, 1.0f, pv (ids::casWow));
        const float tone = pv (ids::casTone);
        const float hiss = juce::jlimit (0.0f, 1.0f, pv (ids::casHiss));
        const float drive = juce::jlimit (0.0f, 1.0f, pv (ids::casDrive));
        const float t = (float) nowMs() * 0.001f;

        // The wobble is the story: a slow wow LFO with a fast flutter on top.
        juce::Path path;
        const float amp = (2.0f + wow * 26.0f) * (area.getHeight() / 120.0f);

        for (int i = 0; i <= 160; ++i)
        {
            const float p = (float) i / 160.0f;
            const float x = area.getX() + p * area.getWidth();
            const float wobble = std::sin (p * juce::MathConstants<float>::twoPi * 2.0f + t * 1.4f) * amp
                               + std::sin (p * juce::MathConstants<float>::twoPi * 31.0f + t * 9.0f) * amp * 0.10f;
            const float y = area.getCentreY() + wobble;

            if (i == 0)
                path.startNewSubPath (x, y);
            else
                path.lineTo (x, y);
        }

        g.setColour (accent.withAlpha (0.85f));
        g.strokePath (path, juce::PathStrokeType (1.5f));

        // Tone and hiss meters.
        auto meter = area.removeFromBottom (26.0f).reduced (0.0f, 8.0f);
        const float toneNorm = juce::jlimit (0.0f, 1.0f, (tone - 1000.0f) / 11000.0f);
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (meter, 3.0f);
        g.setColour (accent.withAlpha (0.55f));
        g.fillRoundedRectangle (meter.withWidth (meter.getWidth() * toneNorm), 3.0f);

        g.setFont (ui::font (10.0f, true));
        g.setColour (ui::text);
        g.drawText ("WOW " + juce::String (wow * 100.0f, 0) + "%   DRIVE " + juce::String (drive * 100.0f, 0) + "%",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredLeft, false);
        g.setColour (ui::dim);
        g.drawText (juce::String (tone / 1000.0f, 1) + " kHz  /  HISS " + juce::String (hiss * 100.0f, 0) + "%",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredRight, false);
    }

    void drawSpin (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float depth = juce::jlimit (0.0f, 1.0f, pv (ids::spinDepth));
        const float rate = juce::jmax (0.5f, pv (ids::spinRate));

        const float t = (float) std::fmod (nowMs() * 0.001, (double) rate) / rate;
        auto yFor = [&] (float speed) { return area.getCentreY() - juce::jlimit (-1.2f, 1.2f, speed) * area.getHeight() * 0.36f; };

        // Normal speed line.
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawHorizontalLine ((int) yFor (1.0f), area.getX(), area.getRight());

        // Brake curve: speed dives through zero and comes back.
        juce::Path curve;
        curve.startNewSubPath (area.getX(), yFor (1.0f));

        for (int i = 1; i <= 96; ++i)
        {
            const float p = (float) i / 96.0f;
            const float speed = 1.0f - 1.75f * depth * std::sin (juce::MathConstants<float>::pi * p);
            curve.lineTo (area.getX() + p * area.getWidth(), yFor (speed));
        }

        g.setColour (accent.withAlpha (0.9f));
        g.strokePath (curve, juce::PathStrokeType (1.8f));

        const float px = area.getX() + t * area.getWidth();
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.fillEllipse (px - 3.0f, yFor (1.0f) - 3.0f, 6.0f, 6.0f);

        g.setFont (ui::font (11.0f, true));
        g.setColour (accent);
        g.drawText ("BACKSPIN " + juce::String (depth * 100.0f, 0) + "%",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredLeft, false);
        g.setFont (ui::font (10.0f));
        g.setColour (ui::dim);
        g.drawText ("EVERY " + juce::String (rate, 1) + " s",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredRight, false);
    }

    void drawEcho (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float timeMs = pv (ids::dlyTime);
        const float fb = pv (ids::dlyFb);
        const int mode = (int) pv (ids::dlyMode);
        const float cy = area.getCentreY();

        g.setColour (juce::Colours::white.withAlpha (0.16f));
        g.drawHorizontalLine ((int) cy, area.getX(), area.getRight());

        float amp = 0.9f;
        float offset = 0.02f;

        for (int i = 0; i < 14 && amp > 0.02f; ++i)
        {
            const float x = area.getX() + juce::jlimit (0.0f, 0.98f, offset) * area.getWidth();
            const float h = amp * area.getHeight() * 0.45f;

            g.setColour (accent.withAlpha (0.25f + amp * 0.6f));
            g.fillRoundedRectangle (x, cy - h, 3.0f, h * 2.0f, 1.5f);

            if (mode == 2 && i % 2 == 1)
                g.fillRoundedRectangle (x, cy - h * 0.6f, 3.0f, h * 1.2f, 1.5f);

            offset += 0.02f + (timeMs / 1200.0f) * 0.075f;
            amp *= fb;
        }

        // A pulse travelling through the taps.
        const float t = std::fmod ((float) nowMs() * 0.001f * 0.5f, 1.0f);
        const float px = area.getX() + t * area.getWidth();
        g.setColour (accent.withAlpha (0.9f));
        g.fillEllipse (px - 3.0f, cy - 3.0f, 6.0f, 6.0f);

        static const char* names[] = { "ANALOG", "TAPE", "PING PONG", "DUCKING" };
        label (g, area, names[juce::jlimit (0, 3, mode)], false);
        label (g, area, juce::String (timeMs, 0) + " ms", true);
    }

    void drawSlice (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const int mode = (int) pv (ids::stutMode);
        const float rate = pv (ids::stutRate);
        const float slice = pv (ids::stutSlice);

        const int cells = 16;
        const float cellW = area.getWidth() / (float) cells;
        const float phase = std::fmod ((float) nowMs() * 0.001f * rate * 0.5f, 1.0f);
        const int active = (int) (phase * (float) cells);

        for (int i = 0; i < cells; ++i)
        {
            auto cell = juce::Rectangle<float> (area.getX() + cellW * (float) i + 1.5f, area.getY(),
                                                cellW - 3.0f, area.getHeight() - 26.0f);
            g.setColour (i == active ? accent.withAlpha (0.75f) : accent.withAlpha (0.10f));
            g.fillRoundedRectangle (cell, 2.0f);
        }

        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText (juce::String (slice, 0) + " ms slice  /  " + juce::String (rate, 1) + " Hz",
                    area.toNearestInt().removeFromBottom (20), juce::Justification::centredLeft, false);

        static const char* names[] = { "MICRO CHOP", "MACHINE GUN", "HYPERPOP", "REVERSE CHAOS", "FREEZE", "GLITCH STORM" };
        g.setColour (accent);
        g.setFont (ui::font (11.0f, true));
        g.drawText (names[juce::jlimit (0, 5, mode)], area.toNearestInt().removeFromBottom (20),
                    juce::Justification::centredRight, false);
    }

    void drawSpeed (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float slow = pv (ids::chopSlow);
        const float pitch = pv (ids::chopPitch);
        const float drift = pv (ids::chopDrift);

        const float rate = juce::jlimit (0.15f, 1.0f, (1.0f - slow * 0.55f) * std::pow (2.0f, pitch / 12.0f));
        const float y = area.getBottom() - rate * area.getHeight();

        g.setColour (accent.withAlpha (0.20f));
        g.fillRect (area.getX(), y, area.getWidth(), area.getBottom() - y);

        juce::Path path;
        const float t = (float) nowMs() * 0.001f;

        for (int i = 0; i <= 96; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / 96.0f;
            const float wobble = std::sin (t * 2.0f + (float) i * 0.25f) * 0.05f * drift;
            const float yy = y + wobble * area.getHeight();

            if (i == 0)
                path.startNewSubPath (x, yy);
            else
                path.lineTo (x, yy);
        }

        g.setColour (accent);
        g.strokePath (path, juce::PathStrokeType (1.8f));

        g.setFont (ui::font (11.0f, true));
        g.setColour (ui::text);
        g.drawText ("SPEED " + juce::String (rate * 100.0f, 0) + "%   PITCH " + juce::String (pitch, 1) + " st",
                    area.toNearestInt().removeFromBottom (20), juce::Justification::centredLeft, false);

        // Reel slots.
        for (int i = 0; i < 10; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / 9.0f;
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRoundedRectangle (x - 2.0f, area.getY(), 4.0f, 8.0f, 1.5f);
        }
    }

    void drawSibilance (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float freq = pv (ids::deessFreq);
        const float gr = juce::jlimit (0.0f, 18.0f, proc.scope.essReduction.load());
        const float t = (float) nowMs() * 0.002f;

        juce::Path path;
        path.startNewSubPath (area.getX(), area.getBottom());

        for (int i = 0; i <= 64; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / 64.0f;
            const float band = std::exp (-std::pow (((float) i - 34.0f) / 12.0f, 2.0f));
            const float dip = std::exp (-std::pow (((float) i - 44.0f) / 6.0f, 2.0f)) * (gr / 18.0f);
            const float ripple = 0.05f * std::sin (t + (float) i * 0.4f);
            const float y = area.getBottom() - (0.25f + band * 0.6f - dip * 0.6f + ripple) * area.getHeight();
            path.lineTo (x, y);
        }

        path.lineTo (area.getRight(), area.getBottom());
        path.closeSubPath();
        g.setColour (accent.withAlpha (0.25f));
        g.fillPath (path);
        g.setColour (accent);
        g.strokePath (path, juce::PathStrokeType (1.4f));

        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText (juce::String (freq, 0) + " Hz   -" + juce::String (gr, 1) + " dB",
                    area.toNearestInt().removeFromBottom (18), juce::Justification::centredLeft, false);
    }

    void drawCleanup (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const char* names[] = { "HUM", "BREATH", "CLICKS" };
        const float values[] = { pv (ids::nrHum), pv (ids::nrBreath), pv (ids::nrClicks) };

        auto top = area.removeFromTop (area.getHeight() * 0.62f);
        const float rowH = top.getHeight() / 3.0f;

        for (int i = 0; i < 3; ++i)
        {
            auto row = top.removeFromTop (rowH).reduced (0.0f, 5.0f);
            auto nameArea = row.removeFromLeft (70.0f);
            g.setFont (ui::font (11.0f));
            g.setColour (ui::dim);
            g.drawText (names[i], nameArea.toNearestInt(), juce::Justification::centredLeft, false);

            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRoundedRectangle (row, 3.0f);
            g.setColour (accent.withAlpha (0.8f));
            g.fillRoundedRectangle (row.withWidth (row.getWidth() * juce::jlimit (0.0f, 1.0f, values[i])), 3.0f);
        }

        label (g, area, "SPECTRAL CLEANUP", false);
    }

    void drawFx (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const int type = 0;
        const float amount = 0.5f;
        const float t = (float) nowMs() * 0.002f;

        juce::Path path;
        const int steps = 96;

        for (int i = 0; i <= steps; ++i)
        {
            const float x = area.getX() + area.getWidth() * (float) i / (float) steps;
            const float phase = (float) i / (float) steps * juce::MathConstants<float>::twoPi * 4.0f + t;
            float v = std::sin (phase);

            switch (type)
            {
                case 0: v = std::sin (phase * 1.0f) * (0.4f + amount * 0.5f); break;          // reverse
                case 1: v = std::sin (phase * (1.0f - amount * 0.85f)); break;               // tape stop
                case 2: v = std::sin (phase) * 0.7f + 0.05f * std::sin (phase * 37.0f); break; // vinyl
                case 3: v = std::tanh (std::sin (phase) * (1.0f + amount * 6.0f)); break;    // megaphone
                case 4: v = std::sin (phase) * 0.6f; break;                                  // telephone
                case 5: v = std::sin (phase) * std::sin (phase * 0.25f); break;              // robot
                case 6: v = std::round (std::sin (phase) * (4.0f - amount * 3.0f)) / 4.0f; break; // bit crush
                default: v = std::sin (phase + std::sin (t + (float) i) * amount * 3.0f); break;  // granular
            }

            const float y = area.getCentreY() - juce::jlimit (-1.2f, 1.2f, v) * area.getHeight() * 0.4f;

            if (i == 0)
                path.startNewSubPath (x, y);
            else
                path.lineTo (x, y);
        }

        g.setColour (accent);
        g.strokePath (path, juce::PathStrokeType (1.6f));

        static const char* names[] = { "REVERSE FX", "TAPE STOP", "VINYL STOP", "MEGAPHONE",
                                       "TELEPHONE", "ROBOT VOICE", "BIT CRUSHER", "GRANULAR ECHO" };
        label (g, area, names[juce::jlimit (0, 7, type)], false);
    }

    void drawPitchFx (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float semis = pv (ids::pitchFxSemis);
        const float formant = pv (ids::pitchFxFormant);
        const float mix = pv (ids::pitchFxMix);

        auto yFor = [&] (float st) { return area.getCentreY() - (st / 24.0f) * area.getHeight() * 0.44f; };

        // Register ladder.
        g.setFont (ui::font (9.0f));

        for (int st = -24; st <= 24; st += 12)
        {
            const float y = yFor ((float) st);
            g.setColour (juce::Colours::white.withAlpha (st == 0 ? 0.20f : 0.07f));
            g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
            g.setColour (ui::dim.withAlpha (0.8f));
            g.drawText ((st > 0 ? "+" : "") + juce::String (st),
                        juce::Rectangle<int> ((int) area.getX(), (int) y - 7, 26, 14),
                        juce::Justification::centredLeft, false);
        }

        auto ladder = area.withTrimmedLeft (32.0f);
        const float centre = ladder.getCentreX();
        const float y0 = yFor (0.0f);
        const float y1 = yFor (semis);

        // Pitch offset pill.
        g.setColour (accent.withAlpha (0.20f));
        g.fillRoundedRectangle (juce::Rectangle<float> (centre - 34.0f, juce::jmin (y0, y1), 68.0f, std::abs (y1 - y0) + 1.0f), 6.0f);
        g.setColour (accent);
        g.fillRoundedRectangle (centre - 34.0f, y1 - 2.0f, 68.0f, 4.0f, 2.0f);
        g.fillEllipse (centre - 4.0f, y1 - 4.0f, 8.0f, 8.0f);

        // Formant marker: independent of the pitch, which is the whole point of the module.
        const float yf = yFor (formant);
        juce::Path diamond;
        diamond.startNewSubPath (centre + 54.0f, yf - 7.0f);
        diamond.lineTo (centre + 61.0f, yf);
        diamond.lineTo (centre + 54.0f, yf + 7.0f);
        diamond.lineTo (centre + 47.0f, yf);
        diamond.closeSubPath();
        g.setColour (juce::Colour (0xffc65cff));
        g.fillPath (diamond);

        // Harmonic stack, moved by the pitch and coloured by the formant.
        for (int h = 1; h <= 6; ++h)
        {
            const float y = area.getBottom() - area.getHeight() * (float) h / 7.0f;
            const float shift = (semis / 24.0f) * area.getHeight() * 0.10f * (float) h / 6.0f;
            g.setColour (accent.withAlpha (0.10f + mix * 0.12f));
            g.drawHorizontalLine ((int) (y - shift), ladder.getX(), ladder.getRight());
        }

        g.setFont (ui::font (12.0f, true));
        g.setColour (ui::text);
        g.drawText ((semis >= 0.0f ? "+" : "") + juce::String (semis, 1) + " st",
                    area.toNearestInt().removeFromBottom (20), juce::Justification::centredLeft, false);
        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText ((formant >= 0.0f ? "FORMANT +" : "FORMANT ") + juce::String (formant, 1) + " st",
                    area.toNearestInt().removeFromBottom (20), juce::Justification::centredRight, false);
    }

    void drawRegularStutter (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float period = juce::jmax (100.0f, pv (ids::rstutPeriod));
        const float slice = juce::jmax (20.0f, pv (ids::rstutSlice));
        const float reps = juce::jmax (1.0f, pv (ids::rstutReps));

        auto bar = area.reduced (0.0f, 18.0f);
        const float cycle = period;
        const float sliceW = juce::jlimit (2.0f, bar.getWidth(), bar.getWidth() * slice / cycle);
        const float startX = bar.getX();

        // The trigger window.
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.fillRoundedRectangle (bar, 4.0f);

        float x = startX;

        for (int i = 0; i < (int) reps; ++i)
        {
            g.setColour (accent.withAlpha (0.55f - 0.04f * (float) i));
            g.fillRoundedRectangle (x + 1.0f, bar.getY() + 4.0f, juce::jmax (2.0f, sliceW - 2.0f),
                                    bar.getHeight() - 8.0f, 3.0f);
            x += sliceW;
        }

        const float phase = std::fmod ((float) nowMs() * 0.001f, period / 1000.0f) / (period / 1000.0f);
        const float px = bar.getX() + phase * bar.getWidth();

        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.drawVerticalLine ((int) px, bar.getY(), bar.getBottom());
        g.setColour (accent);
        g.fillEllipse (px - 3.0f, bar.getY() - 3.0f, 6.0f, 6.0f);

        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText (juce::String (period / 1000.0f, 2) + " s cycle  /  " + juce::String (slice, 0) + " ms slice  /  "
                        + juce::String (reps, 0) + " repeats",
                    area.toNearestInt().removeFromBottom (16), juce::Justification::centredLeft, false);

        g.setFont (ui::font (11.0f, true));
        g.setColour (accent);
        g.drawText ("REGULAR", area.toNearestInt().removeFromBottom (16), juce::Justification::centredRight, false);
    }

    void drawLoudness (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const auto wave = proc.scope.wave;
        const int size = (int) wave.size();
        const int start = proc.scope.waveWrite;
        const float mid = area.getCentreY();

        juce::Path path;

        for (int i = 0; i < size; ++i)
        {
            const float v = juce::jlimit (0.0f, 1.0f, wave[(size_t) ((start + i) % size)]);
            const float x = area.getX() + area.getWidth() * (float) i / (float) (size - 1);
            const float h = v * area.getHeight() * 0.48f;

            if (h > 0.6f)
                g.setColour (accent.withAlpha (0.35f));
            else
                g.setColour (juce::Colours::white.withAlpha (0.10f));

            g.drawLine (x, mid - h, x, mid + h, 1.4f);
        }

        const float lufs = proc.scope.lufs.load();
        const float corr = proc.scope.correlation.load();

        auto bar = area.removeFromBottom (22.0f);
        g.setFont (ui::font (12.0f, true));
        g.setColour (accent);
        g.drawText (juce::String (lufs, 1) + " LUFS", bar.toNearestInt(), juce::Justification::centredLeft, false);
        g.setColour (ui::dim);
        g.drawText ("CORR " + juce::String (corr, 2), bar.toNearestInt(), juce::Justification::centredRight, false);

        // Loudness target marker.
        const float t = juce::jlimit (0.0f, 1.0f, (lufs + 30.0f) / 30.0f);
        const float x = area.getX() + t * area.getWidth();
        g.setColour (accent.withAlpha (0.8f));
        g.drawVerticalLine ((int) x, area.getY(), area.getBottom());
        (void) path;
    }

    VocalROIDZProcessor& proc;
    modules::ScopeKind kind;
    juce::Colour accent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopeView)
};
