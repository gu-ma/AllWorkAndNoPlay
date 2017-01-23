#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxJSON.h"
#include "ofLogAudio.h"
#include "MotionAmplifier.h"
#include "datasetInfo.h"
#include "ofxDatGui.h"
#include "ofxEasing.h"
#include "ofxAbletonLive.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    
    //
    vector<datasetInfo> dataSets;
    ofDirectory dir;
    ofTexture movieTexture, screenTexture;
    void refreshData(bool allText);
    string currentFileName, currentPoem;
    bool showMotionAmp, resetLive;
    void updateLive(), initLive();
    ofxAbletonLive live;
    ofxDatGuiValuePlotter* s1;
    ofxDatGuiValuePlotter* s2;
    ofxDatGuiValuePlotter* s3;
    float sinOfTime1,sinOfTime2,sinOfTime3;
    
    //32 bits red, 32 bits green, 32 bits blue, from 0 to 1 in 'infinite' steps
    ofFbo movieFbo, motionAmplifierFbo; // with alpha
    void drawMotionAmplifierFbo();
    void drawMovieFbo();
    
    // General
    string wrapString(string text, int width);
    void tweenVariables();
    map<string, ofVec2f> animationVariables;  // not very clear to use a ofVec2f
    float initTime, maskAlpha;
    int step, newsCount;
    float resScale;
    bool showGui, animate, drawMesh, isPlaying;
    
    // text
    vector<string> newsKeywords, sentences, dates;
    string textBuffer, scrollingText;
    ofTrueTypeFont consolasScrolling, consolasTitle;
    
    // audio + video
    ofLogAudio log;  // speech
    ofVideoPlayer movie;
    ofSoundPlayer music;
    
    //
    MotionAmplifier motionAmplifier;

    // datGUI
    ofxDatGui* gui;
    void onToggleEvent(ofxDatGuiToggleEvent e);
    
    // ofParam
    ofParameter<float> blurAmount, windowSize, learningRate, strength, sinFreq01, sinFreq02, sinFreq03;
    ofParameter<float> pbas, pmod, rate, volm, volMelody, volBass;
    ofParameter<float> volVoice, melodyRate01, melodyRate02, bassRate01, bassRate02, voiceRate01, voiceRate02;

    // unused
    ofFbo fbo;
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);    

};
