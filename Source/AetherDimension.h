#pragma once

#include "AetherCommon.h"
#include <vector>

namespace aether
{

// Simple Schroeder All-Pass Filter
// Used to manipulate phase without altering frequency magnitude response.
// Transfer Function H(z) = ( -g + z^-D ) / ( 1 - g * z^-D )
// This creates a frequency-dependent delay (dispersion).
class AllPassFilter
{
public:
    void setDelay(int samples)
    {
        buffer.assign(samples, 0.0f);
        pos = 0;
    }

    void clear()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        pos = 0;
    }

    float process(float input)
    {
        if (buffer.empty()) return input;

        // Schroeder All-Pass Implementation:
        // By mixing the input with the delayed signal and feeding back the result,
        // we create a "smearing" of the signal in time (phase shift).
        // 
        // y[n] = -g * x[n] + x[n-D] + g * y[n-D]
        //
        // This effectively "rotates" the phase of frequencies differently 
        // depending on the delay length 'D'.
        
        float delayed = buffer[pos];
        float g = 0.5f; // Fixed coefficient (0.5 is optimal for smooth wide dispersion)
        
        // Feedforward and Feedback paths
        float out = delayed - g * input;
        float feed = input + g * delayed;
        
        // SAFETY: Soft clip feedback to prevent internal explosion if g > 1 or resonating
        if (feed > 2.0f) feed = 2.0f;
        if (feed < -2.0f) feed = -2.0f;
        
        buffer[pos] = feed;
        
        pos++;
        if (pos >= buffer.size()) pos = 0;
        
        return out;
    }

private:
    std::vector<float> buffer;
    int pos = 0;
};

/**
 * AetherDimension: "Wider" Style Phase-Decorrelation Imager
 * 
 * ALGORITHM EXPLANATION:
 * Unlike traditional delays (Haas Effect) which sound phasey/metallic in mono,
 * this uses a network of All-Pass Filters to decorrelate the phase of a "Side" signal
 * without affecting the amplitude of any frequency.
 * 
 * SIGNAL FLOW:
 * 1. Mono Sum (Mid) = (L + R) / 2
 * 2. Side Signal = APF4( APF3( APF2( APF1( Mid ) ) ) )
 *    - The chain of APFs creates a "scrambled" version of the Mid signal.
 *    - It sounds identical in tone but has completely different phase alignment.
 * 
 * 3. Stereo Injection:
 *    Left  = InputL + (Side * Width)
 *    Right = InputR - (Side * Width)
 * 
 * MONO COMPATIBILITY MATH:
 * When summed to Mono (L + R):
 * Mono = (InputL + Side) + (InputR - Side)
 * Mono = InputL + InputR
 * Mono = Original Signal
 * 
 * The "Side" signal (the artificial width) mathematically vanishes.
 * This guarantees 100% Mono Compatibility with zero phasing artifacts.
 */
class AetherDimension
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (float)spec.sampleRate;
        
        // Tuning: Prime delays for smooth dense dispersion without metallic resonance
        // Scaled by sample rate to keep consistent time
        int base = (int)(sampleRate * 0.001f); // 1ms base
        
        apf1.setDelay(base * 2 + 3);   // ~2ms
        apf2.setDelay(base * 3 + 11);  // ~3ms
        apf3.setDelay(base * 7 + 5);   // ~7ms
        apf4.setDelay(base * 11 + 7);  // ~11ms
        
        reset();
    }

    void reset()
    {
        apf1.clear();
        apf2.clear();
        apf3.clear();
        apf4.clear();
    }

    /**
     * Process Stereo Sample
     * @param left  Left sample reference
     * @param right Right sample reference
     * @param width Amount 0.0 (Mono) -> 1.0 (Full Wide) -> 2.0 (Hyper)
     */
    void process(float& left, float& right, float width)
    {
        if (width <= 0.01f) return;

        // 1. Extract Mid (Mono)
        float mid = (left + right) * 0.5f;
        
        // 2. Generate decorrelated "Side" signal via APF chain
        float side = mid;
        side = apf1.process(side);
        side = apf2.process(side);
        side = apf3.process(side);
        side = apf4.process(side);
        
        // 3. Inject Artificial Width
        // The APF chain creates a phase-scrambled version of the mono signal.
        // Adding it to L and subtracting from R creates width.
        // Summing (L+R) cancels this term out perfectly : (I+S + I-S) = 2I.
        
        // Apply width gain
        // "Hyper" mode (width > 1.0) can push this harder
        float amount = width * 1.0f; 
        
        left  += side * amount;
        right -= side * amount;
    }

private:
    float sampleRate = 44100.0f;
    
    // Chain of filters
    AllPassFilter apf1, apf2, apf3, apf4;
};

} // namespace aether
