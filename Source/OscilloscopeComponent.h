/*
  ==============================================================================

    OscilloscopeComponent.h
    Created: 30 Sep 2026 1:16:40am
    Author:  prash

    Description:  A JUCE component that displays an oscilloscope view of audio data. It reads samples from a lock-free AudioFifo and renders the waveform in real-time. The component uses a timer to periodically update the display and handles positive zero-crossing detection for stable waveform rendering.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "AudioFifo.h"
#include <array>
#include <algorithm>

class OscilloscopeComponent : public juce::Component, public juce::Timer
{
public:
  OscilloscopeComponent(AudioFifo<ScopeFrame, 1024> &fifoToUse) : fifo(fifoToUse)
  {
    startTimerHz(60);
  }

  ~OscilloscopeComponent() override
  {
    stopTimer();
  }

  void timerCallback() override
  {
    ScopeFrame frame;
    while (fifo.pop(frame))
    {
      sampleBuffer[bufferWriteIndex] = frame;
      bufferWriteIndex = (bufferWriteIndex + 1) % sampleBuffer.size();
    }
    repaint();
  }

  void paint(juce::Graphics &g) override
  {
    g.fillAll(juce::Colour(0xffe8f5ec));

    g.setColour(juce::Colour(0xff2e8b57));
    g.drawRect(getLocalBounds(), 1);

    g.setColour(juce::Colour(0x261e5631));
    g.drawHorizontalLine(getHeight() / 2, 0.0f, static_cast<float>(getWidth()));

    size_t triggerIndex = 0;
    bool triggerFound = false;

    const size_t bufSize = sampleBuffer.size();

    for (size_t i = 0; i + 1 < bufSize; ++i)
    {
      const size_t idx0 = (bufferWriteIndex + i) % bufSize;
      const size_t idx1 = (idx0 + 1) % bufSize;

      const float sample0 = sampleBuffer[idx0].master;
      const float sample1 = sampleBuffer[idx1].master;

      if (sample0 <= 0.0f && sample1 > 0.0f)
      {
        triggerIndex = idx1;
        triggerFound = true;
        break;
      }
    }

    if (!triggerFound)
      triggerIndex = bufferWriteIndex;

    const auto width = static_cast<float>(getWidth());
    const auto height = static_cast<float>(getHeight());
    const auto centerY = height * 0.5f;

    const auto numPointsToDraw = std::min<size_t>(256, static_cast<size_t>(getWidth()));
    const auto xInc = width / static_cast<float>(numPointsToDraw);

    juce::Path osc1Path;
    juce::Path osc2Path;
    juce::Path masterPath;

    for (size_t i = 0; i < numPointsToDraw; ++i)
    {
      const auto sampleIdx = (triggerIndex + i) % bufSize;
      const auto &frame = sampleBuffer[sampleIdx];
      const auto x = static_cast<float>(i) * xInc;

      const auto yosc1 = centerY - (frame.osc1 * centerY * 0.85f);
      const auto yOsc2 = centerY - (frame.osc2 * centerY * 0.85f);
      const auto yMaster = centerY - (frame.master * centerY * 0.85f);

      if (i == 0)
      {
        osc1Path.startNewSubPath(x, yosc1);
        osc2Path.startNewSubPath(x, yOsc2);
        masterPath.startNewSubPath(x, yMaster);
      }
      else
      {
        osc1Path.lineTo(x, yosc1);
        osc2Path.lineTo(x, yOsc2);
        masterPath.lineTo(x, yMaster);
      }
    }

    g.setColour(juce::Colour(0x4db8860b));
    g.strokePath(osc1Path, juce::PathStrokeType(1.0f));

    g.setColour(juce::Colour(0x4d8b4513));
    g.strokePath(osc2Path, juce::PathStrokeType(1.0f));

    g.setColour(juce::Colour(0xff2e8b57));
    g.strokePath(masterPath, juce::PathStrokeType(2.0f));
  }

private:
  AudioFifo<ScopeFrame, 1024> &fifo;
  std::array<ScopeFrame, 1024> sampleBuffer{};
  size_t bufferWriteIndex{0};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeComponent)
};
