#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <cmath>

namespace aether
{

// Standard constants for calculations
static constexpr float PI = 3.14159265358979323846f;
static constexpr float TWO_PI = 6.28318530717958647692f;

/** 
 * Distortion Algorithm Types
 */
enum class DistortionAlgo
{
    None,
    SoftClip,
    HardClip,
    SineFold,
    TriangleWarp,
    BitCrush,
    SampleReduce,
    AsymSaturation,
    Rectify,
    Tanh,
    SoftFold,
    Chebyshev,
    Count
};

/**
 * Filter Categories for DnB
 */
enum class FilterMode
{
    Basic,
    Morph,
    Peaking,
    Harmonic
};

// Helper for fast atan/tanh if needed later
inline float fastTanh(float x)
{
    if (x < -3.0f) return -1.0f;
    if (x > 3.0f) return 1.0f;
    return x * (27.0f + x * x) / (27.0f + 9.0f * x * x);
}

} // namespace aether
