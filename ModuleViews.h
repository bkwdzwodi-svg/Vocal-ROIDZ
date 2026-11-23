#pragma once
#include "PluginProcessor.h"
#include "Modules.h"
#include "UiStyle.h"
#include <array>

// =======================================================================================
// EqGraphView
//
// A real parametric EQ display: the curve is computed from the same coefficients the DSP
// runs, and every band is a draggable node. Drag horizontally for frequency, vertically for
// gain, and use the wheel over the bell to change its Q.
// =======================================================================================
class EqGraphView : public juce::Component, private juce::Timer
{
public:
    EqGraphView (VocalROIDZProcessor& p, juce::Colour c)
        : proc (p), accent (c)
    {
        startTimerHz (30);
    }

    ~EqGraphView() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (ui::panel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (accent.withAlpha (0.22f));
        g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

        plot = b.reduced (16.0f, 20.0f);

        drawGrid (g);
        drawSpectrum (g);

        const auto filters = makeFilters();
        const double fs = juce::jmax (8000.0, proc.getSampleRate());

        juce::Path curve;
        bool started = false;

        for (int i = 0; i <= 240; ++i)
        {
            const float x = plot.getX() + plot.getWidth() * (float) i / 240.0f;
            const double hz = xToFreq (x);
            double mag = 1.0;

            for (auto& f : filters)
                mag *= (double) f->getMagnitudeForFrequency (hz, fs);

            const float db = (float) juce::Decibels::gainToDecibels (mag, -60.0);
            const float y = gainToY (juce::jlimit (-24.0f, 24.0f, db));

            if (! started)
            {
                curve.startNewSubPath (x, y);
                started = true;
            }
            else
            {
                curve.lineTo (x, y);
            }
        }

        auto filled = curve;
        filled.lineTo (plot.getRight(), gainToY (0.0f));
        filled.lineTo (plot.getX(), gainToY (0.0f));
        filled.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.30f), plot.getX(), plot.getY(),
                                                 accent.withAlpha (0.02f), plot.getX(), plot.getBottom(), false));
        g.fillPath (filled);

        g.setColour (accent.withAlpha (0.95f));
        g.strokePath (curve, juce::PathStrokeType (2.0f));

        drawNodes (g);

        // Readout of the band under the cursor.
        g.setFont (ui::font (11.0f));
        g.setColour (ui::dim);
        g.drawText ("DRAG A NODE  /  WHEEL ON THE BELL FOR Q",
                    getLocalBounds().reduced (16, 4).removeFromBottom (14),
                    juce::Justification::centredLeft, false);

        if (hoverBand >= 0)
        {
            const auto& d = bands()[(size_t) hoverBand];
            const float f = nodeFreq (hoverBand);
            const float gain = d.gainId != nullptr ? param (d.gainId) : 0.0f;
            juce::String text (d.name);

            if (d.frequency)
                text << "  " << juce::String (f < 1000.0f ? juce::String (f, 0) + " Hz"
                                                          : juce::String (f / 1000.0f, 2) + " kHz");

            if (d.gainId != nullptr)
                text << "  " << (gain >= 0.0f ? "+" : "") << juce::String (gain, 1) << " dB";

            if (isBell (hoverBand))
                text << "  Q " << juce::String (param (hoverBand == 2 ? ids::eqMidQ : ids::eqMid2Q), 2);

            g.setFont (ui::font (11.0f, true));
            g.setColour (ui::text);
            g.drawText (text, getLocalBounds().reduced (16, 4).removeFromBottom (14),
                        juce::Justification::centredRight, false);
        }
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int hit = findNode (e.position);
        const bool mid = isBell (hit);

        setMouseCursor (hit >= 0 ? (mid ? juce::MouseCursor::UpDownLeftRightResizeCursor
                                       : juce::MouseCursor::DraggingHandCursor)
                                 : juce::MouseCursor::NormalCursor);

        if (hit != hoverBand)
        {
            hoverBand = hit;
            repaint();
        }
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        if (hoverBand != -1)
        {
            hoverBand = -1;
            repaint();
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        activeBand = findNode (e.position);

        if (activeBand < 0)
            return;

        beginGesture (activeBand);
        applyDrag (e.position);
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (activeBand >= 0)
            applyDrag (e.position);
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        if (activeBand >= 0)
            endGesture (activeBand);

        activeBand = -1;
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& w) override
    {
        const int hit = findNode (e.position);

        if (isBell (hit) && std::abs (w.deltaY) > 0.001f)
        {
            const char* qid = hit == 2 ? ids::eqMidQ : ids::eqMid2Q;
            const float q = juce::jlimit (0.25f, 8.0f, param (qid) * (1.0f + w.deltaY * 0.35f));
            setParam (qid, q);
            repaint();
        }
    }

private:
    struct BandDef
    {
        const char* name;
        const char* freqId;
        const char* gainId;
        float fixedFreq;
        juce::Colour colour;
        bool frequency;
        bool gain;
    };

    static const std::array<BandDef, 7>& bands()
    {
        static const std::array<BandDef, 7> defs =
        {{
            { "HIGH PASS", ids::eqHpf,      nullptr,        0.0f,    juce::Colour (0xff30e6ff), true,  false },
            { "LOW",       ids::eqLowFreq,  ids::eqLow,     0.0f,    juce::Colour (0xffffc93c), true,  true  },
            { "MID LOW",   ids::eqMidFreq,  ids::eqMid,     0.0f,    juce::Colour (0xff86ff2b), true,  true  },
            { "MID HIGH",  ids::eqMid2Freq, ids::eqMid2,    0.0f,    juce::Colour (0xff4fd8ff), true,  true  },
            { "HIGH",      ids::eqHighFreq, ids::eqHigh,    0.0f,    juce::Colour (0xffc65cff), true,  true  },
            { "AIR",       nullptr,         ids::eqAir,     12000.0f, juce::Colour (0xffff8d1f), false, true  },
            { "LOW PASS",  ids::eqLpf,      nullptr,        0.0f,    juce::Colour (0xff30e6ff), true,  false }
        }};

        return defs;
    }

    static bool isBell (int band) { return band == 2 || band == 3; }

    void timerCallback() override
    {
        // Repaint only when something moved, so idle pages cost nothing.
        float sum = 0.0f;

        for (const auto& id : { ids::eqHpf, ids::eqLpf, ids::eqLow, ids::eqLowFreq, ids::eqMid,
                                ids::eqMidFreq, ids::eqMidQ, ids::eqMid2, ids::eqMid2Freq, ids::eqMid2Q,
                                ids::eqHigh, ids::eqHighFreq, ids::eqAir })
            sum += param (id);

        if (std::abs (sum - lastSum) > 0.0001f)
        {
            lastSum = sum;
            repaint();
        }
    }

    float param (const char* id) const
    {
        if (id == nullptr)
            return 0.0f;

        if (auto* v = proc.apvts.getRawParameterValue (id))
            return v->load();

        return 0.0f;
    }

    void setParam (const char* id, float value)
    {
        if (auto* p = proc.apvts.getParameter (id))
            p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, p->convertTo0to1 (value)));
    }

    void beginGesture (int band)
    {
        const auto& d = bands()[(size_t) band];
        gestureIds.clear();

        if (d.freqId != nullptr && d.frequency)
            gestureIds.addIfNotAlreadyThere (d.freqId);

        if (d.gainId != nullptr && d.gain)
            gestureIds.addIfNotAlreadyThere (d.gainId);

        // Proper host gestures, so automation records one smooth move rather than steps.
        for (const auto& id : gestureIds)
            if (auto* p = proc.apvts.getParameter (id))
                p->beginChangeGesture();
    }

    void endGesture (int band)
    {
        (void) band;

        for (const auto& id : gestureIds)
            if (auto* p = proc.apvts.getParameter (id))
                p->endChangeGesture();

        gestureIds.clear();
    }

    void applyDrag (juce::Point<float> pos)
    {
        const auto& d = bands()[(size_t) activeBand];

        if (d.frequency && d.freqId != nullptr)
            setParam (d.freqId, xToFreq (juce::jlimit (plot.getX(), plot.getRight(), pos.x)));

        if (d.gain && d.gainId != nullptr)
            setParam (d.gainId, yToGain (juce::jlimit (plot.getY(), plot.getBottom(), pos.y)));

        repaint();
    }

    float nodeFreq (int band) const
    {
        const auto& d = bands()[(size_t) band];
        return d.fixedFreq > 0.0f ? d.fixedFreq : param (d.freqId);
    }

    juce::Point<float> nodePosition (int band) const
    {
        const auto& d = bands()[(size_t) band];
        const float gain = d.gainId != nullptr ? param (d.gainId) : 0.0f;
        return { freqToX (juce::jlimit (20.0f, 20000.0f, nodeFreq (band))),
                 gainToY (juce::jlimit (-24.0f, 24.0f, gain)) };
    }

    int findNode (juce::Point<float> pos) const
    {
        int best = -1;
        float bestDist = 26.0f;

        for (int i = 0; i < (int) bands().size(); ++i)
        {
            const auto p = nodePosition (i);
            const float dist = p.getDistanceFrom (pos);

            if (dist < bestDist)
            {
                bestDist = dist;
                best = i;
            }
        }

        return best;
    }

    void drawGrid (juce::Graphics& g)
    {
        // Horizontal dB grid every 6 dB, stronger at unity.
        for (int db = -24; db <= 24; db += 6)
        {
            const float y = gainToY ((float) db);
            g.setColour (juce::Colours::white.withAlpha (db == 0 ? 0.16f : 0.055f));
            g.drawLine (plot.getX(), y, plot.getRight(), y, db == 0 ? 1.2f : 1.0f);
        }

        // Log frequency grid with a line and label at every useful octave point.
        for (float hz : { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f })
        {
            const bool decade = hz == 100.0f || hz == 1000.0f || hz == 10000.0f;
            g.setColour (juce::Colours::white.withAlpha (decade ? 0.09f : 0.045f));
            g.drawVerticalLine ((int) freqToX (hz), plot.getY(), plot.getBottom());
        }

        g.setFont (ui::font (9.5f));
        g.setColour (ui::dim.withAlpha (0.85f));

        auto labelRow = plot.toNearestInt().removeFromBottom (15);

        for (auto hz : { 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000 })
        {
            const juce::String text (hz >= 1000 ? juce::String (hz / 1000) + "k" : juce::String (hz));
            const int x = (int) freqToX ((float) hz);
            const int w = hz == 20 ? 24 : (hz == 20000 ? 30 : 28);
            juce::Justification just = hz == 20 ? juce::Justification::bottomLeft
                                   : (hz == 20000 ? juce::Justification::bottomRight
                                                  : juce::Justification::centred);
            g.drawText (text, juce::Rectangle<int> (x - w / 2, labelRow.getY(), w, 15), just, false);
        }

        for (int db : { 18, 12, 6, -6, -12, -18 })
        {
            const juce::String text ((db > 0 ? "+" : "") + juce::String (db));
            g.drawText (text, juce::Rectangle<int> ((int) plot.getX() + 2, (int) gainToY ((float) db) - 6, 30, 12),
                        juce::Justification::centredLeft, false);
        }

        g.setFont (ui::font (9.0f, true));
        g.setColour (ui::dim.withAlpha (0.5f));
        g.drawText ("7-BAND PARAMETRIC", getLocalBounds().removeFromTop (16).removeFromRight (150),
                    juce::Justification::centredRight, false);
    }

    void drawSpectrum (juce::Graphics& g)
    {
        const auto& spectrum = proc.scope.spectrum;
        const int bins = (int) spectrum.size();

        if (bins < 2)
            return;

        juce::Path path;
        bool started = false;

        for (int i = 0; i < bins; ++i)
        {
            const float hz = 30.0f * std::pow (600.0f, (float) i / (float) (bins - 1));
            const float x = freqToX (hz);
            const float level = juce::jlimit (0.0f, 1.0f, spectrum[(size_t) i]);
            const float y = plot.getBottom() - level * plot.getHeight() * 0.9f;

            if (! started)
            {
                path.startNewSubPath (x, y);
                started = true;
            }
            else
            {
                path.lineTo (x, y);
            }
        }

        auto filled = path;
        filled.lineTo (plot.getRight(), plot.getBottom());
        filled.lineTo (plot.getX(), plot.getBottom());
        filled.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.10f), plot.getX(), plot.getY(),
                                                 accent.withAlpha (0.01f), plot.getX(), plot.getBottom(), false));
        g.fillPath (filled);

        g.setColour (accent.withAlpha (0.28f));
        g.strokePath (path, juce::PathStrokeType (0.8f));
    }

    void drawNodes (juce::Graphics& g)
    {
        for (int i = 0; i < (int) bands().size(); ++i)
        {
            const auto& d = bands()[(size_t) i];
            const auto p = nodePosition (i);
            const bool hot = i == hoverBand || i == activeBand;
            const float r = hot ? 8.0f : 6.0f;

            g.setColour (d.colour.withAlpha (hot ? 0.95f : 0.75f));
            g.fillEllipse (p.x - r, p.y - r, r * 2.0f, r * 2.0f);
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.fillEllipse (p.x - 2.0f, p.y - 2.0f, 4.0f, 4.0f);
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.drawEllipse (p.x - r, p.y - r, r * 2.0f, r * 2.0f, 1.2f);

            if (isBell (i))
            {
                // Bandwidth halo around the bell.
                const float q = juce::jlimit (0.25f, 8.0f, param (i == 2 ? ids::eqMidQ : ids::eqMid2Q));
                const float w = juce::jlimit (16.0f, 260.0f, 150.0f / q);
                g.setColour (d.colour.withAlpha (0.10f));
                g.fillRoundedRectangle (p.x - w * 0.5f, p.y - 12.0f, w, 24.0f, 8.0f);
            }
        }
    }

    std::vector<juce::dsp::IIR::Coefficients<float>::Ptr> makeFilters() const
    {
        using Coeffs = juce::dsp::IIR::Coefficients<float>;
        const double fs = juce::jmax (8000.0, proc.getSampleRate());
        const float nyq = (float) (fs * 0.5);

        std::vector<Coeffs::Ptr> filters;
        filters.push_back (Coeffs::makeHighPass (fs, juce::jlimit (20.0f, nyq * 0.9f, param (ids::eqHpf)), 0.707f));
        filters.push_back (Coeffs::makeLowPass (fs, juce::jlimit (500.0f, nyq * 0.9f, param (ids::eqLpf)), 0.707f));
        filters.push_back (Coeffs::makeLowShelf (fs, juce::jlimit (30.0f, nyq * 0.5f, param (ids::eqLowFreq)), 0.707f,
                                                 juce::Decibels::decibelsToGain (param (ids::eqLow))));
        filters.push_back (Coeffs::makePeakFilter (fs, juce::jlimit (120.0f, nyq * 0.8f, param (ids::eqMidFreq)),
                                                   juce::jlimit (0.2f, 10.0f, param (ids::eqMidQ)),
                                                   juce::Decibels::decibelsToGain (param (ids::eqMid))));
        filters.push_back (Coeffs::makePeakFilter (fs, juce::jlimit (400.0f, nyq * 0.8f, param (ids::eqMid2Freq)),
                                                   juce::jlimit (0.2f, 10.0f, param (ids::eqMid2Q)),
                                                   juce::Decibels::decibelsToGain (param (ids::eqMid2))));
        filters.push_back (Coeffs::makeHighShelf (fs, juce::jlimit (1000.0f, nyq * 0.85f, param (ids::eqHighFreq)), 0.707f,
                                                  juce::Decibels::decibelsToGain (param (ids::eqHigh))));
        filters.push_back (Coeffs::makeHighShelf (fs, juce::jlimit (1000.0f, nyq * 0.85f, 12000.0f), 0.707f,
                                                  juce::Decibels::decibelsToGain (param (ids::eqAir))));
        return filters;
    }

    float xToFreq (float x) const
    {
        const float t = juce::jlimit (0.0f, 1.0f, (x - plot.getX()) / juce::jmax (1.0f, plot.getWidth()));
        return 20.0f * std::pow (1000.0f, t);          // 20 Hz .. 20 kHz
    }

    float freqToX (float hz) const
    {
        const float t = std::log (juce::jlimit (20.0f, 20000.0f, hz) / 20.0f) / std::log (1000.0f);
        return plot.getX() + t * plot.getWidth();
    }

    float gainToY (float db) const
    {
        return plot.getCentreY() - (db / 24.0f) * plot.getHeight() * 0.5f;
    }

    float yToGain (float y) const
    {
        return juce::jlimit (-24.0f, 24.0f, (plot.getCentreY() - y) / (plot.getHeight() * 0.5f) * 24.0f);
    }

    VocalROIDZProcessor& proc;
    juce::Colour accent;
    juce::Rectangle<float> plot;
    juce::StringArray gestureIds;
    int hoverBand = -1, activeBand = -1;
    float lastSum = -9999.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EqGraphView)
};

// =======================================================================================
// CompressorView
//
// Transfer curve, live input/output metering, gain-reduction history and smooth ballistics,
// all driven from the atomics the audio thread publishes.
// =======================================================================================
class CompressorView : public juce::Component, private juce::Timer
{
public:
    CompressorView (VocalROIDZProcessor& p, juce::Colour c)
        : proc (p), accent (c)
    {
        grHistory.fill (0.0f);
        startTimerHz (30);
    }

    ~CompressorView() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (ui::panel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (accent.withAlpha (0.22f));
        g.drawRoundedRectangle (b.reduced (0.5f), 8.0f, 1.0f);

        auto area = b.reduced (14.0f, 12.0f);
        auto meters = area.removeFromRight (96.0f);
        area.removeFromRight (12.0f);
        auto history = area.removeFromBottom (34.0f);
        area.removeFromBottom (8.0f);

        drawCurve (g, area);
        drawHistory (g, history);
        drawMeters (g, meters);
    }

private:
    static float curve (float inDb, float thresh, float ratio, float knee)
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

    void drawCurve (juce::Graphics& g, juce::Rectangle<float> area)
    {
        const float thresh = juce::jlimit (-60.0f, 0.0f, pv (ids::compThresh));
        const float ratio = juce::jmax (1.0f, pv (ids::compRatio));
        const float knee = juce::jmax (0.1f, pv (ids::compKnee));

        auto xFor = [&] (float db) { return area.getX() + juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f) * area.getWidth(); };
        auto yFor = [&] (float db) { return area.getBottom() - juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f) * area.getHeight(); };

        // Grid.
        g.setColour (juce::Colours::white.withAlpha (0.05f));

        for (int db = -60; db <= 0; db += 12)
            g.drawVerticalLine ((int) xFor ((float) db), area.getY(), area.getBottom());

        for (int db = -60; db <= 0; db += 12)
            g.drawHorizontalLine ((int) yFor ((float) db), area.getX(), area.getRight());

        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.drawLine (area.getX(), area.getBottom(), area.getRight(), area.getY(), 1.0f);

        // Threshold marker and compressed region.
        g.setColour (accent.withAlpha (0.06f));
        g.fillRect (xFor (thresh), area.getY(), area.getRight() - xFor (thresh), area.getHeight());
        g.setColour (accent.withAlpha (0.35f));
        g.drawVerticalLine ((int) xFor (thresh), area.getY(), area.getBottom());
        g.drawHorizontalLine ((int) yFor (thresh), area.getX(), area.getRight());

        juce::Path path;
        path.startNewSubPath (xFor (-60.0f), yFor (curve (-60.0f, thresh, ratio, knee)));

        for (int i = 1; i <= 80; ++i)
        {
            const float db = -60.0f + 60.0f * (float) i / 80.0f;
            path.lineTo (xFor (db), yFor (curve (db, thresh, ratio, knee)));
        }

        g.setColour (accent);
        g.strokePath (path, juce::PathStrokeType (2.0f));

        // Live point on the curve.
        const float px = xFor (shownIn);
        const float py = yFor (curve (shownIn, thresh, ratio, knee));

        g.setColour (accent.withAlpha (0.22f));
        g.drawLine (px, area.getY(), px, area.getBottom(), 1.0f);
        g.drawLine (area.getX(), py, area.getRight(), py, 1.0f);
        g.setColour (juce::Colours::white);
        g.fillEllipse (px - 4.0f, py - 4.0f, 8.0f, 8.0f);
        g.setColour (accent);
        g.drawEllipse (px - 7.0f, py - 7.0f, 14.0f, 14.0f, 1.5f);

        static const char* models[] = { "VCA", "FET", "OPTICAL", "TUBE" };
        g.setFont (ui::font (10.0f, true));
        g.setColour (ui::dim);
        g.drawText (juce::String (models[juce::jlimit (0, 3, (int) pv (ids::compModel))]) + "  DETECTOR",
                    area.toNearestInt().removeFromTop (14), juce::Justification::topLeft, false);

        g.setFont (ui::font (9.5f));
        g.setColour (ui::dim.withAlpha (0.75f));
        g.drawText (juce::String (pv (ids::compAttack), 1) + " ms  /  "
                    + juce::String (pv (ids::compRelease), 0) + " ms",
                    area.toNearestInt().removeFromTop (14), juce::Justification::topRight, false);

        g.setColour (ui::dim.withAlpha (0.8f));
        g.setFont (ui::font (10.0f));
        g.drawText ("-60", area.toNearestInt().removeFromBottom (14).removeFromLeft (24),
                    juce::Justification::bottomLeft, false);
        g.drawText ("0 dB", area.toNearestInt().removeFromBottom (14).removeFromRight (34),
                    juce::Justification::bottomRight, false);
    }

    void drawHistory (juce::Graphics& g, juce::Rectangle<float> area)
    {
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRoundedRectangle (area, 4.0f);

        juce::Path path;
        const int size = (int) grHistory.size();
        const float scale = 18.0f;

        path.startNewSubPath (area.getX(), area.getBottom());

        for (int i = 0; i < size; ++i)
        {
            const int idx = (historyPos + i) % size;
            const float x = area.getX() + area.getWidth() * (float) i / (float) (size - 1);
            const float y = area.getBottom() - juce::jlimit (0.0f, 1.0f, grHistory[(size_t) idx] / scale) * area.getHeight();
            path.lineTo (x, y);
        }

        path.lineTo (area.getRight(), area.getBottom());
        path.closeSubPath();

        g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.35f), area.getX(), area.getY(),
                                                 accent.withAlpha (0.05f), area.getX(), area.getBottom(), false));
        g.fillPath (path);
        g.setColour (accent.withAlpha (0.85f));
        g.strokePath (path, juce::PathStrokeType (1.2f));

        g.setFont (ui::font (9.0f, true));
        g.setColour (ui::dim);
        g.drawText ("GAIN REDUCTION  -" + juce::String (shownGr, 1) + " dB",
                    area.toNearestInt().removeFromTop (12), juce::Justification::topLeft, false);
        g.setFont (ui::font (8.5f));
        g.drawText ("HISTORY", area.toNearestInt().removeFromTop (12), juce::Justification::topRight, false);
    }

    void drawMeters (juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto grBar = area.removeFromLeft (20.0f);
        area.removeFromLeft (10.0f);
        auto inBar = area.removeFromLeft (24.0f);
        area.removeFromLeft (6.0f);
        auto outBar = area.removeFromLeft (24.0f);

        auto drawBar = [&] (juce::Rectangle<float> r, float norm, juce::Colour c)
        {
            g.setColour (juce::Colours::white.withAlpha (0.07f));
            g.fillRoundedRectangle (r, 3.0f);
            const float h = r.getHeight() * juce::jlimit (0.0f, 1.0f, norm);
            g.setColour (c);
            g.fillRoundedRectangle (r.withTop (r.getBottom() - h), 3.0f);
        };

        drawBar (inBar, juce::jlimit (0.0f, 1.0f, (shownIn + 60.0f) / 60.0f), accent.withAlpha (0.75f));
        drawBar (outBar, juce::jlimit (0.0f, 1.0f, (shownOut + 60.0f) / 60.0f), accent);
        drawBar (grBar, juce::jlimit (0.0f, 1.0f, shownGr / 18.0f), juce::Colour (0xffff4d5e));

        g.setFont (ui::font (10.0f, true));
        g.setColour (ui::dim);
        g.drawText ("IN", inBar.reduced (2.0f, 0.0f).translated (0.0f, -14.0f).toNearestInt(),
                    juce::Justification::centred, false);
        g.drawText ("OUT", outBar.translated (0.0f, -14.0f).toNearestInt(), juce::Justification::centred, false);
        g.drawText ("GR", grBar.translated (0.0f, -14.0f).toNearestInt(), juce::Justification::centred, false);

        auto readout = area.withTrimmedLeft (4.0f);
        g.setFont (ui::font (10.0f));
        g.setColour (ui::text);
        g.drawText (juce::String (shownIn, 1), readout.removeFromTop (14.0f).toNearestInt(),
                    juce::Justification::centredLeft, false);
        g.drawText (juce::String (shownOut, 1), readout.removeFromTop (14.0f).toNearestInt(),
                    juce::Justification::centredLeft, false);
        g.setColour (juce::Colour (0xffff4d5e));
        g.drawText ("-" + juce::String (shownGr, 1), readout.removeFromTop (14.0f).toNearestInt(),
                    juce::Justification::centredLeft, false);

        g.setColour (ui::dim);
        g.setFont (ui::font (10.0f));
        g.drawText (juce::String (pv (ids::compRatio), 1) + ":1",
                    getLocalBounds().toFloat().reduced (16.0f, 10.0f).removeFromBottom (14.0f).toNearestInt(),
                    juce::Justification::centredRight, false);
    }

    void timerCallback() override
    {
        const float in = juce::Decibels::gainToDecibels (proc.meterIn.load(), -60.0f);
        const float out = juce::Decibels::gainToDecibels (proc.meterOut.load(), -60.0f);
        const float gr = juce::jlimit (0.0f, 18.0f, proc.scope.compReduction.load());

        shownIn += (in - shownIn) * 0.30f;
        shownOut += (out - shownOut) * 0.30f;
        shownGr += (gr - shownGr) * 0.35f;

        grHistory[(size_t) historyPos] = shownGr;
        historyPos = (historyPos + 1) % (int) grHistory.size();

        repaint();
    }

    float pv (const char* id) const
    {
        if (auto* v = proc.apvts.getRawParameterValue (id))
            return v->load();

        return 0.0f;
    }

    VocalROIDZProcessor& proc;
    juce::Colour accent;
    std::array<float, 120> grHistory {};
    int historyPos = 0;
    float shownIn = -60.0f, shownOut = -60.0f, shownGr = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorView)
};
