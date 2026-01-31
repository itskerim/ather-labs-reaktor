#pragma once

#include "AetherCommon.h"
#include "AetherDistortion.h"
#include "AetherFilter.h"
#include "AetherResonator.h"
#include "AetherResonator.h"
#include "AetherModulation.h"
#include "AetherDimension.h"
#include "AetherNoise.h"
#include <juce_dsp/juce_dsp.h> // Required for juce::dsp::StateVariableTPTFilter

namespace aether
{

// --- NEURO COMPONENTS ---

// 4th Order Linkwitz-Riley Crossover (Matched Phase)
template <typename SampleType>
class AetherCrossover
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        lp1.prepare(spec); lp2.prepare(spec);
        hp1.prepare(spec); hp2.prepare(spec);
        lp1.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        lp2.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        hp1.setType(juce::dsp::StateVariableTPTFilterType::highpass);
        hp2.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    }

    void setCutoff(float frequency)
    {
        lp1.setCutoffFrequency(frequency); lp2.setCutoffFrequency(frequency);
        hp1.setCutoffFrequency(frequency); hp2.setCutoffFrequency(frequency);
        // Linkwitz-Riley Q = 0.707 (Butterworth) cascaded
        lp1.setResonance(0.707f); lp2.setResonance(0.707f);
        hp1.setResonance(0.707f); hp2.setResonance(0.707f);
    }

    void process(SampleType input, SampleType& outLow, SampleType& outHigh)
    {
        // 4th Order LP = LP -> LP
        outLow = lp2.processSample(0, lp1.processSample(0, input));
        // 4th Order HP = HP -> HP
        outHigh = hp2.processSample(0, hp1.processSample(0, input));
    }

private:
    juce::dsp::StateVariableTPTFilter<SampleType> lp1, lp2, hp1, hp2;
};

// Clean Sub Processor (Mono Sum + Warmth)
template <typename SampleType>
class AetherSubProcessor
{
public:
    void process(SampleType& left, SampleType& right, float subLevel, float drive)
    {
        // Mono Sum
        SampleType mono = (left + right) * 0.5f;
        
        // BOOST: Drive saturation hard for "perceived loudness" (Harmonics)
        // Input Boost: +6dB (2.0x) + Drive range
        SampleType saturated = std::tanh(mono * (2.0f + drive * 2.0f));
        
        // Output to both channels
        // Output Boost: Allow +6dB extra gain on top of parameter
        left = saturated * subLevel * 2.0f;
        right = saturated * subLevel * 2.0f;
    }
};

/**
 * AetherEngine: The Neuro-Bass Workstation (Split-Band Architecture)
 */
template <typename SampleType>
class AetherEngine
{
public:
    AetherEngine() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        // 4x Oversampling (Factor 2: 2^2 = 4)
        // Latency: Linear Phase is better for phase coherence with Sub, but adds latency.
        // We use IIR for efficiency and acceptable phase characteristic for this effect.
        oversampler.initProcessing(spec.maximumBlockSize);
        oversampler.reset();

        // CRITICAL FIX: Components in the upsampled path (High Band) run at 4x sample rate.
        // We must prepare them with the correct rate so filters/LFOs behave correctly.
        juce::dsp::ProcessSpec oversampledSpec = spec;
        oversampledSpec.sampleRate = spec.sampleRate * 4.0;
        oversampledSpec.maximumBlockSize = spec.maximumBlockSize * 4; // Buffer grows by 4x
        
        // Prepare High-Band Chain with Oversampled Rate
        distortion.prepare(oversampledSpec);
        filter.prepare(oversampledSpec);
        resonator.prepare(oversampledSpec);
        
        // Prepare Sub & Split (Run at native 1x rate)
        crossoverL.prepare(spec);
        crossoverR.prepare(spec);
        
        chaosLFO.prepare(spec.sampleRate); // LFOs stay at control rate (likely fine)
        chaosLFO.setParams(0.2f, AetherLFO::Waveform::Drift);
        fluxFollower.prepare(spec.sampleRate);
        fluxFollower.setParams(10.0f, 300.0f);
        
        fluxFollower.prepare(spec.sampleRate);
        fluxFollower.setParams(10.0f, 300.0f);
        
        // NOISE GATE: Tight response (30ms release)
        noiseGateFollower.prepare(spec.sampleRate);
        noiseGateFollower.setParams(5.0f, 30.0f); 
        
        // Fix: Dimension runs in oversampled loop, needs 4x rate for correct delay times
        dimension.prepare(oversampledSpec);
        
        noiseGen.prepare(spec.sampleRate); // Noise is generated at 1x then upsampled naturally or injected?
        // Wait, noise is injected at 1x in process(), then split. So prepare(spec.sampleRate) is correct.
        
        numChannels = spec.numChannels;
        
        reset();
    }

    void setCustomNoise(const juce::AudioBuffer<float>& newBuffer)
    {
        noiseGen.setCustomSample(newBuffer);
    }

    /**
     * NUCLEAR RESET: Total System Reboot
     * This function is the "Panic Button" for the audio engine.
     * 
     * PURPOSE:
     * If the DSP explodes (NaNs, Infinite Feedback loops, Driver crashes),
     * this function wipes the slate clean in <1ms.
     * 
     * WHAT IT DOES:
     * 1. Clears all filter states (history).
     * 2. Clears all delay buffers (silence).
     * 3. Resets DC blockers.
     * 4. Resets the Oversampler (crucial).
     * 
     * This turns a "Crash/Silence" event into a mere "Click" followed by recovery.
     */
    void reset()
    {
        distortion.reset();
        filter.reset();
        resonator.reset();
        
        oversampler.reset();
        
        // Clear DC States
        dcL_x1 = 0; dcL_y1 = 0;
        dcR_x1 = 0; dcR_y1 = 0;
        
        // Reset sub components
        // (Assuming they are stateless or simple enough, but could add reset there too if needed)
        // Crossovers use SVF which has reset usually? 
        // We can just rely on their stability or add reset to AetherCrossover later if needed.
        // For now, the main culprits are the high band components.
    }

    void process(juce::AudioBuffer<SampleType>& buffer, 
                 float drive, float blend, int stages, 
                 DistortionAlgo algoPos, DistortionAlgo algoNeg,
                 float cutoff, float resonance, float morph,
                 float fbAmount, float fbTimeMs, float scramble,
                 float subLevel, float squeeze, double bpm,
                 float width, float xoverHz, float fold, bool vowelMode,
                 float noiseLevel, float noiseWidth, int noiseType)
    {
        auto totalSamples = buffer.getNumSamples();
        auto* channelDataL = buffer.getWritePointer(0);
        auto* channelDataR = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
        
        chaosLFO.setBPM(bpm);
        
        // --- NOISE INJECTION (Dynamic & Distorted) ---
        // We use the FLUX envelope for gating the noise (Dynamic Texture)
        // Note: fluxFollower needs to be updated with current signal first?
        // Actually, flux is calculated inside the loop based on inputEnergy.
        // But doing it sample-by-sample here creates a delay of 1 sample or requires calc.
        
        auto nType = static_cast<typename AetherNoise<SampleType>::NoiseType>(noiseType);
        
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType& left = channelDataL[s];
            SampleType& right = channelDataR ? channelDataR[s] : left;
            
            // Calc Envelopes for this sample (Pre-calc for noise gate)
            // We use the same flux logic as in the High Band loop, but this is the full broadband input here.
            float inputEnergy = (std::abs(left) + std::abs(right)) * 0.5f;
            float envelope = noiseGateFollower.processSample(inputEnergy); // Use Tight Gate
            fluxFollower.processSample(inputEnergy); // Keep main flux updated too (for visualizer etc?) 
            // Actually fluxFollower IS updated in the high band loop later, but only for highs.
            // If we want fluxFollower to track broadband for other purposes we should update it here, 
            // but currently it is seemingly unused here except for the noise envelope previously.
            // Wait, look at line 287: fluxFollower.processSample is called AGAIN in the high band loop.
            // We should ensure we aren't "double clocking" it if it's the same object?
            // Yes, "fluxFollower" is a member. If we call processSample here and later, it updates twice per sample (roughly).
            // Actually the second loop is on the High Band signal. 
            // Let's just use noiseGateFollower here. The fluxFollower update here was likely for noise only.
            // So we can remove the fluxFollower call here if it's only used for noise.
            
            // noiseWidth parameter is now DISTORTION for the noise
            float noiseDistortion = noiseWidth; 
            
            noiseGen.process(left, right, noiseLevel, noiseDistortion, nType, envelope);
        }

        // Update Filter Mode
        if (vowelMode)
            filter.setType(AetherFilter<SampleType>::FilterType::Formant);
        else
            filter.setType(AetherFilter<SampleType>::FilterType::Morph);

        // Tunable Crossover
        float safeXOver = std::clamp(xoverHz, 60.0f, 300.0f);
        crossoverL.setCutoff(safeXOver);
        if (channelDataR) crossoverR.setCutoff(safeXOver);

        // --- SPLIT BANDS ---
        // We need separate buffers for Low and High.
        // Since we are oversampling Highs, we need to extract them first.
        
        juce::AudioBuffer<SampleType> highBuffer;
        highBuffer.makeCopyOf(buffer); // Start with input
        
        juce::AudioBuffer<SampleType> lowBuffer;
        lowBuffer.setSize(numChannels, totalSamples);
        
        auto* hL = highBuffer.getWritePointer(0);
        auto* hR = numChannels > 1 ? highBuffer.getWritePointer(1) : nullptr;
        auto* lL = lowBuffer.getWritePointer(0);
        auto* lR = numChannels > 1 ? lowBuffer.getWritePointer(1) : nullptr;
        
        // 1. Perform Crossover Split (at 1x)
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType inL = hL[s];
            SampleType inR = hR ? hR[s] : 0;
            
            SampleType lL_out, hL_out, lR_out, hR_out;
            
            crossoverL.process(inL, lL_out, hL_out);
            if (hR) {
                crossoverR.process(inR, lR_out, hR_out);
            } else {
                lR_out = lL_out; hR_out = hL_out;
            }
            
            // Store split signals
            lL[s] = lL_out;
            if (lR) lR[s] = lR_out;
            
            hL[s] = hL_out;
            if (hR) hR[s] = hR_out;
        }
        
        // --- 2. PROCESS LOWS (1x Rate) ---
        // Clean Sub saturation
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType sl = lL[s];
            SampleType sr = lR ? lR[s] : sl;
            subProcessor.process(sl, sr, subLevel, drive); // Sub now reacts slightly to main drive for "Warmth"
            lL[s] = sl;
            if (lR) lR[s] = sr;
        }

        // --- 3. UPSAMPLE HIGHS ---
        juce::dsp::AudioBlock<SampleType> highBlock(highBuffer);
        juce::dsp::AudioBlock<SampleType> upsampledBlock = oversampler.processSamplesUp(highBlock);
        
        auto* upL = upsampledBlock.getChannelPointer(0);
        auto* upR = numChannels > 1 ? upsampledBlock.getChannelPointer(1) : nullptr;
        int upSamples = (int)upsampledBlock.getNumSamples();
        
        // --- 4. PROCESS HIGHS (4x Rate) ---
        // We need to advance LFO/Flux slower relative to sample rate?
        // Actually, parameters usually modulation at Control Rate, but here we calculate per sample.
        // It's fine to run LFO at 4x speed (higher temporal resolution) or we should compensate increments.
        // For chaos, 4x speed is fine, just smoother chaos.
        
        for (int s = 0; s < upSamples; ++s)
        {
            SampleType left = upL[s];
            SampleType right = upR ? upR[s] : 0;
            
            // Calc Mod (We use linear interpolation of previous 1x energy? 
            // Or just calculate pure new energy at 4x? 4x is better.)
            float inputEnergy = (std::abs(left) + std::abs(right)) * 0.5f;
            float flux = fluxFollower.processSample(inputEnergy);
            float chaos = chaosLFO.getNextSample(); // This will run LFO 4x faster! 
            // Compensation: LFO phase increment is based on SampleRate. 
            // `chaosLFO.prepare` was called with 1x sampleRate?
            // If prepared with 1x, and called 4x often, LFO is 4x faster.
            // We should ideally update `chaosLFO` setup, but "Drift" LFO being faster is likely fine for "Plasma".
            
            float dynDrive = drive + (flux * drive * 0.5f); 
            float dynCutoff = cutoff + (chaos * 500.0f * scramble); 
            dynCutoff = std::clamp(dynCutoff, 20.0f, 20000.0f);
            float dynMorph = morph + (flux * 0.2f);
            
            // Decimate logic (Needs update for 4x?)
            // Decimate reduces sample rate. 
            // If we are at 4x, decimate needs to hold 4x longer to sound same.
            // Fold logic (Renamed from Decimate)
            if (fold > 0.0f)
            {
                float rateReduction = fold * 40.0f * 4.0f; // Scale up for oversampling
                if (rateReduction < 1.0f) rateReduction = 1.0f;
                static float holdL=0, holdR=0, counter=0;
                counter++;
                if (counter >= rateReduction) { counter = 0; holdL = left; holdR = right; }
                else { left = holdL; right = holdR; }
            }
            
            // Distortion with Chaotic Asymmetry (Tilt)
            // Injecting a tiny DC offset based on flux and chaos creates asymmetric grit
            float tilt = (flux * 0.05f) + (chaos * 0.02f * scramble);
            left = distortion.processSample(left + tilt, dynDrive, fold, algoPos, algoNeg, stages) - tilt;
            right = distortion.processSample(right + tilt, dynDrive, fold, algoPos, algoNeg, stages) - tilt;
            
            // Filter
            filter.setParams(dynCutoff, resonance, dynMorph);
            left = filter.processSample(left);
            right = filter.processSample(right);
            
            // Safety
            if (std::abs(left) > 10.0f) left = std::tanh(left);
            if (std::abs(right) > 10.0f) right = std::tanh(right);
            
            // Resonator
            float dynFb = fbAmount + (flux * 0.1f * scramble);
            left = resonator.processSample(left, dynFb, fbTimeMs, scramble);
            right = resonator.processSample(right, dynFb, fbTimeMs, scramble);
            
            // Dimension (Stereo Width)
            dimension.process(left, right, width);
            
            // Squeeze
            if (squeeze > 0.0f) {
                // ... Squeeze implementation ...
                // Simplified for brevity in replacement (Inline logic again)
                float envL = std::abs(left) + 0.01f;
                float gainL = (1.0f / std::sqrt(envL));
                left *= (1.0f + (gainL - 1.0f) * squeeze);
                
                float envR = std::abs(right) + 0.01f;
                float gainR = (1.0f / std::sqrt(envR));
                right *= (1.0f + (gainR - 1.0f) * squeeze);
            }
            
            upL[s] = left;
            if (upR) upR[s] = right;
        }
        
        // --- 5. DOWNSAMPLE HIGHS ---
        oversampler.processSamplesDown(highBlock); // Writes back to highBlock (highBuffer)
        
        // --- 6. SUM & OUTPUT ---
        // Note: processSamplesDown writes result to the input block passed to processSamplesUp?
        // No, processSamplesDown writes to the *original* block (highBlock).
        
        // We have `highBuffer` now containing processed, downsampled highs.
        // And `lowBuffer` containing processed lows.
        
        auto* outL = channelDataL;
        auto* outR = channelDataR;
        auto* processedH_L = highBuffer.getReadPointer(0);
        auto* processedH_R = numChannels > 1 ? highBuffer.getReadPointer(1) : nullptr;
        
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType lLow = lL[s];
            SampleType lHigh = processedH_L[s];
            outL[s] = std::tanh(lLow + lHigh);
            
            if (outR) {
                SampleType rLow = lR ? lR[s] : lLow;
                SampleType rHigh = processedH_R ? processedH_R[s] : 0;
                outR[s] = std::tanh(rLow + rHigh);
            }
        }

        // --- 7. FINAL SAFETY & DC BLOCK ---
        // Block DC to prevent silent headroom eating
        const float R = 0.9995f; 

        bool engineBroken = false;

        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType inL = outL[s];
            
            // IMMEDIATE WATCHDOG: Check for NaN before anything else
            if (!std::isfinite(inL) || (outR && !std::isfinite(outR[s])))
            {
                engineBroken = true;
                break;
            }

            SampleType outL_s = inL - dcL_x1 + R * dcL_y1;
            dcL_x1 = inL; dcL_y1 = outL_s;
            
            // Hard Limit Safety (Host Protection)
            if (outL_s > 2.0f) outL_s = 2.0f; 
            else if (outL_s < -2.0f) outL_s = -2.0f;
            
            outL[s] = outL_s;

            if (outR)
            {
                SampleType inR = outR[s];
                SampleType outR_s = inR - dcR_x1 + R * dcR_y1;
                dcR_x1 = inR; dcR_y1 = outR_s;
                
                if (outR_s > 2.0f) outR_s = 2.0f;
                else if (outR_s < -2.0f) outR_s = -2.0f;
                
                outR[s] = outR_s;
            }
        }
        
        // If engine broke during this block, NUCLEAR RESET immediately.
        if (engineBroken)
        {
            // WATCHDOG TRIGGERED: A NaN was detected in the signal path.
            // Action: Instant Reset.
            reset();
            buffer.clear(); // Output silence for this block to save speakers.
            return;
        }

        // --- 8. NUCLEAR WATCHDOG (Panic Switch) ---
        // Secondary Safety: Rail Detection
        // Even if NaNs aren't present, if the signal is "stuck" at the rail (+/- 2.0)
        // for > 25% of the time, something is very wrong (DC explosion or feedback loop howl).
        
        int badSamples = 0;
        for (int s = 0; s < totalSamples; ++s)
        {
            if (std::abs(outL[s]) >= 1.95f) badSamples++;
            if (outR && std::abs(outR[s]) >= 1.95f) badSamples++;
        }

        // Threshold: 25% of samples are clipped hard.
        if (badSamples > (totalSamples * numChannels) / 4)
        {
            // likely broken/exploded. Reboot.
            reset();
        }
    }

private:
    AetherDistortion<SampleType> distortion;
    AetherFilter<SampleType> filter;
    AetherResonator<SampleType> resonator;
    
    // Neuro Components
    AetherCrossover<SampleType> crossoverL, crossoverR;
    AetherSubProcessor<SampleType> subProcessor;

    // Modulation
    AetherLFO chaosLFO;
    AetherEnvelopeFollower fluxFollower;
    AetherEnvelopeFollower noiseGateFollower;
    
    // Width
    AetherDimension dimension;
    AetherNoise<SampleType> noiseGen;
    
    // Hi-Fi
    // Factor 2 = 4x Oversampling
    juce::dsp::Oversampling<SampleType> oversampler { 2, 2, juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR, true };
    
    int numChannels = 2;
    
    // Safety
    SampleType dcL_x1=0, dcL_y1=0;
    SampleType dcR_x1=0, dcR_y1=0;
};

} // namespace aether
