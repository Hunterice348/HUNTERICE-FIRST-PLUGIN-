#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

HuntericeFirstPluginAudioProcessor::HuntericeFirstPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

HuntericeFirstPluginAudioProcessor::~HuntericeFirstPluginAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout
HuntericeFirstPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"drive", 1}, "Drive",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 0.5f), 20.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"mix", 1}, "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f), 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"output", 1}, "Output",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f, 1.0f), 80.0f));

    return {params.begin(), params.end()};
}

const juce::String HuntericeFirstPluginAudioProcessor::getName() const { return JucePlugin_Name; }
bool HuntericeFirstPluginAudioProcessor::acceptsMidi() const { return false; }
bool HuntericeFirstPluginAudioProcessor::producesMidi() const { return false; }
bool HuntericeFirstPluginAudioProcessor::isMidiEffect() const { return false; }
double HuntericeFirstPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int HuntericeFirstPluginAudioProcessor::getNumPrograms() { return 1; }
int HuntericeFirstPluginAudioProcessor::getCurrentProgram() { return 0; }
void HuntericeFirstPluginAudioProcessor::setCurrentProgram(int) {}
const juce::String HuntericeFirstPluginAudioProcessor::getProgramName(int) { return {}; }
void HuntericeFirstPluginAudioProcessor::changeProgramName(int, const juce::String&) {}

void HuntericeFirstPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    const double rampDurationSec = 0.02; // 20 мс — плавное изменение параметров
    smoothedDrive.reset(sampleRate, rampDurationSec);
    smoothedMix.reset(sampleRate, rampDurationSec);
    smoothedOutput.reset(sampleRate, rampDurationSec);
}

void HuntericeFirstPluginAudioProcessor::releaseResources() {}

bool HuntericeFirstPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    const auto& mainIn  = layouts.getMainInputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() &&
        mainOut != juce::AudioChannelSet::stereo())
        return false;

    if (mainIn != mainOut)
        return false;

    return true;
}

// Реальный алгоритм Soft-Clip Distortion (tanh-based)
inline float softClip(float sample, float driveGain)
{
    return std::tanh(sample * driveGain);
}

void HuntericeFirstPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                      juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // Очищаем неиспользуемые выходные каналы
    for (int channel = totalNumInputChannels; channel < totalNumOutputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    // Обновляем целевые значения сглаживания
    const float driveParam   = apvts.getRawParameterValue("drive")->load();
    const float mixParam     = apvts.getRawParameterValue("mix")->load();
    const float outputParam  = apvts.getRawParameterValue("output")->load();

    // Преобразуем параметры в рабочие значения
    // Drive: 0..100 -> коэффициент усиления 1..20
    const float driveGain = 1.0f + (driveParam / 100.0f) * 19.0f;
    // Mix: 0..100 -> 0..1
    const float mix = mixParam / 100.0f;
    // Output: 0..100 -> 0..1 (компенсация громкости)
    const float outputGain = outputParam / 100.0f;

    smoothedDrive.setTargetValue(driveGain);
    smoothedMix.setTargetValue(mix);
    smoothedOutput.setTargetValue(outputGain);

    const int numSamples = buffer.getNumSamples();

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float currentDrive  = smoothedDrive.getNextValue();
            const float currentMix    = smoothedMix.getNextValue();
            const float currentOutput = smoothedOutput.getNextValue();

            const float dry = channelData[sample];
            const float wet = softClip(dry, currentDrive);

            // Линейный кроссфейд между сухим и обработанным сигналом
            const float mixed = dry * (1.0f - currentMix) + wet * currentMix;

            // Применяем выходную громкость с ограничением
            float out = mixed * currentOutput;
            out = juce::jlimit(-1.0f, 1.0f, out);

            channelData[sample] = out;
        }
    }
}

bool HuntericeFirstPluginAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* HuntericeFirstPluginAudioProcessor::createEditor()
{
    return new HuntericeFirstPluginAudioProcessorEditor(*this);
}

void HuntericeFirstPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void HuntericeFirstPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HuntericeFirstPluginAudioProcessor();
}
