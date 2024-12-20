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
/* This is for demonstration purposes. Very inefficient, calculates the magnitude (Y pos) for each pixel (X pos)
wide in the graph, so you run through the X axis and stop at every pixel to calculate where the line is drawn there

Ideas to optimize: 
    1. reduce draw points (more than 1 pixel)
    2. Use timer to draw less frequently (dont draw in main gui thread)
    3. store the mags and dont draw points if didnt change, or better: only trigger redrawing as a whole when a param changed */
    using namespace juce;
    g.fillAll(Colours::black);
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto graphWidth = responseArea.getWidth(); // width of the EQ graph in pixels

    auto& lowCut = monochain.get<MonoChainIdx::LowCut>();
    auto& peak = monochain.get<MonoChainIdx::Peak>();
    auto& hiCut = monochain.get<MonoChainIdx::HiCut>();

    //  (e.g., 44100 Hz, 48000 Hz). This is needed to calculate the filter's magnitude response at specific frequencies.
    auto srate = audioProcessor.getSampleRate();

    std::vector<double> mags;
    mags.resize(graphWidth);

    for (size_t i = 0; i < graphWidth; i++) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(graphWidth), 20.0, 20000.0);

        if (!monochain.isBypassed<MonoChainIdx::Peak>()) {
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, srate);
        }
        if (!lowCut.isBypassed<0>()) mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, srate);
        if (!lowCut.isBypassed<1>()) mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, srate);
        if (!lowCut.isBypassed<2>()) mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, srate);
        if (!lowCut.isBypassed<3>()) mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, srate);
        
        if (!hiCut.isBypassed<0>()) mag *= hiCut.get<0>().coefficients->getMagnitudeForFrequency(freq, srate);
        if (!hiCut.isBypassed<1>()) mag *= hiCut.get<1>().coefficients->getMagnitudeForFrequency(freq, srate);
        if (!hiCut.isBypassed<2>()) mag *= hiCut.get<2>().coefficients->getMagnitudeForFrequency(freq, srate);
        if (!hiCut.isBypassed<3>()) mag *= hiCut.get<3>().coefficients->getMagnitudeForFrequency(freq, srate);

        mags[i] = Decibels::gainToDecibels(mag);
    }

    Path respCurve;

    const double outMin = responseArea.getBottom();
    const double outMax = responseArea.getY();

    auto map = [outMin, outMax](double input)
    {
        // -24 to 24 is the range of the Peak band gain
        return jmap(input, -24.0, 24.0, outMin, outMax);
    };

    respCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 0; i < mags.size(); i++) {
        respCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);

    g.setColour(Colours::white);
    g.strokePath(respCurve, PathStrokeType(2.f));
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

void Tutorial_EQAudioProcessorEditor::parameterValueChanged (int parameterIndex, float newValue)
{
    is_params_changed.set(true);
}

void Tutorial_EQAudioProcessorEditor::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
    // if(is_params_changed.compareAndSetValue(false, true)) {

    // }
}

void Tutorial_EQAudioProcessorEditor::timerCallback()
{

}
