#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <atomic>
#include <cmath>

// ---------------------------------------------------------------------------------------
// SyringeBase: a syringe-shaped gauge full of juice.
//
//  * The juice level is driven by a spring, so it overshoots and settles when the value
//    changes, and the surface sloshes harder the faster the level moves.
//  * Bubbles drift up through the juice (faster while it's sloshing).
//  * The plunger stopper rides on top of the juice and the rod pulls out as the level rises.
//
// Subclasses just say what the target level (0..1) is and what text to show.
// ---------------------------------------------------------------------------------------
class SyringeBase : public juce::Component, private juce::Timer
{
public:
    SyringeBase (juce::Colour juiceColour, juce::String labelText, bool redlineAtTop,
                 float springStiffness, float springDamping)
        : juice (juiceColour), label (std::move (labelText)), redline (redlineAtTop),
          stiffness (springStiffness), damping (springDamping)
    {
        for (auto& b : bubbles)
            respawn (b, true);

        startTimerHz (45);
    }

    ~SyringeBase() override { stopTimer(); }

    void paint (juce::Graphics& g) override
    {
        const auto geo = makeGeometry();
        const float lv = juce::jlimit (0.0f, 1.0f, level);

        auto jc = juice;
        if (redline)
            jc = juice.interpolatedWith (juce::Colour (0xffff2e4d),
                                         juce::jlimit (0.0f, 1.0f, (lv - 0.75f) / 0.25f));

        // glass back
        g.setColour (juce::Colours::white.withAlpha (0.05f));
        g.fillRoundedRectangle (geo.barrel, 4.0f);

        // juice
        const float liquidH  = lv * geo.liquidMaxH;
        const float surfaceY = geo.liquidBottom - liquidH;
        const float amp      = 0.9f + slosh * 4.0f;

        if (liquidH > 0.5f)
        {
            juce::Path clip;
            clip.addRoundedRectangle (geo.inner, 3.0f);

            juce::Graphics::ScopedSaveState state (g);
            g.reduceClipRegion (clip);

            const float x0 = geo.inner.getX();
            const float x1 = geo.inner.getRight();

            juce::Path liquid, foam;
            liquid.startNewSubPath (x0, surfaceY + wave (x0, amp));
            foam.startNewSubPath (x0, surfaceY + wave (x0, amp));

            for (float x = x0 + 2.0f; x < x1 + 2.0f; x += 2.0f)
            {
                const float xx = juce::jmin (x, x1);
                const float y = surfaceY + wave (xx, amp);
                liquid.lineTo (xx, y);
                foam.lineTo (xx, y);
            }

            liquid.lineTo (x1, geo.liquidBottom + 3.0f);
            liquid.lineTo (x0, geo.liquidBottom + 3.0f);
            liquid.closeSubPath();

            g.setGradientFill (juce::ColourGradient (jc.brighter (0.45f), 0.0f, surfaceY,
                                                     jc.darker (0.65f), 0.0f, geo.liquidBottom, false));
            g.fillPath (liquid);

            g.setColour (juce::Colours::white.withAlpha (0.55f));
            g.strokePath (foam, juce::PathStrokeType (1.3f));

            for (const auto& b : bubbles)
            {
                const float bx = x0 + b.x * geo.inner.getWidth() + std::sin (phase * 0.7f + b.frac * 8.0f) * 1.2f;
                const float by = geo.liquidBottom - b.frac * liquidH;
                if (by < surfaceY + 3.0f)
                    continue;

                g.setColour (juce::Colours::white.withAlpha (0.28f));
                g.fillEllipse (bx - b.r, by - b.r, b.r * 2.0f, b.r * 2.0f);
                g.setColour (juce::Colours::white.withAlpha (0.55f));
                g.drawEllipse (bx - b.r, by - b.r, b.r * 2.0f, b.r * 2.0f, 0.7f);
            }

            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.fillRect (x0 + 2.0f, surfaceY + 3.0f, 2.5f, juce::jmax (0.0f, liquidH - 6.0f));
        }

        // graduation marks
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        for (int i = 0; i <= 10; ++i)
        {
            const float ty  = geo.liquidBottom - (float) i / 10.0f * geo.liquidMaxH;
            const float len = (i % 5 == 0) ? 7.0f : 4.0f;
            g.drawLine (geo.barrel.getRight() - 2.0f - len, ty, geo.barrel.getRight() - 2.0f, ty, 1.0f);
        }

        // glass outline + gloss
        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.drawRoundedRectangle (geo.barrel, 4.0f, 1.5f);
        g.setColour (juce::Colours::white.withAlpha (0.14f));
        g.fillRoundedRectangle (geo.barrel.getX() + 3.0f, geo.barrel.getY() + 5.0f,
                                2.5f, geo.barrel.getHeight() - 10.0f, 1.2f);

        // finger flange
        const float flangeW = geo.barrel.getWidth() + 18.0f;
        g.setColour (juce::Colour (0xffc3c9ce));
        g.fillRoundedRectangle (geo.cx - flangeW * 0.5f, geo.barrel.getY() - 4.0f, flangeW, 5.0f, 2.0f);

        // plunger: the stopper rides on the juice, the rod pulls out as the level rises
        const float stopperBottom = juce::jmax (geo.inner.getY() + 8.0f, surfaceY - 5.0f - amp);
        const float stopperTop    = stopperBottom - 10.0f;
        const float thumbY        = geo.barrel.getY() - 12.0f - lv * 26.0f;
        const float rodTop        = thumbY + 5.0f;

        g.setColour (juce::Colour (0xffd5dade));
        g.fillRoundedRectangle (geo.cx - 3.5f, rodTop, 7.0f, juce::jmax (0.0f, stopperTop - rodTop + 1.0f), 2.0f);
        g.fillRoundedRectangle (geo.cx - 15.0f, thumbY, 30.0f, 6.0f, 3.0f);
        g.setColour (juce::Colour (0xff17191b));
        g.fillRoundedRectangle (geo.inner.getX() + 0.5f, stopperTop, geo.inner.getWidth() - 1.0f, 10.0f, 2.0f);

        // hub + needle
        juce::Path hub;
        hub.startNewSubPath (geo.barrel.getX() + 2.0f, geo.barrel.getBottom() - 1.0f);
        hub.lineTo (geo.barrel.getRight() - 2.0f, geo.barrel.getBottom() - 1.0f);
        hub.lineTo (geo.cx + 3.0f, geo.hubBottom);
        hub.lineTo (geo.cx - 3.0f, geo.hubBottom);
        hub.closeSubPath();
        g.setColour (juce::Colour (0xffa4abb0));
        g.fillPath (hub);
        g.setColour (juce::Colour (0xffe6eaec));
        g.drawLine (geo.cx, geo.hubBottom, geo.cx, geo.tipY, 1.6f);

        // value + label
        auto ta = geo.textArea;
        g.setFont (12.5f);
        g.setColour (jc.brighter (0.3f));
        g.drawFittedText (getValueText(), ta.removeFromTop (17.0f).toNearestInt(), juce::Justification::centred, 1);
        g.setFont (12.0f);
        g.setColour (juce::Colours::white.withAlpha (0.8f));
        g.drawText (label, ta.toNearestInt(), juce::Justification::centredTop, false);
    }

protected:
    virtual float getTargetLevel() = 0;        // 0..1, polled every animation frame
    virtual juce::String getValueText() = 0;

    struct Geo
    {
        juce::Rectangle<float> textArea, barrel, inner;
        float cx = 0.0f, hubBottom = 0.0f, tipY = 0.0f, liquidBottom = 0.0f, liquidMaxH = 1.0f;
    };

    Geo makeGeometry() const
    {
        Geo geo;
        auto b = getLocalBounds().toFloat();
        geo.textArea = b.removeFromBottom (36.0f);

        auto s = b.reduced (2.0f, 2.0f);
        geo.cx = s.getCentreX();

        const float barrelW   = juce::jmin (26.0f, s.getWidth() * 0.55f);
        const float needleLen = juce::jlimit (24.0f, 54.0f, s.getHeight() * 0.13f);
        const float hubH      = 12.0f;
        const float barrelTop = s.getY() + 44.0f;
        const float barrelBot = s.getBottom() - needleLen - hubH;

        geo.barrel = { geo.cx - barrelW * 0.5f, barrelTop, barrelW, juce::jmax (30.0f, barrelBot - barrelTop) };
        geo.inner = geo.barrel.reduced (3.0f, 2.5f);
        geo.liquidBottom = geo.inner.getBottom();
        geo.liquidMaxH = juce::jmax (4.0f, geo.inner.getHeight() - 16.0f);
        geo.hubBottom = geo.barrel.getBottom() + hubH;
        geo.tipY = s.getBottom();
        return geo;
    }

    void snapTo (float v) { level = v; vel = 0.0f; }

private:
    struct Bubble { float x = 0.5f, frac = 0.0f, speed = 0.005f, r = 1.5f; };

    float wave (float x, float amp) const
    {
        return amp * std::sin (phase + x * 0.45f) + amp * 0.45f * std::sin (phase * 1.7f + x * 0.9f + 1.3f);
    }

    void respawn (Bubble& b, bool initial)
    {
        b.x = 0.15f + rng.nextFloat() * 0.7f;
        b.frac = initial ? rng.nextFloat() : 0.0f;
        b.speed = 0.004f + rng.nextFloat() * 0.008f;
        b.r = 1.1f + rng.nextFloat() * 1.6f;
    }

    void timerCallback() override
    {
        const float target = getTargetLevel();

        // underdamped spring -> the juice overshoots a little and settles
        vel += (target - level) * stiffness;
        vel *= damping;
        level += vel;
        level = juce::jlimit (-0.08f, 1.08f, level);

        slosh = juce::jmax (slosh * 0.94f, juce::jmin (1.0f, std::abs (vel) * 20.0f));
        phase += 0.10f + slosh * 0.30f;

        for (auto& b : bubbles)
        {
            b.frac += b.speed * (1.0f + slosh * 6.0f);
            if (b.frac > 1.0f)
                respawn (b, false);
        }

        repaint();
    }

    juce::Colour juice;
    juce::String label;
    bool redline;
    float stiffness, damping;

    float level = 0.0f, vel = 0.0f, slosh = 0.0f, phase = 0.0f;
    std::array<Bubble, 7> bubbles;
    juce::Random rng;
};

// ---------------------------------------------------------------------------------------
// SyringeMeter: shows a level coming from the audio thread (input/output peak, gain reduction).
// ---------------------------------------------------------------------------------------
class SyringeMeter : public SyringeBase
{
public:
    enum class Kind { levelDb, reductionDb };

    SyringeMeter (std::atomic<float>& src, Kind k, float rangeDb, juce::Colour c,
                  juce::String labelText, bool redlineAtTop)
        : SyringeBase (c, std::move (labelText), redlineAtTop, 0.28f, 0.74f),
          source (src), kind (k), range (rangeDb)
    {
    }

protected:
    float getTargetLevel() override
    {
        const float raw = source.load();
        float norm = 0.0f;

        if (kind == Kind::levelDb)
        {
            const float db = juce::Decibels::gainToDecibels (raw, -60.0f);
            norm = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -60.0f, 0.0f, 0.0f, 1.0f));
        }
        else
        {
            norm = juce::jlimit (0.0f, 1.0f, raw / range);
        }

        // fast attack, slower fall
        smoothed = norm > smoothed ? norm : smoothed + (norm - smoothed) * 0.10f;
        shownDb = kind == Kind::levelDb ? juce::jmap (smoothed, 0.0f, 1.0f, -60.0f, 0.0f)
                                        : smoothed * range;
        return smoothed;
    }

    juce::String getValueText() override
    {
        if (kind == Kind::levelDb)
            return shownDb <= -59.0f ? juce::String ("-inf") : juce::String (shownDb, 1) + " dB";

        return shownDb < 0.05f ? juce::String ("0.0 dB") : "-" + juce::String (shownDb, 1) + " dB";
    }

private:
    std::atomic<float>& source;
    Kind kind;
    float range;
    float smoothed = 0.0f, shownDb = -60.0f;
};

// ---------------------------------------------------------------------------------------
// SyringeControl: a draggable syringe bound to a plugin parameter.
// Drag up = draw more juice, drag down = push it out. Double-click resets. Mouse wheel nudges.
// ---------------------------------------------------------------------------------------
class SyringeControl : public SyringeBase
{
public:
    SyringeControl (juce::AudioProcessorValueTreeState& apvts, const char* paramId,
                    juce::Colour juiceColour, juce::String labelText)
        : SyringeBase (juiceColour, std::move (labelText), false, 0.16f, 0.80f),
          param (*apvts.getParameter (paramId)),
          attachment (param, [this] (float v) { targetNorm = param.convertTo0to1 (v); })
    {
        attachment.sendInitialUpdate();
        snapTo (targetNorm);
        setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        attachment.beginGesture();
        setFromY (e.position.y);
    }

    void mouseDrag (const juce::MouseEvent& e) override { setFromY (e.position.y); }

    void mouseUp (const juce::MouseEvent&) override { attachment.endGesture(); }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        targetNorm = param.getDefaultValue();
        attachment.setValueAsPartOfGesture (param.convertFrom0to1 (targetNorm));
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override
    {
        targetNorm = juce::jlimit (0.0f, 1.0f, targetNorm + wheel.deltaY * 0.1f);
        attachment.setValueAsCompleteGesture (param.convertFrom0to1 (targetNorm));
    }

protected:
    float getTargetLevel() override { return targetNorm; }

    juce::String getValueText() override
    {
        return (param.getCurrentValueAsText() + " " + param.getLabel()).trim();
    }

private:
    void setFromY (float y)
    {
        const auto geo = makeGeometry();
        targetNorm = juce::jlimit (0.0f, 1.0f, (geo.liquidBottom - y) / geo.liquidMaxH);
        attachment.setValueAsPartOfGesture (param.convertFrom0to1 (targetNorm));
    }

    juce::RangedAudioParameter& param;
    float targetNorm = 0.0f;
    juce::ParameterAttachment attachment;
};
