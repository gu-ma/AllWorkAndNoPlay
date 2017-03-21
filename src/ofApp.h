#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofLogAudio.h"
#include "MotionAmplifier.h"
#include "datasetInfo.h"
#include "ofxDatGui.h"
#include "ofxAbletonLive.h"
#include "ofxCenteredTrueTypeFont.h"

class ofApp : public ofBaseApp{

public:
    
    void setup();
    void update();
    void draw();
    void exit();
    void keyPressed(int key);
    
    // Dir and data
    vector<datasetInfo> dataSets,dataSetsClone;
    ofDirectory dir;
    string currentFileName, currentPoem;
    void refreshData(bool allText);
    // Ableton live
    void updateLive(), initLive();
    ofxAbletonLive live;
    // FBOs
    ofFbo movieFbo, motionAmplifierFbo;
    // General
    float initTime, maskAlpha, resScale, prevStep;
    int step;
    bool showGui, animate, drawMesh, isPlaying, wordByWord, showMotionAmp, resetLive;
    void setParameters();
    string wrapString(string text, int width);
    void tweenVariables();
    // Animation
    float sinOfTime1, sinOfTime2, sinOfTime3;
    map<string, ofVec2f> animationVariables;  // not very clear to use a ofVec2f
    // text
    vector<string> sentences;
    string textBuffer, scrollingText;
    ofTrueTypeFont consolasScrolling, consolasTitle;
    ofxCenteredTrueTypeFont helveticaScrolling, helveticaTtitle;
    // audio + video
    ofLogAudio log;
    ofVideoPlayer movie;
    //
    MotionAmplifier motionAmplifier;
    // datGUI
    ofxDatGui* gui;
    ofxDatGuiValuePlotter* s1;
    ofxDatGuiValuePlotter* s2;
    ofxDatGuiValuePlotter* s3;
    void onToggleEvent(ofxDatGuiToggleEvent e);
    void setupGUI();
    // ofParam
    ofParameter<float> blurAmount, windowSize, learningRate, strength, sinFreq01, sinFreq02, sinFreq03;
    ofParameter<float> pbas, pmod, rate, volm, volMelody, volChords, volBass, volVoice;
    ofParameter<float> melodyRate01, melodyRate02, chordsRate01, chordsRate02, bassRate01, bassRate02, voiceRate01, voiceRate02;

};
