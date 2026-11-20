#include "PluginProcessor.h"
#include "PluginEditor.h"

HuntericeFirstPluginAudioProcessorEditor::HuntericeFirstPluginAudioProcessorEditor(
    HuntericeFirstPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(500, 300);

    // Заголовок
    driveLabel.setText("HUNTERICE FIRST PLUGIN", juce::dontSendNotification);
    driveLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);

    // Drive
    driveSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(driveSlider);
    driveAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "drive", driveSlider);

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);

    // Mix
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "mix", mixSlider);

    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    // Output
    outputSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(outputSlider);
    outputAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "output", outputSlider);

    outputLabel.setText("Output", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputLabel);
}

HuntericeFirstPluginAudioProcessorEditor::~HuntericeFirstPluginAudioProcessorEditor() {}

void HuntericeFirstPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(20, 20, 25));
    g.setColour(juce::Colour::fromRGB(180, 30, 30));
    g.drawRect(getLocalBounds(), 2);
}

void HuntericeFirstPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20);

    // Заголовок
    driveLabel.setBounds(bounds.removeFromTop(40));

    // Три слайдера в ряд
    auto sliderArea = bounds.reduced(0, 20);
    const int sliderWidth = sliderArea.getWidth() / 3;

    driveSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(10));
    mixSlider.setBounds(sliderArea.removeFromLeft(sliderWidth).reduced(10));
    outputSlider.setBounds(sliderArea.reduced(10));
}
