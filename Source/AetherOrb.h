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
    private:
        // --- Member Variables (Moved to top for safety) ---
        std::vector<OrbParticle> particles;
        std::vector<OrbParticle> subParticles; // Secondary particle system for the "Sub Core"
        float rotation = 0.0f;
        float frame = 0.0f;
        
        // View Rotation
        float viewYaw = 0.0f;
        float viewPitch = 0.0f;
        float velYaw = 0.0f; 
        float velPitch = 0.0f;
        juce::Point<int> lastMousePos;
        
        // Parameters
        float currentLevel = 0.0f; 
        float morphValue = 0.0f;   
        float widthValue = 0.0f; 
        float driveValue = 0.0f;   
        
        float subValue = 0.0f;
        float squeezeValue = 0.0f;
        float gainValue = 1.0f;
        float mixValue = 1.0f;
        float xoverValue = 150.0f;
        
        float noiseLevel = 0.0f;
        float noiseDistort = 0.0f;
        
        float cutoffHz = 20000.0f;
        float resonance = 0.0f;
        
        float fbAmt = 0.0f;
        float fbTime = 0.0f;
        float fbSpace = 0.0f;
        
        float expansion = 0.0f; 
        float targetExpansion = 0.0f;
        float pulse = 0.0f;
        
        juce::Colour baseCol;

    public:
        AetherOrb()
        {
            // Initialize Main Point Cloud (Use System Random for better seed)
            juce::Random rng = juce::Random::getSystemRandom();

            for (int i = 0; i < 400; ++i)
            {
                OrbParticle p;
                float theta = i * 2.3999632f; 
                float y = 1 - (i / (float)(400 - 1)) * 2; 
                float radius = std::sqrt(1 - y * y);
                
                p.x = std::cos(theta) * radius;
                p.y = y;
                p.z = std::sin(theta) * radius;
                
                p.baseSize = rng.nextFloat() * 1.5f + 0.5f;
                p.phaseOffset = rng.nextFloat() * juce::MathConstants<float>::twoPi;
                p.speed = rng.nextFloat() * 0.5f + 0.5f;
                p.brightness = rng.nextFloat() * 0.5f + 0.5f;
                
                particles.push_back(p);
            }

            // Initialize Sub Core Particles (Dense, small cluster)
            for (int i = 0; i < 80; ++i)
            {
                OrbParticle p;
                float theta = i * 2.3999632f;
                float y = 1 - (i / (float)(80 - 1)) * 2;
                float radius = std::sqrt(1 - y * y);
                
                // Radius is much tighter (0.15 instead of 1.0)
                float rScale = 0.15f; 
                p.x = std::cos(theta) * radius * rScale;
                p.y = y * rScale;
                p.z = std::sin(theta) * radius * rScale;
                
                p.baseSize = rng.nextFloat() * 2.0f + 1.0f; // Slightly chunkier points
                p.phaseOffset = rng.nextFloat() * juce::MathConstants<float>::twoPi;
                p.speed = rng.nextFloat() * 0.8f + 0.8f; 
                p.brightness = 1.0f; 
                
                subParticles.push_back(p);
            }
            
            // Interaction Cursor - Start with PointingHand (Open)
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }

        // --- Visual Parameter Setters ---
        void setLevel(float lvl) { currentLevel = lvl; } 
        void setMorph(float m) { morphValue = m; }       
        
        // DNB / Spatial
        void setWidth(float w) { widthValue = w; }       
        
        void setSqueeze(float s) { squeezeValue = s; }
        
        // Gain: Decibels to Linear, but kept as a "scale factor"
        void setGain(float db) { gainValue = std::pow(10.0f, db / 20.0f); }
        
        void setMix(float m) { mixValue = m; }
        
        void setSub(float s) { subValue = s; }
        
        void setXOver(float x) { xoverValue = x; } // New XOver setter

        // Distortion / Noise
        // Drive controls expansion
        void setDrive(float d) { driveValue = d; }       
        
        void setNoise(float n, float distort) { noiseLevel = n; noiseDistort = distort; }
        
        // Filter
        void setFilter(float cutoff, float res) { cutoffHz = cutoff; resonance = res; }
        
        // Feedback
        void setFeedback(float amt, float time, float space) { fbAmt = amt; fbTime = time; fbSpace = space; }

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
            // Sensitivity tuned for 1:1 feel (300px ~ 180deg)
            float sens = 0.015f; 
            
            // Direct Manipulation (Drag Right -> Move Right)
            float dYaw = -delta.x * sens;
            float dPitch = -delta.y * sens;
            
            viewYaw += dYaw;
            viewPitch += dPitch;
            
            // Capture velocity for momentum
            velYaw = dYaw;
            velPitch = dPitch;
            
            repaint();
        }
        
        void mouseUp(const juce::MouseEvent&) override
        {
            // Momentum decay takes over in advance()
        }

        void paint(juce::Graphics& g) override
        {
            float w = (float)getWidth();
            float h = (float)getHeight();
            float cx = w * 0.5f;
            float cy = h * 0.5f;
            
            // 1. MIX: Visually Desaturate and Fade based on Mix
            // If Mix is 0, we look transparent and greyish.
            float globalAlpha = 0.2f + (mixValue * 0.8f); 
            g.setOpacity(globalAlpha);

            // 2. GAIN: Directly affects global scale
            // Clamp to avoid overflow. Max 1.4x instead of 2.5x
            float gainScale = std::clamp(gainValue, 0.3f, 1.4f);
            
            // Base visual scale
            float widthMult = 1.0f + widthValue * 0.8f;
            float expansionEst = 1.0f + driveValue * 0.5f; 
            // Multiplier reduced to 0.19f (from 0.35f) because we are now FULL SCREEN (1000x700).
            // This maintains the original visual size but allows infinite expansion without clipping.
            float baseScale = std::min(w, h) * 0.19f * gainScale; 
            
            float safeScale = baseScale / (std::max(1.0f, widthMult * 0.7f));
            
            // Audio Reactivity
            // Sub adds MASSIVE pulse
            float pulseIntensity = 0.5f + subValue * 0.5f; 
            targetExpansion = 1.0f + (driveValue * 0.3f) + (currentLevel * pulseIntensity);
            
            // Smooth expansion
            expansion += (targetExpansion - expansion) * 0.1f;
            pulse = (expansion - 1.0f); 

            // Base Color Calculation (Filter & Morph)
            // Cutoff decides Hue: Low (Red/Dark) -> High (Cyan/Bright)
            float normCutoff = (std::log(cutoffHz) - std::log(80.0f)) / (std::log(20000.0f) - std::log(80.0f));
            normCutoff = std::clamp(normCutoff, 0.0f, 1.0f);
            
            // "Warm" Palette (Morph 0): Brand Cyan (0.5) to Blue (0.6)
            // Default load (Cutoff 20k -> norm 1.0) should be CYAN.
            // Let's map normCutoff (0..1) to Hue range.
            // If Norm=1 (High Freq), we want Cyan (0.5).
            // If Norm=0 (Low Freq), maybe darker Blue/Purple?
            // Actually, let's just stick to the requested "Brand Cyan".
            // Hue 0.5 is Cyan. 
            juce::Colour warm = juce::Colour::fromHSV(0.5f + (1.0f - normCutoff) * 0.05f, 0.85f, 0.9f + resonance*0.1f, 1.0f); 
            
            // "Cool" Palette (Morph Max): Violet -> Purple
            juce::Colour cool = juce::Colour::fromHSV(0.78f + normCutoff * 0.1f, 0.85f, 0.9f, 1.0f); 
            
            juce::Colour baseCol = warm.interpolatedWith(cool, morphValue);
            
            // Mix Desaturation
            if (mixValue < 0.95f)
                baseCol = baseCol.withSaturation(mixValue);

            // Draw Core Glow
            juce::ColourGradient grad(baseCol.withAlpha(0.3f), cx, cy, 
                                      juce::Colours::transparentBlack, cx, cy - safeScale * 1.5f, true);
            g.setGradientFill(grad);
            g.fillEllipse(cx - safeScale * 1.2f, cy - safeScale * 1.2f, safeScale * 2.4f, safeScale * 2.4f);
            
            // Feedback Halo (Outer Ring)
            if (fbAmt > 0.01f)
            {
                // Halo expands with Space and Pulse
                float haloSize = safeScale * (2.2f + fbSpace * 1.0f) + (pulse * 30.0f);
                float haloAlpha = fbAmt * 0.4f * mixValue; // Controlled by Mix
                g.setColour(baseCol.withAlpha(haloAlpha));
                g.drawEllipse(cx - haloSize/2, cy - haloSize/2, haloSize, haloSize, 2.0f + fbAmt * 10.0f);
            }
            
            // Pre-calculate rotation matrices (Moved UP for Sub Beam)
            float cp = std::cos(viewPitch);
            float sp = std::sin(viewPitch);
            float cyaw = std::cos(viewYaw);
            float syaw = std::sin(viewYaw);

            // SUB CORE: Tiny White Bulb (2% of body)
            if (subValue > 0.1f)
            {
                // "2% of the whole body"
                // Body is roughly safeScale * 2. So 2% of that is safeScale * 0.04.
                float bulbSz = safeScale * 0.05f * subValue; // Small subtle bulb
                
                // 1. Inner white hot core
                g.setColour(juce::Colours::white.withAlpha(0.9f * subValue));
                g.fillEllipse(cx - bulbSz/2, cy - bulbSz/2, bulbSz, bulbSz);
                
                // 2. Tiny soft outer glow (just a bit larger)
                float glowSz = bulbSz * 2.5f;
                juce::Colour glowCol = baseCol.withAlpha(0.5f * subValue);
                juce::ColourGradient glowGrad(glowCol, cx, cy, 
                                              juce::Colours::transparentBlack, cx, cy - glowSz, true);
                g.setGradientFill(glowGrad);
                g.fillEllipse(cx - glowSz/2, cy - glowSz/2, glowSz, glowSz);
            }

            juce::Random& rng = juce::Random::getSystemRandom();
            
            for (size_t i = 0; i < particles.size(); ++i)
            {
                auto& p = particles[i];
                
                // 1. SIMULATION SWIRL
                float shapeMod = std::sin(p.z * 5.0f + morphValue * 3.0f) * std::cos(p.y * 5.0f) * morphValue * 0.3f;
                float dance = 0.0f;
                if (currentLevel > 0.01f) {
                     dance = std::sin(frame * p.speed * 10.0f + p.phaseOffset) * currentLevel * 0.2f;
                }
                
                float mx = p.x + shapeMod * p.x + dance;
                float my = p.y + shapeMod * p.y + dance;
                float mz = p.z + shapeMod * p.z;
                
                // NOISE JITTER (Shake)
                if (noiseLevel > 0.0f || noiseDistort > 0.0f)
                {
                    float jitterAmt = (noiseLevel * 0.15f) + (noiseDistort * 0.08f);
                    mx += (rng.nextFloat() - 0.5f) * jitterAmt;
                    my += (rng.nextFloat() - 0.5f) * jitterAmt;
                    mz += (rng.nextFloat() - 0.5f) * jitterAmt;
                }
                
                // Internal Spin (The "Swirl")
                float rotPhase = rotation * p.speed;
                float swirledX = mx * std::cos(rotPhase) - mz * std::sin(rotPhase);
                float swirledZ = mx * std::sin(rotPhase) + mz * std::cos(rotPhase);
                float swirledY = my;
                
                // X-OVER (X Power) -> Vertical Splits / Dispersion
                // User requested removal of shape distortion: "dont need to do anything with x power"
                // Disabling vertical stretch/squash from X-Over to ensure perfect sphere.
                // float normXOver = (xoverValue - 60.0f) / 240.0f; 
                // swirledY *= (1.0f + normXOver * 0.5f * std::abs(swirledY));

                // SQUEEZE: Flatten vertically (Pancake)
                // Range 0-1.
                // Aggressive: Flatten by up to 98% (almost flat)
                float squeezeMult = 1.0f - (squeezeValue * 0.98f); 
                swirledY *= squeezeMult;
                
                // WIDTH: Horizontal Stretch
                float widthMult = 1.0f + widthValue * 1.5f; 
                float stretchedX = swirledX * widthMult;
                
                // 3. VIEW ROTATION
                float y1 = swirledY * cp - swirledZ * sp;
                float z1 = swirledY * sp + swirledZ * cp;
                float x1 = stretchedX; 
                
                float x2 = x1 * cyaw - z1 * syaw;
                float z2 = x1 * syaw + z1 * cyaw;
                float y2 = y1;

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
            // FEEDBACK TIME -> Rotation Speed
            // Constant slow drift (Replaced unstable reactive speed)
            float baseSpeed = 0.002f; 
            
            // Subtle modulation from Pulse only
            rotation += baseSpeed + (pulse * 0.01f); 
            frame += 0.05f; 
            
            if (!isMouseButtonDown())
            {
                viewYaw += velYaw;
                viewPitch += velPitch;
                velYaw *= 0.95f;
                velPitch *= 0.95f;
            }
        }
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AetherOrb)
    };
}
