/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotSlider : juce::Slider
{
    CustomRotSlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                          juce::Slider::TextEntryBoxPosition::NoTextBox)
    {

    }
};

//==============================================================================
/**
*/
class Tutorial_EQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Tutorial_EQAudioProcessorEditor (Tutorial_EQAudioProcessor&);
    ~Tutorial_EQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Tutorial_EQAudioProcessor& audioProcessor;


    /*! \brief Instances of our struct that inherits from some type of specific slider */
    CustomRotSlider peakFreqSlider, peakGainSlider, peakQSlider;
    CustomRotSlider lowCutSlider, hiCutSlider;
    CustomRotSlider lowCutSlopeSlider, hiCutSlopeSlider;
  
    
    /*! \brief Attachments connect the sliders to params we created in Pluginprocessor */
    using Apvts = juce::AudioProcessorValueTreeState;
    using Attachment = Apvts::SliderAttachment;
    Attachment peakFreqSliderAttach, peakGainSliderAttach, peakQSliderAttach;
    Attachment lowCutSliderAttach, hiCutSliderAttach;
    Attachment lowCutSlopeSliderAttach, hiCutSlopeSliderAttach;

    std::vector<juce::Component*> getComps();

    MonoChain monochain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tutorial_EQAudioProcessorEditor)
};
