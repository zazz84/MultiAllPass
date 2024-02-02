/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class FirstOrderAllPass
{
public:
	FirstOrderAllPass();

	void init(int sampleRate);
	void setCoefrequencyParameter(float frequency);
	void setCoef(float coef);
	float process(float in);

protected:
	float m_SampleRate;
	float m_a1 = -1.0f; // all pass filter coeficient
	float m_d = 0.0f;   // history d = x[n-1] - a1y[n-1]
};

//==============================================================================
class SecondOrderAllPass
{
public:
	SecondOrderAllPass();

	void init(int sampleRate);
	void setCoefrequencyParameter(float frequency, float Q);
	float process(float in);

protected:
	float m_SampleRate;

	float m_xnz2 = 0.0f;
	float m_xnz1 = 0.0f;
	float m_ynz2 = 0.0f;
	float m_ynz1 = 0.0f;

	float m_a0 = 0.0f;
	float m_a1 = 0.0f;
};

//==============================================================================
class MultiAllPassAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{

public:
    //==============================================================================
    MultiAllPassAudioProcessor();
    ~MultiAllPassAudioProcessor() override;

	static const int N_ALL_PASS_FO = 100;
	static const int N_ALL_PASS_SO = 50;
	static const int FREQUENCY_MIN = 20;
	static const int FREQUENCY_MAX = 20000;
	static const std::string paramsNames[];

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
	bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	inline float FrequencyToMel(float frequency)
	{
		return 2595.0f * log10f(1.0f + frequency / 700.0f);
	}
	inline float MelToFrequency(float mel)
	{
		return 700.0f * (powf(10.0f, mel / 2595.0f) - 1.0f);
	}

	using APVTS = juce::AudioProcessorValueTreeState;
	static APVTS::ParameterLayout createParameterLayout();

	APVTS apvts{ *this, nullptr, "Parameters", createParameterLayout() };

private:	
	//==============================================================================

	std::atomic<float>* frequencyParameter = nullptr;
	std::atomic<float>* styleParameter = nullptr;
	std::atomic<float>* intensityParameter = nullptr;
	std::atomic<float>* volumeParameter = nullptr;

	juce::AudioParameterBool* button1Parameter = nullptr;
	juce::AudioParameterBool* button2Parameter = nullptr;

	FirstOrderAllPass m_firstOrderAllPass[N_ALL_PASS_FO * 2] = {};
	SecondOrderAllPass m_secondOrderAllPass[N_ALL_PASS_SO * 2] = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiAllPassAudioProcessor)
};
