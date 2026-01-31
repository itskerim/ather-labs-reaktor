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

        // Tooltip: on-brand dark panel + cyan accent
        setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xff18181b));
        setColour(juce::TooltipWindow::textColourId, juce::Colour(0xffe4e4e7));
        setColour(juce::TooltipWindow::outlineColourId, juce::Colour(0xff00d4ff).withAlpha(0.5f));
    }

    juce::Rectangle<int> getTooltipBounds(const juce::String& tipText, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override
    {
        const int maxWidth = 380;
        const int paddingX = 14;
        const int paddingY = 10;
        juce::AttributedString s;
        s.setWordWrap(juce::AttributedString::WordWrap::byWord);
        s.setJustification(juce::Justification::left);
        s.append(tipText, juce::Font(juce::FontOptions("Inter", 13.0f, juce::Font::plain)), findColour(juce::TooltipWindow::textColourId));
        juce::TextLayout tl;
        tl.createLayoutWithBalancedLineLengths(s, (float)maxWidth - paddingX * 2);
        auto w = (int)juce::jmin((float)maxWidth, tl.getWidth() + paddingX * 2.0f);
        auto h = (int)(tl.getHeight() + paddingY * 2.0f);
        return juce::Rectangle<int>(
            screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 16) : screenPos.x + 20,
            screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 8) : screenPos.y + 8,
            w, h).constrainedWithin(parentArea);
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
        const float cornerSize = 6.0f;
        const int paddingX = 12;
        const int paddingY = 8;

        g.setColour(findColour(juce::TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle(bounds, cornerSize);

        g.setColour(findColour(juce::TooltipWindow::outlineColourId));
        g.drawRoundedRectangle(bounds.reduced(0.5f), cornerSize, 1.0f);

        juce::AttributedString s;
        s.setWordWrap(juce::AttributedString::WordWrap::byWord);
        s.setJustification(juce::Justification::left);
        s.append(text, juce::Font(juce::FontOptions("Inter", 13.0f, juce::Font::plain)), findColour(juce::TooltipWindow::textColourId));
        juce::TextLayout tl;
        tl.createLayoutWithBalancedLineLengths(s, (float)width - paddingX * 2.0f);
        tl.draw(g, juce::Rectangle<float>((float)paddingX, (float)paddingY, (float)width - paddingX * 2.0f, (float)height - paddingY * 2.0f));
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
          juce::Path arc;
          arc.addCentredArc(center.x, center.y, radius * 0.92f, radius * 0.92f, 0.0f, rotaryStartAngle, toAngle, true);
          g.setColour(accent);
          g.strokePath(arc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    
    // --- DROPDOWN: INSET SCREEN ---
    void drawComboBox(juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
    {
        auto area = juce::Rectangle<int>(0, 0, width, height).toFloat();
        
        // 1. Dark Glass Inset
        g.setColour(juce::Colour(0xff09090b));
        g.fillRoundedRectangle(area, 4.0f);
        
        // 2. Neon Accent Side-Strip
        g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.6f));
        g.fillRect(0.0f, 0.0f, 3.0f, (float)height);
        
        // 3. Techy Outer Border
        g.setColour(juce::Colour(0xff27272a));
        g.drawRoundedRectangle(area, 4.0f, 1.0f);

        // 4. Down Arrow (Minimal)
        juce::Path p;
        float arrowW = 8.0f;
        p.startNewSubPath(width - 15.0f, height * 0.45f);
        p.lineTo(width - 15.0f + arrowW/2, height * 0.45f + 5.0f);
        p.lineTo(width - 15.0f + arrowW, height * 0.45f);
        g.setColour(juce::Colour(0xffa1a1aa));
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(10, 0, box.getWidth() - 30, box.getHeight());
        label.setFont(juce::Font(juce::FontOptions("Inter", 11.0f, juce::Font::bold)));
    }

    // --- POPUP MENU: NEON DASHBOARD STYLE ---
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        auto area = juce::Rectangle<int>(0, 0, width, height).toFloat();
        
        // Deep Black Matte
        g.setColour(juce::Colour(0xff09090b));
        g.fillRoundedRectangle(area, 6.0f);
        
        // Cyber Outline
        g.setColour(juce::Colour(0xff3f3f46));
        g.drawRoundedRectangle(area, 6.0f, 1.5f);
        
        // Subtle Scanline Header
        g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.05f));
        for (int y = 0; y < height; y += 4)
            g.drawHorizontalLine(y, 0, (float)width);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           const bool isSeparator, const bool isActive,
                           const bool isHighlighted, const bool isTicked,
                           const bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* const textColourToUse) override
    {
        auto r = area.toFloat();

        if (isSeparator)
        {
            g.setColour(juce::Colour(0xff27272a));
            g.drawLine(r.getX() + 10, r.getCentreY(), r.getRight() - 10, r.getCentreY(), 1.0f);
            return;
        }

        if (isHighlighted && isActive)
        {
            // Neon Glow Highlight
            g.setColour(juce::Colour(0xff00d4ff).withAlpha(0.15f));
            g.fillRect(r.reduced(2.0f));
            
            g.setColour(juce::Colour(0xff00d4ff));
            g.drawRect(r.reduced(2.0f), 1.0f);
        }

        g.setColour(isHighlighted ? juce::Colours::white : juce::Colour(0xffe4e4e7));
        g.setFont(juce::Font(juce::FontOptions("Inter", 13.0f, isHighlighted ? juce::Font::bold : juce::Font::plain)));
        
        auto textRect = r.reduced(15, 0);
        g.drawText(text, textRect, juce::Justification::centredLeft, true);

        if (isTicked)
        {
            g.setColour(juce::Colour(0xff00d4ff));
            g.fillEllipse(r.getX() + 5, r.getCentreY() - 2, 4, 4);
        }
    }
};

} // namespace aether
