#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class HuntericeFirstPluginAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit HuntericeFirstPluginAudioProcessorEditor(HuntericeFirstPluginAudioProcessor&);
    ~HuntericeFirstPluginAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HuntericeFirstPluginAudioProcessor& processorRef;

    juce::Slider driveSlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;

    juce::Label driveLabel;
    juce::Label mixLabel;
    juce::Label outputLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HuntericeFirstPluginAudioProcessorEditor)
};
