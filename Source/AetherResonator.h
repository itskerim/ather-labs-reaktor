#pragma once

#include "AetherCommon.h"
#include <vector>
#include "AetherModulation.h"

namespace aether
{

/**
 * AetherResonator: A tuned feedback delay for metallic DnB textures.
 */
template <typename SampleType>
class AetherResonator
{
public:
    AetherResonator() 
    {
        // Massive 1M sample buffer (~5-20s depending on SR)
        buffer.assign(1048576, 0.0f); 
    }

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = (float)spec.sampleRate;
        reset();
        lfo.prepare(spec.sampleRate);
        lfo.setParams(0.5f, AetherLFO::Waveform::Sine); // Slow breather
    }

    void reset()
    {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writeIndex = 0;
    }

    /**
     * @param feedback Feedback amount (0 to 1.0+)
     * @param timeMs Delay time in milliseconds
     * @param plasma Chaos/Modulation amount (0..1)
     */
    SampleType processSample(SampleType x, float feedback, float timeMs, float plasma = 0.0f)
    {
        if (buffer.empty()) return x;

        // Plasma LFO modulation
        float lfoVal = lfo.getNextSample();
        
        // Modulate time slightly for "Black Hole" detune
        // Clamp modTime to safe range (0.1ms to 4000ms)
        float modTime = std::clamp(timeMs + (lfoVal * plasma * 10.0f), 0.1f, 4000.0f); 
        float delaySamples = (modTime / 1000.0f) * sampleRate;
        
        // Fractional delay read
        float readPos = (float)writeIndex - delaySamples;
        float bufSize = (float)buffer.size();
        
        // Robust wrapping
        while (readPos < 0) readPos += bufSize;
        while (readPos >= bufSize) readPos -= bufSize;
        
        int i1 = (int)readPos;
        int i2 = (i1 + 1) % buffer.size();
        float frac = readPos - (float)i1;
        
        // Final safety check for indices
        i1 = std::clamp(i1, 0, (int)buffer.size() - 1);
        i2 = std::clamp(i2, 0, (int)buffer.size() - 1);

        SampleType delayedSample = buffer[i1] * (1.0f - frac) + buffer[i2] * frac;
        
        // Feedback loop with Plasma saturation
        SampleType output = x + delayedSample * feedback;
        
        // "Event Horizon" Saturation (Hard clipping at edges, linear in middle)
        // This keeps the feedback loop from exploding forever but allows it to scream
        SampleType saturated = std::tanh(output * (1.0f + plasma * 0.5f));
        
        // NAN CHECK: Protect feedback buffer from poisoning
        if (!std::isfinite(saturated)) 
        {
            saturated = 0.0f;
            // Optional: You could reset the whole buffer here if things went really wrong, 
            // but correcting the stream is usually enough to recover.
        }
        
        buffer[writeIndex] = saturated;
        writeIndex = (writeIndex + 1) % buffer.size();
        
        return output;
    }

private:
    float sampleRate = 44100.0f;
    std::vector<SampleType> buffer;
    int writeIndex = 0;
    AetherLFO lfo;
};

} // namespace aether
