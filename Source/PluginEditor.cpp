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
    
    // UX Improvements: Resizable Window
    setResizable(true, true);
    setResizeLimits(800, 600, 1600, 1200);
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
    
    // Grid alignment correction
    int startY = 80; 
    
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
    // Contains the Global Controls
    auto deck = area.removeFromBottom(110);
    
    // Deck Layout: [Sub|XOver] ... [Squeeze|Width] ... [Out|Mix]
    // We used to have 6 knobs. Let's center them.
    int knobSize = juce::jmin(80, deck.getHeight() - 30);
    int gap = 15;
    int groupGap = 40;
    
    // Calculate total width
    int totalDeckW = (knobSize * 6) + (gap * 4) + (groupGap * 2);
    int startX = deck.getCentreX() - (totalDeckW / 2);
    int y = deck.getCentreY() - (knobSize / 2) + 5;
    
    auto placeDeckKnob = [&](juce::Slider& s, juce::Label& l, int& x) {
        s.setBounds(x, y, knobSize, knobSize);
        l.setBounds(x, s.getBottom() - 10, knobSize, 20);
        x += knobSize + gap;
    };
    
    int currentX = startX;
    
    // Group 1: Bass
    placeDeckKnob(subSlider, subLabel, currentX);
    placeDeckKnob(xoverSlider, xoverLabel, currentX);
    
    currentX += groupGap; // Gap
    
    // Group 2: Texture
    placeDeckKnob(squeezeSlider, squeezeLabel, currentX);
    placeDeckKnob(widthSlider, widthLabel, currentX);
    
    currentX += groupGap; // Gap
    
    // Group 3: Master
    placeDeckKnob(outputSlider, outputLabel, currentX);
    placeDeckKnob(mixSlider, mixLabel, currentX);

    // --- 3. OSCILLOSCOPE (Above Deck) ---
    auto oscArea = area.removeFromBottom(60).reduced(10, 0); // reduced width slightly?
    osc.setBounds(oscArea);

    // --- 4. MAIN WORKSPACE ---
    auto grid = area.reduced(10);
    
    // Calculate Column Widths based on remaining space
    // Design calls for: [Left Column] [Center Orb] [Right Column]
    // We want the Orb to be maximum size in the center.
    
    int colWidth = 110; 
    
    auto leftCol = grid.removeFromLeft(colWidth * 2); // 2 knobs wide roughly
    auto rightCol = grid.removeFromRight(colWidth * 2);
    
    // CENTER: THE ORB
    auto centerArea = grid;
    int orbSize = juce::jmin(centerArea.getWidth(), centerArea.getHeight());
    orbSize = juce::jmin(orbSize, 360); // Max cap
    orb.setBounds(centerArea.getCentreX() - orbSize/2, centerArea.getCentreY() - orbSize/2, orbSize, orbSize);
    
    // CENTER OVERLAYS
    // Morph Slider: Above Orb, or Inside?
    // Previous layout: Above Morph was Transfer Vis.
    // Morph Slider at Center Top of Orb? 
    // Let's place Morph Slider centered above the Orb, and Transfer Vis above that.
    
    int centerTopY = orb.getY() - 10;
    
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
    
    // LEFT COLUMN (Distortion + Noise)
    // Row 1: Drive | Fold
    placeRow(leftCol, driveSlider, driveLabel, &foldSlider, &foldLabel);
    
    // Row 2: Noise Level | Noise Width
    placeRow(leftCol, noiseLevelSlider, noiseLevelLabel, &noiseWidthSlider, &noiseWidthLabel);
    
    // Row 3: Noise Type
    auto noiseRow = leftCol.removeFromTop(30);
    noiseTypeSelector.setBounds(noiseRow.removeFromLeft(noiseRow.getWidth() - 30).reduced(5, 0));
    loadNoiseButton.setBounds(noiseRow.reduced(2));
    
    // Bottom Left: Feedback
    // Push down
    auto leftBottom = leftCol.removeFromBottom(knobH + 20);
    // FB Amt | FB Time (Space moved?)
    // Let's squeeze 3 here? Or just 2? 
    // Previous: Amt, Time, Space
    // Space is valuable. Let's put Space in Right Col bottom or squeeze 3.
    // Let's try to fit 2 here (Amt/Time) and put Space elsewhere or tightly.
    // Actually, Space fits nicely in Right Col.
    
    fbAmountSlider.setBounds(leftBottom.getX(), leftBottom.getY(), 90, 90);
    fbAmountLabel.setBounds(fbAmountSlider.getX(), fbAmountSlider.getBottom()-12, 90, 20);
    
    fbTimeSlider.setBounds(leftBottom.getRight() - 90, leftBottom.getY(), 90, 90);
    fbTimeLabel.setBounds(fbTimeSlider.getX(), fbTimeSlider.getBottom()-12, 90, 20);
    
    
    // RIGHT COLUMN (Filter + Reactor)
    // Row 1: Cutoff | Res
    placeRow(rightCol, cutoffSlider, cutoffLabel, &resSlider, &resLabel);
    
    // Row 2: Mode Button
    auto modeRow = rightCol.removeFromTop(30);
    filterModeBtn.setBounds(modeRow.reduced(20, 2));

    // Bottom Right: Reactor Area
    // Previous: StagesTank, StagesLabel
    // Let's put Space here too?
    // Or Space in Left bottom and Feedback gets cramped?
    // Let's put Space Slider above the Reactor Tank
    
    auto rightBottom = rightCol.removeFromBottom(160); // Tank height
    
    // Place Space Slider above Tank
    spaceSlider.setBounds(rightBottom.getX() + 10, rightBottom.getY() - 90, 80, 80);
    spaceLabel.setBounds(spaceSlider.getX(), spaceSlider.getBottom()-10, 80, 20);
    
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
