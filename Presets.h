#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include "Parameters.h"

namespace presets
{
    struct Param   { const char* id; float value; };
    struct Factory { const char* name; std::vector<Param> params; };

    // Starting points, not gospel: every voice and mic is different, so tweak to taste.
    // apply() resets everything to defaults first, so only changed params are listed.
    inline const std::vector<Factory>& factory()
    {
        static const std::vector<Factory> list =
        {
            { "Clean Lead", {
                { ids::hpfFreq, 80.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.f }, { ids::compAttack, 15.f },
                { ids::compRelease, 120.f }, { ids::compMakeup, 3.f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.f } } },

            { "Pop Lead - Bright and Wide", {
                { ids::hpfFreq, 100.f },
                { ids::compThresh, -22.f }, { ids::compRatio, 4.f }, { ids::compAttack, 8.f },
                { ids::compRelease, 90.f }, { ids::compMakeup, 4.f },
                { ids::deessFreq, 7500.f }, { ids::deessThresh, -32.f }, { ids::deessRatio, 4.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 250.f }, { ids::delayFb, 0.3f },
                { ids::delayTone, 5000.f }, { ids::delayMix, 0.14f } } },

            { "Rap Lead - Tight", {
                { ids::hpfFreq, 90.f },
                { ids::compThresh, -24.f }, { ids::compRatio, 4.5f }, { ids::compAttack, 5.f },
                { ids::compRelease, 70.f }, { ids::compMakeup, 5.f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 4.f } } },

            { "Ad-libs - Big Echo", {
                { ids::hpfFreq, 150.f },
                { ids::compThresh, -20.f }, { ids::compRatio, 3.5f }, { ids::compAttack, 10.f },
                { ids::compRelease, 100.f }, { ids::compMakeup, 3.f },
                { ids::deessFreq, 7000.f }, { ids::deessThresh, -30.f }, { ids::deessRatio, 3.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 375.f }, { ids::delayFb, 0.45f },
                { ids::delayTone, 3500.f }, { ids::delayMix, 0.30f },
                { ids::outGain, -2.f } } },

            { "Background Vocals - Smooth", {
                { ids::hpfFreq, 140.f },
                { ids::compThresh, -18.f }, { ids::compRatio, 2.5f }, { ids::compAttack, 20.f },
                { ids::compRelease, 150.f }, { ids::compMakeup, 2.f },
                { ids::deessFreq, 6500.f }, { ids::deessThresh, -26.f }, { ids::deessRatio, 3.f },
                { ids::delayOn, 1.f }, { ids::delayTime, 180.f }, { ids::delayFb, 0.2f },
                { ids::delayTone, 3000.f }, { ids::delayMix, 0.10f },
                { ids::outGain, -3.f } } },
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
