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
        ic5eq = 0; ic6eq = 0;
    }
    
    void setType(FilterType t) 
    { 
        if (filterType != t)
        {
            filterType = t; 
            reset(); // Clear state when switching modes
        }
    }

    /**
     * Set filter coefficients
     * @param cutoff Frequency in Hz
     * @param res Resonance (0.0 to 1.0)
     * @param morph Morph value (0.0 to 1.0)
     */
    void setParams(float cutoff, float res, float morph)
    {
        cutoff = std::clamp(cutoff, 20.0f, sampleRate * 0.45f);
        currentCutoff = cutoff;
        currentResonance = res;
        currentMorph = std::clamp(morph, 0.0f, 0.999f); 
        
        float g = std::tan(PI * cutoff / sampleRate);
        float r = 2.0f - (1.95f * res); 
        
        a1 = 1.0f / (1.0f + g * (g + r));
        a2 = g * a1;
        a3 = g * a2;
        k_val = r;
    }

    SampleType processSample(SampleType x)
    {
        // --- 1. Audio Stability Guard ---
        if (!std::isfinite(s1) || !std::isfinite(s2)) reset();

        // Standard SVF State Update
        SampleType v3 = x - s2;
        SampleType v1 = a1 * s1 + a2 * v3;
        SampleType v2 = s2 + a3 * s1 + a2 * v1;
        
        s1 = 2.0f * v1 - s1;
        s2 = 2.0f * v2 - s2;
        
        SampleType output = 0;

        switch (filterType)
        {
            case FilterType::LowPass:
                output = v2;
                break;
            case FilterType::BandPass:
                output = v1;
                break;
            case FilterType::HighPass:
                output = x - k_val * v1 - v2;
                break;
            case FilterType::Notch:
                output = x - k_val * v1;
                break;
            case FilterType::Morph:
                // Morphing between LP -> BP -> HP
                if (currentMorph < 0.5f)
                {
                    float m = currentMorph * 2.0f;
                    output = v2 * (1.0f - m) + v1 * m;
                }
                else
                {
                    float m = (currentMorph - 0.5f) * 2.0f;
                    output = v1 * (1.0f - m) + (x - k_val * v1 - v2) * m;
                }
                break;
            case FilterType::Formant:
            {
                // Safety: Reset Formant states if invalid
                if (!std::isfinite(ic1eq) || !std::isfinite(ic3eq)) reset();

                struct Vowel { float f1, f2, f3; };
                static const Vowel vowelTable[5] = {
                    { 730.0f, 1090.0f, 2440.0f }, // A
                    { 530.0f, 1840.0f, 2480.0f }, // E
                    { 270.0f, 2290.0f, 3010.0f }, // I
                    { 570.0f, 840.0f,  2410.0f }, // O
                    { 300.0f, 870.0f,  2240.0f }  // U
                };
                
                float m = currentMorph * 3.99f;
                int i = (int)m;
                float frac = m - i;
                int next = (i + 1) % 5;
                
                float f1 = vowelTable[i].f1 * (1.0f - frac) + vowelTable[next].f1 * frac;
                float f2 = vowelTable[i].f2 * (1.0f - frac) + vowelTable[next].f2 * frac;
                float f3 = vowelTable[i].f3 * (1.0f - frac) + vowelTable[next].f3 * frac;
                
                float shift = std::pow(currentCutoff / 800.0f, 0.5f); 
                f1 *= shift; f2 *= shift; f3 *= shift;
                
                float q = 1.0f + (currentResonance * 15.0f); 
                
                auto processPeak = [&](float freq, SampleType& sA, SampleType& sB) -> SampleType
                {
                    freq = std::clamp(freq, 40.0f, sampleRate * 0.45f);
                    float gp = std::tan(PI * freq / sampleRate);
                    float rp = 1.0f / q;
                    float a1p = 1.0f / (1.0f + gp * (gp + rp));
                    float a2p = gp * a1p;
                    float a3p = gp * a2p;

                    SampleType v3p = x - sB;
                    SampleType v1p = a1p * sA + a2p * v3p;
                    SampleType v2p = sB + a3p * sA + a2p * v1p;
                    
                    sA = 2.0f * v1p - sA;
                    sB = 2.0f * v2p - sB;
                    
                    return v1p; // Bandpass output
                };

                SampleType p1 = processPeak(f1, ic1eq, ic2eq);
                SampleType p2 = processPeak(f2, ic3eq, ic4eq);
                SampleType p3 = processPeak(f3, ic5eq, ic6eq);
                
                // Sum formant peaks; gain compensation so Vowel mode isn't much quieter than Morph
                output = (p1 * 1.0f + p2 * 0.8f + p3 * 0.6f) * 0.8f;
                const float formantGainComp = 3.5f; // Formant is very selective, restore level
                output = std::tanh(output * formantGainComp);
                break;
            }
        }
        return output;
    }

private:
    float sampleRate = 44100.0f;
    float a1=0, a2=0, a3=0;
    float k_val = 1.0f;
    SampleType s1 = 0, s2 = 0;
    
    // Formant Filter State
    SampleType ic1eq = 0, ic2eq = 0;
    SampleType ic3eq = 0, ic4eq = 0;
    SampleType ic5eq = 0, ic6eq = 0;
    
    float currentCutoff = 1000.0f;
    float currentResonance = 0.5f;
    float currentMorph = 0.0f;
    
    FilterType filterType = FilterType::Morph;
};

} // namespace aether
