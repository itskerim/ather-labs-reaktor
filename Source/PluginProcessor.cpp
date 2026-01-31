#include "PluginProcessor.h"
#include "PluginEditor.h"

AetherAudioProcessor::AetherAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       )
#endif
{
    formatManager.registerBasicFormats();
}

AetherAudioProcessor::~AetherAudioProcessor()
{
}

const juce::String AetherAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AetherAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AetherAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AetherAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AetherAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AetherAudioProcessor::getNumPrograms()
{
    return 1;
}

int AetherAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AetherAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AetherAudioProcessor::getProgramName (int index)
{
    return {};
}

void AetherAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void AetherAudioProcessor::loadCustomNoise(const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader)
    {
        juce::AudioBuffer<float> newBuffer(reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&newBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        
        aetherEngine.setCustomNoise(newBuffer);
        
        // Note: UI updates param to "Custom" automatically
    }
}

void AetherAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    aetherEngine.prepare(spec);
    
    visualiser.setSamplesPerBlock(256);
    visualiser.setBufferSize(1024);
}

void AetherAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AetherAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
  #endif
}
#endif

void AetherAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // --- Params ---
    float drive = apvts.getRawParameterValue("drive")->load();
    int stages = (int)apvts.getRawParameterValue("stages")->load();
    auto algoPos = (aether::DistortionAlgo)(int)apvts.getRawParameterValue("algoPos")->load();
    auto algoNeg = (aether::DistortionAlgo)(int)apvts.getRawParameterValue("algoNeg")->load();
    
    float cutoff = apvts.getRawParameterValue("cutoff")->load();
    float res = apvts.getRawParameterValue("res")->load();
    float morph = apvts.getRawParameterValue("morph")->load();
    
    float fbAmount = apvts.getRawParameterValue("fbAmount")->load();
    float fbTime = apvts.getRawParameterValue("fbTime")->load();
    float scramble = *apvts.getRawParameterValue("scramble"); // Load Plasma
    
    float mix = apvts.getRawParameterValue("mix")->load();
    float outputDb = apvts.getRawParameterValue("output")->load();

    // Store Dry Signal for Mix
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    float sub = *apvts.getRawParameterValue("sub");
    float squeeze = *apvts.getRawParameterValue("squeeze");
    float width = *apvts.getRawParameterValue("width");
    float xover = *apvts.getRawParameterValue("xover");
    
    float fold = *apvts.getRawParameterValue("fold");
    bool vowelMode = *apvts.getRawParameterValue("filterMode") > 0.5f;

    // --- Get BPM ---
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
            if (pos->getBpm().hasValue())
                bpm = *pos->getBpm();
    }

    float noiseLevel = apvts.getRawParameterValue("noiseLevel")->load();
    float noiseWidth = apvts.getRawParameterValue("noiseWidth")->load();
    int noiseType = (int)apvts.getRawParameterValue("noiseType")->load();

    // Process Audio
    aetherEngine.process(buffer, drive, 0.0f, stages, algoPos, algoNeg, cutoff, res, morph, fbAmount, fbTime, scramble, sub, squeeze, bpm, width, xover, fold, vowelMode, noiseLevel, noiseWidth, noiseType);
    
    // Apply Mix
    // Apply Mix (Standard Linear Crossfade)
    // "Parallel Chain" feel: Ensure we blend properly without volume dip bug
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* wet = buffer.getWritePointer(ch);
        const auto* dry = dryBuffer.getReadPointer(ch);
        
        for (int s = 0; s < buffer.getNumSamples(); ++s)
        {
            // Standard Mix: Wet*Mix + Dry*(1-Mix)
            // This allows full Wet (Mix=1) and full Dry (Mix=0)
            wet[s] = wet[s] * mix + dry[s] * (1.0f - mix);
        }
    }

    for (int s = 0; s < buffer.getNumSamples(); ++s)
    {
        // Average channels for mono visualizer
        float sum = 0;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            sum += buffer.getSample(ch, s);
        audioFifo.push(sum / buffer.getNumChannels());
    }

    visualiser.pushBuffer(buffer);

    // Final Output Gain
    float gain = juce::Decibels::decibelsToGain(outputDb);
    buffer.applyGain(gain);
}

bool AetherAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AetherAudioProcessor::createEditor()
{
    return new PhatRackAudioProcessorEditor (*this);
}

void AetherAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AetherAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout AetherAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // --- Distortion ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));
    layout.add(std::make_unique<juce::AudioParameterInt>("stages", "Stages", 1, 12, 1));
    
    juce::StringArray algos;
    algos.add("None"); algos.add("SoftClip"); algos.add("HardClip"); algos.add("SineFold");
    algos.add("TriangleWarp"); algos.add("BitCrush"); algos.add("Rectify"); algos.add("Tanh");
    algos.add("SoftFold"); algos.add("Chebyshev");

    layout.add(std::make_unique<juce::AudioParameterChoice>("algoPos", "Positive Algo", algos, 1));
    layout.add(std::make_unique<juce::AudioParameterChoice>("algoNeg", "Negative Algo", algos, 1));
    layout.add(std::make_unique<juce::AudioParameterFloat>("bias", "Warp Bias", -1.0f, 1.0f, 0.0f));

    // --- Filter ---
    // Min 80Hz prevents "Dead Zone" at bottom. Skew 0.4 for better sweep feel.
    layout.add(std::make_unique<juce::AudioParameterFloat>("cutoff", "Filter Cutoff", juce::NormalisableRange<float>(80.0f, 20000.0f, 0.1f, 0.4f), 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("res", "Resonance", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("morph", "Filter Morph", 0.0f, 1.0f, 0.0f));

    // --- Feedback / Resonator ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("fbAmount", "Feedback", 0.0f, 1.1f, 0.0f)); // Allow self-oscillation
    layout.add(std::make_unique<juce::AudioParameterFloat>("fbTime", "Feedback Time", 0.1f, 500.0f, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("scramble", "Plasma/Scramble", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("fold", "Wavefolder", 0.0f, 1.0f, 0.0f)); 
    
    // --- Modes ---
    layout.add(std::make_unique<juce::AudioParameterBool>("filterMode", "Vowel Mode", false)); // False=Morph, True=Formant
    
    // --- Neuro Engine (New) ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("sub", "Sub Level", 0.0f, 2.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("squeeze", "Squeeze (OTT)", 0.0f, 1.0f, 0.4f)); // Default 40%
    
    // --- DnB Essentials (New) ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("width", "Hyper Width", 0.0f, 1.5f, 0.0f)); // 0 = Mono/Bypass, 1.5 = Super Wide
    layout.add(std::make_unique<juce::AudioParameterFloat>("xover", "Crossover Freq", 60.0f, 300.0f, 150.0f));

    // --- Global & UI ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("output", "Output Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("mix", "Dry/Wet", 0.0f, 1.0f, 1.0f));

    // --- Noise Engine ---
    layout.add(std::make_unique<juce::AudioParameterFloat>("noiseLevel", "Noise Level", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("noiseWidth", "Noise Distortion", 0.0f, 1.0f, 0.0f)); // Renamed Width to Distortion
    juce::StringArray noiseTypes; noiseTypes.add("White"); noiseTypes.add("Pink"); noiseTypes.add("Crackle"); noiseTypes.add("Custom");
    layout.add(std::make_unique<juce::AudioParameterChoice>("noiseType", "Noise Type", noiseTypes, 0));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AetherAudioProcessor();
}
