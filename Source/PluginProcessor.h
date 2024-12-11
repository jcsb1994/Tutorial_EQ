/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// NOTE: dflt member init is from C++11 */
struct ChainSettings {
    float peakFreq { 0 }, peakGaindB { 0 }, peakQ {1.f};
    float lowCutFreq {0}, hiCutFreq {0};
    int lowCutSlope {0}, hiCutSlope {0};

};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

//==============================================================================
/**
*/
class Tutorial_EQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Tutorial_EQAudioProcessor();
    ~Tutorial_EQAudioProcessor() override;

    //==============================================================================
    
    /*! \note 1/2 Important function! */
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    /*! \note 2/2 Important function! */
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

    /*! \note Declared static as it does not use any class variable */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParamLayout();

    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters",
         createParamLayout() };
  
private:

    using Filter = juce::dsp::IIR::Filter<float>;
    // NOTE: Processing context passed to a chain. A IIR filter is 12db per oct, need 4 if want 48
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

    // NOTE: Represent the whole monopath of our 3-band parametric EQ
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

    MonoChain LChain, RChain;

    enum MonoChainIdx {
        LowCut,
        Peak,
        HiCut
    };

    // NOTE: Helper function to prevent redundantly updating peak filters in processblock and in preparetoplay
    void UpdatePeakFilter(const ChainSettings& cs);

    using Coefs = Filter::CoefficientsPtr; // NOTE: Alias to this cryptic type for getting coefficents of UI
    static inline void UpdateCoefficients(Coefs &old, const Coefs &replacements) {
        *old = *replacements;
    }


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tutorial_EQAudioProcessor)
};
