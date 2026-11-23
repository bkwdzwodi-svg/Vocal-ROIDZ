#include "PluginEditor.h"
#include "Presets.h"

// The opening screen lives on its own overlay so it paints above the whole rack.
class SplashOverlay : public juce::Component
{
public:
    SplashOverlay()
    {
        setInterceptsMouseClicks (false, false);
        startMs = juce::Time::getMillisecondCounterHiRes();
    }

    bool finished() const
    {
        return (juce::Time::getMillisecondCounterHiRes() - startMs) / 1000.0 > 2.8;
    }

    void paint (juce::Graphics& g) override
    {
        const double elapsed = (juce::Time::getMillisecondCounterHiRes() - startMs) / 1000.0;
        const float fadeIn = juce::jlimit (0.0f, 1.0f, (float) elapsed / 0.30f);
        const float fadeOut = juce::jlimit (0.0f, 1.0f, (float) (2.8 - elapsed) / 0.5f);
        const float alpha = juce::jmin (fadeIn, fadeOut);

        if (alpha <= 0.0f)
            return;

        auto b = getLocalBounds().toFloat();
        g.setColour (ui::bg.withAlpha (0.97f * alpha));
        g.fillRect (b);

        auto centreArea = b.withSizeKeepingCentre (b.getWidth(), 230.0f);

        const auto titleFont = ui::font (54.0f, true);
        const float vocalWidth = ui::textWidth (titleFont, "VOCAL ");
        const float totalWidth = vocalWidth + ui::textWidth (titleFont, "ROIDS");

        auto titleArea = centreArea.removeFromTop (66.0f).withSizeKeepingCentre (totalWidth + 8.0f, 66.0f);

        g.setFont (titleFont);
        g.setColour (juce::Colours::white.withAlpha (alpha));
        g.drawText ("VOCAL", titleArea.withWidth (vocalWidth).toNearestInt(),
                    juce::Justification::centredLeft, false);
        g.setColour (ui::green.withAlpha (alpha));
        g.drawText ("ROIDS", titleArea.withTrimmedLeft (vocalWidth).toNearestInt(),
                    juce::Justification::centredLeft, false);

        g.setFont (ui::font (15.0f, true));
        g.setColour (ui::text.withAlpha (alpha * 0.9f));
        g.drawText ("THE ULTIMATE VOCAL PRODUCTION WORKSTATION",
                    centreArea.removeFromTop (28.0f).toNearestInt(), juce::Justification::centred, false);

        g.setFont (ui::font (13.0f));
        g.setColour (ui::dim.withAlpha (alpha));
        g.drawText ("DESIGNED BY BKWDZWODI   AKA   BKWDZ STUDIO",
                    centreArea.removeFromTop (26.0f).toNearestInt(), juce::Justification::centred, false);

        g.setFont (ui::font (19.0f, true));
        g.setColour (ui::green.withAlpha (alpha));
        g.drawText ("ENGINEER LESS.  CREATE MORE.",
                    centreArea.removeFromTop (44.0f).toNearestInt(), juce::Justification::centred, false);
    }

private:
    double startMs = 0.0;
};


// =======================================================================================
// ModulePage
// =======================================================================================
ModulePage::ModulePage (VocalROIDZProcessor& p, const modules::Module& m, int index)
    : proc (p), meta (m), number (index + 1), col (juce::Colour (m.colour))
{
    power.setButtonText ("POWER");
    power.setColour (juce::ToggleButton::tickColourId, col);
    power.setColour (juce::ToggleButton::textColourId, ui::text);
    powerAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, meta.powerId, power);
    addAndMakeVisible (power);

    if (meta.extraToggle != nullptr)
    {
        extra = std::make_unique<juce::ToggleButton> (juce::String (meta.extraToggleName));
        extra->setColour (juce::ToggleButton::tickColourId, col);
        extra->setColour (juce::ToggleButton::textColourId, ui::text);
        extraAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, meta.extraToggle, *extra);
        addAndMakeVisible (*extra);
    }

    for (const auto& c : meta.controls)
    {
        auto s = std::make_unique<SyringeControl> (proc.apvts, c.id, col, juce::String (c.label));
        addAndMakeVisible (*s);
        controls.push_back (std::move (s));
    }

    scope = std::make_unique<ScopeView> (proc, meta.scope, col);
    addAndMakeVisible (*scope);
}

void ModulePage::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();

    g.setColour (ui::panel);
    g.fillRoundedRectangle (b, 12.0f);
    g.setColour (ui::line);
    g.drawRoundedRectangle (b.reduced (0.5f), 12.0f, 1.0f);

    // Accent strip down the left edge.
    g.setColour (col.withAlpha (0.85f));
    g.fillRoundedRectangle (b.getX() + 1.0f, b.getY() + 14.0f, 3.0f, b.getHeight() - 28.0f, 1.5f);

    auto title = titleArea.toFloat();

    g.setFont (ui::font (11.0f, true));
    g.setColour (col.withAlpha (0.8f));
    g.drawText ("MODULE " + juce::String (number).paddedLeft ('0', 2),
                title.withHeight (16.0f).toNearestInt(), juce::Justification::bottomLeft, false);

    g.setFont (ui::font (24.0f, true));
    g.setColour (ui::text);
    g.drawText (meta.name, title.withTrimmedTop (12.0f).toNearestInt(), juce::Justification::topLeft, false);

    g.setFont (ui::font (12.0f));
    g.setColour (ui::dim);
    g.drawText (meta.subtitle, title.translated (0.0f, 34.0f).toNearestInt(),
                juce::Justification::topLeft, false);
}

void ModulePage::resized()
{
    auto area = getLocalBounds().reduced (18);

    titleArea = area.removeFromTop (54);

    auto switchers = titleArea.removeFromRight (meta.extraToggle != nullptr ? 230 : 110);
    power.setBounds (switchers.removeFromRight (100).withSizeKeepingCentre (100, 26));

    if (extra != nullptr)
    {
        switchers.removeFromRight (8);
        extra->setBounds (switchers.withSizeKeepingCentre (110, 26));
    }

    area.removeFromTop (10);

    scopeArea = area.removeFromTop (juce::jlimit (150, 270, area.getHeight() * 5 / 12));
    scope->setBounds (scopeArea);

    area.removeFromTop (12);
    controlsArea = area;

    const int n = juce::jmax (1, (int) controls.size());
    const int cols = n <= 5 ? n : (n + 1) / 2;
    const int rows = (n + cols - 1) / cols;
    const int cellW = controlsArea.getWidth() / cols;
    const int cellH = controlsArea.getHeight() / rows;

    int i = 0;

    for (auto& c : controls)
    {
        const int r = i / cols;
        const int colIndex = i % cols;
        c->setBounds (controlsArea.getX() + colIndex * cellW,
                      controlsArea.getY() + r * cellH, cellW, cellH);
        ++i;
    }
}

// =======================================================================================
// VocalROIDZEditor
// =======================================================================================
VocalROIDZEditor::VocalROIDZEditor (VocalROIDZProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    laf.setColourScheme (juce::LookAndFeel_V4::getMidnightColourScheme());
    laf.setColour (juce::Slider::textBoxTextColourId, ui::text);
    laf.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    laf.setColour (juce::ToggleButton::textColourId, ui::text);
    laf.setColour (juce::ToggleButton::tickColourId, ui::green);
    laf.setColour (juce::ComboBox::backgroundColourId, ui::panelAlt);
    laf.setColour (juce::ComboBox::textColourId, ui::text);
    laf.setColour (juce::ComboBox::outlineColourId, ui::line);
    laf.setColour (juce::ComboBox::arrowColourId, ui::dim);
    laf.setColour (juce::PopupMenu::backgroundColourId, ui::panelAlt);
    laf.setColour (juce::PopupMenu::textColourId, ui::text);
    laf.setColour (juce::PopupMenu::highlightedBackgroundColourId, ui::green.withAlpha (0.28f));
    laf.setColour (juce::PopupMenu::highlightedTextColourId, ui::text);
    laf.setColour (juce::TextButton::buttonColourId, ui::panelAlt);
    laf.setColour (juce::TextButton::textColourOffId, ui::text);
    laf.setColour (juce::TextButton::textColourOnId, ui::bg);
    setLookAndFeel (&laf);

    const auto& list = modules::all();

    for (int i = 0; i < (int) list.size(); ++i)
    {
        const auto& meta = list[(size_t) i];

        auto page = std::make_unique<ModulePage> (proc, meta, i);
        addChildComponent (*page);
        pages.push_back (std::move (page));

        auto tab = std::make_unique<juce::TextButton> (juce::String (i + 1).paddedLeft ('0', 2) + "   " + meta.name);
        tab->setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
        tab->setColour (juce::TextButton::textColourOffId, ui::dim);
        tab->setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        tab->onClick = [this, i] { showPage (i); };
        addAndMakeVisible (*tab);
        tabs.push_back (std::move (tab));
    }

    addAndMakeVisible (presetBox);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);

    presetBox.setColour (juce::ComboBox::backgroundColourId, ui::panelAlt);
    refreshPresetMenu();
    showProgramInBox (proc.getCurrentProgram());

    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();

        if (id >= 1000)
        {
            const int idx = id - 1000;
            if (idx < userFiles.size())
                presets::loadUser (proc.apvts, userFiles[idx]);

            // A user preset isn't a factory program, so keep the box showing it
            // instead of letting the program sync pull it back.
            shownProgram = proc.getCurrentProgram();
        }
        else if (id >= 1)
        {
            const auto& list = presets::factory();
            const int idx = id - 1;

            if (idx < (int) list.size())
            {
                proc.setCurrentProgram (idx);
                shownProgram = idx;
            }
        }
    };

    saveButton.onClick = [this] { promptSavePreset(); };

    prevButton.onClick = [this]
    {
        const int count = (int) presets::factory().size();
        const int next = (proc.getCurrentProgram() - 1 + count) % juce::jmax (1, count);
        proc.setCurrentProgram (next);
        showProgramInBox (next);
    };

    nextButton.onClick = [this]
    {
        const int count = (int) presets::factory().size();
        const int next = (proc.getCurrentProgram() + 1) % juce::jmax (1, count);
        proc.setCurrentProgram (next);
        showProgramInBox (next);
    };

    showPage (0);
    startTimerHz (30);

    setResizable (true, true);
    setResizeLimits (1080, 700, 1800, 1150);
    setSize (1260, 820);

    // Added last so the opening screen paints over the entire rack.
    splashOverlay = std::make_unique<SplashOverlay>();
    addAndMakeVisible (*splashOverlay);
    splashOverlay->setBounds (getLocalBounds());
}

VocalROIDZEditor::~VocalROIDZEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void VocalROIDZEditor::showPage (int index)
{
    const auto& list = modules::all();

    if (list.empty())
        return;

    currentPage = juce::jlimit (0, (int) list.size() - 1, index);

    for (int i = 0; i < (int) pages.size(); ++i)
    {
        const bool active = i == currentPage;
        pages[(size_t) i]->setVisible (active);

        if (auto* tab = tabs[(size_t) i].get())
        {
            const auto col = juce::Colour (list[(size_t) i].colour);
            tab->setColour (juce::TextButton::buttonColourId,
                            active ? col.withAlpha (0.16f) : juce::Colours::transparentBlack);
            tab->setColour (juce::TextButton::textColourOffId, active ? col : ui::dim);
            tab->setColour (juce::TextButton::textColourOnId, active ? col : ui::dim);
        }
    }

    repaint();
}

void VocalROIDZEditor::showProgramInBox (int index)
{
    shownProgram = index;

    if (index >= 0 && index < (int) presets::factory().size())
        presetBox.setSelectedId (index + 1, juce::dontSendNotification);
}

void VocalROIDZEditor::timerCallback()
{
    // The host can change programs behind our back; follow it without re-triggering onChange.
    if (proc.getCurrentProgram() != shownProgram)
        showProgramInBox (proc.getCurrentProgram());

    if (splashOverlay != nullptr)
    {
        splashOverlay->repaint();

        if (splashOverlay->finished())
        {
            splashOverlay->setVisible (false);
            splashOverlay.reset();
            repaint();
        }
    }
}

void VocalROIDZEditor::refreshPresetMenu()
{
    presetBox.clear (juce::dontSendNotification);

    // Factory presets, grouped by style.
    int id = 1;
    juce::String category;

    for (const auto& f : presets::factory())
    {
        if (category != juce::String (f.category))
        {
            category = f.category;
            presetBox.addSectionHeading (category);
        }

        presetBox.addItem (f.name, id++);
    }

    userFiles = presets::listUser();

    if (! userFiles.isEmpty())
    {
        presetBox.addSeparator();
        presetBox.addSectionHeading ("My Presets");
        int uid = 1000;

        for (const auto& f : userFiles)
            presetBox.addItem (f.getFileNameWithoutExtension(), uid++);
    }

    presetBox.setTextWhenNothingSelected ("SELECT PRESET");
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
    // Backdrop.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0a0d0a), 0.0f, 0.0f,
                                             juce::Colour (0xff050605), 0.0f, (float) getHeight(), false));
    g.fillAll();

    // Header brand.
    auto header = headerArea;

    g.setFont (ui::font (28.0f, true));
    const float vocalW = ui::textWidth (ui::font (28.0f, true), "VOCAL ");
    auto brand = header.reduced (20, 12).removeFromLeft (300);

    g.setColour (juce::Colours::white);
    g.drawText ("VOCAL", brand.withWidth ((int) vocalW).withHeight (34), juce::Justification::bottomLeft, false);
    g.setColour (ui::green);
    g.drawText ("ROIDS", brand.withTrimmedLeft ((int) vocalW).withHeight (34), juce::Justification::bottomLeft, false);

    g.setFont (ui::font (10.0f, true));
    g.setColour (ui::dim);
    g.drawText ("STUDIO WORKSTATION   /   BKWDZ", brand.translated (0, 36).withHeight (16),
                juce::Justification::topLeft, false);

    // Tagline on the far right of the header.
    g.setFont (ui::font (10.0f, true));
    g.setColour (ui::green.withAlpha (0.7f));
    g.drawText ("ENGINEER LESS. CREATE MORE.", header.reduced (20, 0).removeFromRight (250),
                juce::Justification::centredRight, false);

    // Header divider.
    g.setColour (ui::line);
    g.drawHorizontalLine (headerArea.getBottom() - 1, 16.0f, (float) getWidth() - 16.0f);

    // Rail.
    g.setColour (ui::rail);
    g.fillRect (railArea);
    g.setColour (ui::line);
    g.drawVerticalLine (railArea.getRight() - 1, (float) railArea.getY(), (float) railArea.getBottom());

    g.setFont (ui::font (9.0f, true));
    g.setColour (ui::dim.withAlpha (0.8f));
    g.drawText ("MODULES", railArea.reduced (18, 0).removeFromTop (26).translated (0, 4),
                juce::Justification::topLeft, false);

    g.setFont (ui::font (9.0f));
    g.setColour (ui::dim.withAlpha (0.6f));
    g.drawText ("v2.0  //  VST3", railArea.reduced (18, 12).removeFromBottom (18),
                juce::Justification::bottomLeft, false);
}

void VocalROIDZEditor::resized()
{
    auto area = getLocalBounds();

    headerArea = area.removeFromTop (82);
    railArea = area.removeFromLeft (198);
    pageArea = area;

    auto header = headerArea.reduced (20, 12);
    header.removeFromLeft (300);

    auto controls = header.removeFromRight (440);
    presetBox.setBounds (controls.removeFromLeft (252).withSizeKeepingCentre (252, 30));
    controls.removeFromLeft (8);
    prevButton.setBounds (controls.removeFromLeft (30).withSizeKeepingCentre (30, 30));
    controls.removeFromLeft (4);
    nextButton.setBounds (controls.removeFromLeft (30).withSizeKeepingCentre (30, 30));
    controls.removeFromLeft (10);
    saveButton.setBounds (controls.removeFromLeft (72).withSizeKeepingCentre (72, 30));

    auto rail = railArea.reduced (10, 34);
    rail.removeFromTop (6);

    const int tabHeight = juce::jlimit (24, 38, rail.getHeight() / juce::jmax (1, (int) tabs.size()));

    for (auto& tab : tabs)
        tab->setBounds (rail.removeFromTop (tabHeight).reduced (0, 2));

    auto pageBounds = pageArea.reduced (14, 12);

    for (auto& page : pages)
        page->setBounds (pageBounds);

    if (splashOverlay != nullptr)
        splashOverlay->setBounds (getLocalBounds());
}
