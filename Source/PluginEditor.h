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
    aether::AetherReactorTank stagesReactor; 
    juce::Label stagesLabel;
    
    // --- BIPOLAR SIDEBARS ---
    aether::AetherAlgorithmSelector posSelector, negSelector;
    
    // --- NOISE ENGINE ---
    juce::Slider noiseLevelSlider, noiseWidthSlider;
    juce::ComboBox noiseTypeSelector;
    juce::Label noiseLevelLabel, noiseWidthLabel;
    
    // Custom Noise Loader
    juce::TextButton loadNoiseButton { "L" };

    // Custom Icon Button
    class HeadphoneButton : public juce::ToggleButton
    {
    public:
        HeadphoneButton() : juce::ToggleButton("SOLO") {}
        
        void paintButton(juce::Graphics& g, bool shouldDrawButtonAsMouseOver, bool shouldDrawButtonAsDown) override
        {
            auto bounds = getLocalBounds().toFloat();
            // Background
            if (getToggleState())
                g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.2f));
            else if (shouldDrawButtonAsMouseOver)
                g.setColour(juce::Colours::white.withAlpha(0.1f));
            else
                g.setColour(juce::Colours::transparentBlack);
            
            g.fillRoundedRectangle(bounds, 4.0f);
            
            // Icon Color
            juce::Colour iconCol = getToggleState() ? juce::Colour(0xff00d4ff) : juce::Colours::grey;
            if (shouldDrawButtonAsMouseOver && !getToggleState()) iconCol = juce::Colours::white;
            
            g.setColour(iconCol);
            
            // Draw Headphone Icon
            juce::Path p;
            float w = bounds.getWidth();
            float h = bounds.getHeight();
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            float r = juce::jmin(w, h) * 0.35f;
            
            // Headband (Arc)
            p.addArc(cx - r, cy - r * 0.8f, r * 2.0f, r * 2.0f, -0.8f, 0.8f + juce::MathConstants<float>::pi, true);
            
            // Ear Cups
            float cupW = r * 0.5f;
            float cupH = r * 0.8f;
            p.addRoundedRectangle(cx - r - cupW*0.5f, cy, cupW, cupH, 2.0f);
            p.addRoundedRectangle(cx + r - cupW*0.5f, cy, cupW, cupH, 2.0f);
            
            g.strokePath(p, juce::PathStrokeType(2.0f));
        }
    };

    HeadphoneButton noiseSoloBtn; // Replaces standard ToggleButton
    std::unique_ptr<juce::FileChooser> fileChooser;
    
    // --- CENTRAL STAGE ---
    aether::AetherTransferVisualizer transferVis;
    aether::AetherOrb orb; // NEW CENTRAL CORE
    aether::AetherLogo logo; // NEW BRANDING
    aether::AetherSpectrum osc; // Output Spectrum
    
    // --- PRESETS & HELP ---
    juce::ComboBox presetSelector;
    juce::TextButton helpButton { "?" };
    juce::Label presetLabel;

    // --- DISTORTION ---
    juce::Slider driveSlider, lowCutSlider, foldSlider;
    juce::Label driveLabel, lowCutLabel, foldLabel;

    // --- FILTER MODULE ---
    juce::Slider cutoffSlider, resSlider, morphSlider;
    juce::Label cutoffLabel, resLabel, morphLabel;
    juce::TextButton filterModeBtn { "MORPH" }; // Toggles Morph/Vowel

    // --- FEEDBACK / RESONATOR ---
    juce::Slider fbAmountSlider, fbTimeSlider;
    juce::Label fbAmountLabel, fbTimeLabel;

    // --- EXPERIMENTAL ---
    juce::Slider spaceSlider;
    juce::Label spaceLabel;
    
    // --- GLOBAL ---
    juce::Slider outputSlider, mixSlider, subSlider, widthSlider;
    juce::Slider squeezeSlider, xoverSlider; 
    juce::Label outputLabel, mixLabel, subLabel, widthLabel;
    juce::Label squeezeLabel, xoverLabel;

    // --- ATTACHMENTS ---
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    std::unique_ptr<Attachment> driveAtt, lowCutAtt, cutoffAtt, resAtt, morphAtt;
    std::unique_ptr<ButtonAttachment> filterModeAtt;
    std::unique_ptr<Attachment> fbAmountAtt, fbTimeAtt, outputAtt, mixAtt, widthAtt, subAtt;
    std::unique_ptr<Attachment> squeezeAtt, xoverAtt;
    std::unique_ptr<Attachment> foldAtt, spaceAtt;
    std::unique_ptr<Attachment> noiseLevelAtt, noiseWidthAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> noiseTypeAtt;
    std::unique_ptr<ButtonAttachment> noiseSoloAtt;



    bool tooltipsEnabled = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhatRackAudioProcessorEditor)
};
