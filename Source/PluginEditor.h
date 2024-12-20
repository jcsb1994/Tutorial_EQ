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
class Tutorial_EQAudioProcessorEditor  : public juce::AudioProcessorEditor,
    
    /*! \note Inheriting from the Listener class allows subscribing to parameters */
    juce::AudioProcessorParameter::Listener, // Inherit from more than one classes!
    juce::Timer
{
public:
    Tutorial_EQAudioProcessorEditor (Tutorial_EQAudioProcessor&);
    ~Tutorial_EQAudioProcessorEditor() override;

    void toggleParameterListeners(bool enableListeners);

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // AudioProcessorParameter::Listener OVERRIDE FCTs
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;

    // Timer OVERRIDEs
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Tutorial_EQAudioProcessor& audioProcessor;

    /*! \note The brace initialization is from C++11. It helps prevent implicit conversions (safer)
        it is prefered in modern cpp code. */
    juce::Atomic<bool> is_params_changed { false };

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
