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

struct RespCurveCmp:
    juce::Component, // Allows setting bounds in the UI, so we can draw inside without going over
    /*! \note Inheriting from the Listener class allows subscribing to parameters */
    juce::AudioProcessorParameter::Listener,
    juce::Timer
{
    RespCurveCmp (Tutorial_EQAudioProcessor& p)
        : audioProcessor(p) // NOTE: refs must be initialized here
    {
        toggleParameterListeners(true);

        startTimer(60); // knows this fct because it inherits from the juce::Timer class`   `
    }
    ~RespCurveCmp()
    {
        toggleParameterListeners(false);
    }

    // constructor helper
    inline void toggleParameterListeners(bool enableListeners) {
        const auto& params = audioProcessor.getParameters(); // Returns an array of pointers
        for (auto param : params) {
            if (enableListeners) {
                param->addListener(this); // Adding the pluginEditor instance as a listener for each param
            } else {
                param->removeListener(this);
            }
        }
    }

    // Component overrides
    void paint(juce::Graphics& g) override;

    // AudioProcessorParameter::Listener OVERRIDE FCTs
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;

    // Timer OVERRIDEs
    void timerCallback() override;


private:        
    Tutorial_EQAudioProcessor& audioProcessor;
    /*! \note The brace initialization is from C++11. It helps prevent implicit conversions (safer)
        it is prefered in modern cpp code. */
    juce::Atomic<bool> is_params_changed { false };
    
    /*! \note We must have a process chain so we can "simulate" the EQ and show what it does */
    MonoChain monochain;
};



//==============================================================================
/**
*/
class Tutorial_EQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Tutorial_EQAudioProcessorEditor (Tutorial_EQAudioProcessor&);
    ~Tutorial_EQAudioProcessorEditor() override;

    void toggleParameterListeners(bool enableListeners);

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

    RespCurveCmp respCurveComponent;
  
    
    /*! \brief Attachments connect the sliders to params we created in Pluginprocessor */
    using Apvts = juce::AudioProcessorValueTreeState;
    using Attachment = Apvts::SliderAttachment;
    Attachment peakFreqSliderAttach, peakGainSliderAttach, peakQSliderAttach;
    Attachment lowCutSliderAttach, hiCutSliderAttach;
    Attachment lowCutSlopeSliderAttach, hiCutSlopeSliderAttach;

    std::vector<juce::Component*> getComps();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tutorial_EQAudioProcessorEditor)
};
