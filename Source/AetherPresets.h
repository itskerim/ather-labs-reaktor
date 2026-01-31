#pragma once

#include "AetherCommon.h"
#include <juce_audio_processors/juce_audio_processors.h>

namespace aether
{

struct PresetData
{
    juce::String name;
    float drive, stages, cutoff, res, morph, fbAmount, fbTime, mix, subLevel, squeeze;
    int algoPos, algoNeg;
};

class AetherPresets
{
public:
    static std::vector<PresetData> getFactoryPresets()
    {
        return {
            // Name                  Drv   Stg  Cut      Res   Mrph  FbA   FbT   Mix   Sub   Sqz  Pos  Neg
            { "Default / Init",      0.30f, 1,  20000.f, 0.4f, 0.2f, 0.0f, 20.f, 1.0f, 1.0f, 0.4f, 1,   1 }, // Wide Open, Instant OTT
            { "Neuro Reese",         0.65f, 4,  350.f,  0.6f, 0.7f, 0.2f, 15.f, 0.8f, 1.4f, 3,   7 }, // "Ooh" vowel, heavy sub
            { "Metallic Screech",    0.80f, 2,  1200.f, 0.8f, 0.2f, 0.7f, 8.0f, 0.7f, 1.0f, 6,   6 }, 
            { "Liquid Warmth",       0.35f, 1,  800.f,  0.2f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1,   8 }, // Tape-style sat
            { "Jump Up Wobble",      0.55f, 3,  800.f,  0.6f, 0.5f, 0.0f, 0.0f, 1.0f, 1.3f, 2,   4 }, 
            { "Techstep Growl",      0.70f, 5,  250.f,  0.7f, 0.9f, 0.3f, 25.f, 0.9f, 1.1f, 9,   3 }, 
            { "Bitcrush Bass",       1.00f, 1,  2000.f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 5,   5 }, 
            { "Vowel Talker",        0.60f, 3,  650.f,  0.8f, 0.6f, 0.4f, 12.f, 0.8f, 1.0f, 8,   1 }, // "Yoi" sound
            { "Industrial Lead",     0.90f, 6,  3000.f, 0.3f, 0.0f, 0.5f, 40.f, 0.8f, 0.8f, 7,   7 }, 
            { "Sub Focus",           0.15f, 1,  120.f,  0.1f, 0.0f, 0.0f, 0.0f, 1.0f, 1.8f, 1,   1 }, // Pure Sub
            { "Noisia Feedback",     0.50f, 3,  450.f,  0.9f, 0.3f, 0.85f, 3.5f, 0.7f, 1.1f, 4,   6 }, 
            { "Broken Speaker",      1.00f, 1,  20000.f,0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 2,   2 }  
        };
    }

    static void setParam(juce::AudioProcessorValueTreeState& apvts, const juce::String& id, float value)
    {
        if (auto* p = apvts.getParameter(id))
        {
            // Normalize if needed, but setValueNotifyingHost usually expects 0..1 for some host updates,
            // or the plain value if it's internal. JUCE `setValueNotifyingHost` takes normalized 0..1.
            // Our PresetData seems to store Real World Values (e.g. 20000Hz).
            // We must convert to 0..1.
            
            if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(p))
            {
                p->setValueNotifyingHost(floatParam->range.convertTo0to1(value));
            }
            else if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(p))
            {
                // Choice params are usually just index normalized? 
                // Actually setValueNotifyingHost takes 0..1, so index / (numChoices-1).
                // But `*choice = value` operator likely handles index assignment if exposed.
                // Let's use the explicit copyValue for safety if it's the right type.
                *choiceParam = (int)value;
            }
            else
            {
                // Fallback for others
                 p->setValueNotifyingHost(value);
            }
        }
    }

    static void loadPreset(juce::AudioProcessorValueTreeState& apvts, int index)
    {
        auto presets = getFactoryPresets();
        if (index < 0 || index >= presets.size()) return;
        
        const auto& p = presets[index];
        
        setParam(apvts, "drive", p.drive);
        
        // Stages: 1..6 mapped to 0..1 range? 
        // AudioParameterInt range: (val - min) / (max - min)
        // Min 1, Max 6. (p.stages - 1) / 5.0f
        setParam(apvts, "stages", (p.stages - 1.0f) / 5.0f);
        
        setParam(apvts, "cutoff", p.cutoff);
        setParam(apvts, "res", p.res);
        setParam(apvts, "morph", p.morph);
        setParam(apvts, "fbAmount", p.fbAmount);
        setParam(apvts, "fbTime", p.fbTime);
        setParam(apvts, "mix", p.mix);
        setParam(apvts, "subLevel", p.subLevel);
        setParam(apvts, "squeeze", p.squeeze);
        
        // Int/Choices
        if (auto* pChoice = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("algoPos")))
            *pChoice = p.algoPos;
        if (auto* pChoice = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("algoNeg")))
            *pChoice = p.algoNeg;
    }
};

} // namespace aether
