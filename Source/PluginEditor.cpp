/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 400




void MyLookAndFeel::drawRotarySlider (juce::Graphics& g,
                            int x, int y, int width, int height,
                            float sliderPosProportional,
                            float rotaryStartAngle,
                            float rotaryEndAngle,
                            juce::Slider&)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colour(97, 18, 170));
    g.fillEllipse(bounds);

    g.setColour(Colour(255, 150, 1));
    g.drawEllipse(bounds, 1); // Draws a circle of thick 1


    Path p;

    auto center = bounds.getCentre();
    Rectangle<float> r;
    r.setLeft(center.getX() - 2);
    r.setRight(center.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(center.getY());

    p.addRectangle(r);

    jassert(rotaryEndAngle > rotaryStartAngle); // NOTE: jassert is from juce, run time assert only for debug builds
    // from normalized to angle in Rads (wonder why passing as normalized angle then..)
    auto sliderAngle = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle); 
    p.applyTransform(AffineTransform().rotated(sliderAngle, center.getX(), center.getY()));

    g.fillPath(p); // NOTE: this is what draws the arrow inside the knobs
}











void CustomRotSlider::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAngle = degreesToRadians(180 + 45);
    auto endAngle = degreesToRadians(180 - 45) + MathConstants<float>::twoPi; // Add 2pi to calc jmap

    auto range = getRange(); // Get the slider's normalized range

    auto bounds = getSliderBounds();

    // DEBUG: draw red lines around the slider
    g.setColour(Colours::red);
    g.drawRect(getLocalBounds()); // Bounds of the whole component (slider + surrounding area)
    g.setColour(Colours::yellow);
    g.drawRect(bounds); // Bounds of the slider only (as per our getSliderBounds() fct)

    //===========

    // Call our custom LnF draw function
    getLookAndFeel().drawRotarySlider(
        g,                                                                      // Graphics instance 
        bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),    // XYWH positioning of the slider
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),           // Current pos normalized (set to 0-1)
        startAngle, endAngle,                                                   // Allowed angle range
        *this);                                                                 // slider instance
}

juce::Rectangle<int> CustomRotSlider::getSliderBounds() const
{
    // return getLocalBounds();
    /* getLocalBounds() gives the full region we defined somewhere? We want the slider bounds to be
        smaller than this otherwise the knobs will bump eachother */

    auto bounds = getLocalBounds();

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextHeight() * 2;
    
    // Draw a square rectangle that contains the knob
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0); // rectangle centered with the slider X
    r.setY(2); // Absolute within the component's bounds. So 2 pixels below top of the component

    return r;
}







//==============================================================================
Tutorial_EQAudioProcessorEditor::Tutorial_EQAudioProcessorEditor (Tutorial_EQAudioProcessor& p)
    : AudioProcessorEditor (&p),
    // Call constructors for the custom sliders
    peakFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
    peakGainSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
    peakQSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
    lowCutSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
    hiCutSlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
    lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "dB/Oct"),
    hiCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "dB/Oct"),

    audioProcessor (p),
    respCurveComponent(audioProcessor),
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
    using namespace juce;
    g.fillAll(Colours::black);
}

void RespCurveCmp::paint (juce::Graphics& g)
{
/* This is for demonstration purposes. Very inefficient, calculates the magnitude (Y pos) for each pixel (X pos)
wide in the graph, so you run through the X axis and stop at every pixel to calculate where the line is drawn there

Ideas to optimize: 
    1. reduce draw points (more than 1 pixel)
    2. Use timer to draw less frequently (dont draw in main gui thread)
    3. store the mags and dont draw points if didnt change, or better: only trigger redrawing as a whole when a param changed */
    using namespace juce;
    g.fillAll(Colours::black);
    auto bounds = getLocalBounds(); // Were already configured in editor.resised
    auto graphWidth = bounds.getWidth(); // width of the EQ graph in pixels

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

    const double outMin = bounds.getBottom();
    const double outMax = bounds.getY();

    auto map = [outMin, outMax](double input)
    {
        // -24 to 24 is the range of the Peak band gain
        return jmap(input, -24.0, 24.0, outMin, outMax);
    };

    respCurve.startNewSubPath(bounds.getX(), map(mags.front()));

    for (size_t i = 0; i < mags.size(); i++) {
        respCurve.lineTo(bounds.getX() + i, map(mags[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(bounds.toFloat(), 4.f, 1.f);

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
    respCurveComponent.setBounds(responseArea); // NOTE: setbounds is inherited from the component class

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
        &lowCutSlider, &hiCutSlider, &lowCutSlopeSlider, &hiCutSlopeSlider,
        & respCurveComponent };
}


void RespCurveCmp::timerCallback()
{
    // addListener() must be called for this to work, i.e. we need to listen to our parameters
    // to know if they have changed

    // Update the editor's monochain
    if(is_params_changed.get() == true) {
        is_params_changed.set(false);
        auto cs = getChainSettings(audioProcessor.apvts);
        auto pkcoefs = MakePeakFilter(cs, audioProcessor.getSampleRate());
        UpdateCoefficients(monochain.get<MonoChainIdx::Peak>().coefficients, pkcoefs);

        auto lccoefs = MakeLowCutFilter(cs, audioProcessor.getSampleRate());
        auto hccoefs = MakeHighCutFilter(cs, audioProcessor.getSampleRate());

        UpdateCutFilter(monochain.get<MonoChainIdx::LowCut>(), lccoefs, cs.lowCutSlope);
        UpdateCutFilter(monochain.get<MonoChainIdx::HiCut>(), hccoefs, cs.hiCutSlope);

        repaint();
    }

    // repaint();
}

void RespCurveCmp::parameterValueChanged (int parameterIndex, float newValue)
{
    is_params_changed.set(true);
}

void RespCurveCmp::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
}