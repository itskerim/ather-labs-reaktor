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
    
    driveLabel.setText("DRIVE", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);

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

    squeezeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    squeezeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(squeezeSlider);
    squeezeAtt = std::make_unique<Attachment>(audioProcessor.apvts, "squeeze", squeezeSlider);
    squeezeLabel.setText("SQUEEZE", juce::dontSendNotification);
    squeezeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(squeezeLabel);

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
    noiseWidthLabel.setText("DISTORT", juce::dontSendNotification);
    noiseWidthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noiseWidthLabel);

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
                // Note: We need to access parameter to update UI, 
                // but direct setValue on parameter is better for host sync.
                if (auto* p = audioProcessor.apvts.getParameter("noiseType"))
                    p->setValueNotifyingHost(p->getNormalisableRange().convertTo0to1(3.0f)); // Index 3 = 4th item
            }
        });
    };

    spaceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    spaceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(spaceSlider);
    spaceAtt = std::make_unique<Attachment>(audioProcessor.apvts, "space", spaceSlider);
    spaceLabel.setText("SPACE", juce::dontSendNotification);
    spaceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(spaceLabel);

    // --- DnB Essentials ---
    widthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    widthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(widthSlider);
    widthAtt = std::make_unique<Attachment>(audioProcessor.apvts, "width", widthSlider);
    widthLabel.setText("WIDTH", juce::dontSendNotification);
    widthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(widthLabel);
    
    xoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    xoverSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addChildComponent(xoverSlider); // Might be small or option
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
    subSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffffffff));
    addAndMakeVisible(subSlider);
    subAtt = std::make_unique<Attachment>(audioProcessor.apvts, "subLevel", subSlider);
    
    subLabel.setText("SUB", juce::dontSendNotification);
    subLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subLabel);

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
    xoverSlider.setTooltip("Where the sound is split between the sub (bass) and the rest. Keeps bass and highs in sync so they work together instead of fighting. Adjust to fit your source.");
    squeezeSlider.setTooltip("Compresses the sound to bring out small details and make it punchier. Can add grit and presence—like squashing the dynamics so quiet parts pop more.");
    widthSlider.setTooltip("Makes the stereo image wider or narrower. The sub (bass) stays centered; this mainly affects the rest. Use to get a bigger or tighter stereo field.");
    
    outputSlider.setTooltip("Overall volume of the plugin. Use this to match the level of your mix when the effect is on.");
    mixSlider.setTooltip("Balance between your dry (original) signal and the wet (effected) signal. 100% = full effect; 0% = bypass (original only). Use it to blend in the amount of distortion you want.");

    mixLabel.setText("DRY/WET", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    startTimerHz(60); // 60 FPS for smooth Orb animation
    setSize (1000, 700);
}

PhatRackAudioProcessorEditor::~PhatRackAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PhatRackAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. Deep Void Background (Solid Dark)
    g.fillAll(juce::Colour(0xff050505)); 
    
    float orbCX = getWidth() / 2.0f;
    float orbCY = getHeight() / 2.0f;

    // --- Subtle Cyber Grid (Holodeck) ---
    g.setColour(juce::Colour(0xff1a1a1a));
    float gridSz = 40.0f;
    for (float x = 0; x < getWidth(); x += gridSz)
        g.drawVerticalLine((int)x, 80.0f, (float)getHeight());
    for (float y = 80; y < getHeight(); y += gridSz)
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
    auto header = area.removeFromTop(60);
    
    logo.setBounds(header.getX() + 15, header.getY(), 300, header.getHeight());
    
    auto helpArea = header.removeFromRight(40).reduced(10);
    helpButton.setBounds(helpArea);
    
    presetSelector.setBounds(header.removeFromRight(180).reduced(10));
    
    // Bottom Scope (Wide)
    osc.setBounds(area.removeFromBottom(80).reduced(10));
    
    // Main Workspace
    auto grid = area.reduced(20);
    
    // Reactor Center (Larger, more space)
    int reactorSize = 320; // Bigger Orb
    orb.setBounds(grid.getCentreX() - reactorSize/2, grid.getCentreY() - reactorSize/2, reactorSize, reactorSize);
    
    // -- CORNER LAYOUT (Spread Out) --
    int colW = 90; // Knob width
    int gap = 10;
    
    int startY = grid.getY() + 20;
    int startX = grid.getX() + 20;
    int endX = grid.getRight() - 20 - (colW * 3 + gap * 2);
    // Push bottom knobs UP slightly to make room for macros/deck
    int bottomY = grid.getBottom() - 160; 
    
    // --- TOP LEFT: DISTORTION ---
    auto placeKnob = [&](juce::Slider& s, juce::Label& l, int x, int y)
    {
        s.setBounds(x, y, colW, colW);
        l.setBounds(x, y + colW - 15, colW, 20); // Label slightly tucked
    };
    
    // Row 1 (Distortion)
    placeKnob(driveSlider, driveLabel, startX, startY);
    placeKnob(foldSlider, foldLabel, startX + colW + gap, startY);
    
    // Row 2 (Noise) - Below Drive/Decimate
    // Row 2 (Noise) - Below Drive/Decimate
    placeKnob(noiseLevelSlider, noiseLevelLabel, startX, startY + colW + 20);
    placeKnob(noiseWidthSlider, noiseWidthLabel, startX + colW + gap, startY + colW + 20);
    
    // Noise Selector & Load Button
    int selectorY = startY + (colW + 20) * 2 - 5;
    int loadBtnW = 25;
    int selectorW = (colW * 2) + gap - loadBtnW - 5;
    
    noiseTypeSelector.setBounds(startX, selectorY, selectorW, 20);
    loadNoiseButton.setBounds(startX + selectorW + 5, selectorY, loadBtnW, 20);
    
    // --- TOP RIGHT: FILTER ---
    // Mirror position
    placeKnob(cutoffSlider, cutoffLabel, endX, startY);
    placeKnob(resSlider, resLabel, endX + colW + gap, startY);
    
    // Row 2 (Filter Extras)
    // Mode Button (Small, under/near Filter section)
    filterModeBtn.setBounds(endX, startY + colW + 20, 90, 20); 
    
    // Scramble/Plasma control removed from this layout (Stages took its spot).
    
    // Re-arranging Filter Row:
    // [ Cutoff ] [ Res ] [ Stages ]
    // --- BOTTOM LEFT: FEEDBACK ---
    placeKnob(fbAmountSlider, fbAmountLabel, startX, bottomY);
    placeKnob(fbTimeSlider, fbTimeLabel, startX + colW + gap, bottomY);
    placeKnob(spaceSlider, spaceLabel, startX + (colW + gap)*2, bottomY);
    
    // --- BOTTOM RIGHT: REACTOR CORE ---
    // The "Reactor Tank" filling slider
    int tankW = 50;
    int tankH = 150;
    stagesReactor.setBounds(grid.getRight() - tankW - 10, grid.getBottom() - tankH - 10, tankW, tankH);
    stagesLabel.setBounds(stagesReactor.getX() - 40, stagesReactor.getBottom() + 5, tankW + 80, 20);
    // Moved to Deck below... disable these here or move them?
    // Actually, "Output" block is now the Deck. 
    // Let's keep the symmetry but move some things.
    // We needed room.
    
    // --- CENTER CONTROLS (Overlaid on Orb Periphery) ---
    // --- CENTER CONTROLS (Overlaid on Orb Periphery) ---
    // Morph Slider (Top Center of Orb)
    // Shifted down slightly to clear the Transfer Graph
    morphSlider.setBounds(grid.getCentreX() - 40, orb.getY() - 50, 80, 80);
    morphLabel.setBounds(morphSlider.getX(), morphSlider.getBottom() - 10, 80, 20);
    
    // Macros Removed per user request

    // --- 3. LAYOUT VISUALS (Background Layer) ---
    // Orb is already placed.
    
    // Transfer visualizer still small at top
    // Made slightly shorter to add gap
    transferVis.setBounds(grid.getCentreX() - 50, header.getBottom() + 10, 100, 50); 
    
    // --- 7. BOTTOM ROW (Global / DnB Deck) ---
    // We have 6 globs now: Sub, XOver, Squeeze, Width, Output, Mix
    // Let's group them: 
    // [ BASS FOUNDATION ] [ TEXTURE POP ] [ MASTER ]
    // [ Sub | XOver ]     [ Squeeze | Width ] [ Out | Mix ]
    
    int knobSize = colW; 
    int bottomY_global = getHeight() - 110;
    int deckWidth = (knobSize + gap) * 6;
    int startX_deck = (getWidth() - deckWidth) / 2 + 50; 
    
    // 1. Sub Foundation
    subSlider.setBounds(startX_deck, bottomY_global, knobSize, knobSize);
    subLabel.setBounds(subSlider.getX(), subSlider.getBottom() - 10, knobSize, 20);
    
    xoverSlider.setBounds(startX_deck + 80, bottomY_global, knobSize, knobSize);
    xoverLabel.setBounds(xoverSlider.getX(), xoverSlider.getBottom() - 10, knobSize, 20);
    
    // 2. Texture Pop
    squeezeSlider.setBounds(startX_deck + 180, bottomY_global, knobSize, knobSize); // Gap
    squeezeLabel.setBounds(squeezeSlider.getX(), squeezeSlider.getBottom() - 10, knobSize, 20);
    
    widthSlider.setBounds(startX_deck + 260, bottomY_global, knobSize, knobSize);
    widthLabel.setBounds(widthSlider.getX(), widthSlider.getBottom() - 10, knobSize, 20);
    
    // 3. Master
    outputSlider.setBounds(startX_deck + 360, bottomY_global, knobSize, knobSize); // Gap
    outputLabel.setBounds(outputSlider.getX(), outputSlider.getBottom() - 10, knobSize, 20);
    
    mixSlider.setBounds(startX_deck + 440, bottomY_global, knobSize, knobSize);
    mixLabel.setBounds(mixSlider.getX(), mixSlider.getBottom() - 10, knobSize, 20);

    // Selectors & Visuals (Tucked Top Center)
    // transferVis.setBounds(grid.getCentreX() - 50, header.getBottom() + 10, 100, 60); // Original line, now moved
    
    // User Request: Move to Middle, Together, Above Morph
    int selW = 90;
    int selH = 20;
    int selGap = 5;
    int totalSelW = (selW * 2) + selGap;
    
    // Position above Morph (which is at grid.getCentreX())
    int selStartX = grid.getCentreX() - (totalSelW / 2);
    // Morph is at: grid.getCentreX() - 40, orb.getY() - 50...
    // Let's put them nicely above the Morph Slider
    int selY = morphSlider.getY() - selH - 5;
    
    posSelector.setBounds(selStartX, selY, selW, selH); 
    negSelector.setBounds(selStartX + selW + selGap, selY, selW, selH);
    
    // VISUALIZER: Position directly ABOVE the selectors
    // The "sinewave" graph
    int visW = 120;
    int visH = 50; 
    transferVis.setBounds(grid.getCentreX() - (visW/2), selY - visH - 5, visW, visH); 
    negSelector.setBounds(selStartX + selW + selGap, selY, selW, selH);
}

void PhatRackAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Clean - no beams
}

void PhatRackAudioProcessorEditor::timerCallback()
{

    // Update Transfer visualizer
    auto pos = (aether::DistortionAlgo)(int)audioProcessor.apvts.getRawParameterValue("algoPos")->load();
    auto neg = (aether::DistortionAlgo)(int)audioProcessor.apvts.getRawParameterValue("algoNeg")->load();
    auto drive = audioProcessor.apvts.getRawParameterValue("drive")->load();
    auto stages_raw = audioProcessor.apvts.getRawParameterValue("stages")->load();
    auto stages = (int)stages_raw;
    
    float fold = audioProcessor.apvts.getRawParameterValue("fold")->load();
    transferVis.setParams(pos, neg, drive, stages, fold);
    
    // Animate Transfer Dot
    float time = (float)juce::Time::getMillisecondCounter() * 0.002f;
    float sweep = std::sin(time) * 0.8f; 
    transferVis.updateInputLevel(sweep);
    
    // Update Orb & Scope
    // REAL AUDIO REACTIVITY:
    float meter = audioProcessor.outputMeter.load();
    
    // Get Morph & Cutoff for Reactivity
    auto morph = audioProcessor.apvts.getRawParameterValue("morph")->load();
    auto width = audioProcessor.apvts.getRawParameterValue("width")->load();
    
    orb.setLevel(meter); // Drives expansion and "Dancing"
    orb.setMorph(morph); // Drives color and Shape distortion
    orb.setWidth(width); // Drives Horizontal stretch
    orb.setDrive(drive); // Drives Base Size
    orb.advance(); 
            
    // Sync Stages Reactor with Central Theme
    stagesReactor.setValue(stages_raw);
    stagesReactor.setMorph(morph);
    
    // --- CLASSIC SPECTRUM FEED ---
    juce::AudioBuffer<float> vizNoise(1, 256);
    auto* w_viz = vizNoise.getWritePointer(0);
    for(int i=0; i<256; ++i) w_viz[i] = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * drive;
    
    // Multi-parameter reactivity
    float fb_val = audioProcessor.apvts.getRawParameterValue("fbAmount")->load();
    
    osc.pushBuffer(vizNoise); 
    osc.setMorph(morph);
    osc.setChaos(0.0f); 
    osc.setIntensity(fb_val);   

    // REPAINT ORB (Physical Drift)
    logo.advance();
    logo.setMorph(morph);

    orb.repaint();
}
