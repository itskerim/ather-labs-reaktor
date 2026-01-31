#pragma once

#include "AetherCommon.h"
#include <juce_dsp/juce_dsp.h>

namespace aether
{

/**
 * AetherDimension: Hyper-Dimension Stereo Widener
 * Uses a short delay on the side channel or Haas effect to widen the stereo image.
 * Designed for High-Band content (Neuro tops).
 */
class AetherDimension
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (float)spec.sampleRate;
        // Max delay 20ms
        delayBuffer.setSize(2, (int)(0.05f * sampleRate) + 1);
        delayBuffer.clear();
    }

    void reset()
    {
        delayBuffer.clear();
        writePos = 0;
    }

    /**
     * Process Stereo Sample (High Band)
     * @param left  Left sample reference
     * @param right Right sample reference
     * @param width Amount 0.0 (Mono) -> 1.0 (Full Wide) -> 2.0 (Hyper)
     */
    void process(float& left, float& right, float width)
    {
        if (width <= 0.01f) return; // Passthrough

        // 1. Haas / Dimension Effect
        // Delay one channel slightly (e.g., Right) or invert phase of delayed signal on Sides.
        // Dimension Expander style:
        // L = L + dry
        // R = R - delayed_inverted ... complex.
        
        // Simple Haas for Neuro:
        // Delay Right channel by ~10-15ms.
        // BUT Haas collapses to comb filter in Mono. 
        // Better: Mono-compatible Chorus/Detune? 
        // Aether uses "Dimension": Two oppositely delayed signals mixed in.
        
        // Let's implement a safe "Side Diff" Widener.
        // Side = (L - R) * width.
        // Mid = (L + R)
        // If signal is mono input (L=R), Side is 0. Widener does nothing.
        // We need to CREATE width from Mono.
        
        // -- DIMENSION TRICK --
        // Delay L by X ms, Delay R by Y ms. Mix in inverted?
        // Let's go with:
        // L_out = L + (Delayed_R_inverted * amount)
        // R_out = R + (Delayed_L_inverted * amount)
        
        int delaySamps = (int)(0.012f * sampleRate); // 12ms fixed delay
        
        // Circular Buffer Write
        auto* dL = delayBuffer.getWritePointer(0);
        auto* dR = delayBuffer.getWritePointer(1);
        int len = delayBuffer.getNumSamples();
        
        dL[writePos] = left;
        dR[writePos] = right;
        
        // Read delayed
        int rPos = writePos - delaySamps;
        if (rPos < 0) rPos += len;
        
        float delayedL = dL[rPos];
        float delayedR = dR[rPos];
        
        // Cross-inject inverted delay (Dimension style)
        // Boosted intensity for "Hyper-Dimension" (User requested)
        left  -= delayedR * width * 0.85f;
        right -= delayedL * width * 0.85f;
        
        // Advance
        writePos++;
        if (writePos >= len) writePos = 0;
    }

private:
    float sampleRate = 44100.0f;
    juce::AudioBuffer<float> delayBuffer;
    int writePos = 0;
};

} // namespace aether
