/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FirstOrderAllPass::FirstOrderAllPass()
{
}

void FirstOrderAllPass::init(int sampleRate)
{
	m_SampleRate = sampleRate;
}

void FirstOrderAllPass::setCoefFrequency(float frequency)
{
	if (m_SampleRate == 0)
	{
		return;
	}

	const float tmp = tanf(3.14f * frequency / m_SampleRate);
	m_a1 = (tmp - 1.0f) / (tmp + 1.0f);
}

void FirstOrderAllPass::setCoef(float coef)
{
	m_a1 = coef;
}

float FirstOrderAllPass::process(float in)
{
	const float tmp = m_a1 * in + m_d;
	m_d = in - m_a1 * tmp;
	return tmp;
}

//==============================================================================

const std::string MultiAllPassAudioProcessor::paramsNames[] = { "FMin", "FMax", "Count", "Mix", "Volume" };

//==============================================================================
MultiAllPassAudioProcessor::MultiAllPassAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	fMinParameter      = apvts.getRawParameterValue(paramsNames[0]);
	fMaxParameter      = apvts.getRawParameterValue(paramsNames[1]);
	countParameter     = apvts.getRawParameterValue(paramsNames[2]);
	mixParameter       = apvts.getRawParameterValue(paramsNames[3]);
	volumeParameter    = apvts.getRawParameterValue(paramsNames[4]);

	buttonAParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonA"));
	buttonBParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonB"));
	buttonCParameter = static_cast<juce::AudioParameterBool*>(apvts.getParameter("ButtonC"));
}

MultiAllPassAudioProcessor::~MultiAllPassAudioProcessor()
{
}

//==============================================================================
const juce::String MultiAllPassAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MultiAllPassAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MultiAllPassAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MultiAllPassAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MultiAllPassAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MultiAllPassAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MultiAllPassAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MultiAllPassAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MultiAllPassAudioProcessor::getProgramName (int index)
{
    return {};
}

void MultiAllPassAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MultiAllPassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	for (int i = 0; i < N_ALL_PASS * 2; i++)
	{
		m_firstOrderAllPass[i].init((int)(sampleRate));
	}
}

void MultiAllPassAudioProcessor::releaseResources()
{
	
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MultiAllPassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MultiAllPassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	// Get params
	const auto fMin = fMinParameter->load();
	const auto fMax = fMaxParameter->load();
	const auto count = countParameter->load();
	const auto mix = mixParameter->load();
	const auto volume = juce::Decibels::decibelsToGain(volumeParameter->load());

	// Buttons
	const auto buttonA = buttonAParameter->get();
	const auto buttonB = buttonBParameter->get();
	const auto buttonC = buttonCParameter->get();

	// Mics constants
	const float mixInverse = 1.0f - mix;
	const int channels = getTotalNumOutputChannels();
	const int samples = buffer.getNumSamples();

	for (int channel = 0; channel < channels; ++channel)
	{
		// Channel pointer
		auto* channelBuffer = buffer.getWritePointer(channel);

		if (buttonA == true)
		{
			const float fMinMel = FrequencyToMel(fMin);
			const float fMaxMel = FrequencyToMel(fMax);
			const float stepMel = (fMaxMel - fMinMel) / count;

			for (int i = 0; i < count; i++)
			{
				const float frequency = MelToFrequency(fMinMel + i * stepMel);
				m_firstOrderAllPass[channel * N_ALL_PASS + i].setCoefFrequency(frequency);
			}
		}
		else if (buttonB == true)
		{
			const float step = (fMax - fMin) / count;

			for (int i = 0; i < count; i++)
			{
				const float frequency = fMin + i * step;
				m_firstOrderAllPass[channel * N_ALL_PASS + i].setCoefFrequency(frequency);
			}
		}
		else if (buttonC == true)
		{
			const float factor = powf(fMax - fMin, 1.0f / count);

			for (int i = 0; i < count; i++)
			{
				const float frequency = fMin + pow(factor, i);
				m_firstOrderAllPass[channel * N_ALL_PASS + i].setCoefFrequency(frequency);
			}
		}

		for (int sample = 0; sample < samples; ++sample)
		{
			// Get input
			const float in = channelBuffer[sample];

			float inAllPass = in;

			for (int i = 0; i < count; i++)
			{
				inAllPass = m_firstOrderAllPass[channel * N_ALL_PASS + i].process(inAllPass);
			}

			// Apply volume, mix and send to output
			channelBuffer[sample] = volume * (mix * inAllPass + mixInverse * in);
		}
	}
}

//==============================================================================
bool MultiAllPassAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MultiAllPassAudioProcessor::createEditor()
{
    return new MultiAllPassAudioProcessorEditor (*this, apvts);
}

//==============================================================================
void MultiAllPassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{	
	auto state = apvts.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void MultiAllPassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName(apvts.state.getType()))
			apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessorValueTreeState::ParameterLayout MultiAllPassAudioProcessor::createParameterLayout()
{
	APVTS::ParameterLayout layout;

	using namespace juce;

	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[0], paramsNames[0], NormalisableRange<float>(  10.0f,    1000.0f, 10.0f, 0.5f),  100.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[1], paramsNames[1], NormalisableRange<float>(1000.0f,   10000.0f, 10.0f, 0.5f), 2000.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[2], paramsNames[2], NormalisableRange<float>(   1.0f, N_ALL_PASS,  1.0f, 1.0f),   10.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[3], paramsNames[3], NormalisableRange<float>(   0.0f,       1.0f, 0.05f, 1.0f),   1.0f));
	layout.add(std::make_unique<juce::AudioParameterFloat>(paramsNames[4], paramsNames[4], NormalisableRange<float>( -12.0f,      12.0f,  0.1f, 1.0f),   0.0f));

	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonA", "ButtonA", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonB", "ButtonB", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("ButtonC", "ButtonC", false));

	return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultiAllPassAudioProcessor();
}
