/*
  ==============================================================================

    AetherReactorTank.h
    Created: 31 Jan 2026
    Author:  Antigravity

    Description:
    A custom segment-based slider that visualizes distortion "STAGES"
    as a filling reactor tank with neon glow.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

class AetherReactorTank : public juce::Component, public juce::SettableTooltipClient
{
public:
    AetherReactorTank()
    {
        setTooltip("Distortion Stages: Controls how many times the signal is processed. More stages = deeper saturation.");
    }

    void setValue(int newValue)
    {
        if (currentValue != newValue)
        {
            currentValue = std::clamp(newValue, 1, 12);
            repaint();
        }
    }

    void setMorph(float m) { morphValue = m; repaint(); }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        float segmentGap = 3.0f;
        float totalGaps = segmentGap * 11.0f;
        float segmentHeight = (bounds.getHeight() - totalGaps) / 12.0f;
        
        juce::Colour c1 = juce::Colour(0xff00d4ff); // Cyan
        juce::Colour c2 = juce::Colour(0xffbc13fe); // Purple
        juce::Colour baseCol = c1.interpolatedWith(c2, morphValue);

        // Background Rail
        g.setColour(juce::Colour(0xff18181b));
        g.fillRoundedRectangle(bounds, 4.0f);
        
        for (int i = 0; i < 12; ++i)
        {
            // Draw from bottom up
            float y = bounds.getBottom() - (i + 1) * segmentHeight - i * segmentGap;
            juce::Rectangle<float> segRect(bounds.getX() + 4.0f, y, bounds.getWidth() - 8.0f, segmentHeight);
            
            bool isActive = (i < currentValue);
            
            if (isActive)
            {
                // Active segment with Glow
                float intensity = (float)(i + 1) / 12.0f;
                juce::Colour segCol = baseCol.withMultipliedSaturation(0.8f).withMultipliedBrightness(1.2f);
                
                // Outer Glow
                g.setColour(segCol.withAlpha(0.3f));
                g.fillRoundedRectangle(segRect.expanded(2.0f), 2.0f);
                
                // Core
                g.setColour(segCol);
                g.fillRoundedRectangle(segRect, 1.5f);
                
                // Tech highlight
                g.setColour(juce::Colours::white.withAlpha(0.4f));
                g.fillRect(segRect.getX(), segRect.getY(), segRect.getWidth(), 1.0f);
            }
            else
            {
                // Inactive segment (ghostly)
                g.setColour(juce::Colour(0xff3f3f46).withAlpha(0.3f));
                g.fillRoundedRectangle(segRect, 1.5f);
            }
        }

        // Tech Label Frame
        g.setColour(juce::Colour(0xff3f3f46).withAlpha(0.5f));
        g.drawRoundedRectangle(bounds.expanded(1.0f), 4.0f, 1.0f);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        updateValueFromMouse(e);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        updateValueFromMouse(e);
    }

    std::function<void(int)> onValueChanged;

private:
    void updateValueFromMouse(const juce::MouseEvent& e)
    {
        auto bounds = getLocalBounds().toFloat();
        float hitY = 1.0f - (e.position.y / bounds.getHeight());
        int newValue = std::clamp((int)std::ceil(hitY * 12.0f), 1, 12);
        
        if (newValue != currentValue)
        {
            currentValue = newValue;
            if (onValueChanged) onValueChanged(currentValue);
            repaint();
        }
    }

    int currentValue = 1;
    float morphValue = 0.0f;
};

} // namespace aether
