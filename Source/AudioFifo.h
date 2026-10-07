/*
  ==============================================================================

    AudioFifo.h
    Created: 30 Sep 2026 1:10:18am
    Author:  prash

    Description: A lock-free FIFO buffer for audio data.
  ==============================================================================
*/

#pragma once
#include <atomic>
#include <array>
#include <cstdint>

struct ScopeFrame
{
  float osc1{0.0f};
  float osc2{0.0f};
  float master{0.0f};
};

template <typename T, size_t Capacity = 1024>
class AudioFifo
{
public:
  AudioFifo() noexcept = default;
  ~AudioFifo() noexcept = default;

  bool push(T sample) noexcept
  {
    const auto currentWrite = writeIdx.load(std::memory_order_relaxed);
    const auto currentRead = readIdx.load(std::memory_order_acquire);

    if (((currentWrite + 1) % Capacity) == currentRead)
      return false;

    buffer[currentWrite] = sample;
    writeIdx.store((currentWrite + 1) % Capacity, std::memory_order_release);

    return true;
  }

  bool pop(T &sample) noexcept
  {
    const auto currentRead = readIdx.load(std::memory_order_relaxed);
    const auto currentWrite = writeIdx.load(std::memory_order_acquire);

    if (currentRead == currentWrite)
      return false;

    sample = buffer[currentRead];
    readIdx.store((currentRead + 1) % Capacity, std::memory_order_release);

    return true;
  }

  void reset() noexcept
  {
    writeIdx.store(0, std::memory_order_relaxed);
    readIdx.store(0, std::memory_order_relaxed);
  }

private:
  std::array<T, Capacity> buffer{};
  std::atomic<size_t> writeIdx{0};
  std::atomic<size_t> readIdx{0};
};
