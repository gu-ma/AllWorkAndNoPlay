#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // Window
    ofBackground(0);
    ofSetVerticalSync(false);
    float r = 0.9; // ratio for the width of the window on the screen
    resScale = ofGetScreenWidth()*r/ofGetWidth();
//    ofSetFullscreen(true);
    ofSetWindowShape((int)ofGetWidth()*resScale,(int)ofGetHeight()*resScale);
    ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);
    ofSetFrameRate(29); // match framerate of the vid
    
    // General init
    showGui = false;
    animate = true;
    drawMesh = false;
    showMotionAmp = true;
    isPlaying = true;
    resetLive = true;
    //
    step = 0;
    
    // by default, connect to AbletonLive on local machine
    live.setup();
    
    // FBO - Init
    #ifdef TARGET_OPENGLES
        movieFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA ); // with alpha, 32 bits red, 32 bits green, 32 bits blue, 32 bits alpha, from 0 to 1 in 'infinite' steps
        motionAmplifierFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA );
        ofLogWarning("ofApp") << "GL_RGBA32F_ARB is not available for OPENGLES.  Using RGBA.";
    #else
        movieFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB); // with alpha, 32 bits red, 32 bits green, 32 bits blue, 32 bits alpha, from 0 to 1 in 'infinite' steps
        motionAmplifierFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB );
    #endif
    // FBO - clear
    movieFbo.begin();
        ofClear(255,255,255,0);
    movieFbo.end();
    motionAmplifierFbo.begin();
        ofClear(255,255,255,0);
    motionAmplifierFbo.end();
    
    // Browse dataset dir
    dir.allowExt("mov");
    dir.listDir("datasets/"); // to improve
    dir.sort();
    
    // Iterate through the .mov files and load the associated txt in the dataSet Info array
    for(int i = 0; i < (int)dir.size(); i++){
        string fileName, fileTxt;
        fileName = dir.getPath(i);
        ofFile txtFile(ofToDataPath( dir.path() + dir.getFile(i).getBaseName() + ".txt"));
        if ( txtFile.exists() ){
            ofBuffer buffer = txtFile.readToBuffer();
            fileTxt = buffer.getText();
        } else {
            fileTxt = "";
        }
        cout << fileName + " - " + fileTxt << endl;
        datasetInfo ds( 0, 0, fileTxt, fileName );
        dataSets.push_back(ds);
    }
    
    // video setup
    movie.setLoopState(OF_LOOP_NORMAL);
    movie.setVolume(0);
//    motionAmplifier.setup(ofGetWidth(), ofGetHeight(), 2, .10);
    motionAmplifier.setup(ofGetWidth(), ofGetHeight(), 2, .05);
    
    // load first movie
    refreshData(true);

    // audio log
    log.start();
    
    /////////
    // Animations values - Shader
    animationVariables["strength"].set(4,15);
    animationVariables["learningRate"].set(.2,.6);
    animationVariables["blurAmount"].set(20,1);
    animationVariables["windowSize"].set(2,10);
    // Animations values - Voice
    animationVariables["pbas"].set(130,130);
//    animationVariables["pmod"].set(50,200);
    animationVariables["pmod"].set(200,200);
    animationVariables["rate"].set(140,140);
    animationVariables["volm"].set(1,1);
    // Animations values - Live sound
    animationVariables["volMelody"].set(.5,.7);
    animationVariables["volBass"].set(.6,1);
    animationVariables["volVoice"].set(.6,.6);
    animationVariables["melodyRate01"].set(.2,.6);
    animationVariables["melodyRate02"].set(0,.5);
    animationVariables["bassRate01"].set(0,0);
    animationVariables["bassRate02"].set(0,0);
    animationVariables["voiceRate01"].set(0,.1);
    animationVariables["voiceRate02"].set(0,.05);
    
    /////////
    //ofParameters
    sinFreq01.set("sinFreq01", 10, 1, 60);
    sinFreq02.set("sinFreq02", 14, 1, 60);
    sinFreq03.set("sinFreq03", 18, 1, 60);
    strength.set("Strength", animationVariables["strength"].x, -50, 50);
    learningRate.set("Learning Rate", animationVariables["learningRate"].x, 0, 2);
    blurAmount.set("Blur Amount", animationVariables["blurAmount"].x, 0, 100);
    windowSize.set("Window Size", animationVariables["windowSize"].x, 1, 200);
    //
    pbas.set("pbas",animationVariables["pbas"].x,-500,500);
    pmod.set("pmod",animationVariables["pmod"].x,-500,500);
    rate.set("rate",animationVariables["rate"].x,1,600);
    volm.set("Speech",animationVariables["volm"].x,0,2);
    //
    volMelody.set("volMelody",animationVariables["volMelody"].x,0,1);
    volBass.set("volBass",animationVariables["volBass"].x,0,1);
    volVoice.set("volVoice",animationVariables["volVoice"].x,0,1);
    melodyRate01.set("melodyRate01",animationVariables["melodyRate01"].x,0,1);
    melodyRate02.set("melodyRate02",animationVariables["melodyRate02"].x,0,1);
    bassRate01.set("bassRate01",animationVariables["bassRate01"].x,0,1);
    bassRate02.set("bassRate02",animationVariables["bassRate02"].x,0,1);
    voiceRate01.set("voiceRate01",animationVariables["voiceRate01"].x,0,1);
    voiceRate02.set("voiceRate02",animationVariables["voiceRate02"].x,0,1);
    
    /////////
    // datGUI
    gui = new ofxDatGui();
    //
    ofxDatGuiFolder* f1 = gui->addFolder("animation", ofColor::white);
    f1->addToggle("animate");
    s1 = f1->addValuePlotter("sin01", -1, 1);
    s2 = f1->addValuePlotter("sin02", -1, 1);
    s3 = f1->addValuePlotter("sin03", -1, 1);
    s1->setDrawMode(ofxDatGuiGraph::LINES);
    s2->setDrawMode(ofxDatGuiGraph::LINES);
    s3->setDrawMode(ofxDatGuiGraph::LINES);
//    folder->addFRM();
    f1->addSlider(sinFreq01);
    f1->addSlider(sinFreq02);
    f1->addSlider(sinFreq03);
    gui->addBreak()->setHeight(10);
    //
    ofxDatGuiFolder* f2 = gui->addFolder("motion amplifier", ofColor::white);
    f2->addLabel("Motion");
    f2->addToggle("drawMesh");
    f2->addSlider(strength);
    f2->addSlider(learningRate);
    f2->addSlider(blurAmount);
    f2->addSlider(windowSize);
    gui->addBreak()->setHeight(10);
    //
    ofxDatGuiFolder* f3 = gui->addFolder("voice", ofColor::white);
    f3->addLabel("Voice");
    f3->addSlider(pbas);
    f3->addSlider(pmod);
    f3->addSlider(rate);
    f3->addSlider(volm);
    gui->addBreak()->setHeight(10);
    //
    ofxDatGuiFolder* f4 = gui->addFolder("music", ofColor::white);
    f4->addLabel("Sound");
    f4->addSlider(volMelody);
    f4->addSlider(volBass);
    f4->addSlider(volVoice);
    f4->addSlider(melodyRate01);
    f4->addSlider(melodyRate02);
    f4->addSlider(bassRate01);
    f4->addSlider(bassRate02);
    f4->addSlider(voiceRate01);
    f4->addSlider(voiceRate02);
    gui->addHeader(" :::: ");
    //
    gui->setPosition( ofGetWidth()-gui->getWidth()-30*resScale, 30*resScale);
    gui->setTheme(new ofxDatGuiThemeCharcoal());
    gui->setVisible( (showGui) ? true : false );
    // once the gui has been assembled, register callbacks to listen for component specific events //
    gui->onToggleEvent(this, &ofApp::onToggleEvent);

    //fonts
    ofTrueTypeFont::setGlobalDpi(96*resScale);
    consolasScrolling.load("consolas.ttf", 27, false);
    consolasScrolling.setLineHeight(47.0f*resScale);
    consolasScrolling.setLetterSpacing(1.137);
    consolasTitle.load("consolas.ttf", 50, false);
    
    //
    initTime = ofGetElapsedTimef();

}

//--------------------------------------------------------------
void ofApp::update(){

    //
    sinOfTime1 = sin((ofGetElapsedTimef())/sinFreq01);
    sinOfTime2 = sin((ofGetElapsedTimef()+PI/2)/sinFreq02);
    sinOfTime3 = sin((ofGetElapsedTimef()+PI)/sinFreq03);
    s1->setValue(sinOfTime1);
    s2->setValue(sinOfTime2);
    s2->setValue(sinOfTime3);
//    s2->setSpeed(gui->getSlider("sweep speed")->getValue());
    
    //
    tweenVariables();
    //
    live.update();
    updateLive();
    
    //
    ofFpsCounter fps;
    ofSetWindowTitle(ofToString((int) ofGetFrameRate()) + " fps");
//    ofSetWindowTitle(ofToString((int) ofGetElapsedTimef()) + " s");
    
    // update shader
    motionAmplifier.setStrength(strength);
    motionAmplifier.setLearningRate(learningRate);
    motionAmplifier.setBlurAmount(blurAmount);
    motionAmplifier.setWindowSize(windowSize);
    
    //
    movie.update();
    if(movie.isFrameNew()) {
        movieFbo.begin();
            movie.draw(0,0);
        movieFbo.end();
        
    }

    // speech
    if (!log.startSpeaking && ofGetElapsedTimef() > (log.endTime+4.f) && ofGetElapsedTimef()-initTime > 5.f) {
        refreshData(false);
        // build speech settings
        string voice = "Tom";
        string pb = ofToString(pbas).substr(0,4);
        string pm = ofToString(pmod).substr(0,4);
        string ra = ofToString(rate).substr(0,4);
        string vl = ofToString(volm).substr(0,3);
        string msg = currentPoem;
//        ofStringReplace(msg,"'","\'");
//        string misc = "--interactive=green/black";
        string misc = "";
        log.LogAudio(voice, pb, pm, ra, vl, msg);
        scrollingText.clear();
    }
    if (log.speechUpdate()) scrollingText.append(log.getCurrentWord() + " ");
  
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofBackground(0);
    // define preview rectangle
    ofRectangle previewWindow(0, 0, ofGetWidth(), ofGetHeight());
    ofRectangle videoGrabberRect(0,0,movie.getWidth(),movie.getHeight());
    videoGrabberRect.scaleTo(previewWindow,OF_SCALEMODE_FILL);
    movieFbo.draw(videoGrabberRect);

    //
    float padding = 60*resScale;
    string tempScrollingText = wrapString(scrollingText,ofGetWidth()/1.5);
    consolasScrolling.drawString(tempScrollingText, padding, ofGetHeight()/2-consolasScrolling.stringHeight(tempScrollingText)/2);
    string title = "All work and no play";
    if (step<3) consolasTitle.drawString(title, padding, ofGetHeight()/2-consolasTitle.stringHeight(title)/2-padding-consolasScrolling.stringHeight(tempScrollingText)/2);
    
    
    if (showMotionAmp) {

        ofImage screenImg;
        screenImg.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        
//        screenImg.getTexture().allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
//        screenImg.getTexture().loadScreenData(0, 0, ofGetWidth(), ofGetHeight());
        motionAmplifier.update(screenImg);
        ofSetupScreenOrtho(ofGetWidth(), ofGetHeight(), -300, +300);
        motionAmplifier.draw(screenImg);
        if (drawMesh) motionAmplifier.drawMesh();
    }
    
}

//--------------------------------------------------------------
void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
    if (e.target->is("animate")) animate = !animate;
    if (e.target->is("drawMesh")) drawMesh = !drawMesh;
    cout << "onToggleEvent: " << e.target->getLabel() << " " << e.checked << endl;
}

//--------------------------------------------------------------
void ofApp::tweenVariables(){
    
    if (animate) {
        //
        strength = ofMap(sinOfTime1, -1, 1, animationVariables["strength"].x, animationVariables["strength"].y);
        learningRate = ofMap(sinOfTime2, -1, 1, animationVariables["learningRate"].x, animationVariables["learningRate"].y);
        blurAmount = ofMap(sinOfTime1, -1, 1, animationVariables["blurAmount"].x, animationVariables["blurAmount"].y);
        windowSize = ofMap(sinOfTime2, -1, 1, animationVariables["windowSize"].x, animationVariables["windowSize"].y);
        //
        pbas = ofMap(sinOfTime1, -1, 1, animationVariables["pbas"].x, animationVariables["pbas"].y);
        pmod = ofMap(sinOfTime2, -1, 1, animationVariables["pmod"].x, animationVariables["pmod"].y);
        rate = ofMap(sinOfTime1, -1, 1, animationVariables["rate"].x, animationVariables["rate"].y);
        volm = ofMap(sinOfTime2, -1, 1, animationVariables["volm"].x, animationVariables["volm"].y);
        //
        volMelody = ofMap(sinOfTime2, -1, 1, animationVariables["volMelody"].x, animationVariables["volMelody"].y);
        volBass = ofMap(sinOfTime1, -1, 1, animationVariables["volBass"].x, animationVariables["volBass"].y);
        volVoice = ofMap(sinOfTime1, -1, 1, animationVariables["volVoice"].x, animationVariables["volVoice"].y);
        //
        melodyRate01 = ofMap(sinOfTime1, -1, 1, animationVariables["melodyRate01"].x, animationVariables["melodyRate01"].y);
        melodyRate02 = ofMap(sinOfTime2, -1, 1, animationVariables["melodyRate02"].x, animationVariables["melodyRate02"].y);
        bassRate01 = ofMap(sinOfTime1, -1, 1, animationVariables["bassRate01"].x, animationVariables["bassRate01"].y);
        bassRate02 = ofMap(sinOfTime2, -1, 1, animationVariables["bassRate02"].x, animationVariables["bassRate02"].y);
        voiceRate01 = ofMap(sinOfTime1, -1, 1, animationVariables["voiceRate01"].x, animationVariables["voiceRate01"].y);
        voiceRate02 = ofMap(sinOfTime2, -1, 1, animationVariables["voiceRate02"].x, animationVariables["voiceRate02"].y);
    }
}

void ofApp::updateLive() {

    if (!live.isLoaded()) {
        return;
    }

    if (resetLive) {
        initLive();
        resetLive = false;
    }
    
    ofxAbletonLiveTrack *trackMelody = live.getTrack("Melody");
    ofxAbletonLiveTrack *trackBass = live.getTrack("Bass");
    ofxAbletonLiveTrack *trackVoice = live.getTrack("Voice");
    
    // tracks have a number of features with getters/setters
    trackMelody->setVolume(volMelody);
    trackBass->setVolume(volBass);
    trackVoice->setVolume(volVoice);
    
    ofxAbletonLiveDevice *device = trackMelody->getDevice("Massive");
    ofxAbletonLiveParameter *noiseColor = device->getParameter(1);
    ofxAbletonLiveParameter *noiseAmp = device->getParameter(2);
    noiseColor->setValue(melodyRate01);
    noiseAmp->setValue(melodyRate02);

    ofxAbletonLiveDevice *device1 = trackVoice->getDevice("Reaktor 6 FX");
    ofxAbletonLiveParameter *synth = device1->getParameter(2);
    ofxAbletonLiveParameter *fx = device1->getParameter(5);
    synth->setValue(voiceRate01);
    fx->setValue(voiceRate01);
    
}


//--------------------------------------------------------------
string ofApp::wrapString(string text, int width) {
    
    string typeWrapped = "";
    string tempString = "";
    vector <string> words = ofSplitString(text, " ");
    
    for(int i=0; i<words.size(); i++) {
        
        string wrd = words[i];
        
        // if we aren't on the first word, add a space
        if (i > 0) {
            tempString += " ";
        }
        tempString += wrd;
        
        int stringwidth = consolasScrolling.stringWidth(tempString);
        
        if(stringwidth >= width) {
            typeWrapped += "\n";
            tempString = wrd;		// make sure we're including the extra word on the next line
        } else if (i > 0) {
            // if we aren't on the first word, add a space
            typeWrapped += " ";  
        }  
        
        typeWrapped += wrd;  
    }  
    
    return typeWrapped;  
    
}

//--------------------------------------------------------------
void ofApp::refreshData(bool allText) {
    
    int r = int(ofRandom(dataSets.size()));
    currentFileName = dataSets[r].fileName;
    if (allText) {
        currentPoem = dataSets[r].poem;
    } else {
        vector <string> sentences = ofSplitString(dataSets[r].poem, ".", true, true);
        if ( sentences.size()>1 ) {
            int a = ofRandom(sentences.size());
            currentPoem = sentences[a] + ".";
//            int a = ofRandom(sentences.size()-1);
//            currentPoem = sentences[a] + ".\n" + sentences[a+1] + ".";
        } else {
            currentPoem = dataSets[r].poem;
        }
    }
    //
    movie.closeMovie();
//    cout << "loading: " + currentFileName << endl;
    movie.load(currentFileName);
    movie.play();
    movie.setVolume(0);
    
    // FBO settings
    movieFbo.allocate(movie.getWidth(), movie.getHeight());
//    motionAmplifierFbo.allocate(movie.getWidth(), movie.getHeight());
//    motionAmplifier.setup(movie.getWidth(), movie.getHeight(), 2, .10);
    step ++;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'g') {
        showGui = !showGui;
        gui->setVisible( (showGui) ? true : false );
    }
    if (key == 'm') {
        showMotionAmp = !showMotionAmp;
    }
    if (key == 'n') {
        drawMesh = !drawMesh;
    }
    if (key == 'l') {
        live.printAll();
    }
    if (key == '.') {
        float v = live.getVolume();
        live.setVolume( ofClamp(v + 0.1, 0, 1) );
    }
    if (key == ',') {
        float v = live.getVolume();
        live.setVolume( ofClamp(v - 0.1, 0, 1) );
    }
    if (key == '0') {
        initLive();
    }
    
}

//--------------------------------------------------------------
void ofApp::initLive(){
    if (live.isLoaded()) {
        for ( int x=0; x<live.getNumTracks() ; x++ ){
            ofxAbletonLiveTrack *track = live.getTrack(x);
            track->setVolume(0);
        }
        live.setVolume(0.4);
        live.stop();
        live.play();
        live.setTempo(90);
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

