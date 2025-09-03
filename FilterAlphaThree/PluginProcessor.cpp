#include "PluginProcessor.h"
#include "PluginEditor.h"

// Constructor
TeeBeeAudioProcessor::TeeBeeAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    apvts(*this, nullptr, "PARAMS", createParameterLayout())
{
    cutoffSmoothed.setCurrentAndTargetValue(1000.0);
    resonanceSmoothed.setCurrentAndTargetValue(0.2);
    driveSmoothed.setCurrentAndTargetValue(0.0);
    fbHpSmoothed.setCurrentAndTargetValue(300.0);
    fbAmpSmoothed.setCurrentAndTargetValue(0.5);
}

// Destructor
TeeBeeAudioProcessor::~TeeBeeAudioProcessor() {}

// Editor Creation
juce::AudioProcessorEditor* TeeBeeAudioProcessor::createEditor()
{
    return new TeeBeeAudioProcessorEditor(*this);
}

// Parameter Layout
juce::AudioProcessorValueTreeState::ParameterLayout TeeBeeAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "cutoff", "Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 1000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "resonance", "Resonance",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 20.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "drive", "Drive",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "mode", "Mode", juce::StringArray{ "TB-303", "LP 24dB", "LP 18dB", "LP 12dB", "HP 12dB", "Flat" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fbhp", "Feedback HP",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 300.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fbamp", "Feedback Amp",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 50.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "automode", "Automation Mode", false));
    return { params.begin(), params.end() };
}

// Prepare to Play
void TeeBeeAudioProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    (void)samplesPerBlock;
    sampleRate = newSampleRate;
    automationMode = apvts.getRawParameterValue("automode")->load() > 0.5f;
    const double smoothTime = automationMode ? 0.001 : 0.05;
    cutoffSmoothed.reset(sampleRate, smoothTime);
    resonanceSmoothed.reset(sampleRate, smoothTime);
    driveSmoothed.reset(sampleRate, smoothTime);
    fbHpSmoothed.reset(sampleRate, smoothTime);
    fbAmpSmoothed.reset(sampleRate, smoothTime);
    for (auto& filter : filters) {
        filter.setSampleRate(sampleRate);
        filter.reset();
    }
}

// Release Resources
void TeeBeeAudioProcessor::releaseResources() {}

// Channel Layout Check
#ifndef JucePlugin_PreferredChannelConfigurations
bool TeeBeeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() == layouts.getMainInputChannelSet();
}
#endif

// Latency
double TeeBeeAudioProcessor::getLatencyInSamples() const { return 0.0; }

// Process Block
void TeeBeeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0 || numChannels == 0) return;
    automationMode = apvts.getRawParameterValue("automode")->load() > 0.5f;
    cutoffSmoothed.setTargetValue(apvts.getRawParameterValue("cutoff")->load());
    resonanceSmoothed.setTargetValue(apvts.getRawParameterValue("resonance")->load() * 0.01);
    driveSmoothed.setTargetValue(apvts.getRawParameterValue("drive")->load());
    fbHpSmoothed.setTargetValue(apvts.getRawParameterValue("fbhp")->load());
    fbAmpSmoothed.setTargetValue(apvts.getRawParameterValue("fbamp")->load() * 0.01);
    for (auto& filter : filters)
    {
        filter.setCutoff(cutoffSmoothed.getNextValue());
        filter.setResonance(resonanceSmoothed.getNextValue());
        filter.setDriveDb(driveSmoothed.getNextValue());
        filter.setFeedbackHP(fbHpSmoothed.getNextValue());
        filter.setFeedbackAmp(fbAmpSmoothed.getNextValue());
        filter.setMode(static_cast<int>(apvts.getRawParameterValue("mode")->load()));
    }
    for (int ch = 0; ch < juce::jmin(numChannels, 2); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            data[i] = filters[ch].processSample(data[i]);
            jassert(TeeBeeFilter::isValid(data[i]));
        }
    }
}

// State Management
void TeeBeeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TeeBeeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState) apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

// Filter Implementation
TeeBeeAudioProcessor::TeeBeeFilter::TeeBeeFilter()
{
    calculateCoefficientsApprox();
    updateFeedbackHPCoeffs();
    reset();
}

void TeeBeeAudioProcessor::TeeBeeFilter::setSampleRate(double sr)
{
    if (sr >= 44100.0) {
        sampleRate = sr;
        twoPiOverSampleRate = 2.0 * juce::MathConstants<double>::pi / sr;
        updateFeedbackHPCoeffs();
        calculateCoefficientsApprox();
        reset();
    }
}

void TeeBeeAudioProcessor::TeeBeeFilter::reset()
{
    y1 = y2 = y3 = y4 = fb_hp_z1 = fb_inPrev = fb_lp_z1 = dc_x1 = dc_y1 = 0.0;
}

void TeeBeeAudioProcessor::TeeBeeFilter::setCutoff(double fc, bool updateCoeffs)
{
    cutoff = clip(fc, 20.0, 20000.0);
    if (updateCoeffs) calculateCoefficientsApprox();
}

void TeeBeeAudioProcessor::TeeBeeFilter::setResonance(double rPercent, bool updateCoeffs)
{
    resonanceRaw = clip(rPercent, 0.0, 100.0) * 0.01;
    resonanceSkewed = resonanceRaw;
    if (updateCoeffs) calculateCoefficientsApprox();
}

void TeeBeeAudioProcessor::TeeBeeFilter::setDriveDb(double db)
{
    driveDb = clip(db, -60.0, 60.0);
    driveFactor = std::pow(10.0, driveDb * 0.05) * 0.25;
}

void TeeBeeAudioProcessor::TeeBeeFilter::setMode(int newMode)
{
    if (newMode >= 0 && newMode < NUM_MODES) {
        mode = newMode;
        switch (mode) {
        case TB_303: c0 = 0.0; c1 = 0.0; c2 = 0.0; c3 = 0.0; c4 = 1.0; break;
        case LP_24: c0 = 0.0; c1 = 0.0; c2 = 0.0; c3 = 0.0; c4 = 1.0; break;
        case LP_18: c0 = 0.0; c1 = 0.0; c2 = 0.0; c3 = 1.0; c4 = 0.0; break;
        case LP_12: c0 = 0.0; c1 = 0.0; c2 = 1.0; c3 = 0.0; c4 = 0.0; break;
        case HP_12: c0 = 1.0; c1 = -2.0; c2 = 1.0; c3 = 0.0; c4 = 0.0; break;
        case FLAT: c0 = 1.0; c1 = 0.0; c2 = 0.0; c3 = 0.0; c4 = 0.0; break;
        default: c0 = 1.0; c1 = 0.0; c2 = 0.0; c3 = 0.0; c4 = 0.0; break;
        }
        calculateCoefficientsApprox();
    }
}

void TeeBeeAudioProcessor::TeeBeeFilter::setFeedbackHP(double fc)
{
    feedbackHpCutoff = clip(fc, 20.0, 20000.0);
    updateFeedbackHPCoeffs();
}

void TeeBeeAudioProcessor::TeeBeeFilter::setFeedbackAmp(double amp)
{
    feedbackAmp = clip(amp, 0.0, 100.0) * 0.01;
}

void TeeBeeAudioProcessor::TeeBeeFilter::updateFeedbackHPCoeffs()
{
    double x = std::exp(-2.0 * juce::MathConstants<double>::pi * feedbackHpCutoff / sampleRate);
    fb_hp_a0 = 1.0 + x;
    fb_hp_a1 = -(1.0 + x);
    fb_hp_b1 = -x;
}

void TeeBeeAudioProcessor::TeeBeeFilter::calculateCoefficientsApprox()
{
    double wc = 2.0 * juce::MathConstants<double>::pi * cutoff / sampleRate;
    a1 = -std::exp(-wc);
    b0 = cutoff / sampleRate;
    k = resonanceSkewed * 4.0;
    if (mode == TB_303) {
        k *= 1.5;
    }
}

double TeeBeeAudioProcessor::TeeBeeFilter::dcBlock(double in)
{
    constexpr double R = 0.9995; // 10 Hz @ 44.1 kHz
    double y = in - dc_x1 + R * dc_y1;
    dc_x1 = in;
    dc_y1 = y;
    return y;
}

float TeeBeeAudioProcessor::TeeBeeFilter::processSample(float in)
{
    juce::ScopedNoDenormals noDenormals;
    double input = 0.125 * driveFactor * static_cast<double>(in);
    input = clip(input, -2.0, 2.0);
    auto softClip = [](double x) { return std::tanh(std::clamp(x, -6.0, 6.0)); };
    double fb = k * feedbackAmp * y4;
    double fb_lp = 0.9 * fb_lp_z1 + 0.1 * fb;
    fb_lp_z1 = fb_lp;
    fb = clip(fb_lp, -2.0, 2.0);
    double hp = fb_hp_a0 * fb + fb_hp_a1 * fb_inPrev - fb_hp_b1 * fb_hp_z1;
    fb_inPrev = fb;
    fb_hp_z1 = clip(hp, -2.0, 2.0);
    double y0 = input - hp;
    y0 += 1e-12;
    if (mode == TB_303) {
        double modeGain = 1.0;
        y0 -= k * feedbackAmp * modeGain * y4;
        y1 += b0 * (softClip(y0) - softClip(y1));
        y2 += b0 * (softClip(y1) - softClip(y2));
        y3 += b0 * (softClip(y2) - softClip(y3));
        y4 += b0 * (softClip(y3) - softClip(y4));
        y1 = clip(y1, -2.0, 2.0);
        y2 = clip(y2, -2.0, 2.0);
        y3 = clip(y3, -2.0, 2.0);
        y4 = clip(y4, -2.0, 2.0);
        if (!std::isfinite(y1)) y1 = 0.0;
        if (!std::isfinite(y2)) y2 = 0.0;
        if (!std::isfinite(y3)) y3 = 0.0;
        if (!std::isfinite(y4)) y4 = 0.0;
        double out = y4;
        out = softClip(out * 0.8) * 1.25;
        float result = static_cast<float>(dcBlock(clip(out, -2.0, 2.0)));
        return isValid(result) ? result : 0.0f;
    }
    else {
        double modeGain = 0.7;
        y0 -= k * feedbackAmp * modeGain * y4;
        y1 = y0 + a1 * (y0 - y1);
        y2 = y1 + a1 * (y1 - y2);
        y3 = y2 + a1 * (y2 - y3);
        y4 = y3 + a1 * (y3 - y4);
        y1 = clip(y1, -2.0, 2.0);
        y2 = clip(y2, -2.0, 2.0);
        y3 = clip(y3, -2.0, 2.0);
        y4 = clip(y4, -2.0, 2.0);
        if (!std::isfinite(y1)) y1 = 0.0;
        if (!std::isfinite(y2)) y2 = 0.0;
        if (!std::isfinite(y3)) y3 = 0.0;
        if (!std::isfinite(y4)) y4 = 0.0;
        double out = 5.0 * (c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3 + c4 * y4);
        out = softClip(out * 0.8) * 1.25;
        float result = static_cast<float>(dcBlock(clip(out, -2.0, 2.0)));
        return isValid(result) ? result : 0.0f;
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TeeBeeAudioProcessor();
}