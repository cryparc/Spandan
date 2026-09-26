#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <cmath>

class ADSR
{
public:
	ADSR() noexcept;
	~ADSR() noexcept = default;

	void setSampleRate(double newSampleRate) noexcept;
	void reset() noexcept;

	float process() noexcept;

	void gate(bool on) noexcept;
	void gate(int on) noexcept { gate(on != 0); }

	void setAttackTime(float timeInSeconds) noexcept;
	void setDecayTime(float timeInSeconds) noexcept;
	void setSustainLevel(float level) noexcept;
	void setReleaseTime(float timeInSeconds) noexcept;

	void setAttackRate(float rateInSamples) noexcept;
	void setDecayRate(float rateInSamples) noexcept;
	void setReleaseRate(float rateInSamples) noexcept;

	void setTargetRatioA(float targetRatio) noexcept;
	void setTargetRatioDR(float targetRatio) noexcept;

	float getOutput() const noexcept { return output; }
	int getState() const noexcept { return state; }
	bool isActive() const noexcept { return state != env_idle; }

private:
    enum envState
    {
        env_idle = 0,
        env_attack,
        env_decay,
        env_sustain,
        env_release
    };

    void updateBaseMultipliers() noexcept;
    float calcCoef(float rate, float targetRatio) const noexcept;

    int state { env_idle };
    float output { 0.0f };

    double sampleRate { 44100.0 };

    // Stored in seconds
    float attackTimeInSeconds { 0.01f };
    float decayTimeInSeconds { 0.1f };
    float releaseTimeInSeconds { 0.2f };

    // Stored in samples (calculated based on sample rate)
    float attackRate { 0.0f };
    float decayRate { 0.0f };
    float releaseRate { 0.0f };

    float attackCoef { 0.0f };
    float decayCoef { 0.0f };
    float releaseCoef { 0.0f };

    float sustainLevel { 1.0f };
    float targetRatioA { 0.3f };
    float targetRatioDR { 0.0001f };

    float attackBase { 0.0f };
    float decayBase { 0.0f };
    float releaseBase { 0.0f };
};
