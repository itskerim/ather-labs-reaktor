#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AetherPresets.h"

PhatRackAudioProcessorEditor::PhatRackAudioProcessorEditor (AetherAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      posSelector("POSITIVE", p.apvts, "algoPos"),
      negSelector("NEGATIVE", p.apvts, "algoNeg"),
      tooltipWindow(this, 700)
{
    // Apply AETHER Global LookAndFeel
    setLookAndFeel(&aetherLF);

    // --- 1. Branding & Header ---
    // Custom Logo is drawn in paint()
    
    // Presets
    addAndMakeVisible(presetSelector);
    auto presets = aether::AetherPresets::getFactoryPresets();
    presetSelector.addItem("INIT / MANUAL", 1);
    presetSelector.addSeparator();
    for (int i = 0; i < presets.size(); ++i)
        presetSelector.addItem(presets[i].name, i + 2);
    
    presetSelector.onChange = [this] {
        int id = presetSelector.getSelectedId();
        if (id > 1) {
            aether::AetherPresets::loadPreset(audioProcessor.apvts, id - 2);
            // Force slider updates
            repaint();
        }
    };
    presetSelector.setText("FACTORY PRESETS");
    
    // Help Button
    addAndMakeVisible(helpButton);
    helpButton.setClickingTogglesState(true);
    helpButton.setToggleState(true, juce::dontSendNotification); // Default ON
    helpButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00d4ff)); // Cyan when Active
    helpButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    helpButton.setTooltip("Turn this ON to see helpful descriptions when you hover over any control. Turn it OFF to hide the pop-up tips.");
    
    helpButton.onClick = [this] {
        tooltipsEnabled = helpButton.getToggleState();
        // Global toggle logic: TooltipWindow doesn't have an 'active' state easily,
        // so we'll just check this flag in timer or handle it via individual comp tooltips.
        // For simplicity, we just change the button color to show it's active.
    };

    // --- 2. Central Stage ---
    // --- 2. Central Stage ---
    addAndMakeVisible(orb);     // Middle (The Orb)
    addAndMakeVisible(osc);     // Bottom
    addAndMakeVisible(transferVis);
    addAndMakeVisible(posSelector);
    addAndMakeVisible(negSelector);
    posSelector.setTitleVisible(false);
    negSelector.setTitleVisible(false);
    addAndMakeVisible(logo);

    // --- 3. Primary Distortion Controls ---
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    driveSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d4ff));
    addAndMakeVisible(driveSlider);
    driveAtt = std::make_unique<Attachment>(audioProcessor.apvts, "drive", driveSlider);
    driveLabel.setText("DISTORT", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);

    lowCutSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowCutSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lowCutSlider);
    lowCutAtt = std::make_unique<Attachment>(audioProcessor.apvts, "lowCut", lowCutSlider);
    lowCutLabel.setText("LOW CUT", juce::dontSendNotification);
    lowCutLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lowCutLabel);

    lowCutSlider.onValueChange = [this] {
        if (lowCutSlider.isMouseOverOrDragging())
        {
            float val = (float)lowCutSlider.getValue();
            juce::String freqStr;
            if (val >= 1000.0f) freqStr = juce::String(val / 1000.0f, 2) + " kHz";
            else freqStr = juce::String((int)val) + " Hz";
            lowCutLabel.setText(freqStr, juce::dontSendNotification);
        }
    };
    lowCutSlider.onDragStart = [this] { lowCutSlider.onValueChange(); };
    lowCutSlider.onDragEnd = [this] { lowCutLabel.setText("LOW CUT", juce::dontSendNotification); };

    foldSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    foldSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(foldSlider);
    foldAtt = std::make_unique<Attachment>(audioProcessor.apvts, "fold", foldSlider);
    foldLabel.setText("FOLD", juce::dontSendNotification);
    foldLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(foldLabel);

    stagesReactor.onValueChanged = [this](int val) {
        if (auto* p = audioProcessor.apvts.getParameter("stages"))
            p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1((float)val));
    };
    addAndMakeVisible(stagesReactor);
    
    stagesLabel.setText("12-STAGE REACTOR", juce::dontSendNotification);
    stagesLabel.setVisible(false); // Label removed per user request
    addAndMakeVisible(stagesLabel);

    // --- 4. Morphing Filter Section ---
    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    cutoffSlider.setLookAndFeel(&industrialKnobLF); // Use premium style
    addAndMakeVisible(cutoffSlider);
    cutoffAtt = std::make_unique<Attachment>(audioProcessor.apvts, "cutoff", cutoffSlider);
    
    cutoffLabel.setText("CUTOFF", juce::dontSendNotification);
    addAndMakeVisible(cutoffLabel);

    resSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    resSlider.setLookAndFeel(&industrialKnobLF);
    addAndMakeVisible(resSlider);
    resAtt = std::make_unique<Attachment>(audioProcessor.apvts, "res", resSlider);

    morphSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    morphSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    morphSlider.setLookAndFeel(&industrialKnobLF);
    addAndMakeVisible(morphSlider);
    morphSlider.setLookAndFeel(&industrialKnobLF);
    addAndMakeVisible(morphSlider);
    morphAtt = std::make_unique<Attachment>(audioProcessor.apvts, "morph", morphSlider);
    
    morphLabel.setText("MORPH", juce::dontSendNotification);
    morphLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(morphLabel);
    
    // Filter Mode Button
    addAndMakeVisible(filterModeBtn);
    filterModeBtn.setClickingTogglesState(true);
    filterModeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27272a));
    filterModeBtn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00d4ff)); // Cyan when Vowel
    filterModeBtn.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    filterModeAtt = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "filterMode", filterModeBtn);
    
    // Update Text on Click
    filterModeBtn.onClick = [this] {
        if (filterModeBtn.getToggleState()) filterModeBtn.setButtonText("VOWEL");
        else filterModeBtn.setButtonText("MORPH");
    };
    // Initialize text
    if ((int)audioProcessor.apvts.getRawParameterValue("filterMode")->load() > 0) filterModeBtn.setButtonText("VOWEL");

    resLabel.setText("RESONANCE", juce::dontSendNotification);
    resLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resLabel);

    // --- 5. Feedback / Resonator ---
    fbAmountSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    fbAmountSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(fbAmountSlider);
    fbAmountAtt = std::make_unique<Attachment>(audioProcessor.apvts, "fbAmount", fbAmountSlider);

    fbTimeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    fbTimeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(fbTimeSlider);
    addAndMakeVisible(fbTimeSlider);
    fbTimeAtt = std::make_unique<Attachment>(audioProcessor.apvts, "fbTime", fbTimeSlider);
    
    fbAmountLabel.setText("FEEDBACK", juce::dontSendNotification);
    fbAmountLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fbAmountLabel);

    fbTimeLabel.setText("TIME", juce::dontSendNotification);
    fbTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(fbTimeLabel);

    // --- 6. Experimental (Now Fold) ---
    foldSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    foldSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(foldSlider);
    foldAtt = std::make_unique<Attachment>(audioProcessor.apvts, "fold", foldSlider);
    foldLabel.setText("FOLD", juce::dontSendNotification);
    foldLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(foldLabel);

    // --- Noise Engine ---
    noiseLevelSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    noiseLevelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(noiseLevelSlider);
    noiseLevelAtt = std::make_unique<Attachment>(audioProcessor.apvts, "noiseLevel", noiseLevelSlider);
    noiseLevelLabel.setText("NOISE", juce::dontSendNotification);
    noiseLevelLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noiseLevelLabel);

    noiseWidthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    noiseWidthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(noiseWidthSlider);
    noiseWidthAtt = std::make_unique<Attachment>(audioProcessor.apvts, "noiseWidth", noiseWidthSlider);
    noiseWidthLabel.setText("CRUNCH", juce::dontSendNotification);
    noiseWidthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noiseWidthLabel);

    // Noise Sub Mod Slider Removed (Plan: EQ instead)

    noiseTypeSelector.addItem("WHITE", 1);
    noiseTypeSelector.addItem("PINK", 2);
    noiseTypeSelector.addItem("CRACK", 3);
    noiseTypeSelector.addItem("CUSTOM", 4);
    addAndMakeVisible(noiseTypeSelector);
    noiseTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "noiseType", noiseTypeSelector);
    
    // Load Custom Noise Button
    addAndMakeVisible(loadNoiseButton);
    loadNoiseButton.setTooltip("Load Custom Noise Sample (WAV/AIF)");
    loadNoiseButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27272a));
    loadNoiseButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Select Noise Sample...",
                                                          juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                                          "*.wav;*.aif;*.aiff;*.mp3");
        auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                audioProcessor.loadCustomNoise(file);
                // Force selector to "Custom" (ID 4)
                if (auto* p = audioProcessor.apvts.getParameter("noiseType"))
                    p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(3.0f)); // Index 3 = 4th item
            }
        });
    };
    
    // Noise Solo Button
    addAndMakeVisible(noiseSoloBtn);
    noiseSoloBtn.setTooltip("Solo the Texture (Distorted Synth + Noise). Mutes Dry and Sub Bass.");
    noiseSoloAtt = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "noiseSolo", noiseSoloBtn);

    spaceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    spaceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(spaceSlider);
    spaceAtt = std::make_unique<Attachment>(audioProcessor.apvts, "scramble", spaceSlider);
    spaceLabel.setText("SPACE", juce::dontSendNotification);
    spaceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(spaceLabel);

    // --- SQUEEZE & X-OVER (Restored to Deck) ---
    squeezeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    squeezeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(squeezeSlider);
    squeezeAtt = std::make_unique<Attachment>(audioProcessor.apvts, "squeeze", squeezeSlider);
    squeezeLabel.setText("SQUEEZE", juce::dontSendNotification);
    squeezeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(squeezeLabel);
    
    xoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    xoverSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(xoverSlider);
    xoverAtt = std::make_unique<Attachment>(audioProcessor.apvts, "xover", xoverSlider);
    xoverLabel.setText("X-OVER", juce::dontSendNotification);
    xoverLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(xoverLabel);

    // --- 7. Output ---
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(outputSlider);
    outputAtt = std::make_unique<Attachment>(audioProcessor.apvts, "output", outputSlider);

    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(mixSlider);
    mixAtt = std::make_unique<Attachment>(audioProcessor.apvts, "mix", mixSlider);

    outputLabel.setText("GAIN", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputLabel);
    
    // Sub Control
    subSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    subSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addChildComponent(subSlider); // Add but potentially manage layout in resized
    subSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d4ff));
    addAndMakeVisible(subSlider);
    subAtt = std::make_unique<Attachment>(audioProcessor.apvts, "sub", subSlider);
    
    subLabel.setText("SUB", juce::dontSendNotification);
    subLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subLabel);

    // --- 7. Output & Width ---
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(widthSlider);
    widthAtt = std::make_unique<Attachment>(audioProcessor.apvts, "width", widthSlider);
    widthLabel.setText("WIDTH", juce::dontSendNotification);
    widthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(widthLabel);

    // --- 8. UX Tooltips (user-friendly, in plain language) ---
    driveSlider.setTooltip("How hard you're pushing the sound into the effect. Turn it up for more grit, crunch, and saturation; keep it lower for a lighter, cleaner tone.");
    foldSlider.setTooltip("Bends the loudest parts of the sound back on themselves instead of chopping them off. Creates hollow, vocal-like tones and extra harmonics—great for \"talking\" or synthy textures.");
    stagesReactor.setTooltip("How many times the sound gets processed in a row (1–12). More stages = thicker, heavier, more broken-up distortion. Start low and increase for intensity.");
    
    cutoffSlider.setTooltip("The frequency where the filter starts working. Move it left for a darker, muffled sound; move it right for brighter, more open tone. Like a tone knob that focuses on a specific range.");
    resSlider.setTooltip("Emphasizes the frequencies right around the cutoff. Low = smooth and natural; high = whistling, ringing, or growling (especially in Vowel mode). Use carefully for character.");
    morphSlider.setTooltip("Sweeps between different filter shapes (dark to bright) or between vowel sounds (A, E, I, O, U) when Vowel mode is on. Lets you shape the tone in one motion.");
    
    fbAmountSlider.setTooltip("Sends some of the processed sound back into the effect. A little adds body and sustain; a lot can create metallic ringing, screaming tones, or wild resonance. Experiment to find the sweet spot.");
    fbTimeSlider.setTooltip("How long the delay is in the feedback loop (in milliseconds). Short = comb-like, metallic texture; long = stretched, echo-like resonance. Works together with Feedback amount.");
    spaceSlider.setTooltip("Adds a sense of space and diffusion to the feedback—like a small room or tank. Makes the resonance feel more enclosed and dense rather than a single sharp tone.");
    
    noiseLevelSlider.setTooltip("Adds hiss or crackle into the sound so the distortion has something extra to chew on. Great for texture, grit, and high-end sizzle. Turn up to taste.");
    noiseWidthSlider.setTooltip("How wide the added noise is in the stereo field. More width = more spread between left and right; less = more centered. Affects how big the texture feels.");
    noiseTypeSelector.setTooltip("The kind of noise: White = even, flat hiss; Pink = warmer, softer hiss; Crackle = tiny pops and grit. Pick what fits your sound.");
    
    subSlider.setTooltip("A clean, solid low-end (bass) that stays in the center. Keeps the bottom end clear and punchy while the rest of the sound can be heavily distorted.");
    xoverSlider.setTooltip("The frequency where the sound is split between the clean sub-bass and the distorted texture. 60Hz to 800Hz.");
    squeezeSlider.setTooltip("Tightens the distortion by adding internal bias, making it feel more compressed and aggressive.");
    lowCutSlider.setTooltip("High-pass filter for the distortion stage. Cuts the mud before it hits the reactor.");
    widthSlider.setTooltip("Adjusts the stereo width of the distortion part of the sound. 0% = Mono; 100% = Normal; 200% = Extra wide.");

    mixLabel.setText("DRY/WET", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    // UX Improvements: Fixed Size (Resizing breaks model)
    setResizable(false, false);
    setSize (1000, 700);
    
    startTimerHz(60); // 60 FPS for smooth Orb animation
}

PhatRackAudioProcessorEditor::~PhatRackAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PhatRackAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. Deep Void Background (Solid Dark)
    g.fillAll(juce::Colour(0xff050505)); 
    
    // --- Subtle Cyber Grid (Holodeck) ---
    float gridSz = 40.0f;
    
    // Grid alignment correction
    int startY = 80; 
    
    // BRANDING: "REAKTOR" (Behind everything)
    // User Request: "3% opacity" -> "1% opacity", "20% smaller", "Changes color with morph"
    
    // 1. Calculate Dynamic Color (Replicating Orb Logic for consistency)
    auto& apvts = audioProcessor.apvts;
    
    auto getParamSafe = [&](juce::String id) -> float {
        if (auto* p = apvts.getRawParameterValue(id)) return p->load();
        return 0.0f;
    };

    float morph = getParamSafe("morph");
    float cutoff = getParamSafe("cutoff");
    float res = getParamSafe("res");

    float safeCutoff = std::max(20.0f, cutoff);
    float normCutoff = (std::log(safeCutoff) - std::log(80.0f)) / (std::log(20000.0f) - std::log(80.0f));
    normCutoff = std::clamp(normCutoff, 0.0f, 1.0f);

    // Warm (Cyan) -> Cool (Purple)
    juce::Colour warm = juce::Colour::fromHSV(0.5f + (1.0f - normCutoff) * 0.05f, 0.85f, 0.9f + res*0.1f, 1.0f);
    juce::Colour cool = juce::Colour::fromHSV(0.78f + normCutoff * 0.1f, 0.85f, 0.9f, 1.0f);
    
    juce::Colour brandCol = warm.interpolatedWith(cool, morph);
    
    g.setColour(brandCol.withAlpha(0.01f)); // 1% Opacity (Super faint)
    
    // 20% smaller than 250 = 200
    g.setFont(juce::Font("Futura", 200.0f, juce::Font::bold)); 
    g.drawText("REAKTOR", getLocalBounds(), juce::Justification::centred, true);

    // --- Subtle Cyber Grid (Holodeck) ---
    g.setColour(juce::Colours::white.withAlpha(0.10f));

    for (float x = 0; x < getWidth(); x += gridSz)
        g.drawVerticalLine((int)x, (float)startY, (float)getHeight());
    for (float y = startY; y < getHeight(); y += gridSz)
        g.drawHorizontalLine((int)y, 0.0f, (float)getWidth());

    // Header Accent (Subtle Gloss)
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRect(0, 0, getWidth(), 80);
    g.setColour(juce::Colour(0xff1a1a1a));
    g.drawHorizontalLine(79, 0, (float)getWidth());
}

void PhatRackAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    // --- 1. HEADER (Top 80px to match paint) ---
    auto header = area.removeFromTop(80);
    
    auto helpArea = header.removeFromRight(50).reduced(10);
    helpButton.setBounds(helpArea);
    
    auto presetArea = header.removeFromRight(200).reduced(15);
    presetSelector.setBounds(presetArea);
    
    logo.setBounds(header.removeFromLeft(300).reduced(10));
    
    // --- 2. FOOTER / DECK (Bottom 100px) ---
    // --- 2. FOOTER / DECK (Bottom 135px) ---
    // Lifted up to give spacing from bottom
    auto deck = area.removeFromBottom(135);
    
    // Deck Layout: [Sub] [X-Over] [Squeeze] [Width] [Out] [Mix]
    // 6 Knobs
    int knobSize = juce::jmin(80, deck.getHeight() - 30);
    int gap = 15;
    int groupGap = 40;
    
    int totalDeckW = (knobSize * 6) + (gap * 5) + (groupGap * 2);
    int startX = deck.getCentreX() - (totalDeckW / 2);
    // Lift knobs slightly higher in the deck area
    int y = deck.getCentreY() - (knobSize / 2) - 5; 
    
    auto placeDeckKnob = [&](juce::Slider& s, juce::Label& l, int& x) {
        s.setBounds(x, y, knobSize, knobSize);
        l.setBounds(x, s.getBottom() - 10, knobSize, 20);
        x += knobSize + gap;
    };
    
    int currentX = startX;
    
    // Group 1: Split & Tone
    placeDeckKnob(subSlider, subLabel, currentX);
    placeDeckKnob(xoverSlider, xoverLabel, currentX);
    
    currentX += groupGap; // Gap
    
    // Group 2: Texture & Imaging
    placeDeckKnob(squeezeSlider, squeezeLabel, currentX);
    placeDeckKnob(widthSlider, widthLabel, currentX);
    
    currentX += groupGap; // Gap
    
    // Group 3: Master
    placeDeckKnob(outputSlider, outputLabel, currentX);
    placeDeckKnob(mixSlider, mixLabel, currentX);

    // --- 3. OSCILLOSCOPE (Bottom Underlay) ---
    // User Request: "More visible and 100px height"
    osc.setBounds(0, getHeight() - 100, getWidth(), 100);
    
    // Z-Order Management
    // 1. Osc to back (index 0)
    // 2. Orb to back (index 0 implies Osc becomes index 1) -> Orb is BEHIND Osc.
    // Result: [BG -> Orb -> Osc -> Knobs]
    osc.toBack();
    orb.toBack(); 

    // --- 4. MAIN WORKSPACE ---
    auto grid = area.reduced(10);
    
    // ORB BACKGROUND (Full Screen)
    // Restore bounds so it's visible!
    orb.setBounds(getLocalBounds());
    
    // Calculate Column Widths based on remaining space
    int colWidth = 110; 
    
    auto leftCol = grid.removeFromLeft(colWidth * 2); 
    auto rightCol = grid.removeFromRight(colWidth * 2);
    
    // CENTER OVERLAYS
    // We still need 'centerArea' reference for positioning the Morph slider and Viz
    auto centerArea = grid;
    
    // Use centerArea (which is the middle column space) for X alignment.
    int centerTopY = centerArea.getCentreY() - 170; // Hardcoded shift up to clear the Orb center
    
    // Morph Slider
    morphSlider.setBounds(centerArea.getCentreX() - 40, centerTopY - 60, 80, 80);
    morphLabel.setBounds(morphSlider.getX(), morphSlider.getBottom() - 10, 80, 20);
    
    // Selectors above Morph
    int selW = 90; int selH = 20;
    int selY = morphSlider.getY() - selH - 5;
    posSelector.setBounds(centerArea.getCentreX() - selW - 2, selY, selW, selH);
    negSelector.setBounds(centerArea.getCentreX() + 2, selY, selW, selH);
    
    // Transfer Vis above Selectors
    transferVis.setBounds(centerArea.getCentreX() - 60, selY - 50, 120, 45);

    // --- KNOB COLUMNS (Using Flex or manual relative logic) ---
    int knobH = 90;
    int kGap = 10;
    
    // Helper
    auto placeRow = [&](juce::Rectangle<int>& col, juce::Slider& s1, juce::Label& l1, juce::Slider* s2=nullptr, juce::Label* l2=nullptr) {
        auto row = col.removeFromTop(knobH + 20);
        int itemW = row.getWidth() / (s2 ? 2 : 1);
        
        s1.setBounds(row.getX() + (itemW - knobH)/2, row.getY(), knobH, knobH);
        l1.setBounds(s1.getX(), s1.getBottom() - 12, knobH, 20);
        
        if (s2 && l2) {
            s2->setBounds(row.getX() + itemW + (itemW - knobH)/2, row.getY(), knobH, knobH);
            l2->setBounds(s2->getX(), s2->getBottom() - 12, knobH, 20);
        }
    };
    
    // --- LEFT COLUMN (Distortion Section) ---
    // Row 1 (Top): Fold | Crunch
    auto row1 = leftCol.removeFromTop(knobH + 15);
    int itemW1 = row1.getWidth() / 2;
    foldSlider.setBounds(row1.getX() + (itemW1 - knobH)/2, row1.getY(), knobH, knobH);
    foldLabel.setBounds(foldSlider.getX(), foldSlider.getBottom() - 12, knobH, 20);
    
    noiseWidthSlider.setBounds(row1.getX() + itemW1 + (itemW1 - knobH)/2, row1.getY(), knobH, knobH);
    noiseWidthLabel.setBounds(noiseWidthSlider.getX(), noiseWidthSlider.getBottom() - 12, knobH, 20);
    
    // Row 2 (Middle - Prominent): Noise | Distort | Low Cut
    // We fit 3 knobs here, so we use a slightly smaller knob size for bounds.
    auto row2 = leftCol.removeFromTop(knobH + 15);
    int itemW2 = row2.getWidth() / 3;
    int midKnobSize = 75; // Slightly smaller to fit 3 in 220px
    
    auto placeMidKnob = [&](juce::Slider& s, juce::Label& l, int x) {
        s.setBounds(x + (itemW2 - midKnobSize)/2, row2.getY(), midKnobSize, midKnobSize);
        l.setBounds(s.getX(), s.getBottom() - 12, midKnobSize, 20);
    };
    
    placeMidKnob(noiseLevelSlider, noiseLevelLabel, row2.getX());
    placeMidKnob(driveSlider, driveLabel, row2.getX() + itemW2);
    placeMidKnob(lowCutSlider, lowCutLabel, row2.getX() + itemW2 * 2);
    
    // Row 3: Noise Type (Now lower)
    auto noiseRow = leftCol.removeFromTop(30);
    noiseTypeSelector.setBounds(noiseRow.removeFromLeft(noiseRow.getWidth() - 60).reduced(5, 0));
    loadNoiseButton.setBounds(noiseRow.removeFromLeft(30).reduced(2));
    noiseSoloBtn.setBounds(noiseRow.reduced(2)); // Remaining 30px
    
    // Bottom Left: Feedback
    // Push down
    leftCol.removeFromBottom(50);
    auto leftBottom = leftCol.removeFromBottom(knobH + 20);
    // FB Amt | FB Time | Space
    // We need to fit 3 knobs here.
    // Let's ensure there is space.
    // Place Amt and Time side-by-side
    fbAmountSlider.setBounds(leftBottom.getX(), leftBottom.getY(), 90, 90);
    fbAmountLabel.setBounds(fbAmountSlider.getX(), fbAmountSlider.getBottom()-12, 90, 20);
    
    fbTimeSlider.setBounds(leftBottom.getRight() - 90, leftBottom.getY(), 90, 90);
    fbTimeLabel.setBounds(fbTimeSlider.getX(), fbTimeSlider.getBottom()-12, 90, 20);
    
    // Put SPACE centered BELOW them
    spaceSlider.setBounds(leftBottom.getCentreX() - 40, leftBottom.getY() + 85, 80, 80);
    spaceLabel.setBounds(spaceSlider.getX(), spaceSlider.getBottom()-10, 80, 20);
    
    // RIGHT COLUMN (Filter + Reactor)
    // Row 1: Cutoff | Res
    placeRow(rightCol, cutoffSlider, cutoffLabel, &resSlider, &resLabel);
    
    // Row 2: Mode Button
    auto modeRow = rightCol.removeFromTop(30);
    filterModeBtn.setBounds(modeRow.reduced(20, 2));

    // Bottom Right: Reactor Area
    auto rightBottom = rightCol.removeFromBottom(160); // Tank height
    
    stagesReactor.setBounds(rightBottom.getRight() - 60, rightBottom.getBottom() - 150, 50, 140);
    stagesLabel.setBounds(stagesReactor.getX() - 50, stagesReactor.getBottom(), 100, 20);
    
    // Adjust Space label visibility or just keep it tight
}

void PhatRackAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Clean - no beams
}

void PhatRackAudioProcessorEditor::timerCallback()
{

    // Update Transfer visualizer
    // --- Fetch Parameters Safely ---
    auto getParamSafe = [&](juce::String id) -> float {
        if (auto* p = audioProcessor.apvts.getRawParameterValue(id)) return p->load();
        return 0.0f;
    };

    float drive = getParamSafe("drive");
    float stages_raw = getParamSafe("stages");
    int stages = (int)stages_raw;
    
    float pos = getParamSafe("algoPos");
    float neg = getParamSafe("algoNeg");
    float fold = getParamSafe("fold");
    transferVis.setParams((aether::DistortionAlgo)(int)pos, (aether::DistortionAlgo)(int)neg, drive, stages, fold);
    
    // Animate Transfer Dot
    float time = (float)juce::Time::getMillisecondCounter() * 0.002f;
    float sweep = std::sin(time) * 0.8f; 
    transferVis.updateInputLevel(sweep);
    
    // Update Orb & Scope
    float meter = audioProcessor.outputMeter.load();
    
    float morph = getParamSafe("morph");
    float cutoff = getParamSafe("cutoff");
    float res = getParamSafe("res");
    float noiseLvl = getParamSafe("noiseLevel");
    float noiseDist = getParamSafe("noiseWidth");
    float sub = getParamSafe("sub");
    
    float fbAmt = getParamSafe("fbAmount");
    float fbTime = getParamSafe("fbTime");
    float fbSpace = getParamSafe("scramble");

    float gain = getParamSafe("output");
    float mix = getParamSafe("mix");
    float width = getParamSafe("width");
    
    // Update Orb
    orb.setLevel(meter); 
    orb.setMorph(morph); 
    orb.setDrive(drive); 
    orb.setWidth(width); 
    
    orb.setNoise(noiseLvl, 0.0f); 
    orb.setSub(sub);
    orb.setFilter(cutoff, res);
    orb.setFeedback(fbAmt, fbTime, fbSpace);
    orb.setGain(gain);
    orb.setMix(mix);
    
    orb.advance(); 
            
    // Sync Stages Reactor with Central Theme
    stagesReactor.setValue(stages_raw);
    stagesReactor.setMorph(morph);
    
    // --- CYBER DECK SPECTRUM FEED (REAL AUDIO) ---
    // Pull from the Audio FIFO populated by the Processor
    juce::AudioBuffer<float> vizNoise(1, 480); // 480 samples @ 48k is ~10ms. Enough for a snapshot.
    vizNoise.clear();
    
    // Pull fresh data
    audioProcessor.audioFifo.pull(vizNoise);
    
    // ANTAGRAVITY: Injecting Noise Floor ("Always Up" Aesthetic)
    if (drive > 0.01f)
    {
        auto* w = vizNoise.getWritePointer(0);
        // Inject low-level background noise so the spectrum maintains presence
        float floorLvl = 0.015f * drive; 
        for (int i = 0; i < vizNoise.getNumSamples(); ++i)
            w[i] += (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * floorLvl;
    }
    
    
    // Multi-parameter reactivity
    float fb_val = audioProcessor.apvts.getRawParameterValue("fbAmount")->load();
    
    osc.pushBuffer(vizNoise); 
    osc.setMorph(morph);
    // Chaos only kicks in after 60% drive, and ramps up
    float chaos_inject = std::max(0.0f, (drive - 0.6f) * 2.5f); 
    osc.setChaos(chaos_inject); 
    osc.setIntensity(fb_val);   

    // REPAINT ORB (Physical Drift)
    logo.advance();
    logo.setMorph(morph);

    orb.repaint();
}
