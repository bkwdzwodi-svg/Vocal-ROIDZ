#pragma once
#include <functional>
#include <vector>
#include "PluginProcessor.h"
#include "Modules.h"
#include "ScopeView.h"
#include "ModuleViews.h"
#include "Syringe.h"
#include "UiStyle.h"

// One section page: title, tag, live graph and syringe controls.
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

// One effect in the FX rack: name, blurb, on/off pill and its own syringes.
class FxRackRow : public juce::Component
{
public:
    FxRackRow (VocalROIDZProcessor&, const modules::Module&, int number);

    void paint (juce::Graphics&) override;
    void resized() override;

    bool powerChanged();

private:
    const modules::Module& meta;
    int number = 1;
    juce::Colour col;

    juce::ToggleButton power;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttach;
    std::vector<std::unique_ptr<SyringeControl>> controls;

    juce::Rectangle<int> textArea, controlArea;
    bool lastState = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxRackRow)
};

// The FX rack: everything that is not one of the eight sections, in chain order.
class FxRackPage : public juce::Component, private juce::Timer
{
public:
    explicit FxRackPage (VocalROIDZProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutRows();

    juce::Rectangle<int> headArea;
    juce::Component content;      // declared before the viewport so the viewport dies first
    juce::Viewport viewport;
    std::vector<std::unique_ptr<FxRackRow>> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FxRackPage)
};

// A compact button font keeps the nine-section rail readable at any window height.
class RackLookAndFeel : public juce::LookAndFeel_V4
{
public:
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
    {
        return ui::font (juce::jlimit (10.0f, 14.0f, (float) buttonHeight * 0.46f), true);
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
