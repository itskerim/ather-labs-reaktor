/*
  ==============================================================================

    AetherNoise.h
    Created: 31 Jan 2026
    Author:  Antigravity

    Description:
    Wide noise generator for injection into distortion stages.
    Supports White, Pink, and Crackle types with stereo spreading.

  ==============================================================================
*/

#pragma once

#include "AetherCommon.h"
#include <juce_core/juce_core.h>

namespace aether
{

template <typename SampleType>
class AetherNoise
{
public:
    enum class NoiseType { White, Pink, Crackle, Custom };

    AetherNoise() = default;

    void prepare(double sampleRate)
    {
        lastOutL = 0; lastOutR = 0;
        
        // Pink noise filters
        for (int i = 0; i < 7; ++i) {
            bL[i] = 0; bR[i] = 0;
        }

        // HPF Coefficients (approx 200Hz @ 44.1k/48k)
        hpL_x1 = 0; hpL_y1 = 0; hpR_x1 = 0; hpR_y1 = 0;
        hpL_x2 = 0; hpL_y2 = 0; hpR_x2 = 0; hpR_y2 = 0;
        
        customPos = 0;
    }
    
    void setCustomSample(const juce::AudioBuffer<float>& newSample)
    {
        customBuffer.makeCopyOf(newSample);
        customPos = 0;
    }

    void process(SampleType& left, SampleType& right, float volume, float distortion, NoiseType type, float envelope)
    {
        // 1. GATED NOISE
        // The noise volume follows the input signal envelope (sidechain/gate effect)
        // envelope comes from the main engine (Flux/RMS)
        float gatedVol = volume * envelope; 
        
        // Min threshold to avoid processing silence
        if (gatedVol <= 0.0001f) return;

        SampleType nL = 0, nR = 0;

        switch (type)
        {
            case NoiseType::White:
                nL = (random.nextFloat() * 2.0f - 1.0f);
                nR = (random.nextFloat() * 2.0f - 1.0f);
                break;

            case NoiseType::Pink:
            {
                // Pink Noise Algorithm (McCartney)
                float whiteL = (random.nextFloat() * 2.0f - 1.0f);
                bL[0] = 0.99886f * bL[0] + whiteL * 0.0555179f;
                bL[1] = 0.99332f * bL[1] + whiteL * 0.0750759f;
                bL[2] = 0.96900f * bL[2] + whiteL * 0.1538520f;
                bL[3] = 0.86650f * bL[3] + whiteL * 0.3104856f;
                bL[4] = 0.55000f * bL[4] + whiteL * 0.5329522f;
                bL[5] = -0.7616f * bL[5] - whiteL * 0.0168980f;
                nL = bL[0] + bL[1] + bL[2] + bL[3] + bL[4] + bL[5] + bL[6] + whiteL * 0.5362f;
                nL *= 0.11f; 
                bL[6] = whiteL * 0.115926f;

                float whiteR = (random.nextFloat() * 2.0f - 1.0f);
                bR[0] = 0.99886f * bR[0] + whiteR * 0.0555179f;
                bR[1] = 0.99332f * bR[1] + whiteR * 0.0750759f;
                bR[2] = 0.96900f * bR[2] + whiteR * 0.1538520f;
                bR[3] = 0.86650f * bR[3] + whiteR * 0.3104856f;
                bR[4] = 0.55000f * bR[4] + whiteR * 0.5329522f;
                bR[5] = -0.7616f * bR[5] - whiteR * 0.0168980f;
                nR = bR[0] + bR[1] + bR[2] + bR[3] + bR[4] + bR[5] + bR[6] + whiteR * 0.5362f;
                nR *= 0.11f; 
                bR[6] = whiteR * 0.115926f;
                break;
            }

            case NoiseType::Crackle:
                if (random.nextFloat() > 0.985f) nL = (random.nextFloat() * 2.0f - 1.0f);
                if (random.nextFloat() > 0.985f) nR = (random.nextFloat() * 2.0f - 1.0f);
                break;
                
            case NoiseType::Custom:
                if (customBuffer.getNumSamples() > 0)
                {
                    nL = customBuffer.getSample(0, customPos);
                    // Use Right channel if available, else duplicate Left
                    if (customBuffer.getNumChannels() > 1)
                        nR = customBuffer.getSample(1, customPos);
                    else
                        nR = nL;
                        
                    customPos++;
                    if (customPos >= customBuffer.getNumSamples()) customPos = 0;
                }
                break;
        }

        // --- 2. DISTORTION (Replaces "Width") ---
        // User requested "Distortion instead of Wider" for highs.
        // We apply a Drive + Hard Clip to the noise signal.
        if (distortion > 0.0f)
        {
            float drive = 1.0f + (distortion * 20.0f); // Up to 20x gain
            nL = std::tanh(nL * drive);
            nR = std::tanh(nR * drive);
            
            // Bitcrush-like artifact (Sample Hold) if distortion is high?
            // Keep it simple: Hard Driver.
        }
        
        // --- 3. LOW CUT (High Pass @ ~500Hz) ---
        // Keep the mud out
        float hpCoeff = 0.95f; // ~400-500Hz
        
        float hpL1 = hpCoeff * (hpL_y1 + nL - hpL_x1);
        hpL_x1 = nL; hpL_y1 = hpL1;
        float hpR1 = hpCoeff * (hpR_y1 + nR - hpR_x1);
        hpR_x1 = nR; hpR_y1 = hpR1;
        
        float hpL2 = hpCoeff * (hpL_y2 + hpL1 - hpL_x2);
        hpL_x2 = hpL1; hpL_y2 = hpL2;
        float hpR2 = hpCoeff * (hpR_y2 + hpR1 - hpR_x2);
        hpR_x2 = hpR1; hpR_y2 = hpR2;

        nL = hpL2; 
        nR = hpR2;

        // NAN CHECK
        if (!std::isfinite(hpL_y1) || !std::isfinite(hpL_y2) || !std::isfinite(nL)) {
            hpL_x1 = 0; hpL_y1 = 0; hpL_x2 = 0; hpL_y2 = 0; nL = 0;
        }
        if (!std::isfinite(hpR_y1) || !std::isfinite(hpR_y2) || !std::isfinite(nR)) {
            hpR_x1 = 0; hpR_y1 = 0; hpR_x2 = 0; hpR_y2 = 0; nR = 0;
        }

        // Inject into signal
        left  += nL * gatedVol;
        right += nR * gatedVol;
    }

private:
    juce::Random random;
    SampleType lastOutL = 0, lastOutR = 0;
    float bL[7], bR[7];

    // HPF State
    float hpL_x1, hpL_y1, hpR_x1, hpR_y1;
    float hpL_x2, hpL_y2, hpR_x2, hpR_y2;
    
    // Custom Sample
    juce::AudioBuffer<float> customBuffer;
    int customPos = 0;
};

} // namespace aether
