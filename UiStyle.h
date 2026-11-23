#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Shared palette for the whole workstation, so every page reads as one instrument.
namespace ui
{
    inline const juce::Colour bg       { 0xff070908 };
    inline const juce::Colour rail     { 0xff0b0e0b };
    inline const juce::Colour panel    { 0xff10140f };
    inline const juce::Colour panelAlt { 0xff161b14 };
    inline const juce::Colour line     { 0xff27301f };
    inline const juce::Colour text     { 0xffdbe4d2 };
    inline const juce::Colour dim      { 0xff7d8b76 };
    inline const juce::Colour green    { 0xff86ff2b };
    inline const juce::Colour gold     { 0xffffc93c };

    inline juce::Font font (float height, bool bold = false)
    {
        auto options = juce::FontOptions().withHeight (height);

        if (bold)
            options = options.withStyle ("Bold");

        return juce::Font (options);
    }

    inline float textWidth (const juce::Font& f, const juce::String& s)
    {
        juce::GlyphArrangement ga;
        ga.addLineOfText (f, s, 0.0f, 0.0f);
        return ga.getBoundingBox (0, -1, true).getWidth();
    }
}
