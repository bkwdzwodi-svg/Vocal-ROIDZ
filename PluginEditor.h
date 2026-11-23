#pragma once
#include <functional>
#include <vector>
#include "PluginProcessor.h"
#include "Modules.h"
#include "ScopeView.h"
#include "ModuleViews.h"
#include "Syringe.h"
#include "UiStyle.h"

// One hardware unit per module page: title, family tag, live graph and syringe controls.
class ModulePage : public juce::Component
{
public:
    ModulePage (VocalROIDZProcessor&, const modules::Module&, int index);
    ~ModulePage() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::Colour accentColour() const { return col; }

private:
    VocalROIDZProcessor& proc;
    const modules::Module& meta;
    int number = 1;
    juce::Colour col;

    juce::ToggleButton power;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttach;
    std::vector<std::unique_ptr<juce::ToggleButton>> switches;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> switchAttachments;
    std::vector<std::unique_ptr<SyringeControl>> controls;
    std::unique_ptr<juce::Component> graph;

    juce::Rectangle<int> titleArea, scopeArea, controlsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulePage)
};

// A single effect tile in the FX rack: big on/off state, click the card to open its page.
class FxCard : public juce::Component, private juce::Timer
{
public:
    FxCard (VocalROIDZProcessor&, const char* paramId, juce::String title, juce::String blurb,
            juce::Colour colour, std::function<void()> onOpen);
    ~FxCard() override { stopTimer(); }

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent&) override { if (onOpen) onOpen(); }
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override { hover = false; repaint(); }

private:
    void timerCallback() override;

    juce::ToggleButton toggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attach;
    juce::String titleText, blurbText;
    juce::Colour col;
    std::function<void()> onOpen;
    bool hover = false, lastState = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxCard)
};

// The FX menu: every creative effect in one place, each with its own on/off toggle.
class FxMenuPage : public juce::Component
{
public:
    FxMenuPage (VocalROIDZProcessor&, std::function<void(int)> openPage);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Rectangle<int> headArea, gridArea;
    std::vector<std::unique_ptr<FxCard>> cards;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxMenuPage)
};

// A compact button font keeps the eighteen-module rail readable at any window height.
class RackLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        return ui::font (juce::jlimit (9.0f, 13.0f, (float) buttonHeight * 0.54f), true);
    }
};

class SplashOverlay;

class VocalROIDZEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit VocalROIDZEditor (VocalROIDZProcessor&);
    ~VocalROIDZEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void showPage (int index);
    void refreshPresetMenu();
    void promptSavePreset();
    void showProgramInBox (int index);
    void timerCallback() override;

    VocalROIDZProcessor& proc;
    RackLookAndFeel laf;   // declared first so it outlives the components using it

    juce::ComboBox presetBox;
    juce::TextButton saveButton { "SAVE" }, prevButton { "<" }, nextButton { ">" };
    std::vector<std::unique_ptr<juce::TextButton>> tabs;
    std::vector<std::unique_ptr<juce::Component>> pages;
    std::vector<juce::uint32> pageColours;

    int currentPage = 0;
    int shownProgram = -1;
    std::unique_ptr<SplashOverlay> splashOverlay;
    juce::Array<juce::File> userFiles;

    juce::Rectangle<int> headerArea, railArea, pageArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalROIDZEditor)
};
