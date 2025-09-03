#pragma once
#ifndef TEEBEE_AUDIO_PROCESSOR_EDITOR_H_INCLUDED
#define TEEBEE_AUDIO_PROCESSOR_EDITOR_H_INCLUDED

#include <JuceHeader.h>

// Forward declaration
class TeeBeeAudioProcessor;

class TeeBeeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TeeBeeAudioProcessorEditor(TeeBeeAudioProcessor&);
    ~TeeBeeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    TeeBeeAudioProcessor& processorRef;
    juce::Slider cutoffSlider, resonanceSlider, driveSlider, fbHpSlider, fbAmpSlider;
    juce::ComboBox modeBox;
    juce::ToggleButton automodeToggle;
    juce::Label cutoffLabel, resonanceLabel, driveLabel, modeLabel, fbHpLabel, fbAmpLabel, automodeLabel;

    using AttachFloat = juce::AudioProcessorValueTreeState::SliderAttachment;
    using AttachChoice = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using AttachBool = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<AttachFloat> cutoffAttachment, resonanceAttachment, driveAttachment, fbHpAttachment, fbAmpAttachment;
    std::unique_ptr<AttachChoice> modeAttachment;
    std::unique_ptr<AttachBool> automodeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TeeBeeAudioProcessorEditor)
};

#endif // TEEBEE_AUDIO_PROCESSOR_EDITOR_H_INCLUDED