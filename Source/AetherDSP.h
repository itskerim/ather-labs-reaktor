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

// 3-Band EQ for Texture Shaping
template <typename SampleType>
class AetherEQ
{
public:
    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        for (auto& b : bands)
        {
            b.filter.prepare(spec);
            // Initialize with flat coefficients to prevent null-dereference on first process
            b.filter.coefficients = juce::dsp::IIR::Coefficients<SampleType>::makeAllPass(spec.sampleRate, 1000.0f);
        }
    }
    
    void reset()
    {
        for (auto& b : bands)
            b.filter.reset();
    }
    
    struct Band {
        juce::dsp::IIR::Filter<SampleType> filter;
    };
    
    Band bands[3]; // Low, Mid, High
    
    void updateBands(float lowDb, float midDb, float highDb, double rate)
    {
        auto lowCoeffs = juce::dsp::IIR::Coefficients<SampleType>::makeLowShelf(rate, 250.0f, 0.707f, juce::Decibels::decibelsToGain(lowDb));
        auto midCoeffs = juce::dsp::IIR::Coefficients<SampleType>::makePeakFilter(rate, 1200.0f, 0.5f, juce::Decibels::decibelsToGain(midDb));
        auto highCoeffs = juce::dsp::IIR::Coefficients<SampleType>::makeHighShelf(rate, 5000.0f, 0.707f, juce::Decibels::decibelsToGain(highDb));

        if (lowCoeffs) bands[0].filter.coefficients = lowCoeffs;
        if (midCoeffs) bands[1].filter.coefficients = midCoeffs;
        if (highCoeffs) bands[2].filter.coefficients = highCoeffs;
    }
    
    SampleType process(SampleType sample)
    {
        sample = bands[0].filter.processSample(sample);
        sample = bands[1].filter.processSample(sample);
        sample = bands[2].filter.processSample(sample);
        return sample;
    }
};

// Clean Sub Processor (Mono Sum + Warmth)
template <typename SampleType>
class AetherSubProcessor
{
public:
    void process(SampleType& left, SampleType& right, float subLevel)
    {
        // Mono Sum
        SampleType mono = (left + right) * 0.5f;
        
        // Output to both channels (Linear / Clean)
        // Allow +6dB boost
        left = mono * subLevel * 2.0f;
        right = mono * subLevel * 2.0f;
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
        currentSampleRate = spec.sampleRate;
        
        // --- 16K BUFFER FORTRESS ---
        // We pre-allocate everything to a massive 16384 samples.
        // This is the largest block size any reasonable DAW will ever send.
        // We NEVER resize these in the process loop.
        const int fortressSize = 16384; 
        maxSamplesPerBlock = fortressSize;

        // 4x Oversampling
        oversampler.initProcessing(fortressSize);
        oversampler.reset();

        juce::dsp::ProcessSpec oversampledSpec = spec;
        oversampledSpec.sampleRate = spec.sampleRate * 4.0;
        oversampledSpec.maximumBlockSize = fortressSize * 4;
        
        distortion.prepare(oversampledSpec);
        noiseDistortionUnit.prepare(oversampledSpec);
        filter.prepare(oversampledSpec);
        resonator.prepare(oversampledSpec);
        
        auto hpfCoeffs = juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(spec.sampleRate * 4.0, 20.0f);
        distLowCutL.prepare(oversampledSpec);
        distLowCutR.prepare(oversampledSpec);
        *distLowCutL.coefficients = *hpfCoeffs;
        *distLowCutR.coefficients = *hpfCoeffs;
        
        crossoverL.prepare(spec);
        crossoverR.prepare(spec);
        
        chaosLFO.prepare(spec.sampleRate);
        chaosLFO.setParams(0.002f, AetherLFO::Waveform::Drift);
        fluxFollower.prepare(spec.sampleRate);
        fluxFollower.setParams(10.0f, 300.0f);
        
        noiseGateFollower.prepare(spec.sampleRate);
        noiseGateFollower.setParams(5.0f, 10.0f); 
        
        dimension.prepare(oversampledSpec);

        fluxBuffer.assign(fortressSize, 0.0f);
        gateBuffer.assign(fortressSize, 0.0f);
        
        highBuffer.setSize(2, fortressSize, false, true, true);
        lowBuffer.setSize(2, fortressSize, false, true, true);
        
        noiseGen.prepare(oversampledSpec.sampleRate);
        
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
        noiseDistortionUnit.reset();
        filter.reset();
        resonator.reset();
        distLowCutL.reset();
        distLowCutR.reset();
        noiseGen.prepare((double)currentSampleRate * 4.0);
        dimension.reset();
        
        // Clear DC States
        dcL_x1 = 0; dcL_y1 = 0;
        dcR_x1 = 0; dcR_y1 = 0;
        
        // Reset Fold state
        foldL = 0; foldR = 0; foldCounter = 0;
    }

    void process(juce::AudioBuffer<SampleType>& buffer, 
                 float drive, float blend, int stages, 
                 DistortionAlgo algoPos, DistortionAlgo algoNeg,
                 float cutoff, float resonance, float morph,
                 float fbAmount, float fbTimeMs, float scramble,
                 float subLevel, float lowCutFreq, double bpm,
                 float squeeze, float xoverFreq, float fold, bool vowelMode,
                 float noiseLevel, float noiseWidth, int noiseType, bool noiseSolo,
                 float width)
    {
        juce::ScopedNoDenormals noDenormals;
        
        auto totalSamples = buffer.getNumSamples();
        if (totalSamples == 0 || currentSampleRate <= 0) return;

        // --- BLOCK SIZE FORTRESS ---
        // If a DAW ever sends a block larger than 16k, we return early 
        // to prevent a buffer overflow crash. (16k is already 10x larger than standard)
        if (totalSamples > 16384) return;

        auto* channelDataL = buffer.getWritePointer(0);
        auto* channelDataR = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;
        
        chaosLFO.setBPM(bpm);
        auto nType = static_cast<typename AetherNoise<SampleType>::NoiseType>(noiseType);
        
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType left_in = channelDataL[s];
            SampleType right_in = channelDataR ? channelDataR[s] : left_in;
            float energy = (std::abs(left_in) + std::abs(right_in)) * 0.5f;
            
            gateBuffer[s] = noiseGateFollower.processSample(energy);
            fluxBuffer[s] = fluxFollower.processSample(energy);
        }

        if (vowelMode) filter.setType(AetherFilter<SampleType>::FilterType::Formant);
        else filter.setType(AetherFilter<SampleType>::FilterType::Morph);

        // Variable Crossover
        crossoverL.setCutoff(xoverFreq);
        if (channelDataR) crossoverR.setCutoff(xoverFreq);
        
        // Update Distortion Low-Cut
        auto hpfCoeffs = juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(currentSampleRate * 4.0, std::clamp(lowCutFreq, 20.0f, 2000.0f));
        *distLowCutL.coefficients = *hpfCoeffs;
        *distLowCutR.coefficients = *hpfCoeffs;

        // Prep Highs & Lows
        highBuffer.makeCopyOf(buffer, true); // true to avoid resizing if possible
        
        auto* hL = highBuffer.getWritePointer(0);
        auto* hR = numChannels > 1 ? highBuffer.getWritePointer(1) : nullptr;
        auto* lL = lowBuffer.getWritePointer(0);
        auto* lR = numChannels > 1 ? lowBuffer.getWritePointer(1) : nullptr;
        
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType lL_out, hL_out, lR_out, hR_out;
            crossoverL.process(hL[s], lL_out, hL_out);
            lL[s] = lL_out; hL[s] = hL_out;
            if (hR) {
                crossoverR.process(hR[s], lR_out, hR_out);
                lR[s] = lR_out; hR[s] = hR_out;
            }
        }
        
        for (int s = 0; s < totalSamples; ++s)
        {
            SampleType sl = lL[s], sr = lR ? lR[s] : sl;
            subProcessor.process(sl, sr, subLevel);
            lL[s] = sl; if (lR) lR[s] = sr;
        }

        juce::dsp::AudioBlock<SampleType> highBlock(highBuffer);
        juce::dsp::AudioBlock<SampleType> upsampledBlock = oversampler.processSamplesUp(highBlock);
        auto* upL = upsampledBlock.getChannelPointer(0);
        auto* upR = numChannels > 1 ? upsampledBlock.getChannelPointer(1) : nullptr;
        int upSamples = (int)upsampledBlock.getNumSamples();
        
        eq.updateBands(eqLow, eqMid, eqHigh, currentSampleRate * 4.0);

        for (int s = 0; s < upSamples; ++s)
        {
            SampleType& left = upL[s];
            SampleType& right = upR ? upR[s] : left;
            int idx = std::min(s / 4, (int)totalSamples - 1);
            float flux = fluxBuffer[idx];
            float gateEnv = gateBuffer[idx];
            
            // 1. Distortion Low Cut
            left = distLowCutL.processSample(left);
            if (upR) right = distLowCutR.processSample(right);

            float chaos = chaosLFO.getNextSample();
            float dynDrive = drive + (flux * drive * 0.2f); 
            float dynCutoff = std::clamp(cutoff + (chaos * 500.0f * scramble), 20.0f, 20000.0f);
            float dynMorph = morph + (flux * 0.2f);
            
            if (fold > 0.0f)
            {
                float rateReduction = std::max(1.0f, fold * 160.0f);
                if (++foldCounter >= (int)rateReduction) { 
                    foldCounter = 0; 
                    foldL = left; 
                    foldR = right; 
                }
                else { left = foldL; right = foldR; }
            }
            
            // Main Distortion with Squeeze Bias
            float bias = squeeze * 0.4f; 
            left = distortion.processSample(left, dynDrive, bias + fold, algoPos, algoNeg, stages);
            right = distortion.processSample(right, dynDrive, bias + fold, algoPos, algoNeg, stages);
            
            SampleType nL = 0, nR = 0;
            noiseGen.process(nL, nR, noiseLevel, 0.0f, nType, gateEnv);
            
            float crunchPhDelta = 150.0f / ((float)currentSampleRate * 4.0f);
            crunchPhase += crunchPhDelta;
            if (crunchPhase >= 1.0f) crunchPhase -= 1.0f;
            float sq = (crunchPhase < 0.5f) ? 1.0f : 0.05f;
            float crunchMod = juce::jmap(noiseWidth, 0.0f, 1.0f, 1.0f, sq);
            nL *= crunchMod; nR *= crunchMod;

            float boostedDrive = noiseWidth * 2.5f; 
            float nBoost = 1.0f + (noiseWidth * noiseWidth * noiseWidth * 3.0f);
            nL *= nBoost; nR *= nBoost;

            if (boostedDrive > 0.01f) {
                 nL = noiseDistortionUnit.processSample(nL, boostedDrive, fold, algoPos, algoNeg, stages);
                 nR = noiseDistortionUnit.processSample(nR, boostedDrive, fold, algoPos, algoNeg, stages);
            }
            
            if (noiseSolo) { left = nL; right = nR; }
            else { left += nL; right += nR; }

            filter.setParams(dynCutoff, resonance, dynMorph);
            left = filter.processSample(left);
            right = filter.processSample(right);
            
            if (std::abs(left) > 10.0f) left = std::tanh(left);
            if (std::abs(right) > 10.0f) right = std::tanh(right);
            
            float dynFb = fbAmount + (flux * 0.1f * scramble);
            left = resonator.processSample(left, dynFb, fbTimeMs, scramble);
            right = resonator.processSample(right, dynFb, fbTimeMs, scramble);
            
            upL[s] = left;
            if (upR) upR[s] = right;
        }
        
        oversampler.processSamplesDown(highBlock);
        
        for (int s = 0; s < totalSamples; ++s)
        {
            channelDataL[s] = std::tanh(lL[s] + highBuffer.getSample(0, s));
            if (channelDataR)
                channelDataR[s] = std::tanh((lR ? lR[s] : lL[s]) + highBuffer.getSample(1, s));
        }

        const float R = 0.9995f; 
        bool broken = false;
        for (int s = 0; s < totalSamples; ++s) {
            if (!std::isfinite(channelDataL[s])) { broken = true; break; }
            SampleType outL_s = channelDataL[s] - dcL_x1 + R * dcL_y1;
            dcL_x1 = channelDataL[s]; dcL_y1 = outL_s;
            channelDataL[s] = std::clamp(outL_s, -2.0f, 2.0f);

            if (channelDataR) {
                if (!std::isfinite(channelDataR[s])) { broken = true; break; }
                SampleType outR_s = channelDataR[s] - dcR_x1 + R * dcR_y1;
                dcR_x1 = channelDataR[s]; dcR_y1 = outR_s;
                channelDataR[s] = std::clamp(outR_s, -2.0f, 2.0f);
            }
        }
        if (broken) reset();

        // --- FINAL WIDTH IMAGING ---
        // Mid/Side decorrelation for massive stereo spread
        if (channelDataR && width > 0.001f)
        {
            for (int s = 0; s < totalSamples; ++s)
            {
                dimension.process(channelDataL[s], channelDataR[s], width);
            }
        }
    }

private:
    AetherDistortion<SampleType> distortion;
    AetherDistortion<SampleType> noiseDistortionUnit; // Dedicated "Crunch" Reactor
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
    
    // Distortion Stage High Pass
    juce::dsp::IIR::Filter<SampleType> distLowCutL, distLowCutR;

    // Control Buffers
    std::vector<float> fluxBuffer;
    std::vector<float> gateBuffer;
    
    juce::AudioBuffer<SampleType> highBuffer;
    juce::AudioBuffer<SampleType> lowBuffer;
    
    // Hi-Fi
    // Factor 2 = 4x Oversampling
    juce::dsp::Oversampling<SampleType> oversampler { 2, 2, juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR, true };
    
    int numChannels = 2;
    double currentSampleRate = 48000.0;
    size_t maxSamplesPerBlock = 512;
    float crunchPhase = 0.0f;
    
    // Safety
    SampleType dcL_x1=0, dcL_y1=0;
    SampleType dcR_x1=0, dcR_y1=0;

    // Fold state (Track isolated)
    SampleType foldL=0, foldR=0;
    int foldCounter=0;
};

} // namespace aether
