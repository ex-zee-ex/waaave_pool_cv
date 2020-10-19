/*
 * Copyright (c) 2013 Dan Wilcox <danomatika@gmail.com>
 *
 * BSD Simplified License.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * See https://github.com/danomatika/ofxMidi for documentation
 *
 */
#pragma once

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxOMXVideoGrabber.h"

class ofApp : public ofBaseApp, public ofxMidiListener {
	
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void keyPressed(int key);
	void keyReleased(int key);
	
	
	
	void omx_settings();
	
	void omx_updates();
	
	void midibiz();
	
	void newMidiMessage(ofxMidiMessage& eventArgs);
	
	void adc_poll();
	
	int readADC(unsigned char channel, unsigned char cs);
	
	double convertVolts(int val, double vref);
	
	float bipolar(float unipolar);
	
	float round(float var, int precision);
	
	ofxMidiIn midiIn;
	std::vector<ofxMidiMessage> midiMessages;
	std::size_t maxMessages = 10; //< max number of messages to keep track of
	
	
	
	
	ofShader shader_mixer;
	
    
    
    ofFbo framebuffer0;
    ofFbo framebuffer1;
    
    
    ofVideoGrabber cam1;
    
    ofxOMXCameraSettings settings;
    ofxOMXVideoGrabber videoGrabber;
};
