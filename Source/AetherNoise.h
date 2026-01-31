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
    enum class NoiseType { White, Pink, Crackle };

    AetherNoise() = default;

    void prepare(double sampleRate)
    {
        lastOutL = 0; lastOutR = 0;
        envelope = 0;
        
        // Pink noise filters
        for (int i = 0; i < 7; ++i) {
            bL[i] = 0; bR[i] = 0;
        }

        // HPF Coefficients (approx 200Hz @ 44.1k/48k)
        // y[n] = 0.98 * (y[n-1] + x[n] - x[n-1]) Simple HP
        hpL_x1 = 0; hpL_y1 = 0;
        hpR_x1 = 0; hpR_y1 = 0;
    }

    void process(SampleType& left, SampleType& right, float volume, float width, NoiseType type)
    {
        // 1. Audio Gating (Envelope Follower)
        // Measure input magnitude
        float inputMag = std::abs(left) + std::abs(right);
        float attack = 0.05f;  // Snappy attack
        float release = 0.005f; // Faster release for gating
        
        if (inputMag > envelope)
            envelope += attack * (inputMag - envelope);
        else
            envelope += release * (inputMag - envelope);

        float gatedVol = volume * std::min(1.0f, envelope * 10.0f); // 10x gain for threshold
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
        }

        // 2. Low Cut (High Pass @ ~250Hz)
        // y = 0.97 * (y1 + x - x1)
        float hpL = 0.97f * (hpL_y1 + nL - hpL_x1);
        hpL_x1 = nL; hpL_y1 = hpL;
        nL = hpL;

        float hpR = 0.97f * (hpR_y1 + nR - hpR_x1);
        hpR_x1 = nR; hpR_y1 = hpR;
        nR = hpR;

        // 3. Hyper-Wide Stereo (Mono Compatible)
        // Mid/Side with compensation
        // If width = 1: Mid is reduced, Side is amplified.
        // We ensure SideL = -SideR to keep Mono (L+R) consistent with Mid.
        
        SampleType mid = (nL + nR) * 0.5f;
        SampleType sideL = nL - mid;
        SampleType sideR = nR - mid;

        // Amplified Side for Hyper-Width
        float sideGain = 1.0f + (width * 1.5f); // Up to 2.5x side gain
        float midGain = 1.0f - (width * 0.3f);  // Slight mid dip for space
        
        nL = (mid * midGain) + (sideL * sideGain);
        nR = (mid * midGain) + (sideR * sideGain);

        // Inject into signal
        left  += nL * gatedVol;
        right += nR * gatedVol;
    }

private:
    juce::Random random;
    SampleType lastOutL = 0, lastOutR = 0;
    float bL[7], bR[7];
    float envelope = 0;

    // HPF State
    float hpL_x1, hpL_y1, hpR_x1, hpR_y1;
};

} // namespace aether
