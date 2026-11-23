#pragma once
#include <vector>
#include "PluginProcessor.h"
#include "Modules.h"
#include "ScopeView.h"
#include "Syringe.h"
#include "UiStyle.h"

// One premium hardware unit per module page: title, live graph, syringe controls.
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
    std::unique_ptr<juce::ToggleButton> extra;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> extraAttach;
    std::vector<std::unique_ptr<SyringeControl>> controls;
    std::unique_ptr<ScopeView> scope;

    juce::Rectangle<int> titleArea, scopeArea, controlsArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulePage)
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
    juce::LookAndFeel_V4 laf;   // declared first so it outlives the components using it

    juce::ComboBox presetBox;
    juce::TextButton saveButton { "SAVE" }, prevButton { "<" }, nextButton { ">" };
    std::vector<std::unique_ptr<juce::TextButton>> tabs;
    std::vector<std::unique_ptr<ModulePage>> pages;

    int currentPage = 0;
    int shownProgram = -1;
    std::unique_ptr<SplashOverlay> splashOverlay;
    juce::Array<juce::File> userFiles;

    juce::Rectangle<int> headerArea, railArea, pageArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalROIDZEditor)
};
