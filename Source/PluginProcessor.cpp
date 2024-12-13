/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Tutorial_EQAudioProcessor::Tutorial_EQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

Tutorial_EQAudioProcessor::~Tutorial_EQAudioProcessor()
{
}

//==============================================================================
const juce::String Tutorial_EQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Tutorial_EQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Tutorial_EQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Tutorial_EQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Tutorial_EQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Tutorial_EQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Tutorial_EQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Tutorial_EQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Tutorial_EQAudioProcessor::getProgramName (int index)
{
    return {};
}

void Tutorial_EQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Tutorial_EQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    LChain.prepare(spec);
    RChain.prepare(spec);

    auto chainSettings = getChainSettings(apvts); // Will return values of the knobs/ctrls
    float gain_processed = juce::Decibels::decibelsToGain(chainSettings.peakGaindB); // as gain units, not as decibels

    // NOTE: Update the parameters in prepare to play
    UpdatePeakFilter(chainSettings);

    // Coefficients for the cut bands
    
    int lowCut_transferFuncOrder = GetCutFilterTransferOrder(chainSettings.lowCutSlope);
    auto cutCoefs = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, lowCut_transferFuncOrder);
    auto& leftLowCut = LChain.get<MonoChainIdx::LowCut>();
    auto& rightLowCut = RChain.get<MonoChainIdx::LowCut>();

    UpdateCutFilter(leftLowCut, cutCoefs, chainSettings.lowCutSlope);
    UpdateCutFilter(rightLowCut, cutCoefs, chainSettings.lowCutSlope);

}

void Tutorial_EQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Tutorial_EQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Tutorial_EQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    
    
    // Update the parameters before processing
    // ======

    auto chainSettings = getChainSettings(apvts);

    // Peak filter
    UpdatePeakFilter(chainSettings);
    
    // Cut filters
    int lowCut_transferFuncOrder = GetCutFilterTransferOrder(chainSettings.lowCutSlope);
    auto cutCoefs = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), lowCut_transferFuncOrder);
    auto& leftLowCut = LChain.get<MonoChainIdx::LowCut>();
    auto& rightLowCut = RChain.get<MonoChainIdx::LowCut>();

    UpdateCutFilter(leftLowCut, cutCoefs, chainSettings.lowCutSlope);
    UpdateCutFilter(rightLowCut, cutCoefs, chainSettings.lowCutSlope);


    // Block processing
    // ======

    juce::dsp::AudioBlock<float> block(buffer);

    // NOTE: We extract the 2 channels that were created as public members of our processor class
    auto left_block = block.getSingleChannelBlock(0);
    auto right_block = block.getSingleChannelBlock(1);

    // NOTE: create contexts to provide wrappers around our blocks of data
    juce::dsp::ProcessContextReplacing<float> leftContext(left_block);
    juce::dsp::ProcessContextReplacing<float> rightContext(right_block);

    LChain.process(leftContext);
    RChain.process(rightContext);
}

//==============================================================================
bool Tutorial_EQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Tutorial_EQAudioProcessor::createEditor()
{
    // return new Tutorial_EQAudioProcessorEditor (*this);


    /* Line below for showing parameters at start of project!

    The custom editor Tutorial_EQAudioProcessorEditor would need to handle the layout
    and display of parameters manually by adding sliders, buttons, or other controls tha
    are connected to your parameters.
    If you don't implement the code for the layout and parameter bindings in your custom editor,
    nothing will be displayed for the user.
    
    generic editor provided by JUCE that automatically handles the creation of GUI components
    for audio processor parameters. This class can display all the parameters you define
    in your createParamLayout() function, without needing to manually set up custom controls. */
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void Tutorial_EQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Tutorial_EQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Tutorial_EQAudioProcessor();
}

//NOTE: rappel, & fait qu'on travaille direct sur l'objet, pas de copie, ni de pointeur
// avantages to ptrs: null safety
ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;

    // Option 1: ceci retourne une normalized value (0-1), so we dont want that
    // apvts.getParameter("LowCut Freq")->getValue();
    // Option 2: get raw value va donner notre range predetermined
    settings.lowCutFreq = apvts.getRawParameterValue("LowCut Freq")->load();
    settings.hiCutFreq = apvts.getRawParameterValue("HighCut Freq")->load();
    settings.peakFreq = apvts.getRawParameterValue("Peak Freq")->load();
    settings.peakGaindB = apvts.getRawParameterValue("Peak Gain")->load();
    settings.peakQ = apvts.getRawParameterValue("Peak Quality")->load();
    settings.lowCutSlope = apvts.getRawParameterValue("LowCut Slope")->load();
    settings.hiCutSlope = apvts.getRawParameterValue("HighCut Slope")->load();

    return settings;
}

/* static */ juce::AudioProcessorValueTreeState::ParameterLayout Tutorial_EQAudioProcessor::createParamLayout()
{
    const float earMinFreq = 20.f;
    const float earMaxFreq = 20000.f;
    const float skewLogFreqRange = 0.25f; // Allows having 10kHz at 90% of the knob (log scale for hi and lo freqs)

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // We want unique pointers for parameters
    // make_unique uses new implicitly and initializes a std::unique_ptr (dynamic)

    //======================

    // TODO: Change the 3 freq params for less redundance
    // Helper function to create an AudioParameterFloat
    /* auto makeParam = [earMinFreq, earMaxFreq](const juce::String& paramID, const juce::String& paramName, float defaultValue) {
        return std::make_unique<juce::AudioParameterFloat>(
            paramID, paramName,
            juce::NormalisableRange<float>(earMinFreq, earMaxFreq, 1.f, 1.f),
            defaultValue);
    };
    layout.add(makeParam("LowCut Freq", "LowCut Freq", earMinFreq));
    layout.add(makeParam("HighCut Freq", "HighCut Freq", earMaxFreq));
    layout.add(makeParam("Peak Freq", "Peak Freq", 750.f));
    */

    // layout.add(std::make_unique<juce::AudioParameterFloat(const String &paramID,
    //     const String &paramName, NormalisableRange<float) normRng, float dflt);
    // Le range est de 20 - 20kHz (human ear)
    // juce::NormalisableRange<float>(earMinFreq, earMaxFreq, 1.f, ValueType skewFactor)
    // -> Skewfactor permet de setup logaritmiquement. Ici, 1 est aucun skew
    // float dflt -> Set to 20 cuz we dont want to low cut by default
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "LowCut Freq", "LowCut Freq",
        juce::NormalisableRange<float>(earMinFreq, earMaxFreq, 1.f, skewLogFreqRange), earMinFreq));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "HighCut Freq", "HighCut Freq",
        juce::NormalisableRange<float>(earMinFreq, earMaxFreq, 1.f, skewLogFreqRange), earMaxFreq));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Freq", "Peak Freq",
        juce::NormalisableRange<float>(earMinFreq, earMaxFreq, 1.f, skewLogFreqRange), 750.f));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Gain", "Peak Gain",
        juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f), 0.0f));
    
    // Quality refers to Q (a large Q will make a narrow notch filter)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "Peak Quality", "Peak Quality",
        juce::NormalisableRange<float>(0.1f, 10.f, 0.05f, 1.f), 1.f));
    
    // To set the HC / LC filter steepness, we use predetermined options (choices)
    juce::StringArray strs;
    for (int i = 1; i <= 4; i++) {
        juce::String str;
        str << i*12 << "dB/Oct";
        strs.add(str);
    }
    layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope",
        strs, 0)); // Defaults to idx 0
    layout.add(std::make_unique<juce::AudioParameterChoice>("HighCut Slope", "HighCut Slope",
        strs, 0)); // Defaults to idx 0

    return layout;
}


// Private

void Tutorial_EQAudioProcessor::UpdatePeakFilter(const ChainSettings& chainSettings)
{
    float gain_processed = juce::Decibels::decibelsToGain(chainSettings.peakGaindB); // as gain units, not as decibels

    auto newPeakCoefs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQ, gain_processed);

    // NOTE: Pass directly so there is no copy in a variable
    UpdateCoefficients(LChain.get<MonoChainIdx::Peak>().coefficients, newPeakCoefs);
    UpdateCoefficients(RChain.get<MonoChainIdx::Peak>().coefficients, newPeakCoefs);
}