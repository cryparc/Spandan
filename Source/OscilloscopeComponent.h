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
  OscilloscopeComponent(AudioFifo<float, 1024> &fifoToUse) : fifo(fifoToUse)
  {
    startTimerHz(60); // Update at 60 Hz
  }

  ~OscilloscopeComponent() override
  {
    stopTimer();
  }

  void timerCallback() override
  {
    float sample = 0.0f;
    while (fifo.pop(sample))
    {
      sampleBuffer[bufferWriteIndex] = sample;
      bufferWriteIndex = (bufferWriteIndex + 1) % sampleBuffer.size();
    }
    repaint();
  }

  void paint(juce::Graphics &g) override
  {
    g.fillAll(juce::Colour(0xff121214)); // Dark background
    // Draw Oscilloscope Border & Grid Lines
    g.setColour(juce::Colours::darkgrey.withAlpha(0.4f));
    g.drawRect(getLocalBounds(), 1);
    g.drawHorizontalLine(getHeight() / 2, 0.0f, static_cast<float>(getWidth()));

    // Positive Zero-Crossing Detection
    size_t triggerIndex = 0;
    bool triggerFound = false;

    const size_t bufSize = sampleBuffer.size();

    for (size_t i = 0; i < bufSize - 1; ++i)
    {
      size_t idx0 = (bufferWriteIndex + i) % bufSize;
      size_t idx1 = (idx0 + 1) % bufSize;

      if (sampleBuffer[idx0] <= 0.0f && sampleBuffer[idx1] > 0.0f)
      {
        triggerIndex = idx1;
        triggerFound = true;
        break;
      }
    }

    if (!triggerFound)
      triggerIndex = bufferWriteIndex;

    // Render Waveform Path
    juce::Path wavePath;
    const float width = static_cast<float>(getWidth());
    const float height = static_cast<float>(getHeight());
    const float centerY = height / 2.0f;

    const int numPointsToDraw = std::min(256, static_cast<int>(width)); // Limit to 256 points for performance
    const float xInc = width / static_cast<float>(numPointsToDraw - 1);

    for (int i = 0; i < numPointsToDraw; ++i)
    {
      size_t sampleIdx = (triggerIndex + i) % bufSize;
      float sampleVal = sampleBuffer[sampleIdx];
      float x = i * xInc;
      float y = centerY - (sampleVal * (height / 2.0f) * 0.85f); // 85% headroom scaling

      if (i == 0)
        wavePath.startNewSubPath(x, y);
      else
        wavePath.lineTo(x, y);
    }

    g.setColour(juce::Colour(0xff00e5ff)); // Bright Cyan Glow
    g.strokePath(wavePath, juce::PathStrokeType(2.0f));
  }

private:
  AudioFifo<float, 1024> &fifo;
  std::array<float, 1024> sampleBuffer{};
  size_t bufferWriteIndex{0};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeComponent)
};
