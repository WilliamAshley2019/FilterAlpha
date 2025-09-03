#include "PluginEditor.h"
#include "PluginProcessor.h"

TeeBeeAudioProcessorEditor::TeeBeeAudioProcessorEditor(TeeBeeAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setOpaque(true);
    cutoffSlider.setSliderStyle(juce::Slider::Rotary);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 18);
    cutoffSlider.setTextValueSuffix(" Hz");
    cutoffSlider.setNumDecimalPlacesToDisplay(0);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.attachToComponent(&cutoffSlider, false);
    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(cutoffLabel);
    resonanceSlider.setSliderStyle(juce::Slider::Rotary);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 18);
    resonanceSlider.setTextValueSuffix(" %");
    resonanceSlider.setNumDecimalPlacesToDisplay(1);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.attachToComponent(&resonanceSlider, false);
    addAndMakeVisible(resonanceSlider);
    addAndMakeVisible(resonanceLabel);
    driveSlider.setSliderStyle(juce::Slider::Rotary);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 18);
    driveSlider.setTextValueSuffix(" dB");
    driveSlider.setNumDecimalPlacesToDisplay(1);
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.attachToComponent(&driveSlider, false);
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(driveLabel);
    fbHpSlider.setSliderStyle(juce::Slider::Rotary);
    fbHpSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 18);
    fbHpSlider.setTextValueSuffix(" Hz");
    fbHpSlider.setNumDecimalPlacesToDisplay(0);
    fbHpLabel.setText("Feedback HP", juce::dontSendNotification);
    fbHpLabel.attachToComponent(&fbHpSlider, false);
    addAndMakeVisible(fbHpSlider);
    addAndMakeVisible(fbHpLabel);
    fbAmpSlider.setSliderStyle(juce::Slider::Rotary);
    fbAmpSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 70, 18);
    fbAmpSlider.setTextValueSuffix(" %");
    fbAmpSlider.setNumDecimalPlacesToDisplay(1);
    fbAmpLabel.setText("Feedback Amp", juce::dontSendNotification);
    fbAmpLabel.attachToComponent(&fbAmpSlider, false);
    addAndMakeVisible(fbAmpSlider);
    addAndMakeVisible(fbAmpLabel);
    modeBox.addItemList({ "TB-303", "LP 24dB", "LP 18dB", "LP 12dB", "HP 12dB", "Flat" }, 1);
    modeLabel.setText("Mode", juce::dontSendNotification);
    addAndMakeVisible(modeBox);
    addAndMakeVisible(modeLabel);
    automodeToggle.setButtonText("Auto Mode");
    automodeLabel.setText("Automation", juce::dontSendNotification);
    addAndMakeVisible(automodeToggle);
    addAndMakeVisible(automodeLabel);
    auto& params = processorRef.apvts;
    cutoffAttachment = std::make_unique<AttachFloat>(params, "cutoff", cutoffSlider);
    resonanceAttachment = std::make_unique<AttachFloat>(params, "resonance", resonanceSlider);
    driveAttachment = std::make_unique<AttachFloat>(params, "drive", driveSlider);
    fbHpAttachment = std::make_unique<AttachFloat>(params, "fbhp", fbHpSlider);
    fbAmpAttachment = std::make_unique<AttachFloat>(params, "fbamp", fbAmpSlider);
    modeAttachment = std::make_unique<AttachChoice>(params, "mode", modeBox);
    automodeAttachment = std::make_unique<AttachBool>(params, "automode", automodeToggle);
    setSize(720, 300);
}

TeeBeeAudioProcessorEditor::~TeeBeeAudioProcessorEditor() {}

void TeeBeeAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);
    g.drawFittedText("TeeBeeFilter", getLocalBounds().withTrimmedTop(6).removeFromTop(24), juce::Justification::centred, 1);
}

void TeeBeeAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto topRow = area.removeFromTop(160);
    auto bottomRow = area.removeFromTop(80);
    auto left = topRow.removeFromLeft(160);
    cutoffSlider.setBounds(left.reduced(8));
    auto mid = topRow.removeFromLeft(160);
    resonanceSlider.setBounds(mid.reduced(8));
    auto right = topRow.removeFromLeft(160);
    driveSlider.setBounds(right.reduced(8));
    fbHpSlider.setBounds(bottomRow.removeFromLeft(140).reduced(8));
    fbAmpSlider.setBounds(bottomRow.removeFromLeft(140).reduced(8));
    modeBox.setBounds(bottomRow.removeFromLeft(120).reduced(6));
    automodeToggle.setBounds(bottomRow.removeFromLeft(100).reduced(6));
}