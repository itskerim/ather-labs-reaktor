#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AetherCommon.h"

namespace aether
{

/**
 * AetherAlgorithmSelector: Combo + Mini-Visualizer panel
 */
class AetherAlgorithmSelector : public juce::Component
{
public:
    AetherAlgorithmSelector(const juce::String& title, juce::AudioProcessorValueTreeState& vts, const juce::String& paramID)
        : apvts(vts), pid(paramID)
    {
        titleLabel.setText(title, juce::dontSendNotification);
        titleLabel.setJustificationType(juce::Justification::centred);
        titleLabel.setFont(juce::FontOptions(14.0f).withStyle("Bold"));
        addAndMakeVisible(titleLabel);

        juce::StringArray algos;
        algos.add("None"); algos.add("SoftClip"); algos.add("HardClip"); algos.add("SineFold");
        algos.add("TriangleWarp"); algos.add("BitCrush"); algos.add("Rectify"); algos.add("Tanh");
        algos.add("SoftFold"); algos.add("Chebyshev");

        algoCombo.addItemList(algos, 1);
        addAndMakeVisible(algoCombo);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, pid, algoCombo);
    }

    void setTitleVisible(bool visible)
    {
        titleLabel.setVisible(visible);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        if (titleLabel.isVisible())
        {
            titleLabel.setBounds(area.removeFromTop(20));
            area.removeFromTop(5);
        }
        algoCombo.setBounds(area.removeFromTop(24));
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().reduced(2.0f);
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.drawRoundedRectangle(area.toFloat(), 5.0f, 1.0f);
    }

private:
    juce::Label titleLabel;
    juce::ComboBox algoCombo;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String pid;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AetherAlgorithmSelector)
};

} // namespace aether
