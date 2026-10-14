/*
  ==============================================================================

    CyberJungleLookAndFeel.h
    Created: 14 Oct 2026 3:44:34pm
    Author:  prash

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CyberJungleLookAndFeel : public juce::LookAndFeel_V4
{
public:
  CyberJungleLookAndFeel()
  {
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xfff0fdf4));

    setColour(juce::Label::textColourId, juce::Colour(0xff1e5631));
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);

    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff2e8b57));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0x332e8b57));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xffffd700));
    setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xff1e5631));
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xffe2f5e8));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x662e8b57));

    setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff2e8b57));
    setColour(juce::ComboBox::textColourId, juce::Colour(0xff1e5631));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff2e8b57));

    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xfff0fdf4));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xff1e5631));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff2e8b57));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
  }

  juce::Label *createSliderTextBox(juce::Slider &slider) override
  {
    auto *l = juce::LookAndFeel_V4::createSliderTextBox(slider);

    l->setColour(juce::Label::textColourId, juce::Colour(0xff1e5631));
    l->setColour(juce::Label::backgroundColourId, juce::Colour(0xffe2f5e8));
    l->setColour(juce::Label::outlineColourId, juce::Colour(0x662e8b57));

    l->setColour(juce::TextEditor::textColourId, juce::Colour(0xff1e5631));
    l->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xffe2f5e8));
    l->setColour(juce::TextEditor::highlightColourId, juce::Colour(0x4d2e8b57));

    l->setFont(juce::FontOptions(11.0f, juce::Font::bold));
    l->setJustificationType(juce::Justification::centred);

    return l;
  }

  void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider &) override
  {
    auto bounds = juce::Rectangle<int>(x, y, width, height).reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto center = bounds.getCentre();
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineWidth = 3.5f;
    auto arcRadius = radius - lineWidth;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0x262e8b57));
    g.strokePath(backgroundArc, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (sliderPosProportional > 0.0f)
    {
      juce::Path valueArc;
      valueArc.addCentredArc(center.x, center.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, toAngle, true);
      g.setColour(juce::Colour(0xff2e8b57));
      g.strokePath(valueArc, juce::PathStrokeType(lineWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    auto capRadius = radius * 0.70f;
    g.setColour(juce::Colour(0xffe6f5ea));
    g.fillEllipse(center.x - capRadius, center.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    g.setColour(juce::Colour(0xff2e8b57));
    g.drawEllipse(center.x - capRadius, center.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.4f);

    for (float r = 4.0f; r < capRadius; r += 5.0f)
    {
      g.setColour(juce::Colour(0x1f2e8b57));
      g.drawEllipse(center.x - r, center.y - r, r * 2.0f, r * 2.0f, 0.8f);
    }

    auto needleLength = capRadius * 0.85f;
    auto needleEndX = center.x + needleLength * std::sin(toAngle);
    auto needleEndY = center.y - needleLength * std::cos(toAngle);

    juce::Path needle;
    needle.startNewSubPath(static_cast<float>(center.x), static_cast<float>(center.y));
    needle.lineTo(needleEndX, needleEndY);

    g.setColour(juce::Colour(0xffffd700));
    g.strokePath(needle, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour(juce::Colour(0xffffd700));
    g.fillEllipse(center.x - 3.0f, center.y - 3.0f, 6.0f, 6.0f);
  }

  void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox &box) override
  {

    juce::ignoreUnused(isButtonDown, buttonX, buttonY, buttonW, buttonH, box);

    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

    g.setColour(juce::Colours::white);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(juce::Colour(0xff2e8b57));
    g.drawRoundedRectangle(bounds, 4.0f, 1.5f);

    auto arrowX = static_cast<float>(width - 18);
    auto arrowY = static_cast<float>(height) * 0.5f - 2.0f;

    juce::Path triangle;
    triangle.addTriangle(arrowX, arrowY, arrowX + 8.0f, arrowY, arrowX + 4.0f, arrowY + 5.0f);
    g.setColour(juce::Colour(0xff2e8b57));
    g.fillPath(triangle);
  }
};
