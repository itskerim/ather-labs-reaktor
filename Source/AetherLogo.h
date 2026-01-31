/*
  ==============================================================================

    AetherLogo.h
    Created: 31 Jan 2026
    Author:  Antigravity

    Description:
    A futuristic branding component featuring a glowy "Æ" orb
    and "ÆTHER LABS" text.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace aether
{

    struct LogoParticle
    {
        float x, y, z;
        float baseSize;
        float phaseOffset;
        float speed;
        float brightness;
        float px, py, pz; // Render temp
    };

    class AetherLogo : public juce::Component
    {
    public:
        AetherLogo()
        {
            // Initialize Point Cloud for sparkly branding (Increased to 80 points)
            for (int i = 0; i < 80; ++i)
            {
                LogoParticle p;
                float theta = i * 2.3999632f;
                float y = 1 - (i / (float)(80 - 1)) * 2;
                float radius = std::sqrt(1 - y * y);
                p.x = std::cos(theta) * radius;
                p.y = y;
                p.z = std::sin(theta) * radius;
                p.baseSize = juce::Random::getSystemRandom().nextFloat() * 1.0f + 0.6f;
                p.phaseOffset = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
                p.speed = juce::Random::getSystemRandom().nextFloat() * 0.4f + 0.2f;
                p.brightness = juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f;
                particles.push_back(p);
            }
        }

        void setMorph(float m) { morphValue = m; }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            float startX = 20.0f; // Left padding in the component
            float centerY = bounds.getCentreY() - 1.5f; // Vertical alignment
            float orbRadius = bounds.getHeight() * 0.20f; // Increased from 0.15f
            
            juce::Colour c1 = juce::Colour(0xff00d4ff); // Cyan
            juce::Colour c2 = juce::Colour(0xffbc13fe); // Purple
            juce::Colour baseCol = c1.interpolatedWith(c2, morphValue);
            
            // --- 1. Particles (Twinkling & Sparkly) ---
            for (auto& p : particles)
            {
                float rotPhase = rotation * p.speed + p.phaseOffset;
                float rotX = p.x * std::cos(rotPhase) - p.z * std::sin(rotPhase);
                float rotZ = p.x * std::sin(rotPhase) + p.z * std::cos(rotPhase);
                float rotY = p.y;
                
                float zScale = (rotZ + 2.0f) / 3.0f; 
                float screenX = startX + rotX * orbRadius;
                float screenY = centerY + rotY * orbRadius;
                
                // Twinkle effect based on phase
                float twinkle = 0.4f + 0.6f * std::sin(rotation * 2.0f + p.phaseOffset);
                float size = p.baseSize * zScale * 1.8f;
                float alpha = std::min(1.0f, zScale * zScale * p.brightness * 2.5f * glowAlpha * twinkle);
                
                // Draw glow behind particle for sparkle effect
                g.setColour(baseCol.withAlpha(alpha * 0.4f));
                g.fillEllipse(screenX - size, screenY - size, size * 2.0f, size * 2.0f);
                
                // Core particle
                g.setColour(juce::Colours::white.interpolatedWith(baseCol, 0.4f).withAlpha(alpha));
                g.fillEllipse(screenX - size/2, screenY - size/2, size, size);
            }
            
            // --- 2. Futuristic Typography ---
            // Orb edge is at startX + orbRadius. We want exactly 6px gap.
            float textX = startX + orbRadius + 6.0f; 
            
            juce::Font mainFont(juce::FontOptions("Inter", 24.0f, juce::Font::bold));
            mainFont.setHorizontalScale(1.4f);
            g.setFont(mainFont);
            g.setColour(juce::Colours::white);
            g.drawText("AETHER", (int)textX, (int)(bounds.getCentreY() - 14), 200, 24, juce::Justification::left, false);
            
            juce::Font subFont(juce::FontOptions("Inter", 11.0f, juce::Font::plain));
            subFont.setHorizontalScale(1.6f);
            g.setFont(subFont);
            g.setColour(baseCol.withAlpha(0.9f));
            g.drawText("L A B S", (int)textX + 2, (int)(bounds.getCentreY() + 8), 200, 15, juce::Justification::left, false);
            
            // Minimal High-Tech Line
            g.setColour(juce::Colour(0xff3f3f46).withAlpha(0.6f));
            g.drawHorizontalLine((int)(bounds.getCentreY() + 24), textX, textX + 110);
            
            // Neon Anchor
            g.setColour(baseCol);
            g.fillRect(textX, (float)(bounds.getCentreY() + 23), 20.0f, 2.0f);
        }

        void advance()
        {
            rotation += 0.005f; // Slightly faster but still elegant
            phase += 0.04f;
            glowAlpha = 0.8f + 0.2f * std::sin(phase * 0.5f);
            repaint();
        }

    private:
        std::vector<LogoParticle> particles;
        float rotation = 0.0f;
        float phase = 0.0f;
        float glowAlpha = 1.0f;
        float morphValue = 0.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherLogo)
    };

} // namespace aether
