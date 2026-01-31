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
            { "INIT / REESE",        0.45f, 2,  80.f,    0.4f, 0.0f, 0.0f, 20.f, 1.0f, 1.2f, 0.5f, 1,   1 }, 
            { "CRUSHED LIQUID",      0.35f, 1,  450.f,   0.2f, 0.3f, 0.1f, 12.f, 1.0f, 1.0f, 0.3f, 8,   1 }, 
            { "TECHSTEP GROWL",      0.75f, 8,  280.f,   0.75f, 0.8f, 0.4f, 22.f, 0.9f, 1.1f, 0.7f, 3,   9 }, 
            { "METALLIC TALKER",     0.65f, 4,  1200.f,  0.85f, 0.52f, 0.75f, 6.5f, 0.8f, 1.0f, 0.6f, 6,   6 }, 
            { "FURNACE BLAST",       0.90f, 12, 5000.f,  0.3f, 0.0f, 0.2f, 150.f, 0.7f, 1.4f, 1.0f, 2,   2 }, 
            { "VOWEL GRIT AEU",      0.55f, 3,  650.f,   0.90f, 0.35f, 0.3f, 18.f, 1.0f, 1.2f, 0.4f, 4,   8 }, 
            { "SUBMERGED",           0.20f, 1,  1000.f,  0.15f, 0.0f, 0.0f, 0.f,  1.0f, 1.8f, 0.2f, 1,   1 }, 
            { "PLASMA FEEDBACK",     0.50f, 4,  4500.f,  0.95f, 0.1f,  0.95f, 2.5f, 0.6f, 1.0f, 0.8f, 9,   4 }, 
            { "BROKEN RADIO",       1.00f, 1,  20000.f, 0.0f, 0.0f, 0.0f, 0.f,  1.0f, 1.0f, 0.5f, 5,   5 }, 
            { "NEURONAL WIDTH",      0.40f, 3,  2500.f,  0.5f, 0.6f, 0.25f, 33.f, 0.8f, 1.1f, 0.6f, 1,   8 },
            { "HARSH SINEFOLD",      0.82f, 6,  20000.f, 0.2f, 0.0f, 0.15f, 45.f, 0.9f, 1.0f, 0.8f, 3,   3 },
            { "DARK MATTER",         0.60f, 4,  40.f,    0.6f, 0.95f, 0.5f, 500.f, 0.7f, 1.5f, 0.9f, 7,   7 },
            { "ULTIMA REAKTOR",      0.95f, 12, 850.f,   0.88f, 0.45f, 1.10f, 12.f, 0.8f, 1.25f, 0.85f, 3, 6 }
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
        
        // Stages: 1..12 mapped to 0..1 range
        setParam(apvts, "stages", (p.stages - 1.0f) / 11.0f);
        
        setParam(apvts, "cutoff", p.cutoff);
        setParam(apvts, "res", p.res);
        setParam(apvts, "morph", p.morph);
        setParam(apvts, "fbAmount", p.fbAmount);
        setParam(apvts, "fbTime", p.fbTime);
        setParam(apvts, "mix", p.mix);
        setParam(apvts, "sub", p.subLevel);
        setParam(apvts, "squeeze", p.squeeze);
        
        // Int/Choices
        if (auto* pChoice = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("algoPos")))
            *pChoice = p.algoPos;
        if (auto* pChoice = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("algoNeg")))
            *pChoice = p.algoNeg;
    }
};

} // namespace aether
