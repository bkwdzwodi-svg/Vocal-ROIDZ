#include "PluginEditor.h"
#include "Presets.h"

namespace
{
    const juce::Colour toxicGreen (0xff86ff2b);
    const juce::Colour cyan       (0xff30e6ff);
    const juce::Colour purple     (0xffc65cff);
    const juce::Colour orange     (0xffff8d1f);
    const juce::Colour violet     (0xff8f5cff);
}

VocalROIDZEditor::VocalROIDZEditor (VocalROIDZProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    laf.setColourScheme (juce::LookAndFeel_V4::getMidnightColourScheme());
    laf.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    laf.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    laf.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    setLookAndFeel (&laf);

    addAndMakeVisible (presetBox);
    addAndMakeVisible (saveButton);
    refreshPresetMenu();

    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();

        if (id >= 1000)
        {
            const int idx = id - 1000;
            if (idx < userFiles.size())
                presets::loadUser (proc.apvts, userFiles[idx]);
        }
        else if (id >= 1)
        {
            const auto& list = presets::factory();
            const int idx = id - 1;
            if (idx < (int) list.size())
                presets::apply (proc.apvts, list[(size_t) idx].params);
        }
    };

    saveButton.onClick = [this] { promptSavePreset(); };

    // Knob sections (Makeup and Output Gain live on the juice rack as syringes)
    addSection ("High-Pass", ids::hpfOn, 0, cyan, { { ids::hpfFreq, "Freq (Hz)" } });
    addSection ("Compressor", ids::compOn, 0, purple,
                { { ids::compThresh,  "Thresh (dB)" },
                  { ids::compRatio,   "Ratio" },
                  { ids::compAttack,  "Attack (ms)" },
                  { ids::compRelease, "Release (ms)" } });
    addSection ("De-Esser", ids::deessOn, 1, orange,
                { { ids::deessFreq,   "Freq (Hz)" },
                  { ids::deessThresh, "Thresh (dB)" },
                  { ids::deessRatio,  "Ratio" } });
    addSection ("Delay", ids::delayOn, 1, toxicGreen,
                { { ids::delayTime, "Time (ms)" },
                  { ids::delayFb,   "Feedback" },
                  { ids::delayTone, "Tone (Hz)" },
                  { ids::delayMix,  "Mix" } });

    // Juice rack
    meterIn   = std::make_unique<SyringeMeter> (proc.meterIn,   SyringeMeter::Kind::levelDb,     0.0f,  cyan,       "IN",   true);
    meterComp = std::make_unique<SyringeMeter> (proc.meterComp, SyringeMeter::Kind::reductionDb, 20.0f, purple,     "COMP", false);
    meterEss  = std::make_unique<SyringeMeter> (proc.meterEss,  SyringeMeter::Kind::reductionDb, 12.0f, orange,     "ESS",  false);
    ctlMakeup = std::make_unique<SyringeControl> (proc.apvts, ids::compMakeup, violet,     "MAKEUP");
    ctlGain   = std::make_unique<SyringeControl> (proc.apvts, ids::outGain,    toxicGreen, "GAIN");
    meterOut  = std::make_unique<SyringeMeter> (proc.meterOut,  SyringeMeter::Kind::levelDb,     0.0f,  toxicGreen, "OUT",  true);

    for (juce::Component* c : { (juce::Component*) meterIn.get(), (juce::Component*) meterComp.get(),
                                (juce::Component*) meterEss.get(), (juce::Component*) ctlMakeup.get(),
                                (juce::Component*) ctlGain.get(), (juce::Component*) meterOut.get() })
        addAndMakeVisible (c);

    setSize (1000, 520);
}

VocalROIDZEditor::~VocalROIDZEditor()
{
    setLookAndFeel (nullptr);
}

void VocalROIDZEditor::addSection (const juce::String& title, const char* powerId, int row,
                                   juce::Colour accent, std::initializer_list<KnobSpec> knobs)
{
    auto s = std::make_unique<Section>();
    s->row = row;

    s->power = std::make_unique<juce::ToggleButton> (title);
    s->power->setColour (juce::ToggleButton::tickColourId, accent);
    s->powerAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, powerId, *s->power);
    addAndMakeVisible (*s->power);

    for (const auto& spec : knobs)
    {
        auto k = std::make_unique<Knob>();
        k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
        k->slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
        k->slider.setColour (juce::Slider::thumbColourId, accent.brighter (0.3f));
        k->slider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2a302a));
        k->label.setText (spec.label, juce::dontSendNotification);
        k->label.setJustificationType (juce::Justification::centred);
        k->attach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, spec.id, k->slider);
        addAndMakeVisible (k->slider);
        addAndMakeVisible (k->label);
        s->knobs.push_back (std::move (k));
    }

    sections.push_back (std::move (s));
}

void VocalROIDZEditor::refreshPresetMenu()
{
    presetBox.clear (juce::dontSendNotification);

    presetBox.addSectionHeading ("Factory");
    int id = 1;
    for (const auto& f : presets::factory())
        presetBox.addItem (f.name, id++);

    userFiles = presets::listUser();
    if (! userFiles.isEmpty())
    {
        presetBox.addSeparator();
        presetBox.addSectionHeading ("My Presets");
        int uid = 1000;
        for (const auto& f : userFiles)
            presetBox.addItem (f.getFileNameWithoutExtension(), uid++);
    }

    presetBox.setTextWhenNothingSelected ("Select preset...");
}

void VocalROIDZEditor::promptSavePreset()
{
    auto* w = new juce::AlertWindow ("Save preset", "Name your preset:", juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("name", "My Preset");
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<VocalROIDZEditor> safeThis (this);

    w->enterModalState (true, juce::ModalCallbackFunction::create ([safeThis, w] (int result)
    {
        if (result != 1 || safeThis == nullptr)
            return;

        const auto name = w->getTextEditorContents ("name").trim();
        if (name.isEmpty())
            return;

        if (presets::saveUser (safeThis->proc.apvts, name))
        {
            safeThis->refreshPresetMenu();
            safeThis->presetBox.setText (name, juce::dontSendNotification);
        }
    }), true);
}

void VocalROIDZEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0b0e0a));

    // title: VOCAL in white, ROIDZ in toxic green
    const auto titleFont = g.getCurrentFont().boldened().withHeight (34.0f);
    juce::GlyphArrangement ga;
    ga.addLineOfText (titleFont, "VOCAL ", 0.0f, 0.0f);
    const int w1 = (int) ga.getBoundingBox (0, -1, true).getWidth();

    g.setFont (titleFont);
    g.setColour (juce::Colours::white);
    g.drawText ("VOCAL", titleArea.withWidth (w1 + 4), juce::Justification::centredLeft, false);
    g.setColour (toxicGreen);
    g.drawText ("ROIDZ", titleArea.withTrimmedLeft (w1), juce::Justification::centredLeft, false);

    // knob panels
    for (const auto& s : sections)
    {
        g.setColour (juce::Colour (0xff161a15));
        g.fillRoundedRectangle (s->bounds.toFloat(), 8.0f);
        g.setColour (juce::Colour (0xff2a3326));
        g.drawRoundedRectangle (s->bounds.toFloat().reduced (0.5f), 8.0f, 1.0f);
    }

    // juice rack
    g.setColour (juce::Colour (0xff121611));
    g.fillRoundedRectangle (rackBounds.toFloat(), 10.0f);
    g.setColour (toxicGreen.withAlpha (0.35f));
    g.drawRoundedRectangle (rackBounds.toFloat().reduced (0.5f), 10.0f, 1.2f);
}

void VocalROIDZEditor::resized()
{
    auto area = getLocalBounds().reduced (14);

    auto header = area.removeFromTop (44);
    titleArea = header.removeFromLeft (360);
    auto controls = header.removeFromRight (240 + 8 + 110);
    presetBox.setBounds (controls.removeFromLeft (240).withSizeKeepingCentre (240, 30));
    controls.removeFromLeft (8);
    saveButton.setBounds (controls.withSizeKeepingCentre (110, 30));
    area.removeFromTop (10);

    // juice rack on the right
    rackBounds = area.removeFromRight (344);
    area.removeFromRight (12);

    auto rack = rackBounds.reduced (6, 8);
    const int cellW = rack.getWidth() / 6;
    meterIn->setBounds   (rack.removeFromLeft (cellW));
    meterComp->setBounds (rack.removeFromLeft (cellW));
    meterEss->setBounds  (rack.removeFromLeft (cellW));
    ctlMakeup->setBounds (rack.removeFromLeft (cellW));
    ctlGain->setBounds   (rack.removeFromLeft (cellW));
    meterOut->setBounds  (rack.removeFromLeft (cellW));

    // knob sections on the left, two rows
    const int rowHeight = area.getHeight() / 2;

    for (int row = 0; row < 2; ++row)
    {
        auto rowArea = area.removeFromTop (rowHeight).reduced (0, 5);

        int totalKnobs = 0;
        for (const auto& s : sections)
            if (s->row == row)
                totalKnobs += (int) s->knobs.size();

        if (totalKnobs == 0)
            continue;

        const int unit = rowArea.getWidth() / totalKnobs;

        for (auto& s : sections)
        {
            if (s->row != row)
                continue;

            auto secArea = rowArea.removeFromLeft (unit * (int) s->knobs.size());
            s->bounds = secArea.reduced (4, 0);

            auto inner = s->bounds.reduced (8);
            s->power->setBounds (inner.removeFromTop (26));
            inner.removeFromTop (4);

            const int cell = inner.getWidth() / (int) s->knobs.size();
            for (auto& k : s->knobs)
            {
                auto c = inner.removeFromLeft (cell);
                k->label.setBounds (c.removeFromTop (18));
                k->slider.setBounds (c);
            }
        }
    }
}
