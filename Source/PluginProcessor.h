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

    /*! \brief Used to access index of A MonoChain processor chain */
    enum MonoChainIdx {
        LowCut,
        Peak,
        HiCut
    };

    
    // Peak filter
    // =====================================

    // NOTE: Helper function to prevent redundantly updating peak filters in processblock and in preparetoplay
    void UpdatePeakFilter(const ChainSettings& cs);

    using Coefs = Filter::CoefficientsPtr; // NOTE: Alias to this cryptic type for getting coefficents of UI
    static inline void UpdateCoefficients(Coefs &old, const Coefs &replacements) {
        *old = *replacements;
    }

    // Cut filters
    // =====================================    

    static inline int GetCutFilterTransferOrder(int slopeChoiceIdx) {
        /* 
        The order of a filter determines the steepness of its cutoff slope. 
        The order refers to the number of "poles" in the filter, and each pole contributes 6 dB per octave to the attenuation.
        For example:
        A 1st-order filter has a slope of 6 dB/octave.
        A 2nd-order filter has a slope of 12 dB/octave.
        A 4th-order filter has a slope of 24 dB/octave, and so on.
        The transfer function of a filter is a ratio of polynomials in terms of frequency (ss or zz in analog or digital filters, respectively).
        The order is the highest power of ss (or zz) in the denominator of the transfer function.
        Each additional order introduces an extra "pole" to the filter's frequency response, affecting its steepness and roll-off behavior. 
        Each "pole" or "order" represents an energy storage element in the filter (e.g., an inductor or capacitor in an analog filter).
        Higher-order filters can store and dissipate energy in more complex ways, resulting in steeper slopes or sharper cutoffs. */
        
        return (slopeChoiceIdx + 1) * 2;
    }

    template<typename ChainT, typename CoefT>
    /*! \brief Using a templated fct because I don't know what types are returned for the left low cut param,
        returned from LChain.get<MonoChainIdx::LowCut>();
        TODO: find the proper return type */
    void UpdateCutFilter(ChainT& lowCut, const CoefT& cutCoefs, const int slopeChoiceIdx)
    {
        // 4 filters chain, all bypassed, will enable depending on which setting is set by user
        /* NOTE: setBypassed<0> is a dependent name, means: depends on a template parameter

        lowCut is of type ChainT, which is a template type. The compiler cannot know
        whether setBypassed<0> is a normal member, a static member, or a member template function of ChainT.

        So if we had kept rightLowCut.setBypassed<0>(true); ---> <0> could have been seen as less than by compiler,
        i.e error: expected primary-expression before '<' token
        so, use .template when:
            You are calling a member template function of a dependent type.
            The compiler needs clarification that you are using a template function and not something else.
         */
        lowCut.template setBypassed<0>(true);
        lowCut.template setBypassed<1>(true);
        lowCut.template setBypassed<2>(true);
        lowCut.template setBypassed<3>(true);

        switch (slopeChoiceIdx) {
        case 3:
        // NOTE: could use UpdateCoefficients here
            lowCut.template get<3>().coefficients = *cutCoefs[3];
            lowCut.template setBypassed<3>(false);
            break;
        case 2:
            lowCut.template get<2>().coefficients = *cutCoefs[2];
            lowCut.template setBypassed<2>(false);
            break;
        case 1:
            lowCut.template get<1>().coefficients = *cutCoefs[1];
            lowCut.template setBypassed<1>(false);
            break;
        case 0:
            lowCut.template get<0>().coefficients = *cutCoefs[0];
            lowCut.template setBypassed<0>(false);
            break;

        default:
            break;
        }
    }

    void UpdateCutFilters(const ChainSettings& chainSettings);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tutorial_EQAudioProcessor)
};
