#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    if (auto* v = apvts.getRawParameterValue (id))
        return v->load();

    return 0.0f;
}

void VocalROIDZProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (1, samplesPerBlock), (juce::uint32) 2 };

    tuner.prepare (sampleRate, samplesPerBlock, 2);
    pitchFx.prepare (spec);
    harmony.prepare (sampleRate, samplesPerBlock, 2);
    cleanup.prepare (spec);
    gate.prepare (sampleRate);
    eq.prepare (spec);
    compressor.prepare (sampleRate);
    saturation.prepare (sampleRate);
    deesser.prepare (sampleRate);

    makeup.prepare (spec);
    makeup.setRampDurationSeconds (0.02);
    outGain.prepare (spec);
    outGain.setRampDurationSeconds (0.02);

    regularStutter.prepare (sampleRate, 2);
    chopScrew.prepare (sampleRate, 2);
    stutter.prepare (sampleRate, 2);
    doubler.prepare (spec);
    imager.prepare (spec);
    creative.prepare (spec);
    delayLab.prepare (spec);
    reverbSpace.prepare (spec);
    masterFinish.prepare (spec);

    fftFifoIndex = 0;
    fftBlockCounter = 0;
    std::fill (fftFifo.begin(), fftFifo.end(), 0.0f);
    std::fill (fftData.begin(), fftData.end(), 0.0f);
    std::fill (scope.spectrum.begin(), scope.spectrum.end(), 0.0f);
    std::fill (scope.wave.begin(), scope.wave.end(), 0.0f);
    std::fill (scope.pitchDetected.begin(), scope.pitchDetected.end(), 0.0f);
    std::fill (scope.pitchTarget.begin(), scope.pitchTarget.end(), 0.0f);
    scope.waveWrite = 0;
    scope.pitchWrite = 0;
}

bool VocalROIDZProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void VocalROIDZProcessor::pushPitch (float detected, float target)
{
    scope.pitchDetected[(size_t) scope.pitchWrite] = detected;
    scope.pitchTarget[(size_t) scope.pitchWrite] = target;
    scope.pitchWrite = (scope.pitchWrite + 1) % Scope::pitchSize;
}

void VocalROIDZProcessor::runSpectrum (const juce::AudioBuffer<float>& buffer)
{
    const int n = buffer.getNumSamples();
    const int ch = buffer.getNumChannels();

    for (int i = 0; i < n; ++i)
    {
        float mono = 0.0f;

        for (int c = 0; c < ch; ++c)
            mono += buffer.getSample (c, i);

        fftFifo[(size_t) fftFifoIndex++] = mono / (float) juce::jmax (1, ch);

        if (fftFifoIndex >= (int) fftFifo.size())
        {
            fftFifoIndex = 0;

            std::copy (fftFifo.begin(), fftFifo.end(), fftData.begin());
            std::fill (fftData.begin() + 1024, fftData.end(), 0.0f);
            fftWindow.multiplyWithWindowingTable (fftData.data(), 1024);
            fft.performFrequencyOnlyForwardTransform (fftData.data());

            constexpr int numBins = Scope::spectrumSize;
            const float minHz = 30.0f, maxHz = 18000.0f;

            for (int i = 0; i < numBins; ++i)
            {
                const float f0 = minHz * std::pow (maxHz / minHz, (float) i / (float) (numBins - 1));
                const float f1 = minHz * std::pow (maxHz / minHz, (float) (i + 1) / (float) (numBins - 1));
                const int b0 = juce::jlimit (1, 1023, (int) (f0 / (float) currentSampleRate * 2048.0f));
                const int b1 = juce::jlimit (b0 + 1, 1023, (int) (f1 / (float) currentSampleRate * 2048.0f));

                float peak = 0.0f;

                for (int b = b0; b < b1; ++b)
                    peak = juce::jmax (peak, fftData[(size_t) b]);

                const float db = juce::Decibels::gainToDecibels (peak / 512.0f, -90.0f);
                const float norm = juce::jlimit (0.0f, 1.0f, juce::jmap (db, -90.0f, 0.0f, 0.0f, 1.0f));
                auto& target = scope.spectrum[(size_t) i];
                target = target + (norm - target) * 0.55f;
            }
        }
    }
}

void VocalROIDZProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int n = buffer.getNumSamples();

    if (n <= 0)
        return;

    const float inMag = buffer.getMagnitude (0, n);
    scope.inputLevel.store (inMag);
    scope.wave[(size_t) scope.waveWrite] = inMag;
    scope.waveWrite = (scope.waveWrite + 1) % Scope::waveSize;

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    // ---------------------------------------------------------------- 01 Autotune Pro
    if (isOn (ids::tuneOn))
    {
        AutoTune::Settings s;
        s.key = (int) param (ids::tuneKey);
        s.scale = (int) param (ids::tuneScale);
        s.retune = param (ids::tuneRetune);
        s.humanize = param (ids::tuneHumanize);
        s.amount = param (ids::tuneAmount);
        s.hard = isOn (ids::tuneHard);
        s.formant = isOn (ids::tuneFormant);

        tuner.process (buffer, s);

        const float hz = tuner.detectedHz.load();
        const float target = tuner.targetHz.load();

        if (hz > 0.0f && target > 0.0f)
        {
            const float dMidi = 69.0f + 12.0f * std::log2 (hz / 440.0f);
            const float tMidi = 69.0f + 12.0f * std::log2 (target / 440.0f);
            scope.detectedMidi.store (dMidi);
            scope.targetMidi.store (tMidi);
            pushPitch (dMidi, tMidi);
        }
        else
        {
            scope.detectedMidi.store (0.0f);
            scope.targetMidi.store (0.0f);
            pushPitch (0.0f, 0.0f);
        }
    }
    else
    {
        scope.detectedMidi.store (0.0f);
        scope.targetMidi.store (0.0f);
        pushPitch (0.0f, 0.0f);
    }

    // ---------------------------------------------------------------- 02 Pitch FX
    // Creative pitching is deliberately separate from the tuner so correction can never
    // transpose the voice by itself.
    if (isOn (ids::pitchFxOn))
        pitchFx.process (buffer, param (ids::pitchFxSemis), param (ids::pitchFxFormant),
                         param (ids::pitchFxMix));

    // ---------------------------------------------------------------- 08 Harmony Engine
    if (isOn (ids::harmOn))
        harmony.process (buffer, param (ids::harmThird), param (ids::harmFifth),
                         param (ids::harmOctUp), param (ids::harmOctDown));

    // ---------------------------------------------------------------- 16 Noise Removal
    if (isOn (ids::nrOn))
        cleanup.process (buffer, param (ids::nrHum), param (ids::nrBreath),
                         param (ids::nrClicks), param (ids::nrAmount));

    // ---------------------------------------------------------------- 05 Noise Gate Pro
    float gateReduction = 0.0f;
    if (isOn (ids::gateOn))
    {
        gate.setParams (param (ids::gateThresh), param (ids::gateAttack), param (ids::gateHold),
                        param (ids::gateRelease), param (ids::gateRange));
        gateReduction = gate.process (buffer);
    }

    // ---------------------------------------------------------------- 03 Analog EQ
    if (isOn (ids::eqOn))
        eq.process (buffer, param (ids::eqHpf), param (ids::eqLpf),
                    param (ids::eqLow), param (ids::eqLowFreq),
                    param (ids::eqMid), param (ids::eqMidFreq), param (ids::eqMidQ),
                    param (ids::eqHigh), param (ids::eqHighFreq), param (ids::eqAir));

    // ---------------------------------------------------------------- 04 Compressor X
    float compReduction = 0.0f;
    if (isOn (ids::compOn))
    {
        compressor.setParams (param (ids::compThresh), param (ids::compRatio), param (ids::compAttack),
                              param (ids::compRelease), param (ids::compKnee), (int) param (ids::compModel));

        compReduction = compressor.process (buffer, param (ids::compMix), (int) param (ids::compModel));

        makeup.setGainDecibels (param (ids::compMakeup));
        makeup.process (ctx);
    }

    // ---------------------------------------------------------------- 06 Saturation Engine
    if (isOn (ids::satOn))
        saturation.process (buffer, (int) param (ids::satMode), param (ids::satDrive),
                            param (ids::satWarmth), param (ids::satDensity), param (ids::satMix));

    // ---------------------------------------------------------------- 15 De-Esser
    float essReduction = 0.0f;
    if (isOn (ids::deessOn))
    {
        deesser.setParams (param (ids::deessFreq), param (ids::deessThresh), param (ids::deessRatio));
        deesser.process (buffer);
        essReduction = deesser.getReductionDb();
    }

    // ---------------------------------------------------------------- 11 Regular Stutter
    if (isOn (ids::rstutOn))
        regularStutter.process (buffer, param (ids::rstutPeriod), param (ids::rstutSlice),
                                param (ids::rstutReps), param (ids::rstutMix));

    // ---------------------------------------------------------------- 13 Auto Chop-N-Screw
    if (isOn (ids::chopOn))
        chopScrew.process (buffer, (int) param (ids::chopMode), param (ids::chopPitch),
                           param (ids::chopSlow), param (ids::chopDensity), param (ids::chopRepeat),
                           param (ids::chopDrift), param (ids::chopMix));

    // ---------------------------------------------------------------- 12 Hyper Stutter
    if (isOn (ids::stutOn))
        stutter.process (buffer, (int) param (ids::stutMode), param (ids::stutSlice),
                         param (ids::stutRate), param (ids::stutReverse), param (ids::stutRandom),
                         param (ids::stutPitch), param (ids::stutMix));

    // ---------------------------------------------------------------- 07 Vocal Doubler
    if (isOn (ids::dblOn))
        doubler.process (buffer, param (ids::dblAmount), param (ids::dblWidth),
                         param (ids::dblHuman), param (ids::dblPitch));

    // ---------------------------------------------------------------- 14 Stereo Imager
    if (isOn (ids::stereoOn))
        imager.process (buffer, param (ids::stereoWidth), param (ids::stereoBass),
                        param (ids::stereoBalance));

    // ---------------------------------------------------------------- 17 Creative FX Lab
    if (isOn (ids::fxOn))
        creative.process (buffer, (int) param (ids::fxType), param (ids::fxAmount),
                          param (ids::fxTone), param (ids::fxMix));

    // ---------------------------------------------------------------- 10 Delay Lab
    if (isOn (ids::dlyOn))
        delayLab.process (buffer, (int) param (ids::dlyMode), param (ids::dlyTime),
                          param (ids::dlyFb), param (ids::dlyTone), param (ids::dlyWidth),
                          param (ids::dlyMix));

    // ---------------------------------------------------------------- 09 Reverb Space
    if (isOn (ids::revOn))
        reverbSpace.process (buffer, (int) param (ids::revAlgo), param (ids::revDecay),
                             param (ids::revPredelay), param (ids::revWidth), param (ids::revTone),
                             param (ids::revMod), param (ids::revMix));

    // ---------------------------------------------------------------- 18 Master Finish
    masterFinish.process (buffer, isOn (ids::masterOn), param (ids::masterLoud),
                          param (ids::masterPunch), param (ids::masterWarmth),
                          param (ids::masterAir), param (ids::masterGlue),
                          param (ids::masterCeiling));

    outGain.setGainDecibels (param (ids::outGain));
    outGain.process (ctx);

    const float outMag = buffer.getMagnitude (0, n);

    scope.outputLevel.store (outMag);
    scope.compReduction.store (compReduction);
    scope.gateReduction.store (gateReduction);
    scope.essReduction.store (essReduction);
    scope.lufs.store (masterFinish.getLufs());
    scope.correlation.store (masterFinish.getCorrelation());

    meterIn.store (inMag);
    meterOut.store (outMag);
    meterComp.store (compReduction);
    meterEss.store (essReduction);

    if (++fftBlockCounter >= 4)
    {
        fftBlockCounter = 0;
        runSpectrum (buffer);
    }
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
