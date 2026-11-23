#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Peak-hold with a gentle fall so the GUI (polling slower than the audio blocks) doesn't miss peaks.
    void holdMax (std::atomic<float>& target, float value, float decay)
    {
        target.store (juce::jmax (value, target.load() * decay));
    }
}

VocalROIDZProcessor::VocalROIDZProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    // Program 0 is what the host shows at instantiation, so start the chain on it
    // rather than on bare parameter defaults.
    if (! presets::factory().empty())
        presets::apply (apvts, presets::factory().front().params);
}

float VocalROIDZProcessor::param (const char* id) const
{
    return apvts.getRawParameterValue (id)->load();
}

void VocalROIDZProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) 2 };

    hpf.prepare (spec);
    hpf.setType (juce::dsp::StateVariableTPTFilterType::highpass);

    comp.prepare (spec);

    makeup.prepare (spec);
    makeup.setRampDurationSeconds (0.02);
    outGain.prepare (spec);
    outGain.setRampDurationSeconds (0.02);

    deesser.prepare (sampleRate);

    delay.setMaximumDelayInSamples ((int) (sampleRate * 1.1));
    delay.prepare (spec);
    delayTone.prepare (spec);
    delayTone.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    delaySamples.reset (sampleRate, 0.05);
    delayMix.reset (sampleRate, 0.03);
    delaySamples.setCurrentAndTargetValue ((float) (param (ids::delayTime) * 0.001 * sampleRate));
    delayMix.setCurrentAndTargetValue (param (ids::delayOn) > 0.5f ? param (ids::delayMix) : 0.0f);
}

bool VocalROIDZProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void VocalROIDZProcessor::processDelay (juce::AudioBuffer<float>& buffer)
{
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();

    for (int i = 0; i < n; ++i)
    {
        const float d   = delaySamples.getNextValue();
        const float mix = delayMix.getNextValue();

        for (int c = 0; c < numCh; ++c)
        {
            auto* data = buffer.getWritePointer (c);
            const float in = data[i];

            float wet = delay.popSample (c, d);
            wet = delayTone.processSample (c, wet);       // darkens each repeat
            delay.pushSample (c, in + wet * delayFeedback);

            data[i] = in + wet * mix;
        }
    }
}

void VocalROIDZProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();
    const float inMag = buffer.getMagnitude (0, n);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    if (param (ids::hpfOn) > 0.5f)
    {
        hpf.setCutoffFrequency (param (ids::hpfFreq));
        hpf.process (ctx);
    }

    float compReduction = 0.0f;
    if (param (ids::compOn) > 0.5f)
    {
        comp.setThreshold (param (ids::compThresh));
        comp.setRatio (param (ids::compRatio));
        comp.setAttack (param (ids::compAttack));
        comp.setRelease (param (ids::compRelease));

        const float pre = buffer.getMagnitude (0, n);
        comp.process (ctx);
        const float post = buffer.getMagnitude (0, n);

        // approximate gain reduction for the meter: level going in vs coming out (before makeup)
        if (pre > 1.0e-4f)
            compReduction = juce::jlimit (0.0f, 30.0f, juce::Decibels::gainToDecibels (pre)
                                                         - juce::Decibels::gainToDecibels (post));

        makeup.setGainDecibels (param (ids::compMakeup));
        makeup.process (ctx);
    }

    float essReduction = 0.0f;
    if (param (ids::deessOn) > 0.5f)
    {
        deesser.setParams (param (ids::deessFreq), param (ids::deessThresh), param (ids::deessRatio));
        deesser.process (buffer);
        essReduction = deesser.getReductionDb();
    }

    // Delay always runs so toggling it off fades the mix out instead of clicking.
    const bool delayOn = param (ids::delayOn) > 0.5f;
    delaySamples.setTargetValue ((float) (param (ids::delayTime) * 0.001 * currentSampleRate));
    delayMix.setTargetValue (delayOn ? param (ids::delayMix) : 0.0f);
    delayFeedback = param (ids::delayFb);
    delayTone.setCutoffFrequency (param (ids::delayTone));
    processDelay (buffer);

    outGain.setGainDecibels (param (ids::outGain));
    outGain.process (ctx);

    holdMax (meterIn,   inMag,                       0.85f);
    holdMax (meterOut,  buffer.getMagnitude (0, n),  0.85f);
    holdMax (meterComp, compReduction,               0.90f);
    holdMax (meterEss,  essReduction,                0.90f);
}

void VocalROIDZProcessor::setCurrentProgram (int index)
{
    const auto& list = presets::factory();

    if (index < 0 || index >= (int) list.size())
        return;

    currentProgram = index;
    presets::apply (apvts, list[(size_t) index].params);

    // Let the host redraw its program name when the change came from our editor.
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withProgramChanged (true));
}

const juce::String VocalROIDZProcessor::getProgramName (int index)
{
    const auto& list = presets::factory();

    if (index < 0 || index >= (int) list.size())
        return {};

    return list[(size_t) index].name;
}

juce::AudioProcessorEditor* VocalROIDZProcessor::createEditor()
{
    return new VocalROIDZEditor (*this);
}

void VocalROIDZProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty (currentProgramProperty, currentProgram, nullptr);

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void VocalROIDZProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto state = juce::ValueTree::fromXml (*xml);

            // Restore the program index without re-applying it: the saved parameters
            // already hold exactly what the user heard.
            const int savedProgram = (int) state.getProperty (currentProgramProperty, 0);
            currentProgram = juce::jlimit (0, (int) presets::factory().size() - 1, savedProgram);
            state.removeProperty (currentProgramProperty, nullptr);

            apvts.replaceState (state);
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VocalROIDZProcessor();
}
