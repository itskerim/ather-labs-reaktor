#pragma once

#include "AetherCommon.h"

namespace aether
{

/**
 * AetherDistortion: The multi-staged, bipolar distortion engine.
 * Implements various nonlinear transfer functions with separate 
 * processing for positive and negative peaks.
 */
template <typename SampleType>
class AetherDistortion
{
public:
    AetherDistortion() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (float)spec.sampleRate;
    }

    void reset()
    {
        // Stateless, but good practice
    }

    /**
     * Processes a single sample through the bipolar engine.
     * @param input The input sample
     * @param drive Gain amount (0.0 to 1.0)
     * @param fold Wavefolding amount (0.0 to 1.0)
     * @param algoPos Algorithm for positive phase
     * @param algoNeg Algorithm for negative phase
     * @param stages Number of processing iterations (1 to 12)
     */
    SampleType processSample(SampleType input, float drive, float fold, DistortionAlgo algoPos, DistortionAlgo algoNeg, int stages)
    {
        SampleType output = input;
        
        // --- 1. PRE-FOLDING (High-Fidelity Harmonics) ---
        if (fold > 0.001f)
        {
            float foldGain = 1.0f + (fold * 4.0f);
            output = std::sin(output * foldGain * PI * 0.5f);
        }

        // --- 2. MULTI-STAGED SATURATION ---
        // Scale drive for intensity (0 to 24dB approx)
        float driveGain = std::pow(10.0f, (drive * 24.0f) / 20.0f);

        // Distribute drive across stages.
        // With up to 12 stages, we need to ensure each stage adds meaningful grit
        int safeStages = std::max(1, stages);
        output *= (driveGain / std::sqrt((float)safeStages)); 

        for (int i = 0; i < safeStages; ++i)
        {
            if (output >= 0)
                output = applyAlgo(output, algoPos);
            else
                output = applyAlgo(output, algoNeg);
        }

        return output;
    }

private:
    SampleType applyAlgo(SampleType x, DistortionAlgo algo)
    {
        switch (algo)
        {
            case DistortionAlgo::SoftClip:
                return std::atan(x);
            
            case DistortionAlgo::HardClip:
                return std::clamp(x, (SampleType)-1.0, (SampleType)1.0);
            
            case DistortionAlgo::SineFold:
                return std::sin(x * PI * 0.5f);
            
            case DistortionAlgo::SoftFold:
                return x - (0.1f * std::sin(x * PI));

            case DistortionAlgo::TriangleWarp:
                return (SampleType)2.0 * std::abs(x - std::floor(x + (SampleType)0.5)) - (SampleType)1.0;
            
            case DistortionAlgo::BitCrush:
            {
                float step = 0.1f;
                return std::round(x / step) * step;
            }

            case DistortionAlgo::Rectify:
                return std::abs(x);

            case DistortionAlgo::Tanh:
                return std::tanh(x);

            case DistortionAlgo::Chebyshev:
                // 3rd order chebyshev: 4x^3 - 3x
                return (4.0f * x * x * x) - (3.0f * x);

            case DistortionAlgo::Count:
            case DistortionAlgo::None:
            default:
                return x;
        }
    }

    float sampleRate = 44100.0f;
};

} // namespace aether
