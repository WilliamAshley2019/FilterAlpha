#pragma once
#ifndef TEEBEE_AUDIO_PROCESSOR_H_INCLUDED
#define TEEBEE_AUDIO_PROCESSOR_H_INCLUDED

#include <JuceHeader.h>
#include <cmath>

/**
 * TeeBeeFilter VST3 effect plugin for JUCE 8.0.7 (FilterAlphaThree)
 * - TB-303-style 4-pole diode ladder filter with high-pass feedback
 * - Processes incoming stereo audio (no synth/MIDI)
 * - No oversampling (direct processing)
 * - Real-time safe, double precision internally, Visual Studio 2022 / Windows 11 24H2
 */
class TeeBeeAudioProcessor : public juce::AudioProcessor
{
public:
    TeeBeeAudioProcessor();
    ~TeeBeeAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    double getLatencyInSamples() const;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    // TB-303 Filter (double precision internally)
    struct TeeBeeFilter
    {
        enum Mode { TB_303, LP_24, LP_18, LP_12, HP_12, FLAT, NUM_MODES };

        TeeBeeFilter();
        void setSampleRate(double sr);
        void reset();
        void setCutoff(double fc, bool updateCoeffs = true);
        void setResonance(double rPercent, bool updateCoeffs = true);
        void setDriveDb(double db);
        void setMode(int newMode);
        void setFeedbackHP(double fc);
        void setFeedbackAmp(double amp);
        float processSample(float in);

        double cutoff = 1000.0, resonanceRaw = 0.2, driveDb = 0.0, feedbackHpCutoff = 300.0, feedbackAmp = 0.5;
        double sampleRate = 44100.0, twoPiOverSampleRate = 2.0 * juce::MathConstants<double>::pi / 44100.0;
        int mode = TB_303;

        static bool isValid(float v) { return std::isfinite(v); }

    private:
        double b0 = 1.0, a1 = 0.0, k = 0.0, g = 1.0, driveFactor = 1.0, resonanceSkewed = 0.0;
        double y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0;
        double c0 = 0.0, c1 = 0.0, c2 = 0.0, c3 = 0.0, c4 = 1.0;
        double fb_hp_a0 = 1.0, fb_hp_a1 = -1.0, fb_hp_b1 = 0.0, fb_hp_z1 = 0.0, fb_inPrev = 0.0;
        double fb_lp_z1 = 0.0; // Feedback low-pass
        double dc_x1 = 0.0, dc_y1 = 0.0; // DC blocker state
        void calculateCoefficientsApprox();
        void updateFeedbackHPCoeffs();
        double dcBlock(double in);
        static double clip(double v, double lo, double hi) { return juce::jlimit(lo, hi, v); }
    };

    TeeBeeFilter filters[2]; // Stereo independence
    juce::SmoothedValue<double> cutoffSmoothed, resonanceSmoothed, driveSmoothed, fbHpSmoothed, fbAmpSmoothed;
    bool automationMode = false;
    double sampleRate = 44100.0;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TeeBeeAudioProcessor)
};

#endif // TEEBEE_AUDIO_PROCESSOR_H_INCLUDED