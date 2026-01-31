#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

namespace aether
{

class AetherLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AetherLookAndFeel()
    {
        // "Hardware" Aesthetic (Dark, Matte, Physical)
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xff18181b)); // Zinc-950 (Powder Coated Metal)
        setColour(juce::Slider::thumbColourId, juce::Colour(0xff38bdf8)); // Sky-400 (LED Color)
        setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff09090b)); // Zinc-950 (Track background)
        setColour(juce::Label::textColourId, juce::Colour(0xffe4e4e7)); // Zinc-200 (Silkscreen text)
        
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff18181b));
        setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff27272a));
        setColour(juce::ComboBox::textColourId, juce::Colour(0xfff4f4f5));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff18181b));
        setColour(juce::PopupMenu::textColourId, juce::Colour(0xfff4f4f5));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff38bdf8));
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(juce::FontOptions("Inter", 12.0f, juce::Font::bold));
    }
    
    // --- KNOB: PREMIUM 3D MECHA HEXAGON ---
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle,
                          juce::Slider& slider) override
    {
         auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(6.0f); // More padding to prevent clip
         auto radius = std::min(bounds.getWidth(), bounds.getHeight()) / 2.0f;
         auto center = bounds.getCentre();
         auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
         
         // --- 1. HEXAGON CHASSIS (3D Bevel) ---
         juce::Path hexOuter, hexInner;
         for(int i=0; i<6; ++i)
         {
             float angle = i * (6.283f / 6.0f); // Flat top? 0 is right. 30deg (0.523) might be better for "point top".
             // Let's rotate -30 deg (-0.523) to have flat top/bottom
             angle -= 0.523f; 
             
             auto pOut = center.getPointOnCircumference(radius, angle);
             auto pIn = center.getPointOnCircumference(radius * 0.85f, angle);
             
             if(i==0) { hexOuter.startNewSubPath(pOut); hexInner.startNewSubPath(pIn); }
             else { hexOuter.lineTo(pOut); hexInner.lineTo(pIn); }
         }
         hexOuter.closeSubPath();
         hexInner.closeSubPath();
         
         // Draw Base Shadow (Drop Shadow)
         g.setColour(juce::Colours::black.withAlpha(0.6f));
         g.fillPath(hexOuter, juce::AffineTransform::translation(0, 4));

         // Draw Bevel (Dark Metal Gradient)
         juce::ColourGradient chassisGrad(juce::Colour(0xff27272a), center.x, center.y - radius,
                                          juce::Colour(0xff09090b), center.x, center.y + radius, false);
         g.setGradientFill(chassisGrad);
         g.fillPath(hexOuter);
         
         // Accent outline
         g.setColour(juce::Colour(0xff3f3f46)); // Zinc-700
         g.strokePath(hexOuter, juce::PathStrokeType(1.0f));

         // --- 2. INNER FACE (Sunken) ---
         g.setColour(juce::Colour(0xff18181b)); // Darkest base
         g.fillPath(hexInner);
         g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
         g.strokePath(hexInner, juce::PathStrokeType(2.0f)); // Inner shadow stroke
         
         // --- 3. INDICATOR (Mechanical Arm) ---
         juce::Path arm;
         // Base hub
         arm.addEllipse(center.x - radius*0.15f, center.y - radius*0.15f, radius*0.3f, radius*0.3f);
         
         // Pointer
         float pRadius = radius * 0.75f;
         float armW = radius * 0.1f;
         
         juce::Point<float> pTip = center.getPointOnCircumference(pRadius, toAngle);
         juce::Point<float> pBaseL = center.getPointOnCircumference(radius*0.15f, toAngle - 1.57f);
         juce::Point<float> pBaseR = center.getPointOnCircumference(radius*0.15f, toAngle + 1.57f);
         
         juce::Path ptr;
         ptr.startNewSubPath(pBaseL);
         ptr.lineTo(pTip);
         ptr.lineTo(pBaseR);
         ptr.closeSubPath();
         
         juce::Colour accent = slider.findColour(juce::Slider::thumbColourId);
         
         // Draw Arm Glow
         g.setColour(accent.withAlpha(0.15f));
         g.fillPath(ptr);
         
         // Draw Arm Solid
         g.setColour(accent);
         g.fillPath(ptr);
         g.fillEllipse(center.x - 3, center.y - 3, 6, 6); // Center pin
         
         // --- 4. VALUE ARC ---
         // Draw arc behind? Or on top? On top looks "Overlaid HUD".
         juce::Path arc;
         arc.addCentredArc(center.x, center.y, radius * 0.92f, radius * 0.92f, 0.0f, rotaryStartAngle, toAngle, true);
         g.setColour(accent);
         g.strokePath(arc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    // --- DROPDOWN: INSET SCREEN ---
    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto area = juce::Rectangle<int>(0, 0, width, height).toFloat();
        
        // Dark Glass / Inset Look
        g.setColour(juce::Colour(0xff09090b));
        g.fillRoundedRectangle(area, 4.0f);
        
        // Inner Shadow/Border
        g.setColour(juce::Colour(0xff27272a));
        g.drawRoundedRectangle(area, 4.0f, 1.0f);
    }
};

} // namespace aether
