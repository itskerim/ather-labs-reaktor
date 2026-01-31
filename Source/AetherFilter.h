#pragma once

#include "AetherCommon.h"
#include <cmath>
#include <algorithm>

namespace aether
{

/**
 * AetherFilter: State Variable Filter with DnB Morphing
 */
template <typename SampleType>
class AetherFilter
{
public:
    enum class FilterType { LowPass, BandPass, HighPass, Notch, Morph, Formant };
    
    AetherFilter() { reset(); }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (float)spec.sampleRate;
        reset();
    }

    void reset()
    {
        s1 = 0; s2 = 0;
        ic1eq = 0; ic2eq = 0;
        ic3eq = 0; ic4eq = 0;
    }
    
    void setType(FilterType t) { filterType = t; }

    /**
     * Set filter coefficients
     * @param cutoff Frequency in Hz
     * @param res Resonance (0.0 to 1.0)
     * @param morph Morph value (0.0 to 1.0)
     */
    void setParams(float cutoff, float res, float morph)
    {
        // Safe cutoff bounds
        cutoff = std::clamp(cutoff, 20.0f, sampleRate * 0.45f);
        
        currentCutoff = cutoff;
        currentResonance = res;
        currentMorph = std::clamp(morph, 0.0f, 0.999f); // Clamp to prevent index 5
        
        float g = std::tan(PI * cutoff / sampleRate);
        float k = 2.0f - (1.9f * res); // Resonance mapping
        
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    SampleType processSample(SampleType x)
    {
        // Standard SVF State Update
        SampleType v3 = x - s2;
        SampleType v1 = a1 * s1 + a2 * v3;
        SampleType v2 = s2 + a3 * s1 + a2 * v1;
        
        s1 = 2.0f * v1 - s1;
        s2 = 2.0f * v2 - s2;
        
        // SVF Outputs
        SampleType lp = v2;
        SampleType hp = x - k * v1 - v2;
        SampleType bp = v1;
        SampleType notch = x - k * v1;

        SampleType output = 0;

        switch (filterType)
        {
            case FilterType::LowPass:
                output = lp;
                break;
            case FilterType::BandPass:
                output = bp;
                break;
            case FilterType::HighPass:
                output = hp;
                break;
            case FilterType::Notch:
                output = notch;
                break;
            case FilterType::Morph:
                // Morphing between LP -> BP -> HP
                if (currentMorph < 0.5f)
                {
                    float m = currentMorph * 2.0f;
                    output = lp * (1.0f - m) + bp * m;
                }
                else
                {
                    float m = (currentMorph - 0.5f) * 2.0f;
                    output = bp * (1.0f - m) + hp * m;
                }
                break;
            case FilterType::Formant:
                // --- VOWEL / FORMANT MODE (The "Muzz/Skrillex" Growl) ---
                // We hijack the LowPass slot for now to be the default "Growl" filter
                // Morph 0..1 sweeps: A -> E -> I -> O -> U
                
                float vowels[5][2] = {
                    {730.0f, 1090.0f}, // A
                    {530.0f, 1840.0f}, // E
                    {270.0f, 2290.0f}, // I
                    {570.0f, 840.0f},  // O
                    {300.0f, 870.0f}   // U
                };
                
                // Morph Index
                float m = currentMorph * 4.0f; // 0..4
                int i = (int)m;
                if (i > 4) i = 4; // Safety clamp
                float frac = m - i;
                int next = (i + 1) % 5;
                
                // Interpolate Formants
                float f1 = vowels[i][0] * (1.0f - frac) + vowels[next][0] * frac;
                float f2 = vowels[i][1] * (1.0f - frac) + vowels[next][1] * frac;
                
                // Shift by Cutoff knob (so it's tunable but stable)
                // Linear 20Hz-20kHz is too extreme. Map 1000Hz to 1.0x.
                // Range: 0.5x (Deep) to 2.0x (Chipmunk)
                // Logic: (cutoff / 1000.0f) is bad. 
                // Let's use log-like mapping or just clamped range.
                float gender = std::pow(currentCutoff / 1000.0f, 0.3f); // Gentler curve
                
                f1 *= gender;
                f2 *= gender;
                
                // Parallel BandPass Filters for Formants
                // Very high resonance for "Talking" effect
                float q = currentResonance * 8.0f + 1.0f; // High Q needed for distinct vowels
                
                // Formant 1
                float g1 = std::tan(3.14159f * std::clamp(f1, 50.0f, sampleRate * 0.45f) / sampleRate);
                float k1 = 1.0f / q;
                float a1_formant = 1.0f / (1.0f + g1 * (g1 + k1));
                float bp1 = (x - ic1eq - ic2eq * (g1 + k1)) * a1_formant * g1; // BP output
                ic1eq += 2.0f * g1 * (x - ic1eq - ic2eq * (g1 + k1)) * a1_formant;
                ic2eq += 2.0f * bp1;
                
                // Formant 2
                float g2 = std::tan(3.14159f * std::clamp(f2, 50.0f, sampleRate * 0.45f) / sampleRate);
                float k2 = 1.0f / q;
                float a2_formant = 1.0f / (1.0f + g2 * (g2 + k2));
                float bp2 = (x - ic3eq - ic4eq * (g2 + k2)) * a2_formant * g2;
                ic3eq += 2.0f * g2 * (x - ic3eq - ic4eq * (g2 + k2)) * a2_formant;
                ic4eq += 2.0f * bp2;
                
                // Sum with Gain Compensation (Resonant BPFs add gain)
                output = (bp1 + bp2) * 0.5f; 
                break;
        }
        return output;
    }

private:
    float sampleRate = 44100.0f;
    float a1, a2, a3;
    float k = 1.0f;
    float s1 = 0, s2 = 0;
    
    // Formant Filter State
    float ic1eq = 0, ic2eq = 0;
    float ic3eq = 0, ic4eq = 0;
    
    float currentCutoff = 1000.0f;
    float currentResonance = 0.5f;
    float currentMorph = 0.0f;
    
    FilterType filterType = FilterType::Morph; // Default to Morph
};

} // namespace aether
