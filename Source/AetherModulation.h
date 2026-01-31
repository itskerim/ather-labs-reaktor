#pragma once

#include "AetherCommon.h"

namespace aether
{

/**
 * AetherLFO: Multi-waveform host-synced LFO
 */
class AetherLFO
{
public:
    enum class Waveform { Sine, Triangle, Square, Saw, Random, Drift };

    AetherLFO() = default;

    void prepare(double sr) { sampleRate = (float)sr; }

    void setParams(float freq, Waveform wave, bool sync = false)
    {
        frequency = freq;
        currentWave = wave;
        isSynced = sync;
    }
    
    void setBPM(double bpm)
    {
        currentBPM = bpm;
    }

    float getNextSample()
    {
        float actualFreq = frequency;
        
        if (isSynced && currentBPM > 0.0)
        {
            // If Synced, frequency acts as Rate Division.
            // Let's say knob 0..1 maps to common rates (1/4, 1/8, 1/16, etc.)
            // But for Drift, we might just want "Beats per cycle" or something.
            // Simple approach: frequency = Hz. 
            // Better approach: If synced, freq parameter is multiplier of BPM.
            // 2.0 = 2 beats (Half note), 1.0 = 1 beat (Quarter), 0.5 = Eighth.
            // Standard Time in Secs = 60 / BPM.
            // Freq = 1 / (Time * Multiplier).
            // Let's treat 'frequency' input as Hz for now, and calculate it externally?
            // OR: Let AetherEngine handle the Hz calculation.
            // Let's keep LFO simple: it just takes Hz. AetherEngine calculates Hz from BPM.
        }

        float srSafe = std::max(1.0f, sampleRate);
        phase += actualFreq / srSafe;
        if (phase >= 1.0f) phase -= 1.0f;

        float output = 0.0f;
        switch (currentWave)
        {
            case Waveform::Sine:
                output = std::sin(TWO_PI * phase);
                break;
            case Waveform::Triangle:
                output = 2.0f * std::abs(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
                break;
            case Waveform::Square:
                output = (phase < 0.5f) ? 1.0f : -1.0f;
                break;
            case Waveform::Saw:
                output = 2.0f * phase - 1.0f;
                break;
            case Waveform::Random:
                // S&H
                if (phase < actualFreq / sampleRate) 
                    targetRandom = ((float)std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                output = targetRandom;
                break;
            case Waveform::Drift:
                // Smooth Random Walk
                if (phase < actualFreq / sampleRate) 
                    targetRandom = ((float)std::rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                
                // Slew towards target
                currentDrift += (targetRandom - currentDrift) * 0.001f; // Slow slew
                output = currentDrift;
                break;
        }
        return output;
    }

private:
    float phase = 0.0f;
    float frequency = 1.0f;
    float sampleRate = 44100.0f;
    float targetRandom = 0.0f;
    float currentDrift = 0.0f;
    double currentBPM = 120.0;
    bool isSynced = false;
    Waveform currentWave = Waveform::Sine;
};

/**
 * AetherEnvelopeFollower: High-precision signal peak detector
 */
class AetherEnvelopeFollower
{
public:
    void prepare(double sr) { sampleRate = (float)sr; }

    void setParams(float attackMs, float releaseMs)
    {
        attackCoef = std::exp(-1.0f / (attackMs * 0.001f * sampleRate));
        releaseCoef = std::exp(-1.0f / (releaseMs * 0.001f * sampleRate));
    }

    float processSample(float x)
    {
        float inputAbs = std::abs(x);
        if (inputAbs > envelope)
            envelope = attackCoef * envelope + (1.0f - attackCoef) * inputAbs;
        else
            envelope = releaseCoef * envelope + (1.0f - releaseCoef) * inputAbs;
        
        return envelope;
    }

private:
    float sampleRate = 44100.0f;
    float envelope = 0.0f;
    float attackCoef = 0.0f;
    float releaseCoef = 0.0f;
};

} // namespace aether
