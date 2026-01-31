/*
  ==============================================================================

    AetherOrb.h
    Created: 31 Jan 2026
    Author:  Antigravity

    Description:
    A contained 3D "Plasma Orb" visualization.
    Simulates a 3D point cloud sphere (Three.js particle style) that pulses
    to the audio. Replaces the chaotic full-screen reactor.

  ==============================================================================
*/

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <cmath>

namespace aether
{

    struct OrbParticle
    {
        float x, y, z;      // 3D Position (-1 to 1)
        float baseSize;     // Base radius
        float phaseOffset;  // Animation phase
        float speed;        // Rotation speed variation
        float brightness;   // 0.0 to 1.0
        
        // Temp Render Data
        float px, py, pz;
    };

    class AetherOrb : public juce::Component
    {
    public:
        AetherOrb()
        {
            // Initialize Point Cloud
            for (int i = 0; i < 400; ++i)
            {
                OrbParticle p;
                
                // Sphere Distribution (Golden Angle)
                float theta = i * 2.3999632f; // Golden angle in radians
                float y = 1 - (i / (float)(400 - 1)) * 2; // -1 to 1
                float radius = std::sqrt(1 - y * y);
                
                p.x = std::cos(theta) * radius;
                p.y = y;
                p.z = std::sin(theta) * radius;
                
                p.baseSize = juce::Random::getSystemRandom().nextFloat() * 1.5f + 0.5f;
                p.phaseOffset = juce::Random::getSystemRandom().nextFloat() * juce::MathConstants<float>::twoPi;
                p.speed = juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f;
                p.brightness = juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f;
                
                particles.push_back(p);
            }
        }

        void setLevel(float lvl) { currentLevel = lvl; } // Audio Reactivity
        void setMorph(float m) { morphValue = m; }       // Color/Shape shift

        void paint(juce::Graphics& g) override
        {
            float w = (float)getWidth();
            float h = (float)getHeight();
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            float scale = std::min(w, h) * 0.35f; // Orb Radius
            
            // Audio Reactivity
            targetExpansion = 1.0f + (currentLevel * 0.3f);
            
            // Smooth expansion
            expansion += (targetExpansion - expansion) * 0.1f;
            pulse = (expansion - 1.0f) * 2.0f; // Derived pulse for effects
            
            // Rotation moved to advance()
            
            // Base Color (Cyan -> Purple Morph)
            
            // Base Color (Cyan -> Purple Morph)
            juce::Colour c1 = juce::Colour(0xff00d4ff); // Cyan
            juce::Colour c2 = juce::Colour(0xffbc13fe); // Purple
            juce::Colour baseCol = c1.interpolatedWith(c2, morphValue);

            // Draw Core Glow
            juce::ColourGradient grad(baseCol.withAlpha(0.3f), cx, cy, 
                                      juce::Colours::transparentBlack, cx, cy - scale * 1.5f, true);
            g.setGradientFill(grad);
            g.fillEllipse(cx - scale * 1.2f, cy - scale * 1.2f, scale * 2.4f, scale * 2.4f);

            // Render Particles & Neural Connections
            // Reduce count for performance if needed, but 400 is fine for modern CPUs.
            // Let's use a subset for connections to avoid clutter.
            
            for (size_t i = 0; i < particles.size(); ++i)
            {
                auto& p = particles[i];
                
                // 1. Rotate & Project
                float rotX = p.x * std::cos(rotation * p.speed) - p.z * std::sin(rotation * p.speed);
                float rotZ = p.x * std::sin(rotation * p.speed) + p.z * std::cos(rotation * p.speed);
                float rotY = p.y;
                
                float zScale = (rotZ + 2.0f) / 3.0f; 
                float r = scale * expansion;
                
                float screenX = cx + rotX * r;
                float screenY = cy + rotY * r;
                
                // Store projected coordinates for line drawing
                p.px = screenX;
                p.py = screenY;
                p.pz = zScale; // Use as alpha/depth
                
                // Size attenuation
                float size = p.baseSize * zScale * 3.0f + (pulse * 2.0f);
                float alpha = zScale * zScale * p.brightness; 
                
                // Draw Node
                g.setColour(baseCol.withAlpha(alpha));
                g.fillEllipse(screenX - size/2, screenY - size/2, size, size);
            }
            
            // Neural Lines (Plexus)
            // Connect nearby nodes
            g.setColour(baseCol.withAlpha(0.15f)); // Faint lines
            
            for (size_t i = 0; i < particles.size(); i += 2) // Skip every other for performance/style
            {
                auto& p1 = particles[i];
                if (p1.pz < 0.4f) continue; // Don't draw lines for back-facing particles
                
                for (size_t j = i + 1; j < particles.size(); j += 4) // Check sparse neighbors
                {
                    auto& p2 = particles[j];
                    if (p2.pz < 0.4f) continue; 
                    
                    // Screen Space Distance
                    float dx = p1.px - p2.px;
                    float dy = p1.py - p2.py;
                    float distSq = dx*dx + dy*dy;
                    
                    float maxDist = 60.0f * expansion; // Connection threshold
                    
                    if (distSq < maxDist * maxDist)
                    {
                        float dist = std::sqrt(distSq);
                        float lineAlpha = (1.0f - (dist / maxDist)) * p1.pz * p2.pz * 0.4f;
                        
                        g.setColour(baseCol.withAlpha(lineAlpha));
                        g.drawLine(p1.px, p1.py, p2.px, p2.py, 1.0f);
                    }
                }
            }
        }
        
        void advance()
        {
            // Increment rotation
            rotation += 0.005f + (pulse * 0.02f); // Spin faster with drive
            // rotation -= twoPi line removed to prevent phase jumps with random speeds
            
            // Pulse decay
            // We set 'pulse' from setLevel, but we can also animate a heartbeat?
            // Actually setLevel is called from timer, so it acts as the envelope follower.
            // We just need rotation.
        }
        
    private:
        std::vector<OrbParticle> particles;
        float rotation = 0.0f;
        float currentLevel = 0.0f; // 0..1
        float morphValue = 0.0f;   // 0..1
        
        // Animation State
        float expansion = 0.0f; 
        float targetExpansion = 0.0f;
        float pulse = 0.0f;
        
        juce::Colour baseCol;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherOrb)
    };
}
