#pragma once
#include <initializer_list>
#include "PluginProcessor.h"
#include "Syringe.h"

class VocalROIDZEditor : public juce::AudioProcessorEditor
{
public:
    explicit VocalROIDZEditor (VocalROIDZProcessor&);
    ~VocalROIDZEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct KnobSpec { const char* id; const char* label; };

    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attach;
    };

    struct Section
    {
        int row = 0;
        juce::Rectangle<int> bounds;
        std::unique_ptr<juce::ToggleButton> power;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttach;
        std::vector<std::unique_ptr<Knob>> knobs;
    };

    void addSection (const juce::String& title, const char* powerId, int row, juce::Colour accent,
                     std::initializer_list<KnobSpec> knobs);
    void refreshPresetMenu();
    void promptSavePreset();

    VocalROIDZProcessor& proc;
    juce::LookAndFeel_V4 laf;   // declared first so it outlives the components using it

    juce::ComboBox presetBox;
    juce::TextButton saveButton { "Save Preset" };
    juce::Array<juce::File> userFiles;
    std::vector<std::unique_ptr<Section>> sections;

    // the juice rack: in -> comp -> ess -> makeup -> gain -> out
    std::unique_ptr<SyringeMeter>   meterIn, meterComp, meterEss, meterOut;
    std::unique_ptr<SyringeControl> ctlMakeup, ctlGain;

    juce::Rectangle<int> titleArea, rackBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VocalROIDZEditor)
};
