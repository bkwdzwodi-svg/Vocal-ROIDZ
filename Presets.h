#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include "Parameters.h"

namespace presets
{
    struct Param   { const char* id; float value; };
    struct Factory { const char* category; const char* name; std::vector<Param> params; };

    // Starting points, not gospel: every voice and mic is different, so tweak to taste.
    // apply() resets everything to defaults first, so only changed params are listed.
    inline const std::vector<Factory>& factory()
    {
        static const std::vector<Factory> list =
        {
            // ---------------------------------------------------------------- lead vocals
            { "Lead Vocals", "Clean Lead", {
                { ids::hpfFreq, 80.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.f }, { ids::compAttack, 15.f },
                { ids::compRelease, 120.f }, { ids::compMakeup, 3.f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.f } } },

            { "Lead Vocals", "Pop Lead - Bright and Wide", {
                { ids::hpfFreq, 100.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 4.f }, { ids::compAttack, 8.f },
                { ids::compRelease, 90.f }, { ids::compMakeup, 4.f },
                { ids::deessFreq, 7500.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 4.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 250.f }, { ids::delayFb, 0.3f },
                { ids::delayTone, 5000.f }, { ids::delayMix, 0.14f } } },

            { "Lead Vocals", "Rock Lead - Aggressive", {
                { ids::hpfFreq, 105.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 6.f }, { ids::compAttack, 5.f },
                { ids::compRelease, 80.f }, { ids::compMakeup, 6.f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -26.f }, { ids::deessRatio, 3.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 220.f }, { ids::delayFb, 0.25f },
                { ids::delayTone, 4500.f }, { ids::delayMix, 0.10f },
                { ids::outGain, -1.f } } },

            { "Lead Vocals", "R&B Lead - Silky", {
                { ids::hpfFreq, 90.f },
                { ids::compThresh, -18.f }, { ids::compRatio, 2.5f }, { ids::compAttack, 25.f },
                { ids::compRelease, 160.f }, { ids::compMakeup, 2.5f },
                { ids::deessFreq, 7200.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.5f },
                { ids::delayOn, 1.f }, { ids::delayTime, 300.f }, { ids::delayFb, 0.28f },
                { ids::delayTone, 4000.f }, { ids::delayMix, 0.12f } } },

            { "Lead Vocals", "Country Lead - Natural", {
                { ids::hpfFreq, 95.f },
                { ids::compThresh, -16.f }, { ids::compRatio, 2.5f }, { ids::compAttack, 20.f },
                { ids::compRelease, 140.f }, { ids::compMakeup, 2.f },
                { ids::deessFreq, 6800.f }, { ids::deessThresh, -26.f }, { ids::deessRatio, 2.5f },
                { ids::delayOn, 1.f }, { ids::delayTime, 160.f }, { ids::delayFb, 0.18f },
                { ids::delayTone, 6000.f }, { ids::delayMix, 0.08f } } },

            { "Lead Vocals", "Gospel Lead - Powerful", {
                { ids::hpfFreq, 85.f },
                { ids::compThresh, -26.f }, { ids::compRatio, 5.f }, { ids::compAttack, 6.f },
                { ids::compRelease, 100.f }, { ids::compMakeup, 6.f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -28.f }, { ids::deessRatio, 3.f },
                { ids::outGain, -1.f } } },

            // ------------------------------------------------------------ rap and ad-libs
            { "Rap and Ad-libs", "Rap Lead - Tight", {
                { ids::hpfFreq, 90.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.5f }, { ids::compAttack, 5.f },
                { ids::compRelease, 70.f }, { ids::compMakeup, 5.f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 4.f } } },

            { "Rap and Ad-libs", "Trap Ad-lib - Slapback", {
                { ids::hpfFreq, 160.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 5.f }, { ids::compAttack, 4.f },
                { ids::compRelease, 60.f }, { ids::compMakeup, 5.f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 4.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 90.f }, { ids::delayFb, 0.2f },
                { ids::delayTone, 4000.f }, { ids::delayMix, 0.22f } } },

            { "Rap and Ad-libs", "Ad-libs - Big Echo", {
                { ids::hpfFreq, 150.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 10.f },
                { ids::compRelease, 100.f }, { ids::compMakeup, 3.f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 375.f }, { ids::delayFb, 0.45f },
                { ids::delayTone, 3500.f }, { ids::delayMix, 0.30f },
                { ids::outGain, -2.f } } },

            // ----------------------------------------------------- backgrounds and choirs
            { "Backgrounds and Choir", "Background Vocals - Smooth", {
                { ids::hpfFreq, 140.f },
                { ids::compThresh, -18.f }, { ids::compRatio, 2.5f }, { ids::compAttack, 20.f },
                { ids::compRelease, 150.f }, { ids::compMakeup, 2.f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -26.f }, { ids::deessRatio, 3.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 180.f }, { ids::delayFb, 0.2f },
                { ids::delayTone, 3000.f }, { ids::delayMix, 0.10f },
                { ids::outGain, -3.f } } },

            { "Backgrounds and Choir", "Choir Stack - Soft", {
                { ids::hpfFreq, 150.f },
                { ids::compThresh, -14.f }, { ids::compRatio, 2.f }, { ids::compAttack, 30.f },
                { ids::compRelease, 200.f }, { ids::compMakeup, 2.f },
                { ids::deessFreq, 6200.f }, { ids::deessThresh, -24.f }, { ids::deessRatio, 2.5f },
                { ids::delayOn, 1.f }, { ids::delayTime, 120.f }, { ids::delayFb, 0.15f },
                { ids::delayTone, 3000.f }, { ids::delayMix, 0.09f },
                { ids::outGain, -4.f } } },

            // --------------------------------------------------------------- spoken word
            { "Broadcast and Voice", "Podcast - Broadcast Clear", {
                { ids::hpfFreq, 100.f },
                { ids::compThresh, -28.f }, { ids::compRatio, 4.f }, { ids::compAttack, 3.f },
                { ids::compRelease, 120.f }, { ids::compMakeup, 6.f },
                { ids::deessFreq, 8000.f }, { ids::deessThresh, -34.f }, { ids::deessRatio, 5.f },
                { ids::outGain, -1.f } } },

            { "Broadcast and Voice", "Voiceover - Warm and Close", {
                { ids::hpfFreq, 70.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 3.f }, { ids::compAttack, 10.f },
                { ids::compRelease, 150.f }, { ids::compMakeup, 4.f },
                { ids::deessFreq, 7500.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 4.f } } },

            { "Broadcast and Voice", "Whisper / ASMR - Intimate", {
                { ids::hpfFreq, 120.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.f }, { ids::compAttack, 8.f },
                { ids::compRelease, 200.f }, { ids::compMakeup, 7.f },
                { ids::deessFreq, 6000.f }, { ids::deessThresh, -24.f }, { ids::deessRatio, 3.f },
                { ids::outGain, -2.f } } },

            // -------------------------------------------------------------- creative FX
            { "Creative FX", "EDM Topline - Air and Space", {
                { ids::hpfFreq, 120.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 10.f },
                { ids::compRelease, 110.f }, { ids::compMakeup, 3.5f },
                { ids::deessFreq, 8000.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 4.5f },
                { ids::delayOn, 1.f }, { ids::delayTime, 380.f }, { ids::delayFb, 0.5f },
                { ids::delayTone, 6000.f }, { ids::delayMix, 0.30f } } },

            { "Creative FX", "Lo-Fi Tape - Old School", {
                { ids::hpfFreq, 220.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 4.f }, { ids::compAttack, 12.f },
                { ids::compRelease, 90.f }, { ids::compMakeup, 3.f },
                { ids::deessFreq, 5500.f }, { ids::deessThresh, -22.f }, { ids::deessRatio, 2.5f },
                { ids::delayOn, 1.f }, { ids::delayTime, 320.f }, { ids::delayFb, 0.35f },
                { ids::delayTone, 1800.f }, { ids::delayMix, 0.22f },
                { ids::outGain, -3.f } } },

            { "Creative FX", "Live Stage - Feedback Safe", {
                { ids::hpfFreq, 140.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 4.f }, { ids::compAttack, 6.f },
                { ids::compRelease, 70.f }, { ids::compMakeup, 4.f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 4.f },
                { ids::outGain, -2.f } } },
        };
        return list;
    }

    inline void apply (juce::AudioProcessorValueTreeState& apvts, const std::vector<Param>& values)
    {
        for (auto* p : apvts.processor.getParameters())
            p->setValueNotifyingHost (p->getDefaultValue());

        for (auto& v : values)
            if (auto* param = apvts.getParameter (v.id))
                param->setValueNotifyingHost (param->convertTo0to1 (v.value));
    }

    // ---- your own presets, saved as .vrpreset files ----

    inline juce::File userFolder()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("VocalROIDZ").getChildFile ("Presets");
        dir.createDirectory();
        return dir;
    }

    inline juce::Array<juce::File> listUser()
    {
        auto files = userFolder().findChildFiles (juce::File::findFiles, false, "*.vrpreset");
        std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
                   { return a.getFileName().compareIgnoreCase (b.getFileName()) < 0; });
        return files;
    }

    inline bool saveUser (juce::AudioProcessorValueTreeState& apvts, const juce::String& name)
    {
        auto xml = apvts.copyState().createXml();
        if (xml == nullptr)
            return false;

        return xml->writeTo (userFolder().getChildFile (juce::File::createLegalFileName (name) + ".vrpreset"));
    }

    inline bool loadUser (juce::AudioProcessorValueTreeState& apvts, const juce::File& file)
    {
        auto xml = juce::XmlDocument::parse (file);
        if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
            return false;

        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        return true;
    }
}
