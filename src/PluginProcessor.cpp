/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"

#include "PluginEditor.h"

//==============================================================================
TestpluginAudioProcessor::TestpluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
}

TestpluginAudioProcessor::~TestpluginAudioProcessor() {}

//==============================================================================
const juce::String TestpluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool TestpluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool TestpluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool TestpluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double TestpluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int TestpluginAudioProcessor::getNumPrograms() {
  return 1;  // NB: some hosts don't cope very well if you tell them there are 0
             // programs, so this should be at least 1, even if you're not
             // really implementing programs.
}

int TestpluginAudioProcessor::getCurrentProgram() { return 0; }

void TestpluginAudioProcessor::setCurrentProgram(int index) {}

const juce::String TestpluginAudioProcessor::getProgramName(int index) {
  return {};
}

void TestpluginAudioProcessor::changeProgramName(int index,
                                                 const juce::String &newName) {}

//==============================================================================
void TestpluginAudioProcessor::prepareToPlay(double sampleRate,
                                             int samplesPerBlock) {
  // Use this method as the place to do any pre-playback
  // initialisation that you need..
  juce::dsp::ProcessSpec spec;
  spec.maximumBlockSize = samplesPerBlock;
  spec.numChannels = 1;
  spec.sampleRate = sampleRate;
  leftChain.prepare(spec);
  rightChain.prepare(spec);
}

void TestpluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestpluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}
#endif

void TestpluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                            juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();

  // In case we have more outputs than inputs, this code clears any output
  // channels that didn't contain input data, (because these aren't
  // guaranteed to be empty - they may contain garbage).
  // This is here to avoid people getting screaming feedback
  // when they first compile a plugin, but obviously you don't need to keep
  // this code if your algorithm always overwrites all the output channels.
  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());

  juce::dsp::AudioBlock<float> block(buffer);
  auto leftBlock = block.getSingleChannelBlock(0);
  auto rightBlock = block.getSingleChannelBlock(1);

  juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
  juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

  leftChain.process(leftContext);
  rightChain.process(rightContext);
}

//==============================================================================
bool TestpluginAudioProcessor::hasEditor() const {
  return true;  // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *TestpluginAudioProcessor::createEditor() {
  // return new TestpluginAudioProcessorEditor(*this);
  return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void TestpluginAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
}

void TestpluginAudioProcessor::setStateInformation(const void *data,
                                                   int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
}

juce::AudioProcessorValueTreeState::ParameterLayout
TestpluginAudioProcessor::createParameterLayout() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "LowCut Freq", "LowCut Freq",
      juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20.f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
      "HighCut Freq", "HighCut Freq",
      juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 1.f), 20000.f));

  return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new TestpluginAudioProcessor();
}
