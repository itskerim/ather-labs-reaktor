#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "AetherCommon.h"
#include "AetherDistortion.h"

namespace aether
{

/**
 * AetherTransferVisualizer: Visualizes the Bipolar Distortion Curve
 */
class AetherTransferVisualizer : public juce::Component
{
public:
    AetherTransferVisualizer() = default;

    void setParams(DistortionAlgo pos, DistortionAlgo neg, float drive, int stages)
    {
        algoPos = pos;
        algoNeg = neg;
        currentDrive = drive;
        currentStages = stages;
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(12.0f);
        
        // 1. Deep OLED Background
        g.setColour(juce::Colour(0xff09090b));
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // 2. Subtle Border
        g.setColour(juce::Colour(0xff27272a));
        g.drawRoundedRectangle(bounds, 8.0f, 1.0f);

        // 3. Ultra-Muted Grid
        g.setColour(juce::Colours::white.withAlpha(0.02f));
        g.drawHorizontalLine(getHeight() / 2, bounds.getX(), bounds.getRight());
        g.drawVerticalLine(getWidth() / 2, bounds.getY(), bounds.getBottom());

        // 4. Harmonic Curve Rendering
        juce::Path path;
        bool first = true;
        AetherDistortion<float> dummy;
        
        float w = bounds.getWidth();
        float h = bounds.getHeight();
        float centerX = bounds.getCentreX();
        float centerY = bounds.getCentreY();
        
        for (float x = -1.0f; x <= 1.0f; x += 0.01f) // Higher resolution
        {
            float y = dummy.processSample(x, currentDrive, algoPos, algoNeg, currentStages);
            float plotX = centerX + (x * w * 0.48f);
            float plotY = centerY - (y * h * 0.48f);
            
            if (first) { path.startNewSubPath(plotX, plotY); first = false; }
            else { path.lineTo(plotX, plotY); }
        }

        // Sky-400 Neon Glow
        g.setColour(juce::Colour(0xff38bdf8).withAlpha(0.15f));
        g.strokePath(path, juce::PathStrokeType(3.5f));
        
        g.setColour(juce::Colour(0xff38bdf8));
        g.strokePath(path, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved));
    }

private:
    DistortionAlgo algoPos = DistortionAlgo::SoftClip;
    DistortionAlgo algoNeg = DistortionAlgo::SoftClip;
    float currentDrive = 0.5f;
    int currentStages = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AetherTransferVisualizer)
};

} // namespace aether
