/*
  ==============================================================================

    StateVariableFilter.h
    Created: 29 Sep 2026 10:22:45pm
    Author:  prash

    Description: A simple state variable filter implementation supporting low-pass, high-pass, and band-pass filtering.
  ==============================================================================
*/

#pragma once
#include <cmath>
#include <algorithm>
class StateVariableFilter
{
public:
  enum class FilterType
  {
    LowPass = 0,
    HighPass,
    BandPass
  };

  StateVariableFilter() noexcept = default;

  void prepareToPlay(double sampleRate) noexcept
  {
    currentSampleRate = sampleRate;
    reset();
  }

  void reset() noexcept
  {
    s1 = 0.0f;
    s2 = 0.0f;
  }

  void setParameters(FilterType newType, float cutoffHz, float Q) noexcept
  {
    type = newType;

    const float clampedCutoff = std::clamp(cutoffHz, 20.0f, static_cast<float>(currentSampleRate) * 0.49f);
    const float clampedQ = std::max(0.707f, Q);

    // Calculate filter coefficients
    const float wd = 3.14159265358979323846f * clampedCutoff;
    const float g = std::tan(wd / static_cast<float>(currentSampleRate));

    const float R = 1.0f / (2.0f * clampedQ);

    alpha0 = 1.0f / (1.0f + 2.0f * R * g + g * g);
    alpha = g;
    rho = 2.0f * R + g;
  }

  float processSample(float input) noexcept
  {
    const float hp = alpha0 * (input - rho * s1 - s2);
    const float v1 = alpha * hp;
    const float bp = v1 + s1;
    const float v2 = alpha * bp;
    const float lp = v2 + s2;

    s1 = v1 + bp;
    s2 = v2 + lp;

    switch (type)
    {
    case FilterType::LowPass:
      return lp;
    case FilterType::HighPass:
      return hp;
    case FilterType::BandPass:
      return bp;
    default:
      return lp;
    }
  }

private:
  FilterType type{FilterType::LowPass};
  double currentSampleRate{44100.0};

  float s1{0.0f};
  float s2{0.0f};

  float alpha0{0.0f};
  float alpha{0.0f};
  float rho{0.0f};
};
