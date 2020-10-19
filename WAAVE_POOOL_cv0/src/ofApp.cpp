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
#include "ofApp.h"

#include <wiringPiSPI.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <errno.h>

//flip this switch to try different scalings
//0 is 320 1 is 640
//if you reduce scale to 320 you can up the total delay time to
//about 4 seconds/ 120 frames
//so try that out sometime and see how that feels!
bool scaleswitch=1;
const int fbob=60;
//const int fbob=120;

//0 is picaputre, 1 is usbinput
bool inputswitch=1;

//0 is wet (framebuffer fed from final output, internal
//feedback mode
//1 is dry (framebuffer fed direct from camera input,
//traditional video delay mode
bool wet_dry_switch=1;

float az = 1.0;
float sx = 0;
float dc = 0;
float fv = 1;
float gb = 1;
float hn = 1;
float jm = 0.0;
float kk = 0.0;
float ll = 0.0;
float qw = 0.0;

float er = 1.0;
float ty = 0.0;
float ui = 0.0;

float op = 0.0;

float fb1_brightx=0.0;

bool toroid_switch=0;

float testt=1.0;

bool y_skew_switch=FALSE;
bool x_skew_switch=FALSE;

bool luma_switch=FALSE;

bool x_mirror_switch=FALSE;

bool y_mirror_switch=FALSE;

float y_skew=0;

float x_skew=0;



int fb0_delayamount=0;



//dummy variables for midi control


float c1=0.0;
float c2=0.0;
float c3=0.0;
float c4=0.0;
float c5=0.0;
float c6=0.0;
float c7=0.0;
float c8=0.0;
float c9=0.0;
float c10=0.0;
float c11=0.0;
float c12=0.0;
float c13=0.0;
float c14=0.0;
float c15=.0;
float c16=.0;

int width=0;
int height=0;


bool clear_switch=0;
//toggle switch variables
bool fb0_h_invert=FALSE;
bool fb0_s_invert=FALSE;
bool fb0_b_invert=FALSE;

bool cam1_h_invert=FALSE;
bool cam1_s_invert=FALSE;
bool cam1_b_invert=FALSE;

bool fb0_h_mirror=FALSE;
bool fb0_v_mirror=FALSE;

bool clear_flip=FALSE;

bool x_2=FALSE;
bool x_5=FALSE;
bool x_10=FALSE;

bool y_2=FALSE;
bool y_5=FALSE;
bool y_10=FALSE;

bool z_2=FALSE;
bool z_5=FALSE;
bool z_10=FALSE;

bool theta_0=FALSE;
bool theta_1=FALSE;
bool theta_2=FALSE;

bool huexx_0=FALSE;
bool huexx_1=FALSE;
bool huexx_2=FALSE;

bool huexy_0=FALSE;
bool huexy_1=FALSE;
bool huexy_2=FALSE;

bool huexz_0=FALSE;
bool huexz_1=FALSE;
bool huexz_2=FALSE;

//framebuffer management biziness

//fbob is the total number of frames that will be stored.  setting fbob to
//30 while fps is set to 30 means 1 second of video will be stored in memory  
//75 seems to work ok with usb cams but the capture device might need a little more memory



//const int fbob=2;


int framedelayoffset=0;  // this is used as an index to the circular framebuffers eeettt
unsigned int framecount=0; // framecounter used to calc offset eeettt

//this is an array of framebuffer objects
ofFbo pastFrames[fbob];


void incIndex()  // call this every frame to calc the offset eeettt
{
    
    framecount++;
    framedelayoffset=framecount % fbob;
}


int cv_bits[16];

float cv_bits_smoothed[16];

float cv_coefficients[16];

float cv_smoothing=.5;

//heres some ADC biz
//written by Clovis Tessier
// SPI Bus Chip Enable Pins
// Use these macros for selecting which ADC to communicate with
// on the SPI Bus.
// CE0 = CV1-CV8
// CE1 = CV9-CV16
#define CE0 0
#define CE1 1

// SPI Clock Speed (Hz)
// Range is 500,000 - 32,000,000
// Don't expect the highest speeds to be stable

// Per the MCP3008 Datasheet:
// Vdd and Signal Source Impedance (Rs) affect the accuracy of ADC
// readings at higher clock speeds.
// Based on Vdd = 3.3V and Rs = 10K Ohms:
// SPEED <= ~750,000.
#define SPEED 500000

// Number of Input Channels per ADC
#define NUM_CHANNELS 8

// Function to read from the ADC
// Inputs:
// 	channel = ADC channel to read (0-7)
//	cs = SPI Bus chip select line (0-1)
//	singleEnded = Single Ended or Differential reading
// Output:
//	10-bit ADC reading


int ofApp::readADC(unsigned char channel, unsigned char cs)
{
	//cs=1;
	bool singleEnded=true;
	// Check for valid inputs
	if (channel > 7 || channel < 0 || cs > 1 || cs < 0) {
		return -1;
	}

	// MCP3008 bit formatting:
	// first bit high is start bit
	// next bit indicates single-ended or differential reading
	// followed by ADC channel to be read
	unsigned char startBit = 1;

	unsigned char singleBit = 0;
	if (singleEnded) {
		singleBit = (1 << 7);
	}

	unsigned char buffer[] = {startBit, (unsigned char)(singleBit | (channel << 4)), 0};

	// Transfer the data to the ADC, the buffer is overwritten with the reading
	wiringPiSPIDataRW(cs, buffer, 3);

	// ADC reading is located in the last 10 bits of the buffer
	int result = ((buffer[1] & 3) << 8) | buffer[2];
	return result;
}

// Function to convert 10-bit ADC value to voltage
double ofApp::convertVolts(int val, double vref) {
	//vref= 2.048l;
	double result = (val * vref) / 1023;
	return result;
}

//--------------------------------------------------------------
void ofApp::setup() {
	//ofSetVerticalSync(true);

	ofSetFrameRate(30);
    
    ofBackground(0);
	
	//ofToggleFullscreen();
	
    ofHideCursor();
	
		
	if(scaleswitch==0){
		width=320;
		height=240;
	}
	
	if(scaleswitch==1){
		width=640;
		height=480;
	}
	  
	omx_settings();  
    
    
    //pass in the settings and it will start the camera
	if(inputswitch==0){
		videoGrabber.setup(settings);
	}

	
	if(inputswitch==1){
		cam1.initGrabber(width,height);
	}
	
	framebuffer0.allocate(width,height);
	
	framebuffer0.begin();
	ofClear(0,0,0,255);
	framebuffer0.end();
	
	
	/*
	framebuffer1.allocate(width,height);
	
	
	framebuffer1.begin();
	ofClear(0,0,0,255);
	framebuffer1.end();
	*/
	
	 for(int i=0;i<fbob;i++){
        
        pastFrames[i].allocate(width, height);
        pastFrames[i].begin();
        ofClear(0,0,0,255);
        pastFrames[i].end();
        
    
    }//endifor


	
	
	shader_mixer.load("shadersES2/shader_mixer");
	
	
	
	// print input ports to console
	midiIn.listInPorts();
	
	// open port by number (you may need to change this)
	midiIn.openPort(1);
	//midiIn.openPort("IAC Pure Data In");	// by name
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
	
	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);
	
	// add ofApp as a listener
	midiIn.addListener(this);
	
	// print received messages to the console
	midiIn.setVerbose(true);
	
	
	
	
	//spi setup things
	wiringPiSPISetup(CE0, SPEED);
	wiringPiSPISetup(CE1, SPEED);
	
	
	//clear out the cv buffer
	for(int i=0;i<16;i++){
		cv_bits[i]=0;
		cv_bits_smoothed[i]=511;
		
		cv_coefficients[i]=1.0;
	}
	
	
	//0-lumakey
	//1-fb0mix
	//2-hue
	//3-sat
	//8-bright
	//9-fb1mix
	//10-fb1 x
	//11 input contrast
	
	//4-x dis
	//5-y dis
	//6-z dis
	//7-theta
	//12-huex_mod
	//13-huex_offset
	//14-huex-lfo
	//15-delay time
	
	cv_coefficients[1]=2.0;
	cv_coefficients[2]=.5;
	cv_coefficients[3]=.05;
	cv_coefficients[8]=.05;
	
	cv_coefficients[4]=.01;
	cv_coefficients[5]=.01;
	cv_coefficients[6]=.1;
	cv_coefficients[7]=.314159265f;
	
}

//--------------------------------------------------------------
void ofApp::update() {
	
	if(inputswitch==1){
		cam1.update();
	}
	
	if(inputswitch==0){
		
		omx_updates();
		
	}
	
	
	//cvbiz
	adc_poll();
	
	cout << setprecision(4);
	
	for(int i=0;i<16;i++){
		
		cv_bits_smoothed[i]=ofMap(cv_bits[i],0,1017,0,1.0f)*cv_smoothing+cv_bits_smoothed[i]*(1.0f-cv_smoothing);
		
		if(cv_bits_smoothed[i]<.001){
			cv_bits_smoothed[i]=0;
		}
		
		if(cv_bits_smoothed[i]>1){
			cv_bits_smoothed[i]=1;
		}
		
		cv_bits_smoothed[i]=round(cv_bits_smoothed[i],2);
		//cout<<cv_bits[i]<<"\t";
		//cout<<convertVolts(cv_bits[i],2.051)<<"\t";
		cout<<cv_bits_smoothed[i]<<"\t";
	}
	
	cout<<"\n";
}

//-------------------------------------------------------------------------------

float ofApp:: round(float var, int precision){
	
	float rounded=int(var*pow(10,precision)+.5);
	return rounded/pow(10,precision);
}

//--------------------------------------------------------------
void ofApp::draw() {


	//begin midi biz
	
	midibiz();
	
	
	
	//so to keep in mind i want to map the inputs in a different order than numerical to make it match
	//up better with the nanokontrol zones
	//0-lumakey
	//1-fb0mix
	//2-hue
	//3-sat
	//8-bright
	//9-fb1mix
	//10-fb1 x
	//11 input contrast
	
	//4-x dis
	//5-y dis
	//6-z dis
	//7-theta
	//12-huex_mod
	//13-huex_offset
	//14-huex-lfo
	//15-delay time
	
	
	
	
	//control attenuation section
	//dummy variables to organize things
	
	//lets get two functions up for bipolar/unipolarizing these things?
	//default is gonna be .5 ~ so need to fix everything based on that.
	
	
	//float d_lumakey_value=kk+1.01*c1+bipolar(cv_bits_smoothed[0]);
	
	float d_lumakey_value=kk+1.01*c1+bipolar(cv_bits_smoothed[0]);
	
	float d_mix=jm+2.0f*c2+bipolar(cv_bits_smoothed[1]);
	
	float d_hue=fv*(1.0f+.75f*c3)+bipolar(cv_bits_smoothed[2])*cv_coefficients[2];;
	
	float d_sat=gb*(1.0f+.5f*c4)+bipolar(cv_bits_smoothed[3])*cv_coefficients[3];;
	
	float d_bright=hn*(1.0f+.5f*c5)+bipolar(cv_bits_smoothed[8])*cv_coefficients[8];;
							
	float d_fb1_mix=op+1.1f*c6+bipolar(cv_bits_smoothed[9]);
							
	float d_fb1_x=fb1_brightx+c7+bipolar(cv_bits_smoothed[10]);
							
	float d_cam1_x=ll+4.0f*c8+bipolar(cv_bits_smoothed[11]);
							
	float d_x=sx+.01f*c9+bipolar(cv_bits_smoothed[4])*cv_coefficients[4];
							
	float d_y=dc+.01f*c10+bipolar(cv_bits_smoothed[5])*cv_coefficients[5];
							
	float d_z=az*(1.0f+.05f*c11)+bipolar(cv_bits_smoothed[6])*cv_coefficients[6];
							
	float d_rotate=qw+.314159265f*c12+bipolar(cv_bits_smoothed[7])*cv_coefficients[7];
							
	float d_huex_mod=er*(1.0f-c13)+bipolar(cv_bits_smoothed[12]);
							
	float d_huex_off=ty+.25f*c14+bipolar(cv_bits_smoothed[13]);
							
	float d_huex_lfo=ui+.25f*c15+bipolar(cv_bits_smoothed[14]);

	int d_delay=abs(framedelayoffset-fbob-fb0_delayamount-
				int(((c16+ofClamp(bipolar(cv_bits_smoothed[15]),0,1))*(fbob-1.0)-1.0)
				))%fbob;
	
	


    
    
    //----------------------------------------------------------
    //
	
	framebuffer0.begin();
    shader_mixer.begin();

	//videoGrabber.getTextureReference().draw(0, 0, 320, 640);
	if(scaleswitch==0){
		
		if(inputswitch==0){
			videoGrabber.draw(0,0,320,240);
		}
		
		if(inputswitch==1){
			cam1.draw(0,0,320,240);
		}
	}
	
	if(scaleswitch==1){
		if(inputswitch==0){
			videoGrabber.draw(0,0);
		}
		
		if(inputswitch==1){
			cam1.draw(0,0);
		}
	}
	
	//videoGrabber.draw(0,0);
	
	
	//textures
	//textures
	shader_mixer.setUniformTexture("fb0", pastFrames[d_delay].getTexture(),1);
    shader_mixer.setUniformTexture("fb1", pastFrames[(abs(framedelayoffset-fbob)-1)%fbob].getTexture(),2);
	
	//continuous variables
	shader_mixer.setUniform1f("fb0_lumakey_value",d_lumakey_value);
    shader_mixer.setUniform1f("fb0_mix",d_mix);
    shader_mixer.setUniform3f("fb0_hsbx",ofVec3f(d_hue,d_sat,d_bright));
    shader_mixer.setUniform1f("fb1_mix",d_fb1_mix);
    shader_mixer.setUniform1f("fb1_brightx",d_fb1_x );
    shader_mixer.setUniform1f("cam1_brightx",d_cam1_x);
    shader_mixer.setUniform1f("fb0_xdisplace",d_x);
    shader_mixer.setUniform1f("fb0_ydisplace",d_y);
    shader_mixer.setUniform1f("fb0_zdisplace",d_z);
    shader_mixer.setUniform1f("fb0_rotate",d_rotate);
    shader_mixer.setUniform3f("fb0_huex",ofVec3f(d_huex_mod,d_huex_off,d_huex_lfo));
	
    
    
    
   
    
    
    shader_mixer.setUniform1i("fb0_b_invert",fb0_b_invert);
    shader_mixer.setUniform1i("fb0_h_invert",fb0_h_invert);
    shader_mixer.setUniform1i("fb0_s_invert",fb0_s_invert);
    
    shader_mixer.setUniform1i("fb0_h_mirror",fb0_h_mirror);
    shader_mixer.setUniform1i("fb0_v_mirror",fb0_v_mirror);
    
    shader_mixer.setUniform1i("toroid_switch",toroid_switch);
    
    
    
    
    shader_mixer.setUniform1i("luma_switch",luma_switch);
    
    shader_mixer.setUniform1i("x_mirror_switch",x_mirror_switch);
    
    shader_mixer.setUniform1i("y_mirror_switch",y_mirror_switch);

    shader_mixer.end();
	framebuffer0.end();
	
	//_-_-__---_---___
	
	
	
	//_----___---------_-_-----_--_-_--_--_
	
	
	
	//framebuffer0.draw(0,0,ofGetScreenWidth(),ofGetScreenHeight());
	
	framebuffer0.draw(0,0,720,480);
	

	//_-------------------------------------------
	
	
	pastFrames[abs(fbob-framedelayoffset)-1].begin(); //eeettt
    
    ofPushMatrix();
	ofTranslate(framebuffer0.getWidth()/2,framebuffer0.getHeight()/2);
    ofRotateYRad(y_skew);
    ofRotateXRad(x_skew);
    
    if(wet_dry_switch==0){
		
		
		
		
		if(inputswitch==0){
			videoGrabber.draw(-framebuffer0.getWidth()/2,-framebuffer0.getHeight()/2,framebuffer0.getWidth(),framebuffer0.getHeight());
		}
		
		if(inputswitch==1){
			cam1.draw(-framebuffer0.getWidth()/2,-framebuffer0.getHeight()/2,framebuffer0.getWidth(),framebuffer0.getHeight());
		}
	
		
		}//endifwetdry0
	
		
	if(wet_dry_switch==1){
		
		framebuffer0.draw(-framebuffer0.getWidth()/2,-framebuffer0.getHeight()/2);
		
		
		}//endifwetdry1
	ofPopMatrix();
	
    pastFrames[abs(fbob-framedelayoffset)-1].end(); //eeettt

	incIndex();
   
	//ofDrawBitmapString("fps =" + ofToString(getFps()), 10, ofGetHeight() - 5 );

//i use this block of code to print out like useful information for debugging various things and/or just to keep the 
//framerate displayed to make sure i'm not losing any frames while testing out new features.  uncomment the ofDrawBitmap etc etc
//to print the stuff out on screen
   ofSetColor(255);
   string msg="fps="+ofToString(ofGetFrameRate(),2)+" delay"+ofToString(d_delay,0);//+" z="+ofToString(az,5);
   //ofDrawBitmapString(msg,10,10);
 
   	
}
//-------------------------------------------------------------

float ofApp::bipolar(float unipolar){
	return 2.0f*(unipolar-.5f);
	
}

//--------------------------------------------------------------
void ofApp::exit() {
	
	// clean up
	midiIn.closePort();
	midiIn.removeListener(this);
}


void ofApp::omx_settings(){
	
	settings.sensorWidth = 640;
    settings.sensorHeight = 480;
    settings.framerate = 30;
    settings.enableTexture = true;
    settings.sensorMode=7;
    
    settings.whiteBalance ="Off";
    settings.exposurePreset ="Off";
    settings.whiteBalanceGainR = 1.0;
    settings.whiteBalanceGainB = 1.0;
	
	}
//------------------------------------------------------------

void ofApp::omx_updates(){
	
		videoGrabber.setSharpness(50);
		videoGrabber.setBrightness(40);
		videoGrabber.setContrast(100);
		videoGrabber.setSaturation(0);
	
	}
	
	
//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {

	// add the latest message to the message queue
	midiMessages.push_back(msg);

	// remove any old messages if we have too many
	while(midiMessages.size() > maxMessages) {
		midiMessages.erase(midiMessages.begin());
	}
}

//--------------------------------------------------------------

void ofApp:: adc_poll(){
	
	//ADC biz
	//written by Clovis Tessier
	int reading = 0;

	//cout << "Initializing..." << endl;

//we only want this to happen once so put this in the setup area
	// Configure the SPI interface.
	/*
	fd = wiringPiSPISetup(CE0, SPEED);
	cout << "Init Result CE0: " << fd << endl;

	fd = wiringPiSPISetup(CE1, SPEED);
	cout << "Init Result CE1: " << fd << endl;
	*/
	/*
	for (int i = 1; i <= NUM_CHANNELS * 2; i++) {
		cout << "CV" << i << "\t";
	}
	*/
	
	//cout << "\n";
	//cout << setprecision(4);

	// Continuously Poll the 16 ADC Channels
	
	//so this while seems to be preventing the too many open files
	//spi error, while using the while seems to make difficulty engaging the 
	//esgl engine to create the window.  will need to see if i can get this to happen on another thread?
	//while (true) {
		// Read from the channels
		for (int i = 0; i < NUM_CHANNELS; i++) {
			reading = readADC(i, CE0);
			//cout << reading << "\t";
			
			cv_bits[i]=reading;
			// Use the line below instead of the line above
			// if you want to see voltages
			//cout << convertVolts(reading, 2.051) << "\t";

			// Vref was measured at ADC pin to be 2.051V
		}

		for (int i = 0; i < NUM_CHANNELS; i++) {
			reading = readADC(i, CE1);
			//cout << reading << "\t";
			
			cv_bits[i+8]=reading;
			//cout << convertVolts(reading, 2.051) << "\t";
		}
		//cout << "\n";
		sleep(.0333);
	//}
	
	
}
//----------------------------------------------------------------




//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	//error-if i decrement fb0_delayamount it always crashes...
	if (key == '[') {fb0_delayamount += 1;}
    if (key == ']') {
		fb0_delayamount = fb0_delayamount-1;
		if(fb0_delayamount<0){
			fb0_delayamount=fbob-fb0_delayamount;
		}//endiffb0
	}//endifkey
    
    //fb1 mix
    if (key == 'o') {op += .01;}
    if (key == 'p') {op -= .01;}
    
    //fb0 z displace
    if (key == 'a') {az += .0001;}
    if (key == 'z') {az -= .0001;}
    
    //fb0 x displace
    if (key == 's') {sx += .0001;}
    if (key == 'x') {sx -= .0001;}
    
    //fb0 y displace
    if (key == 'd') {dc += .0001;}
    if (key == 'c') {dc -= .0001;}
    
    //fb0 hue attenuate
    if (key == 'f') {fv += .001;}
    if (key == 'v') {fv -= .001;}
    
    //fb0 saturation attenuate
    if (key == 'g') {gb += .001;}
    if (key == 'b') {gb -= .001;}
    
    //fb0 brightness attenuate
    if (key == 'h') {hn += .001;}
    if (key == 'n') {hn -= .001;}
    
    //fb0 mix
    if (key == 'j') {jm += .01;}
    if (key == 'm') {jm -= .01;}
    
    //fb0 lumakey value
    if (key == 'k') {kk = ofClamp(kk+.01,0.0,1.0);}
    if (key == ',') {kk = ofClamp(kk-.01,0.0,1.0);}
    
    
    if (key == 'l') {ll += .01;}
    if (key == '.') {ll -= .01;}
    
    if (key == ';') {fb1_brightx += .01;}
    if (key == '\'') {fb1_brightx -= .01;}
    
    //fb0 rotation
    if (key == 'q') {qw += .0001;}
    if (key == 'w') {qw -= .0001;}


	//hue chaos1
    if (key == 'e') {er += .001;}
    if (key == 'r') {er -= .001;}
    
    //hue chaos2
    if (key == 't') {ty += .01;}
    if (key == 'y') {ty -= .01;}
    
    //hue chaos3
    if (key == 'u') {ui += .01;}
    if (key == 'i') {ui -= .01;}

    if (key == '1') {

        //clear the framebuffer if thats whats up
        framebuffer0.begin();
        ofClear(0, 0, 0, 255);
        framebuffer0.end();

      for(int i=0;i<fbob;i++){
        
       
        pastFrames[i].begin();
        ofClear(0,0,0,255);
        pastFrames[i].end();
        
    
		}//endifor
    }
    
    
    if(key=='2'){fb0_b_invert=!fb0_b_invert;}
    if(key=='3'){fb0_h_invert=!fb0_h_invert;}
    if(key=='4'){fb0_s_invert=!fb0_s_invert;}
    
    if(key=='5'){fb0_v_mirror=!fb0_v_mirror;}
    if(key=='6'){fb0_h_mirror=!fb0_h_mirror;}
    
    if(key=='7'){toroid_switch=!toroid_switch;}
    
    if (key == '-') {y_skew += .01;}
    if (key == '=') {y_skew -= .01;}
    if (key == '9') {x_skew += .01;}
    if (key == '0') {x_skew -= .01;}
    
	//reset button
    if (key == '!') {
		
		
	az = 1.0;
	sx = 0;
    dc = 0;
	fv = 1;
	gb = 1;
	hn = 1;
	jm = 0.0;
	kk = 0.0;
	ll = 0.0;
	qw = 0.0;

	er = 1.0;
	ty = 0.0;
	ui = 0.0;

	op = 0.0;
	fb0_delayamount=0;
	
	fb0_b_invert=0;
	fb0_h_invert=0;
	fb0_s_invert=0;
	
	fb0_v_mirror=0;
	fb0_h_mirror=0;
	
	x_skew=0;
	y_skew=0;
	
	framebuffer0.begin();
    ofClear(0, 0, 0, 255);
    framebuffer0.end();
	 
	for(int i=0;i<fbob;i++){
        
       
        pastFrames[i].begin();
        ofClear(0,0,0,255);
        pastFrames[i].end();
        
    
		}//endifor
		
	}
    
    //no
   // if(key=='='){inputswitch = !inputswitch;}
}

void ofApp::midibiz(){
	for(unsigned int i = 0; i < midiMessages.size(); ++i) {

		ofxMidiMessage &message = midiMessages[i];
	
		if(message.status < MIDI_SYSEX) {
			//text << "chan: " << message.channel;
            if(message.status == MIDI_CONTROL_CHANGE) {
                
                //How to Midi Map
                //uncomment the line that says cout<<message control etc
                //run the code and look down in the console
                //when u move a knob on yr controller keep track of the number that shows up
                //that is the cc value of the knob
                //then go down to the part labled 'MIDIMAPZONE'
                //and change the numbers for each if message.control== statement to the values
                //on yr controller
                
                 // cout << "message.control"<< message.control<< endl;
                 // cout << "message.value"<< message.value<< endl;
                
                
                
                //MIDIMAPZONE
                //these are mostly all set to output bipolor controls at this moment (ranging from -1.0 to 1.0)
                //if u uncomment the second line on each of these if statements that will switch thems to unipolor
                //controls (ranging from 0.0to 1.0) if  you prefer
                
		//these controls are currently set to the default cc values of the korg nanostudio so if you got one of those yr in luck!
		//otherwise you will need to figure out the cc values for the knobs and sliders on your particular controller
		//and for each line where it says " if(message.control==XX)" replace XX with the cc value for the knob that you want to 
		//map for each control.  
		    
                
                /* the nanostudio kontrols
                //c1 maps to fb0 hue attenuation
                if(message.control==20){
                    c1=(message.value-63.0)/63.0;
                   //  c1=(message.value)/127.00;
                    
                }
                
                //c2 maps to fb0 saturation attenuation
                if(message.control==21){
                    c2=(message.value-63.0)/63.0;
                   //   c2=(message.value)/127.00;
                    
                }
                
                //c3 maps to fb0 brightness attenuation
                if(message.control==22){
                    c3=(message.value-63.0)/63.00;
                    //  c3=(message.value)/127.00;
                }
                
                //c4 maps to fb0_mix amount
                if(message.control==23){
                     c4=(message.value-63.0)/63.00;
                   // c4=(message.value)/127.00;
                   
                }
                
                //c5 maps to fb0 x displace
                if(message.control==24){
                     c5=(message.value-63.0)/63.00;
                  //  c5=(message.value)/127.00;
                  
                }
                
                //c6 maps to fb0 y displace
                if(message.control==25){
                    c6=(message.value-63.0)/63.0;
                    //  c4=(message.value)/127.00;
                }
                
                //c7 maps to fb0 z displace
                if(message.control==26){
                    c7=(message.value-63.0)/63.0;
                    //  c4=(message.value)/127.00;
                }
                
                //c8 maps to fb0 luma key value
                if(message.control==27){
                   //  c8=(message.value-63.0)/63.00;
                    c8=(message.value)/127.0;
                   
                }
                
                */
                
                
                if(message.control==32){
					if(message.value==127){
						x_2=TRUE;
					}
					
					if(message.value==0){
						x_2=FALSE;
					}
                }
                
                if(message.control==48){
					if(message.value==127){
						x_5=TRUE;
					}
					
					if(message.value==0){
						x_5=FALSE;
					}
                }
                
                 if(message.control==64){
					if(message.value==127){
						x_10=TRUE;
					}
					
					if(message.value==0){
						x_10=FALSE;
					}
                }
                
                
                
                if(message.control==33){
					if(message.value==127){
						y_2=TRUE;
					}
					
					if(message.value==0){
						y_2=FALSE;
					}
                }
                
                if(message.control==49){
					if(message.value==127){
						y_5=TRUE;
					}
					
					if(message.value==0){
						y_5=FALSE;
					}
                }
                
                 if(message.control==65){
					if(message.value==127){
						y_10=TRUE;
					}
					
					if(message.value==0){
						y_10=FALSE;
					}
                }
                
                
                
                if(message.control==34){
					if(message.value==127){
						z_2=TRUE;
					}
					
					if(message.value==0){
						z_2=FALSE;
					}
                }
                
                if(message.control==50){
					if(message.value==127){
						z_5=TRUE;
					}
					
					if(message.value==0){
						z_5=FALSE;
					}
                }
                
                 if(message.control==66){
					if(message.value==127){
						z_10=TRUE;
					}
					
					if(message.value==0){
						z_10=FALSE;
					}
                }
                
                
                
                
                if(message.control==35){
					if(message.value==127){
						theta_0=TRUE;
					}
					
					if(message.value==0){
						theta_0=FALSE;
					}
                }
                
                if(message.control==51){
					if(message.value==127){
						theta_1=TRUE;
					}
					
					if(message.value==0){
						theta_1=FALSE;
					}
                }
                
                 if(message.control==67){
					if(message.value==127){
						theta_2=TRUE;
					}
					
					if(message.value==0){
						theta_2=FALSE;
					}
                }
                
                
                
                
                if(message.control==36){
					if(message.value==127){
						huexx_0=TRUE;
					}
					
					if(message.value==0){
						huexx_0=FALSE;
					}
                }
                
                if(message.control==52){
					if(message.value==127){
						huexx_1=TRUE;
					}
					
					if(message.value==0){
						huexx_1=FALSE;
					}
                }
                
                 if(message.control==68){
					if(message.value==127){
						huexx_2=TRUE;
					}
					
					if(message.value==0){
						huexx_2=FALSE;
					}
                }
                
                if(message.control==46){
					if(message.value==127){
						toroid_switch=TRUE;
					}
					
					if(message.value==0){
						toroid_switch=FALSE;
					}
                }
                
                
                 if(message.control==39){
					if(message.value==127){
						y_skew_switch=TRUE;
					}
					
					if(message.value==0){
						y_skew_switch=FALSE;
					}
					
                }
                
                if(message.control==55){
					if(message.value==127){
						x_skew_switch=TRUE;
					}
					
					if(message.value==0){
						x_skew_switch=FALSE;
					}
					
                }
                if(y_skew_switch==TRUE){
					y_skew+=.00001;
                }
                
                if(x_skew_switch==TRUE){
					x_skew+=.00001;
                }
                
                
                if(message.control==71){
					if(message.value==127){
						wet_dry_switch=FALSE;
					}
					
					if(message.value==0){
						wet_dry_switch=TRUE;
					}
					
                }
                
                //---------------------
                
                
                
				if(message.control==37){
					if(message.value==127){
						huexy_0=TRUE;
					}
					
					if(message.value==0){
						huexy_0=FALSE;
					}
                }
                
                if(message.control==53){
					if(message.value==127){
						huexy_1=TRUE;
					}
					
					if(message.value==0){
						huexy_1=FALSE;
					}
                }
                
                 if(message.control==69){
					if(message.value==127){
						huexy_2=TRUE;
					}
					
					if(message.value==0){
						huexy_2=FALSE;
					}
                }
                
                
                //---------------------
                
                
                
                if(message.control==38){
					if(message.value==127){
						huexz_0=TRUE;
					}
					
					if(message.value==0){
						huexz_0=FALSE;
					}
                }
                
                if(message.control==54){
					if(message.value==127){
						huexz_1=TRUE;
					}
					
					if(message.value==0){
						huexz_1=FALSE;
					}
                }
                
                 if(message.control==70){
					if(message.value==127){
						huexz_2=TRUE;
					}
					
					if(message.value==0){
						huexz_2=FALSE;
					}
                }
                
                
                //
                
                if(message.control==62){
					if(message.value==127){
						luma_switch=TRUE;
					}
					
					if(message.value==0){
						luma_switch=FALSE;
					}
					
                }
                
                if(message.control==61){
					if(message.value==127){
						x_mirror_switch=TRUE;
					}
					
					if(message.value==0){
						x_mirror_switch=FALSE;
					}
					
                }
                
                 if(message.control==60){
					if(message.value==127){
						y_mirror_switch=TRUE;
					}
					
					if(message.value==0){
						y_mirror_switch=FALSE;
					}
					
                }
                
                
                
                
                
                //nanokontrol2 controls
                 //c1 maps to fb0 lumakey
                if(message.control==16){
                  //  c1=(message.value-63.0)/63.0;
                     c1=(message.value)/127.00;
                    
                }
                
                //c2 maps to fb0 mix
                if(message.control==17){
                    c2=(message.value-63.0)/63.0;
                   //   c2=(message.value)/127.00;
                    
                }
                
                //c3 maps to fb0 huex
                if(message.control==18){
                    c3=(message.value-63.0)/63.00;
                    //  c3=(message.value)/127.00;
                }
                
                //c4 maps to fb0 satx
                if(message.control==19){
                     c4=(message.value-63.0)/63.00;
                   // c4=(message.value)/127.00;
                   
                }
                
                //c5 maps to fb0 brightx
                if(message.control==20){
                     c5=(message.value-63.0)/63.00;
                  //  c5=(message.value)/127.00;
                  
                }
                
                //c6 maps to temporal filter
                if(message.control==21){
                    c6=(message.value-63.0)/63.0;
                     // c6=(message.value)/127.00;
                }
                
                //c7 maps to temporal filter resonance
                if(message.control==22){
                    //c7=(message.value-63.0)/63.0;
                      c7=(message.value)/127.00;
                }
                
                //c8 maps to brightx
                if(message.control==23){
                     //c8=(message.value-63.0)/63.00;
                    c8=(message.value)/127.0;
                   
                }
                
                //c9 maps to fb0 x displace
                if(message.control==120){
                     c9=(message.value-63.0)/63.00;
                     
                     if(x_2==TRUE){
						 c9=2.0*(message.value-63.0)/63.00;
						 }
						 
					 if(x_5==TRUE){
						 c9=5.0*(message.value-63.0)/63.00;
						 }
					 if(x_10==TRUE){
						 c9=10.0*(message.value-63.0)/63.00;
						 }	 	 
                    //c9=(message.value)/127.0;
                   
                }
                
                 //c10 maps to fb0 y displace
                if(message.control==121){
                     c10=(message.value-63.0)/63.00;
                     
                     
                     if(y_2==TRUE){
						 c10=2.0*(message.value-63.0)/63.00;
						 }
						 
					 if(y_5==TRUE){
						 c10=5.0*(message.value-63.0)/63.00;
						 }
					 if(y_10==TRUE){
						 c10=10.0*(message.value-63.0)/63.00;
						 }	 	 
                     
                     
                    //c10=(message.value)/127.0;
                   
                }
                
               
                if(message.control==122){
                     c11=(message.value-63.0)/63.00;
                     
                     if(z_2==TRUE){
						 c11=2.0*(message.value-63.0)/63.00;
						 }
						 
					 if(z_5==TRUE){
						 c11=5.0*(message.value-63.0)/63.00;
						 }
					 if(z_10==TRUE){
						 c11=10.0*(message.value-63.0)/63.00;
						 }	 	 
                     
                    //c11=(message.value)/127.0;
                   
                }
              
                if(message.control==123){
                     c12=(message.value-63.0)/63.00;
                     
                     if(theta_0==TRUE){
						 c12=2*(message.value-63.0)/63.00;
						 }
						 
					 if(theta_1==TRUE){
						 c12=4*(message.value-63.0)/63.00;
						 }
					 if(theta_2==TRUE){
						 c12=8*(message.value-63.0)/63.00;
						 }	 	
                     
                   // c12=(message.value)/127.0;
                   
                }
                
             
                if(message.control==124){
                    // c13=(message.value-63.0)/63.00;
                     
                    //instead of switches to go from different multiples
                    //switche to go from 0-.25, 0-.5,0-.75,0-1 
                     
                    c13=(message.value)/32.0;
                    
                    if(huexx_0==TRUE){
						 c13=message.value/64;
						 }
						 
					 if(huexx_1==TRUE){
						 c13=message.value/96.00;
						 }
					 if(huexx_2==TRUE){
						 c13=message.value/127.00;
						 }	 
                   
                }
              
            
                if(message.control==125){
                     c14=(message.value-63.0)/63.00;
                     
                     if(huexy_0==TRUE){
						 c14=2*(message.value-63.0)/63.00;
						 }
						 
					 if(huexy_1==TRUE){
						 c14=4*(message.value-63.0)/63.00;
						 }
					 if(huexy_2==TRUE){
						 c14=8*(message.value-63.0)/63.00;
						 }	 
                     
                     
                     
                    //c14=(message.value)/127.0;
                   
                }
                
               
                if(message.control==126){
                     c15=(message.value-63.0)/63.00;
                     
                     if(huexz_0==TRUE){
						 c15=2*(message.value-63.0)/63.00;
						 }
						 
					 if(huexz_1==TRUE){
						 c15=4*(message.value-63.0)/63.00;
						 }
					 if(huexz_2==TRUE){
						 c15=8*(message.value-63.0)/63.00;
						 }	 
                     
                    //c15=(message.value)/127.0;
                   
                }
                
                
                if(message.control==127){
                   //  c16=(message.value-63.0)/63.00;
                    c16=(message.value)/127.0;
                   
                }
                
                
                
                
                
                 //cc43 maps to fb0_b_invert
                if(message.control==43){
					if(message.value==127){
						fb0_b_invert=TRUE;
					}
					
					if(message.value==0){
						fb0_b_invert=FALSE;
					}
                }
                
                //cc44 maps to fb0_s_invert
                if(message.control==44){
					if(message.value==127){
						fb0_s_invert=TRUE;
					}
					
					if(message.value==0){
						fb0_s_invert=FALSE;
					}
                }
                
                //cc42 maps to fb0_s_invert
                if(message.control==42){
					if(message.value==127){
						fb0_h_invert=TRUE;
					}
					
					if(message.value==0){
						fb0_h_invert=FALSE;
					}
                }
                
                 
                //cc41 maps to fb0_h_mirror
                if(message.control==41){
					if(message.value==127){
						fb0_h_mirror=TRUE;
					}
					
					if(message.value==0){
						fb0_h_mirror=FALSE;
					}
                }
                
                 //cc45 maps to fb0_h_mirror
                if(message.control==45){
					if(message.value==127){
						fb0_v_mirror=TRUE;
					}
					
					if(message.value==0){
						fb0_v_mirror=FALSE;
					}
                }
                
                
                
                
                
                
                //this needs to be tested out and reworked
                //cc45 maps to fb0 clear
                //still doesn't work arggg
                if(message.control==58){
					if(message.value==127){
						if(clear_switch==0){
						 clear_switch=1;
								//clear the framebuffer if thats whats up
								framebuffer0.begin();
								ofClear(0, 0, 0, 255);
								framebuffer0.end();

								for(int i=0;i<fbob;i++){
									pastFrames[i].begin();
									ofClear(0,0,0,255);
									pastFrames[i].end();
        
    
								}//endifor
								
							}
						
					}
					
					
                }
                
                if(message.control!=58){
					clear_switch=0;
                }
                
                 if(message.control==59){
						
						x_skew=y_skew=c1=c2=c3=c4=c5=c6=c7=c8=c9=c10=c11=c12=c13=c14=c15=c16=0.0;
                }
                 
                
               
               

                
              
            }
            else if(message.status == MIDI_PITCH_BEND) {
                //text << "\tval: " << message.value;
			
			}
			else {
				//text << "\tpitch: " << message.pitch;
				
                
              
               

				
			}
			
		}//

	
	}
	
	
	//end midi biz
	
	}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}


