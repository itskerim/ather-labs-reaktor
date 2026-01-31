#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "AetherDSP.h"

class AetherAudioProcessor  : public juce::AudioProcessor
{
public:
    AetherAudioProcessor();
    ~AetherAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts {*this, nullptr, "Parameters", createParameterLayout()};

    // Audio Visualization FIFO
    struct AudioFIFO {
        static constexpr int size = 4096;
        float buffer[size] {0};
        std::atomic<int> writeIndex {0};
        std::atomic<int> readIndex {0};
        
        void push(float samples) {
            buffer[writeIndex.load() % size] = samples;
            writeIndex++;
        }
        
        void pull(juce::AudioBuffer<float>& out) {
            int w = writeIndex.load();
            int r = readIndex.load();
            int count = std::min((int)out.getNumSamples(), w - r);
            if (count <= 0) return;
            
            auto* d = out.getWritePointer(0);
            for (int i=0; i<count; ++i) {
                d[i] = buffer[(r + i) % size];
            }
            readIndex += count;
        }
    } audioFifo;

    // Waveform Data
    juce::AudioVisualiserComponent visualiser { 1 };

    // Custom Noise Loading
    void loadCustomNoise(const juce::File& file);
    juce::AudioFormatManager formatManager;
    
    // RMS Meter for UI
    std::atomic<float> outputMeter { 0.0f };

private:
    // The AETHER Engine
    aether::AetherEngine<float> aetherEngine;

    // Pre-allocated buffer for dry signal to avoid allocation in audio thread
    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherAudioProcessor)
};
