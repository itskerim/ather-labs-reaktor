#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "AetherDistortion.h" // For DistortionAlgo enum
#include "AetherTransferVisualizer.h"

namespace aether
{

// --- SIMPLE OSCILLOSCOPE ---
// --- CYBER SPECTRUM ANALYZER ---
class AetherSpectrum : public juce::Component, public juce::Timer
{
public:
    AetherSpectrum() 
        : forwardFFT(fftOrder), 
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        startTimerHz(60); 
    }
    
    void pushBuffer(const juce::AudioBuffer<float>& buffer)
    {
        if (buffer.getNumSamples() > 0)
        {
            auto* ch = buffer.getReadPointer(0);
            for (int i = 0; i < buffer.getNumSamples(); ++i)
            {
                if (fifoIndex == fftSize)
                {
                    if (!nextFFTBlockReady)
                    {
                        std::fill(fftData.begin(), fftData.end(), 0.0f);
                        std::copy(fifo.begin(), fifo.end(), fftData.begin());
                        nextFFTBlockReady = true;
                    }
                    fifoIndex = 0;
                }
                fifo[fifoIndex++] = ch[i];
            }
        }
    }
    
    void setMorph(float m) { morphValue = m; }
    void setChaos(float c) { chaosValue = c; }
    void setIntensity(float i) { intensity = i; }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float w = (float)getWidth();
        float h = (float)getHeight();

        // 1. Cyber Glass Board
        g.setColour(juce::Colour(0xff09090b).withAlpha(0.7f));
        g.fillRoundedRectangle(bounds, 8.0f);
        
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        int numLines = 12;
        for(int i=0; i<numLines; ++i) 
            g.drawVerticalLine(w * (i/(float)numLines), 0, h);

        // --- Reactive Color Logic ---
        juce::Colour c1 = juce::Colour(0xff00d4ff); // Cyan
        juce::Colour c2 = juce::Colour(0xffbc13fe); // Purple
        juce::Colour activeCol = c1.interpolatedWith(c2, morphValue);
        
        if (nextFFTBlockReady)
        {
            window.multiplyWithWindowingTable(fftData.data(), fftSize);
            forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
            nextFFTBlockReady = false;
            
            // Apply smoothing
            for(int i=0; i<scopeSize; ++i)
            {
                 float val = fftData[i];
                 float rise = 0.15f + intensity * 0.2f; // Slower, smoother default
                 scopeData[i] = scopeData[i] * (1.0f - rise) + val * rise;
            }
        }

        juce::Path p;
        p.startNewSubPath(0, h);
        
        float time = (float)juce::Time::getMillisecondCounter() * 0.01f;

        for (int i = 0; i < scopeSize; ++i)
        {
             float skews = (float)i / scopeSize; 
             
             // Magnitude to DB
             float mag = juce::Decibels::gainToDecibels(scopeData[i]) - juce::Decibels::gainToDecibels((float)fftSize);
             float normY = juce::jmap(mag, -100.0f, 0.0f, 0.0f, 0.9f); // Increased height scale (Max 90% of height)
             
             // --- CHAOS JITTER ---
             // Add harmonic "vibration" based on chaos param
             float jitter = 0.0f;
             if (chaosValue > 0.01f) // Lower threshold
             {
                 jitter = std::sin(time + i * 0.5f) * chaosValue * 2.0f * normY; // Reduce scale
             }

             float x = w * skews; 
             float y = h - (normY * h) + jitter;
             
             if (y > h) y = h; 
             if (y < 0) y = 0;
             
             p.lineTo(x, y);
        }
        p.lineTo(w, h);
        p.closeSubPath();
        
        // Fill (Reactive Gradient) - MORE VISIBLE
        g.setGradientFill(juce::ColourGradient(activeCol.withAlpha(0.3f), 0, h, 
             activeCol.withAlpha(0.0f), 0, h * 0.5f, false));
        g.fillPath(p);
        
        // Stroke (Laser Line with Glow) - BRIGHTER
        g.setColour(activeCol.withAlpha(0.4f));
        g.strokePath(p, juce::PathStrokeType(3.0f)); // Soft Glow
        g.setColour(activeCol.withAlpha(0.9f));      // Core Line (Almost Solid)
        g.strokePath(p, juce::PathStrokeType(2.0f)); // Thicker Line
    }
    
    void timerCallback() override { repaint(); }

private:
    static constexpr int fftOrder = 10;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int scopeSize = 256; 
    
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    std::array<float, scopeSize> scopeData;
    
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    
    float morphValue = 0.0f;
    float chaosValue = 0.0f;
    float intensity = 0.0f;
};

// --- MECHA-CORE REACTOR (The "Transformer") ---
class AetherReactor : public juce::Component
{
public:
    void setLevel(float l) { level = l; }
    void setChaos(float c) { chaos = c; }
    void setMorph(float m) { morph = m; }
    
    struct Vector3 { float x, y, z; };
    
    AetherReactor() 
    {
        // Pre-compute sphere points (Fibonacci Sphere for even distribution)
        const int numPoints = 1200; // Increased density for "Solid" look
        const float phi = 3.14159265359f * (3.0f - std::sqrt(5.0f)); 

        for (int i = 0; i < numPoints; ++i)
        {
            float y = 1.0f - (i / (float)(numPoints - 1)) * 2.0f; // y goes from 1 to -1
            float radius = std::sqrt(1.0f - y * y);
            float theta = phi * i;
            
            float x = std::cos(theta) * radius;
            float z = std::sin(theta) * radius;
            baseVertices.push_back({x, y, z});
        }
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto center = bounds.getCentre();
        float radius = std::min(bounds.getWidth(), bounds.getHeight()) / 2.5f;
        float time = (float)juce::Time::getMillisecondCounter() / 1000.0f;
        
        // Background - Deep Void (Vignette)
        juce::ColourGradient bgGrad(juce::Colour(0xff09090b), center.x, center.y,
                                    juce::Colour(0xff000000), center.x, center.y - radius*3.0f, true);
        g.setGradientFill(bgGrad);
        g.fillAll(); // Clear full background
        
        // --- 3D RENDER LOOP ---
        auto rotate = [&](Vector3 v, float rx, float ry) -> Vector3 {
             float x = v.x, y = v.y, z = v.z;
             float x1 = x * std::cos(ry) - z * std::sin(ry);
             float z1 = x * std::sin(ry) + z * std::cos(ry);
             x = x1; z = z1;
             float y1 = y * std::cos(rx) - z * std::sin(rx);
             float z2 = y * std::sin(rx) + z * std::cos(rx);
             y = y1; z = z2;
             return {x, y, z};
        };

        float spinSpeed = 0.2f + (level * 1.5f); 
        float rotX = time * (spinSpeed * 0.6f) + morph * 0.5f; 
        float rotY = time * spinSpeed + chaos;
        
        // Buffer
        std::vector<Vector3> currentPoints;
        currentPoints.reserve(baseVertices.size());
        
        float pulse = 1.0f + level * 0.5f * std::sin(time * 25.0f); 
        float chaosMod = chaos * 3.0f;

        // Base Color Logic
        juce::Colour baseCol = juce::Colour(0xff38bdf8); // Cyan
        if (morph > 0.0f) baseCol = baseCol.interpolatedWith(juce::Colour(0xff4ade80), morph * 0.5f); // Green
        if (morph > 0.5f) baseCol = baseCol.interpolatedWith(juce::Colour(0xfffacc15), (morph - 0.5f) * 2.0f); // Gold
        if (chaos > 0.5f) baseCol = baseCol.interpolatedWith(juce::Colour(0xffd946ef), (chaos - 0.5f) * 2.0f); // Magenta
        if (level > 0.6f) baseCol = baseCol.interpolatedWith(juce::Colours::white, (level - 0.6f) * 2.0f); // White Hot

        for (const auto& v : baseVertices)
        {
            // Vertex Shader: Spiky Noise
            float noiseFreq = 3.0f + morph * 6.0f + level * 12.0f;
            float n1 = std::sin(v.x * noiseFreq + time * 4.0f);
            float n2 = std::cos(v.y * (noiseFreq + 1.2f) + time * 3.5f);
            float n3 = std::sin(v.z * noiseFreq + chaos * 15.0f);
            
            float displacementAmt = 0.05f + chaosMod * 0.2f + level * 0.7f;
            float rMod = 1.0f + (n1+n2+n3) * displacementAmt;
            rMod *= (1.0f + level * 0.3f); // Expand
            
            Vector3 modV = {v.x * rMod, v.y * rMod, v.z * rMod};
            Vector3 rV = rotate(modV, rotX, rotY);
            currentPoints.push_back(rV);
        }
        
        // --- DRAWING BEAMS (Mesh Connections) ---
        // Brutal approach: Connect points that are close in screen space? Too slow.
        // Connect sequential points in the buffer? (Fibonacci spiral property puts close ID points somewhat near)
        // Let's connect i to i+1, i+2... 
        
        struct RenderItem { float x, y, z, size, alpha; juce::Colour col; bool isBeam; float px2, py2; };
        std::vector<RenderItem> renderList;
        renderList.reserve(currentPoints.size() * 2);

        for (size_t i = 0; i < currentPoints.size(); ++i)
        {
            const auto& p = currentPoints[i];
            float zFactor = 3.5f + p.z;
            float scale = radius * 3.5f / zFactor; 
            float px = center.x + p.x * scale;
            float py = center.y + p.y * scale;
            float depth = (p.z + 1.2f) * 0.5f; // 0..1
            
            float alpha = 0.1f + depth * 0.9f;
            if(alpha < 0) alpha = 0; if(alpha > 1) alpha = 1;
            float size = scale * (0.1f + level * 0.2f);
            
            renderList.push_back({px, py, p.z, size, alpha, baseCol, false, 0, 0});
            
            // Beams: Connect to a neighbor occasionally
            if (i % 7 == 0 && i + 5 < currentPoints.size()) // Every 7th point connects to neighbor
            {
               const auto& p2 = currentPoints[i + 5]; // Connecting spiral arms
               // Determine dist
               float distSq = (p.x-p2.x)*(p.x-p2.x) + (p.y-p2.y)*(p.y-p2.y) + (p.z-p2.z)*(p.z-p2.z);
               if (distSq < 0.5f) // Only connect if physically close
               {
                   float scale2 = radius * 3.5f / (3.5f + p2.z);
                   float px2 = center.x + p2.x * scale2;
                   float py2 = center.y + p2.y * scale2;
                   renderList.push_back({px, py, (p.z + p2.z)*0.5f, 1.0f, alpha * 0.6f, baseCol, true, px2, py2});
               }
            }
        }
        
        // Sort Z
        std::sort(renderList.begin(), renderList.end(), [](const RenderItem& a, const RenderItem& b) {
            return a.z < b.z;
        });
        
        // Rasterize
        for(const auto& item : renderList)
        {
            if (item.isBeam)
            {
                g.setColour(item.col.withAlpha(item.alpha * 0.5f));
                g.drawLine(item.x, item.y, item.px2, item.py2, 1.0f + level * 2.0f);
            }
            else
            {
                // Glow Halo
                if (level > 0.1f)
                {
                    g.setColour(item.col.withAlpha(item.alpha * 0.3f * level));
                    g.fillEllipse(item.x - item.size, item.y - item.size, item.size*2, item.size*2);
                }
                g.setColour(item.col.withAlpha(item.alpha));
                g.fillEllipse(item.x - item.size/2, item.y - item.size/2, item.size, item.size);
            }
        }
        
        // Center Core Shine
        float coreSize = radius * (0.3f + level * 0.4f);
        g.setGradientFill(juce::ColourGradient(juce::Colours::white.withAlpha(0.8f), center.x, center.y,
                                               baseCol.withAlpha(0.0f), center.x, center.y - coreSize, true));
        g.fillEllipse(center.x - coreSize, center.y - coreSize, coreSize*2, coreSize*2);
    }

private:
    float level = 0.0f;
    float chaos = 0.0f;
    float morph = 0.0f;
    std::vector<Vector3> baseVertices;
};

#include "AetherTransferVisualizer.h"

} // namespace aether
