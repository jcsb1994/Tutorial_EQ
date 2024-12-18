/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400

//==============================================================================
Tutorial_EQAudioProcessorEditor::Tutorial_EQAudioProcessorEditor (Tutorial_EQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    // TODO: do enum to iterate
    lowCutSliderAttach(audioProcessor.apvts, "LowCut Freq", lowCutSlider),
    hiCutSliderAttach(audioProcessor.apvts, "HighCut Freq", hiCutSlider),
    peakFreqSliderAttach(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
    peakGainSliderAttach(audioProcessor.apvts, "Peak Gain", peakGainSlider),
    peakQSliderAttach(audioProcessor.apvts, "Peak Quality", peakQSlider),
    lowCutSlopeSliderAttach(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
    hiCutSlopeSliderAttach(audioProcessor.apvts, "HighCut Slope", hiCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for (auto* comp : getComps()) {
        addAndMakeVisible(comp); 
    }

    setSize (WINDOW_WIDTH, WINDOW_HEIGHT);
}

Tutorial_EQAudioProcessorEditor::~Tutorial_EQAudioProcessorEditor()
{
}

//==============================================================================
void Tutorial_EQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Tutorial_EQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    
    // Y axis cut : leave the third for the FFT 
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);

    // X axis cuts : Left part for high pass, right is low pass
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    
    // Y cuts : Place the low pass freq and slope sliders
    lowCutSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);

    hiCutSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    hiCutSlopeSlider.setBounds(highCutArea);

    // Remove from top -> split remaining Y axis in 3 parts for the peak filter
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.5));
    peakQSlider.setBounds(bounds);
    
}


std::vector<juce::Component*> Tutorial_EQAudioProcessorEditor::getComps()
{
    return { &peakFreqSlider, &peakGainSlider, &peakQSlider,
        &lowCutSlider, &hiCutSlider, &lowCutSlopeSlider, &hiCutSlopeSlider };
}