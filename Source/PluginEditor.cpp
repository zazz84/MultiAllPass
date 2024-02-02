/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MultiAllPassAudioProcessorEditor::MultiAllPassAudioProcessorEditor (MultiAllPassAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState(vts)
{	
	juce::Colour light  = juce::Colour::fromHSV(0.9f, 0.5f, 0.6f, 1.0f);
	juce::Colour medium = juce::Colour::fromHSV(0.9f, 0.5f, 0.5f, 1.0f);
	juce::Colour dark   = juce::Colour::fromHSV(0.9f, 0.5f, 0.4f, 1.0f);

	const int fonthHeight = (int)(SLIDER_WIDTH / FONT_DIVISOR);

	for (int i = 0; i < N_SLIDERS; i++)
	{
		auto& label = m_labels[i];
		auto& slider = m_sliders[i];

		//Lable
		label.setText(MultiAllPassAudioProcessor::paramsNames[i], juce::dontSendNotification);
		label.setFont(juce::Font(fonthHeight, juce::Font::bold));
		label.setJustificationType(juce::Justification::centred);
		addAndMakeVisible(label);

		//Slider
		slider.setLookAndFeel(&otherLookAndFeel);
		slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
		slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, SLIDER_FONT_SIZE);
		addAndMakeVisible(slider);
		m_sliderAttachment[i].reset(new SliderAttachment(valueTreeState, MultiAllPassAudioProcessor::paramsNames[i], slider));
	}

	// Buttons
	addAndMakeVisible(type1Button);
	addAndMakeVisible(type2Button);

	type1Button.setRadioGroupId(TYPE_BUTTON_GROUP);
	type2Button.setRadioGroupId(TYPE_BUTTON_GROUP);

	type1Button.setClickingTogglesState(true);
	type2Button.setClickingTogglesState(true);

	button1Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Button1", type1Button));
	button2Attachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(valueTreeState, "Button2", type2Button));

	type1Button.setColour(juce::TextButton::buttonColourId, light);
	type2Button.setColour(juce::TextButton::buttonColourId, light);

	type1Button.setColour(juce::TextButton::buttonOnColourId, dark);
	type2Button.setColour(juce::TextButton::buttonOnColourId, dark);

	type1Button.setLookAndFeel(&otherLookAndFeel);
	type2Button.setLookAndFeel(&otherLookAndFeel);

	// Canvas
	setResizable(true, true);
	const float width = SLIDER_WIDTH * N_SLIDERS;
	setSize(width, SLIDER_WIDTH);

	if (auto* constrainer = getConstrainer())
	{
		constrainer->setFixedAspectRatio(N_SLIDERS);
		constrainer->setSizeLimits(width * 0.7f, SLIDER_WIDTH * 0.7, width * 2.0f, SLIDER_WIDTH * 2.0f);
	}
}

MultiAllPassAudioProcessorEditor::~MultiAllPassAudioProcessorEditor()
{
}

//==============================================================================
void MultiAllPassAudioProcessorEditor::paint (juce::Graphics& g)
{
	g.fillAll(juce::Colour::fromHSV(0.9f, 0.5f, 0.7f, 1.0f));
}

void MultiAllPassAudioProcessorEditor::resized()
{
	const int width = (int)(getWidth() / N_SLIDERS);
	const int height = getHeight();
	const int fonthHeight = (int)(height / FONT_DIVISOR);
	const int labelOffset = (int)(SLIDER_WIDTH / FONT_DIVISOR) + 5;

	// Sliders + Labels
	for (int i = 0; i < N_SLIDERS; ++i)
	{
		juce::Rectangle<int> rectangle;

		rectangle.setSize(width, height);
		rectangle.setPosition(i * width, 0);
		m_sliders[i].setBounds(rectangle);

		rectangle.removeFromBottom(labelOffset);
		m_labels[i].setBounds(rectangle);

		m_labels[i].setFont(juce::Font(fonthHeight, juce::Font::bold));
	}

	// Buttons
	const int posY = height - (int)(1.8f * fonthHeight);

	type1Button.setBounds((int)(getWidth() * 0.5f - fonthHeight * 1.1f), posY, fonthHeight, fonthHeight);
	type2Button.setBounds((int)(getWidth() * 0.5f + fonthHeight * 0.1f), posY, fonthHeight, fonthHeight);
}