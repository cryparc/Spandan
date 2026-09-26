#include "ADSR.h"

ADSR::ADSR() noexcept
{
	reset();
	setTargetRatioA(0.3f);
	setTargetRatioDR(0.0001f);
	setAttackTime(0.01f);
	setDecayTime(0.1f);
	setReleaseTime(0.2f);
	setSustainLevel(1.0f);
}

void ADSR::setSampleRate(double newSampleRate) noexcept
{
    if (newSampleRate &gt; 0.0 &amp;&amp; sampleRate != newSampleRate)
    {
        sampleRate = newSampleRate;
        setAttackTime(attackTimeInSeconds);
        setDecayTime(decayTimeInSeconds);
        setReleaseTime(releaseTimeInSeconds);
    }
}

void ADSR::reset() noexcept
{
	state = env_idle;
	output = 0.0f;
}

float ADSR::calcCoef(float rate, float targetRatio) const noexcept
{
	if (rate <= 0.0f)
		return 0.0f;

	return std::exp(-std::log((1.0f + targetRatio) / targetRatio) / rate);
}

void ADSR::updateBaseMultipliers() noexcept
{
	attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);
	decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
	releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}

void ADSR::setAttackTime(float timeInSeconds) noexcept
{
	attackTimeInSeconds = std::max(0.0f, timeInSeconds);
	setAttackRate(attackTimeInSeconds * static_cast<float>(sampleRate));
}

void ADSR::setReleaseTime(float timeInSeconds) noexcept
{
	releaseTimeInSeconds = std::max(0.0f, timeInSeconds);
	setReleaseRate(releaseTimeInSeconds * static_cast<float>(sampleRate));
}

void ADSR::setDecayTime(float timeInSeconds) noexcept
{
	decayTimeInSeconds = std::max(0.0f, timeInSeconds);
	setDecayRate(decayTimeInSeconds * static_cast<float>(sampleRate));
}

void ADSR::setSustainLevel(float level) noexcept
{
	sustainLevel = std::clamp(level, 0.0f, 1.0f);
	decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
}

void ADSR::setAttackRate(float rateInSamples) noexcept
{
	attackRate = std::max(0.0f, rateInSamples);
	attackCoef = calcCoef(attackRate, targetRatioA);
	attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);
}

void ADSR::setDecayRate(float rateInSamples) noexcept
{
	decayRate = std::max(0.0f, rateInSamples);
	decayCoef = calcCoef(decayRate, targetRatioDR);
	decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
}

void ADSR::setReleaseRate(float rateInSamples) noexcept
{
	releaseRate = std::max(0.0f, rateInSamples);
	releaseCoef = calcCoef(releaseRate, targetRatioDR);
	releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}

void ADSR::setTargetRatioA(float targetRatio) noexcept
{
	targetRatioA = std::max(0.000000001f, targetRatio);
	attackCoef = calcCoef(attackRate, targetRatioA);
	attackBase = (1.0f + targetRatioA) * (1.0f - attackCoef);
}

void ADSR::setTargetRatioDR(float targetRatio) noexcept
{
	targetRatioDR = std::max(0.000000001f, targetRatio);
	decayCoef = calcCoef(decayRate, targetRatioDR);
	releaseCoef = calcCoef(releaseRate, targetRatioDR);
	decayBase = (sustainLevel - targetRatioDR) * (1.0f - decayCoef);
	releaseBase = -targetRatioDR * (1.0f - releaseCoef);
}

void ADSR::gate(bool on) noexcept
{
	if (on)
	{
		if (state == env_idle)
			output = 0.0f;

		state = env_attack;
	}
	else if (state != env_idle)
	{
		state = env_release;
	}
}

float ADSR::process() noexcept
{
	switch (state)
	{
		case env_idle:
			break;

		case env_attack:
			output = attackBase + output * attackCoef;
			if (output >= 1.0f)
			{
				output = 1.0f;
				state = env_decay;
			}
			break;

		case env_decay:
			output = decayBase + output * decayCoef;
			if (output <= sustainLevel)
			{
				output = sustainLevel;
				state = env_sustain;
			}
			break;

		case env_sustain:
			output = sustainLevel;
			break;

		case env_release:
			output = releaseBase + output * releaseCoef;
			if (output <= 0.0f)
			{
				output = 0.0f;
				state = env_idle;
			}
			break;

		default:
			jassertfalse;
			break;
	}

	return output;
}