#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "AetherPresets.h"

PhatRackAudioProcessorEditor::PhatRackAudioProcessorEditor (AetherAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
      posSelector("POSITIVE", p.apvts, "algoPos"),
      negSelector("NEGATIVE", p.apvts, "algoNeg")
{
    // Apply AETHER Global LookAndFeel
    setLookAndFeel(&aetherLF);

    // --- 1. Branding & Header ---
    tooltipWindow.setMillisecondsBeforeTipAppears(700);
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

    // --- 2. Central Stage ---
    // --- 2. Central Stage ---
    addAndMakeVisible(orb);     // Middle (The Orb)
    addAndMakeVisible(osc);     // Bottom
    addAndMakeVisible(transferVis);
    addAndMakeVisible(posSelector);
    addAndMakeVisible(negSelector);

    // --- 3. Primary Distortion Controls ---
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    driveSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff00d4ff));
    addAndMakeVisible(driveSlider);
    driveAtt = std::make_unique<Attachment>(audioProcessor.apvts, "drive", driveSlider);
    
    driveLabel.setText("DRIVE", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);

    stagesSlider.setSliderStyle(juce::Slider::LinearBar);
    stagesSlider.setRange(1, 6, 1);
    addAndMakeVisible(stagesSlider);
    stagesAtt = std::make_unique<Attachment>(audioProcessor.apvts, "stages", stagesSlider);
    
    stagesLabel.setText("STAGES", juce::dontSendNotification);
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

    // --- 6. Experimental ---
    decimateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    decimateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(decimateSlider);
    decimateAtt = std::make_unique<Attachment>(audioProcessor.apvts, "decimate", decimateSlider);
    decimateLabel.setText("DECIMATE", juce::dontSendNotification);
    decimateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(decimateLabel);

    squeezeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    squeezeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(squeezeSlider);
    squeezeAtt = std::make_unique<Attachment>(audioProcessor.apvts, "squeeze", squeezeSlider);
    squeezeLabel.setText("SQUEEZE", juce::dontSendNotification);
    squeezeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(squeezeLabel);

    scrambleSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    scrambleSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(scrambleSlider);
    scrambleAtt = std::make_unique<Attachment>(audioProcessor.apvts, "scramble", scrambleSlider);
    scrambleLabel.setText("SCRAMBLE", juce::dontSendNotification);
    scrambleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(scrambleLabel);

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

    // --- 8. UX Tooltips ---
    driveSlider.setTooltip("DRIVE: Input gain pushed into the distortion stages. Higher values = More harmonic saturation.");
    stagesSlider.setTooltip("STAGES: Number of series distortion stages. 1 = Subtle, 6 = Total destruction.");
    decimateSlider.setTooltip("DECIMATE: Sample Rate Reduction. Adds digital aliasing and grit.");
    
    cutoffSlider.setTooltip("CUTOFF: Filter Frequency. Controls the brightness or 'talking' vowel position.");
    resSlider.setTooltip("RESONANCE: Filter Peak. High values create 'talking' formants or self-oscillation.");
    morphSlider.setTooltip("MORPH: Sweeps filter shapes (LP->BP->HP) or Vowels (A-E-I-O-U) in Formant Mode.");
    
    fbAmountSlider.setTooltip("FEEDBACK: Resonator intensity. Pushes signal back into itself for metallic tones.");
    fbTimeSlider.setTooltip("TIME: Tuned delay time for the resonator. Short = Karplus Strong, Long = Metallic Echo.");
    spaceSlider.setTooltip("SPACE: Reverb/Diffusion amount applied to the feedback loop.");
    
    scrambleSlider.setTooltip("SCRAMBLE: Chaos LFO Depth. Randomizes filter cutoff and resonator feedback.");
    
    subSlider.setTooltip("SUB: Clean Mono Sub-Bass level. Keyed to the track fundamental via Cross-Over.");
    xoverSlider.setTooltip("X-OVER: Crossover Frequency (Hz). Splits signal into clean Sub and distorted Tops.");
    squeezeSlider.setTooltip("SQUEEZE: Upward Compression (OTT). Smashing dynamics to bring out quiet details.");
    widthSlider.setTooltip("WIDTH: Dimension Expander. Widens the high-band (Tops) while keeping Sub mono.");
    
    outputSlider.setTooltip("GAIN: Master Output Volume.");
    mixSlider.setTooltip("MIX: Dry/Wet Blend. 100% = Full Effect.");
    
    mixSlider.setTooltip("MIX: Dry/Wet Blend. 100% = Full Effect.");

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

    // Logo
    g.setColour(juce::Colour(0xffffffff));
    g.setFont(juce::Font(juce::FontOptions("Inter", 30.0f, juce::Font::bold)));
    g.drawText("AETHER LAB 3.0", 30, 10, 300, 60, juce::Justification::centredLeft);
    
    // Hardware Section Labels (Silkscreen)
    g.setColour(juce::Colour(0xff71717a)); // Zinc-500
    g.setFont(juce::Font(juce::FontOptions("Inter", 14.0f, juce::Font::bold)));
    
    // Draw subtle backgrounds for sections with Cyber Accents
    auto drawTechBox = [&](int x, int y, int w, int h, const juce::String& label)
    {
        // 1. Recessed background
        g.setColour(juce::Colour(0xff121212)); 
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 8.0f);
        
        // 2. Corner Brackets (Cyber)
        g.setColour(juce::Colour(0xff3f3f46)); // Zinc-700
        float bLen = 10.0f;
        g.drawHorizontalLine(y, x, x + bLen); g.drawVerticalLine(x, y, y + bLen); // TL
        g.drawHorizontalLine(y+h, x+w-bLen, x+w); g.drawVerticalLine(x+w, y+h-bLen, y+h); // BR
        
        // 3. Label
        g.setColour(juce::Colour(0xff71717a));
        g.drawText(label, x + 15, y + 5, 100, 20, juce::Justification::left);
    };

    // Shape
    drawTechBox(20, 90, 340, 150, "DISTORTION // CORE");
    
    // Tone
    drawTechBox(20, 260, 340, 130, "FILTERS // SPECTRAL");
    
    // Space
    drawTechBox(20, 420, 340, 130, "MODULATION // TIME");
    
    // Global HUD Text
    g.setColour(juce::Colour(0xff27272a));
    g.setFont(juce::Font(juce::FontOptions("Inter", 10.0f, juce::Font::plain)));
    g.drawText(currentStatusText, getWidth() - 400, getHeight() - 20, 380, 20, juce::Justification::right);
}

void PhatRackAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto header = area.removeFromTop(60);
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
    
    // Row 1
    placeKnob(driveSlider, driveLabel, startX, startY);
    placeKnob(stagesSlider, stagesLabel, startX + colW + gap, startY);
    placeKnob(decimateSlider, decimateLabel, startX + (colW + gap)*2, startY);
    
    // --- TOP RIGHT: FILTER ---
    // Mirror position
    placeKnob(cutoffSlider, cutoffLabel, endX, startY);
    placeKnob(resSlider, resLabel, endX + colW + gap, startY);
    placeKnob(scrambleSlider, scrambleLabel, endX + (colW + gap)*2, startY);
    
    // Mode Button (Small, under/near Filter section)
    filterModeBtn.setBounds(endX, startY + colW + 20, 90, 20); // Under Cutoff/Res
    
    // --- BOTTOM LEFT: FEEDBACK ---
    placeKnob(fbAmountSlider, fbAmountLabel, startX, bottomY);
    placeKnob(fbTimeSlider, fbTimeLabel, startX + colW + gap, bottomY);
    placeKnob(spaceSlider, spaceLabel, startX + (colW + gap)*2, bottomY);
    
    // --- BOTTOM RIGHT: OUTPUT ---
    // Moved to Deck below... disable these here or move them?
    // Actually, "Output" block is now the Deck. 
    // Let's keep the symmetry but move some things.
    // We needed room.
    
    // --- CENTER CONTROLS (Overlaid on Orb Periphery) ---
    // Morph Slider (Top Center of Orb)
    morphSlider.setBounds(grid.getCentreX() - 40, orb.getY() - 60, 80, 80);
    morphLabel.setBounds(morphSlider.getX(), morphSlider.getBottom() - 10, 80, 20);
    
    // Macros Removed per user request

    // --- 3. LAYOUT VISUALS (Background Layer) ---
    // Orb is already placed.
    
    // Transfer visualizer still small at top
    transferVis.setBounds(grid.getCentreX() - 50, header.getBottom() + 10, 100, 60); 
    
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
    posSelector.setBounds(startX, header.getBottom() + 5, 90, 20); // Move near distortion
    negSelector.setBounds(startX + 100, header.getBottom() + 5, 90, 20);
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
    auto stages = (int)audioProcessor.apvts.getRawParameterValue("stages")->load();
    
    transferVis.setParams(pos, neg, drive, stages);
    
    // Update Orb & Scope
    // We can peek at the Atomic RMS if we added it, but let's fake it with 'drive' level for visual reactivity
    
    // Get Morph & Cutoff for Reactivity
    auto morph = audioProcessor.apvts.getRawParameterValue("morph")->load();
    
    orb.setLevel(drive); 
    orb.setMorph(morph);
    
    // Ideally we push real buffer here if we had an Atomic FIFO. 
    // For now, let's just make the scope show a static line or noise to prove it exists
    // (Real scope needs AudioProcessor -> Editor FIFO)
    juce::AudioBuffer<float> noise(1, 256);
    auto* w = noise.getWritePointer(0);
    for(int i=0; i<256; ++i) w[i] = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * drive;
    osc.pushBuffer(noise);
    
    // --- UX: Interactive Status Bar ---
    // Check what mouse is hovering
    auto* comp = juce::Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
    juce::String statusText = "SYS.OP.ACTIVE // AETHER.KERNEL.V5"; // Default
    
    if (comp)
    {
        if (auto* slider = dynamic_cast<juce::Slider*>(comp))
        {
            // If Hovering a Slider: Show Name + Value + Tooltip Summary
            juce::String name = slider->getName(); 
            // NOTE: We didn't set names explicitly for all sliders, but we can rely on Tooltip logic?
            // Better: Use the Tooltip text as the description
            
            juce::String tip = slider->getTooltip();
            if (tip.isNotEmpty())
            {
                // Format: "VALUE: 0.50 | DESCRIPTION"
                juce::String valText = juce::String(slider->getValue(), 2);
                // Extract Name from Tooltip (before colon)
                juce::String paramName = tip.upToFirstOccurrenceOf(":", false, false);
                juce::String desc = tip.fromFirstOccurrenceOf(":", false, false).trim();
                
                statusText = paramName + ": " + valText + " // " + desc;
            }
        }
    }
    
    // Force repaint of footer area to update text
    // (We draw this text in paint(), but Paint call is expensive if valid)
    // Optimization: Store text in a member string and only repaint if changed?
    // For now, let's just trigger repaint of the bottom strip.
    repaint(0, getHeight() - 30, getWidth(), 30);
    
    // Hack: We need to pass this string to paint() somehow.
    // Let's add a member `currentStatusText` to Editor.h
    currentStatusText = statusText;
    
    // REPAINT ORB (Critical for animation)
    orb.repaint();
    
    // Repaint Footer
    repaint(0, getHeight() - 30, getWidth(), 30);
}
