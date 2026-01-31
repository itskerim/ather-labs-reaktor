#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include "PluginProcessor.h"
#include "AetherLookAndFeel.h"
#include "AetherAlgorithmSelector.h"
#include "AetherVisualizer.h"
#include "AetherCustomKnob.h"
#include "AetherOrb.h"
#include "AetherLogo.h"
#include "AetherReactorTank.h"

// ...

/**
 * PhatRackAudioProcessorEditor: The 'Industrial' AETHER Dashboard.
 * 20+ controls, tabbed sections, and real-time X/Y visualization.
 */
class PhatRackAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    PhatRackAudioProcessorEditor (AetherAudioProcessor&);
    ~PhatRackAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    AetherAudioProcessor& audioProcessor;

    // AETHER LookAndFeel & Custom Styles
    aether::AetherLookAndFeel aetherLF;
    aether::AetherCustomKnob industrialKnobLF;
    
    // UX Components
    juce::TooltipWindow tooltipWindow; // Separate init if needed

    // --- MAIN ENGINE CONTROLS ---
    juce::Slider driveSlider;
    aether::AetherReactorTank stagesReactor; 
    juce::Label driveLabel, stagesLabel;
    
    // --- BIPOLAR SIDEBARS ---
    aether::AetherAlgorithmSelector posSelector, negSelector;
    
    // --- NOISE ENGINE ---
    juce::Slider noiseLevelSlider, noiseWidthSlider;
    juce::ComboBox noiseTypeSelector;
    juce::Label noiseLevelLabel, noiseWidthLabel;
    
    // --- CENTRAL STAGE ---
    aether::AetherTransferVisualizer transferVis;
    aether::AetherOrb orb; // NEW CENTRAL CORE
    aether::AetherLogo logo; // NEW BRANDING
    aether::AetherSpectrum osc; // Output Spectrum
    
    // --- PRESETS & HELP ---
    juce::ComboBox presetSelector;
    juce::TextButton helpButton { "?" };
    juce::Label presetLabel;

    // --- FILTER MODULE ---
    juce::Slider cutoffSlider, resSlider, morphSlider;
    juce::Label cutoffLabel, resLabel, morphLabel;
    juce::TextButton filterModeBtn { "MORPH" }; // Toggles Morph/Vowel

    // --- FEEDBACK / RESONATOR ---
    juce::Slider fbAmountSlider, fbTimeSlider;
    juce::Label fbAmountLabel, fbTimeLabel;

    // --- EXPERIMENTAL ---
    juce::Slider foldSlider, spaceSlider;
    juce::Label foldLabel, spaceLabel;
    
    // --- GLOBAL ---
    juce::Slider outputSlider, mixSlider, subSlider, squeezeSlider; 
    juce::Slider widthSlider, xoverSlider; // DnB Essentials
    juce::Label outputLabel, mixLabel, subLabel, squeezeLabel;
    juce::Label widthLabel, xoverLabel;

    // --- ATTACHMENTS ---
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    std::unique_ptr<Attachment> driveAtt, cutoffAtt, resAtt, morphAtt;
    std::unique_ptr<ButtonAttachment> filterModeAtt;
    std::unique_ptr<Attachment> fbAmountAtt, fbTimeAtt, outputAtt, mixAtt, subAtt, squeezeAtt;
    std::unique_ptr<Attachment> widthAtt, xoverAtt;
    std::unique_ptr<Attachment> foldAtt, spaceAtt;
    std::unique_ptr<Attachment> noiseLevelAtt, noiseWidthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> noiseTypeAtt;

    juce::String currentStatusText = "SYS.OP.ACTIVE // AETHER.KERNEL.V5";
    bool tooltipsEnabled = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhatRackAudioProcessorEditor)
};
