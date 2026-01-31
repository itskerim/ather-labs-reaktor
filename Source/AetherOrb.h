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
        void setWidth(float w) { widthValue = w; }       // Width Expansion
        void setDrive(float d) { driveValue = d; }       // Drive Expansion

        void mouseDown(const juce::MouseEvent& e) override
        {
            lastMousePos = e.getPosition();
        }
        
        void mouseDrag(const juce::MouseEvent& e) override
        {
            auto pos = e.getPosition();
            auto delta = pos - lastMousePos;
            lastMousePos = pos;
            
            // X Drag -> Yaw (Rotate around Y)
            viewYaw += delta.x * 0.01f;
            
            // Y Drag -> Pitch (Rotate around X)
            viewPitch += delta.y * 0.01f;
            
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            float w = (float)getWidth();
            float h = (float)getHeight();
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            
            // Auto-Scaling to prevent clipping
            float widthMult = 1.0f + widthValue * 0.8f;
            float expansionEst = 1.0f + driveValue * 0.5f; 
            
            // USER REQUEST: Middle Ground Size (0.40f)
            // Was 0.35 (Small) -> 0.45 (Huge). 0.40 is the sweet spot.
            float baseScale = std::min(w, h) * 0.40f;
            
            // Dynamic Zoom out: Account for BOTH Width and Drive stretching
            // We want the final "Wide Radius" to fit.
            float combinedMult = widthMult * expansionEst;
            
            // Tuned Divider: 0.72f
            // Ensures safety at edges without shrinking too early.
            float safeScale = baseScale / (std::max(1.0f, combinedMult * 0.72f)); 
            
            // Audio Reactivity & Drive
            targetExpansion = 1.0f + (driveValue * 0.3f) + (currentLevel * 0.5f);
            
            // Smooth expansion
            expansion += (targetExpansion - expansion) * 0.1f;
            pulse = (expansion - 1.0f); 

            // Base Color
            juce::Colour c1 = juce::Colour(0xff00d4ff); 
            juce::Colour c2 = juce::Colour(0xffbc13fe); 
            juce::Colour baseCol = c1.interpolatedWith(c2, morphValue);

            // Draw Core Glow
            juce::ColourGradient grad(baseCol.withAlpha(0.3f), cx, cy, 
                                      juce::Colours::transparentBlack, cx, cy - safeScale * 1.5f, true);
            g.setGradientFill(grad);
            g.fillEllipse(cx - safeScale * 1.2f, cy - safeScale * 1.2f, safeScale * 2.4f, safeScale * 2.4f);

            // Pre-calculate rotation matrices (simplistic)
            float cp = std::cos(viewPitch);
            float sp = std::sin(viewPitch);
            float cyaw = std::cos(viewYaw);
            float syaw = std::sin(viewYaw);

            for (size_t i = 0; i < particles.size(); ++i)
            {
                auto& p = particles[i];
                
                // 1. SIMULATION SWIRL (Internal Motion)
                // Existing logic: shape mod + dancing + basic swirl
                float shapeMod = std::sin(p.z * 5.0f + morphValue * 3.0f) * std::cos(p.y * 5.0f) * morphValue * 0.3f;
                float dance = 0.0f;
                if (currentLevel > 0.01f) {
                     dance = std::sin(frame * p.speed * 10.0f + p.phaseOffset) * currentLevel * 0.2f;
                }
                
                float mx = p.x + shapeMod * p.x + dance;
                float my = p.y + shapeMod * p.y + dance;
                float mz = p.z + shapeMod * p.z;
                
                // Internal Spin (The "Swirl")
                float rotPhase = rotation * p.speed;
                float swirledX = mx * std::cos(rotPhase) - mz * std::sin(rotPhase);
                float swirledZ = mx * std::sin(rotPhase) + mz * std::cos(rotPhase);
                float swirledY = my;
                
                // 2. VIEW ROTATION (Interactive Orbit)
                // First Pitch (Rotate around X)
                float y1 = swirledY * cp - swirledZ * sp;
                float z1 = swirledY * sp + swirledZ * cp;
                float x1 = swirledX;
                
                // Then Yaw (Rotate around Y)
                float x2 = x1 * cyaw - z1 * syaw;
                float z2 = x1 * syaw + z1 * cyaw;
                float y2 = y1;
                
                // 3. FINAL PROJECTION
                // Apply Width Stretch AFTER view rotation? 
                // No, width stretch is usually a shape property, so it should be before view rotation 
                // effectively making the object an elongated ellipsoid.
                // Re-ordering: Apply width stretch to swirledX BEFORE rotation.
                // NOTE: If we stretch AFTER rotation, it looks like a 2D distortion of the screen.
                // If we stretch BEFORE rotation, it looks like a pill shape spinning.
                // User said "widening", usually implies the stereo field or the shape. 
                // Let's stretch the OBEJCT.
                
                // Re-calculating with Stretch applied to OBJECT COORDINATES
                // swirledX *= (1.0f + widthValue * 0.8f); 
                // wait, if I stretch swirledX, that's just one axis. 
                // Let's do it right here:
                
                float stretchedX = swirledX * (1.0f + widthValue * 0.8f);
                
                // Now View Rotate the Stretched Object
                y1 = swirledY * cp - swirledZ * sp;
                z1 = swirledY * sp + swirledZ * cp;
                x1 = stretchedX; // Use stretched
                
                x2 = x1 * cyaw - z1 * syaw;
                z2 = x1 * syaw + z1 * cyaw;
                y2 = y1;

                float zScale = (z2 + 2.5f) / 3.5f; 
                float r = safeScale * expansion;
                
                float screenX = cx + x2 * r;
                float screenY = cy + y2 * r;
                
                p.px = screenX;
                p.py = screenY;
                p.pz = zScale;
                
                float size = p.baseSize * zScale * 3.0f + (pulse * 3.0f);
                float alpha = std::min(1.0f, zScale * zScale * p.brightness * 1.5f);
                
                g.setColour(baseCol.withAlpha(alpha));
                g.fillEllipse(screenX - size/2, screenY - size/2, size, size);
            }
            
            // Neural Lines
            g.setColour(baseCol.withAlpha(0.12f)); 
            
            for (size_t i = 0; i < particles.size(); i += 2) 
            {
                auto& p1 = particles[i];
                if (p1.pz < 0.45f) continue; 
                
                for (size_t j = i + 1; j < particles.size(); j += 4) 
                {
                    auto& p2 = particles[j];
                    if (p2.pz < 0.45f) continue; 
                    
                    float dx = p1.px - p2.px;
                    float dy = p1.py - p2.py;
                    float maxDist = (60.0f + widthValue * 20.0f) * expansion;
                    
                    if (dx*dx + dy*dy < maxDist * maxDist)
                    {
                        g.drawLine(p1.px, p1.py, p2.px, p2.py, 1.0f);
                    }
                }
            }
        }
        
        void advance()
        {
            rotation += 0.005f + (pulse * 0.05f); 
            frame += 0.05f; 
        }
        
    private:
        std::vector<OrbParticle> particles;
        float rotation = 0.0f;
        float frame = 0.0f;
        
        // View Rotation
        float viewYaw = 0.0f;
        float viewPitch = 0.0f;
        juce::Point<int> lastMousePos;
        
        float currentLevel = 0.0f; 
        float morphValue = 0.0f;   
        float widthValue = 0.0f; 
        float driveValue = 0.0f;   
        
        float expansion = 0.0f; 
        float targetExpansion = 0.0f;
        float pulse = 0.0f;
        
        juce::Colour baseCol;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherOrb)
    };
}
