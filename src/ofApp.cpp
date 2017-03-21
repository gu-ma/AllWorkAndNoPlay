#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // Setup
    ofBackground(0);
    ofSetVerticalSync(false);
    ofSetFrameRate(30); // match framerate of the vid
    float r = 0.8; // ratio for the width of the window on the screen
    resScale = ofGetScreenWidth()*r/ofGetWidth();
    ofSetFullscreen(true);
//    ofSetWindowShape((int)ofGetWidth()*resScale,(int)ofGetHeight()*resScale);
//    ofSetWindowPosition((ofGetScreenWidth()-ofGetWidth())/2, (ofGetScreenHeight()-ofGetHeight())/2);

    // General init
    showGui = false;
    animate = true;
    drawMesh = false;
    showMotionAmp = true;
    isPlaying = true;
    resetLive = true;
    wordByWord = true;
    step = 0;
    
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
    dataSetsClone = dataSets;
    // video setup
    movie.setLoopState(OF_LOOP_NORMAL);
    movie.setVolume(0);
    motionAmplifier.setup(ofGetWidth(), ofGetHeight(), 2, .05);
    // load first movie
    refreshData(true);
    // audio log
    log.start();
    //
    setParameters();
    setupGUI();
    // Live
    live.setup();
    
    // Fonts
    ofTrueTypeFont::setGlobalDpi(96*resScale);
    consolasScrolling.load("consolas.ttf", 40, false);
    consolasScrolling.setLineHeight(47.0f*resScale);
    consolasScrolling.setLetterSpacing(1.137);
    consolasTitle.load("consolas.ttf", 50, false);
    helveticaScrolling.load("HelveticaNeueLTStd-Md.otf", 30, true, true, true, 0.1);
    helveticaTtitle.load("HelveticaNeueLTStd-Bd.otf", 80, true, false, true, 0.1);
    helveticaScrolling.setLineHeight(50.0f*resScale);
    
    //
    initTime = ofGetElapsedTimef();
}


//--------------------------------------------------------------
void ofApp::update(){
    // SOT
//    sinOfTime1 = sin((ofGetElapsedTimef()-initTime)/sinFreq01);
//    sinOfTime2 = sin((ofGetElapsedTimef()-initTime+PI/3)/sinFreq02);
//    sinOfTime3 = sin((ofGetElapsedTimef()-initTime+2*PI/3)/sinFreq03);
    sinOfTime1 = ofSignedNoise((ofGetElapsedTimef()-initTime)/(sinFreq01*4), 0);
    sinOfTime2 = ofSignedNoise((ofGetElapsedTimef()-initTime+sinFreq01)/(sinFreq01*4), 0);
    sinOfTime3 = ofSignedNoise((ofGetElapsedTimef()-initTime+2*sinFreq01)/(sinFreq01*4), 0);
    s1->setValue(sinOfTime1);
    s2->setValue(sinOfTime2);
    s3->setValue(sinOfTime3);
    //
    tweenVariables();
    //
    live.update();
    updateLive();
    //
    ofFpsCounter fps;
    ofSetWindowTitle(ofToString((int) ofGetFrameRate()) + " fps");
    // ofSetWindowTitle(ofToString((int) ofGetElapsedTimef()) + " s");
    // update shader
    motionAmplifier.setStrength(strength);
    motionAmplifier.setLearningRate(learningRate);
    motionAmplifier.setBlurAmount(blurAmount);
    motionAmplifier.setWindowSize(windowSize);
    //
    movie.update();
    if(movie.isFrameNew()) {
        movieFbo.begin();
//            float movieRatio = movie.getWidth()/movie.getHeight();
//            float windowRatio = float(ofGetWidth())/float(ofGetHeight());
//            float w = ofGetWidth();
//            float h = ofGetHeight();
//            if (movieRatio>windowRatio) {
//                // Landscape
//                float scaling = ofGetHeight()/movie.getHeight();
//                float scaledWindowWidth = ofGetWidth()/scaling;
//                float scaledWindowHeight = ofGetHeight()/scaling;
//                float scrollX = ofMap(mouseX, 0, ofGetWidth(), -(scaledWindowWidth-movie.getWidth())/2, (scaledWindowWidth-movie.getWidth())/2);
//                movie.draw(scrollX,0);
//                cout << "scrollX: " + ofToString(scrollX) << endl;
//            } else {
//                // Portrait
//                float scaling = ofGetWidth()/movie.getWidth();
//                float scaledWindowWidth = ofGetWidth()/scaling;
//                float scaledWindowHeight = ofGetHeight()/scaling;
//                float scrollY = ofMap(mouseY, 0, ofGetHeight(), -(scaledWindowHeight-movie.getHeight())/2, (scaledWindowHeight-movie.getHeight())/2);
//                movie.draw(0,scrollY);
//                cout << "scrollY: " + ofToString(scrollY) << endl;
//            }
//            ofSetColor(255,ofMap(sinOfTime3, 1, -1, 20,200));
//            ofSetColor(255,ofMap(mouseX, 0, ofGetWidth(), 0,200));
//            if ( int(floor(prevStep)) != step) {
//                ofSetColor(255,50);
//                prevStep+=0.05;
//                cout << prevStep << endl;
//            } else {
//                ofSetColor(255,ofMap(mouseX, 0, ofGetWidth(), 0,255));
//            }
            movie.draw(0,0);
            ofClearAlpha();
        movieFbo.end();
    }
    // speech
    if (!log.startSpeaking && ofGetElapsedTimef() > (log.endTime+5.f) && ofGetElapsedTimef()-initTime > 5.f) {
        refreshData(false);
        // build speech settings
        string voice = "Kate";
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
    if (wordByWord) {
        if (log.speechUpdate()) scrollingText.append(log.getCurrentWord() + " ");
    } else {
        scrollingText = currentPoem;
    }
}


//--------------------------------------------------------------
void ofApp::draw(){
    // define preview rectangle
    ofRectangle previewWindow(0, 0, ofGetWidth(), ofGetHeight());
    ofRectangle videoGrabberRect(0,0,movie.getWidth(),movie.getHeight());
    videoGrabberRect.scaleTo(previewWindow,OF_SCALEMODE_FILL);
    movieFbo.draw(videoGrabberRect);
    //
    float padding = 60*resScale;
    string tempScrollingText = wrapString(scrollingText,ofGetWidth()/1.4);
    string title = "ALL WORK \nAND NO PLAY";
    if (step<2) {
        ofSetColor(0,50);
        helveticaTtitle.drawStringAsShapes(title, ofGetWidth()/2-helveticaTtitle.stringWidth(title)/2, ofGetHeight()/2-helveticaTtitle.stringHeight(title)/2);
        ofSetColor(255);
        helveticaTtitle.drawStringAsShapes(title, ofGetWidth()/2-helveticaTtitle.stringWidth(title)/2-3, ofGetHeight()/2-helveticaTtitle.stringHeight(title)/2-3);
    }
    if (step%3!=0) {
        ofSetColor(0,50);
        helveticaScrolling.drawStringAsShapes(tempScrollingText, ofGetWidth()/2-helveticaScrolling.stringWidth(tempScrollingText)/2, ofGetHeight()/2-helveticaScrolling.stringHeight(tempScrollingText)/2);
        ofSetColor(255);
        helveticaScrolling.drawStringAsShapes(tempScrollingText, ofGetWidth()/2-helveticaScrolling.stringWidth(tempScrollingText)/2-3, ofGetHeight()/2-helveticaScrolling.stringHeight(tempScrollingText)/2-3);
    }
    //
    if (showMotionAmp) {
//        ofSetColor(255,ofMap(mouseY,0,ofGetHeight(),0,255));
        ofImage screenImg;
        screenImg.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
        //        screenImg.getTexture().allocate(ofGetWidth(), ofGetHeight(), GL_RGBA);
        //        screenImg.getTexture().loadScreenData(0, 0, ofGetWidth(), ofGetHeight());
        motionAmplifier.update(screenImg);
        ofSetupScreenOrtho(ofGetWidth(), ofGetHeight(), -300, +300);
        motionAmplifier.draw(screenImg);
        if (drawMesh) motionAmplifier.drawMesh();
    }
    //
    if (step%3==0) {
        ofSetColor(0,50);
        helveticaScrolling.drawStringAsShapes(tempScrollingText, ofGetWidth()/2-helveticaScrolling.stringWidth(tempScrollingText)/2, ofGetHeight()/2-helveticaScrolling.stringHeight(tempScrollingText)/2);
        ofSetColor(255);
        helveticaScrolling.drawStringAsShapes(tempScrollingText, ofGetWidth()/2-helveticaScrolling.stringWidth(tempScrollingText)/2-3, ofGetHeight()/2-helveticaScrolling.stringHeight(tempScrollingText)/2-3);
    }
}


//--------------------------------------------------------------
void ofApp::onToggleEvent(ofxDatGuiToggleEvent e)
{
    if (e.target->is("animate")) animate = !animate;
    if (e.target->is("drawMesh")) drawMesh = !drawMesh;
    if (e.target->is("wordByWord")) wordByWord = !wordByWord;
    cout << "onToggleEvent: " << e.target->getLabel() << " " << e.checked << endl;
}


//--------------------------------------------------------------
void ofApp::tweenVariables(){
    if (animate) {
        //
        strength = ofMap(sinOfTime3, 1, -1, animationVariables["strength"].x, animationVariables["strength"].y);
        learningRate = ofMap(sinOfTime1, 1, -1, animationVariables["learningRate"].x, animationVariables["learningRate"].y);
        blurAmount = ofMap(sinOfTime1, 1, -1, animationVariables["blurAmount"].x, animationVariables["blurAmount"].y);
        windowSize = ofMap(sinOfTime3, 1, -1, animationVariables["windowSize"].x, animationVariables["windowSize"].y);
        //
        pbas = ofMap(sinOfTime1, 1, -1, animationVariables["pbas"].x, animationVariables["pbas"].y);
        pmod = ofMap(sinOfTime2, 1, -1, animationVariables["pmod"].x, animationVariables["pmod"].y);
        rate = ofMap(sinOfTime1, 1, -1, animationVariables["rate"].x, animationVariables["rate"].y);
        volm = ofMap(sinOfTime2, 1, -1, animationVariables["volm"].x, animationVariables["volm"].y);
        //
        volMelody = ofMap(sinOfTime2, 1, -1, animationVariables["volMelody"].x, animationVariables["volMelody"].y);
        volChords = ofMap(sinOfTime3, 1, -1, animationVariables["volChords"].x, animationVariables["volChords"].y);
        volBass = ofMap(sinOfTime1, 1, -1, animationVariables["volBass"].x, animationVariables["volBass"].y);
        volVoice = ofMap(sinOfTime1, 1, -1, animationVariables["volVoice"].x, animationVariables["volVoice"].y);
        //
        melodyRate01 = ofMap(sinOfTime3, 1, -1, animationVariables["melodyRate01"].x, animationVariables["melodyRate01"].y);
        melodyRate02 = ofMap(sinOfTime2, 1, -1, animationVariables["melodyRate02"].x, animationVariables["melodyRate02"].y);
        chordsRate01 = ofMap(sinOfTime2, 1, -1, animationVariables["chordsRate01"].x, animationVariables["chordsRate01"].y);
        chordsRate02 = ofMap(sinOfTime3, 1, -1, animationVariables["chordsRate02"].x, animationVariables["chordsRate02"].y);
        bassRate01 = ofMap(sinOfTime1, 1, -1, animationVariables["bassRate01"].x, animationVariables["bassRate01"].y);
        bassRate02 = ofMap(sinOfTime3, 1, -1, animationVariables["bassRate02"].x, animationVariables["bassRate02"].y);
        voiceRate01 = ofMap(sinOfTime1, 1, -1, animationVariables["voiceRate01"].x, animationVariables["voiceRate01"].y);
        voiceRate02 = ofMap(sinOfTime2, 1, -1, animationVariables["voiceRate02"].x, animationVariables["voiceRate02"].y);
    }
}


//--------------------------------------------------------------
void ofApp::updateLive() {
    if (!live.isLoaded()) {
        return;
    }
    if (resetLive) {
        initLive();
        resetLive = false;
    }
    // Get tracks
    ofxAbletonLiveTrack *trackMelody = live.getTrack("Melody");
    ofxAbletonLiveTrack *trackChords = live.getTrack("Chords");
    ofxAbletonLiveTrack *trackBass = live.getTrack("Bass");
    ofxAbletonLiveTrack *trackVoice = live.getTrack("Voice");
    // Volumes
    trackMelody->setVolume(volMelody);
    trackChords->setVolume(volChords);
    trackBass->setVolume(volBass);
    trackVoice->setVolume(volVoice);
    // Fx - Melody
    ofxAbletonLiveDevice *device = trackMelody->getDevice("Massive");
    ofxAbletonLiveParameter *noiseColor = device->getParameter(1);
    ofxAbletonLiveParameter *noiseAmp = device->getParameter(2);
    ofxAbletonLiveParameter *dryWet = device->getParameter(3);
    noiseColor->setValue(melodyRate01);
    noiseAmp->setValue(melodyRate02);
    dryWet->setValue(melodyRate01);
    // Fx - Chords
    ofxAbletonLiveDevice *device1 = trackChords->getDevice("Massive");
    ofxAbletonLiveParameter *noiseColor1 = device1->getParameter(1);
    ofxAbletonLiveParameter *noiseAmp1 = device1->getParameter(2);
    ofxAbletonLiveParameter *cutOff = device1->getParameter(3);
    ofxAbletonLiveParameter *resonance = device1->getParameter(4);
    ofxAbletonLiveParameter *intensity = device1->getParameter(5);
    noiseColor1->setValue(melodyRate01);  // reuse of same ofParameter not good
    noiseAmp1->setValue(melodyRate02/4);
    cutOff->setValue(chordsRate01);
    resonance->setValue(chordsRate02);
    intensity->setValue(chordsRate02/2);
    // Fx - Bass
    ofxAbletonLiveDevice *device2 = trackBass->getDevice("Massive");
    ofxAbletonLiveParameter *cutOff1 = device2->getParameter(1);
    ofxAbletonLiveParameter *resonance1 = device2->getParameter(2);
    ofxAbletonLiveParameter *intensity1 = device2->getParameter(4);
    cutOff1->setValue(bassRate01);
    resonance1->setValue(bassRate01);
    intensity1->setValue(bassRate02);
    // Fx - Voice
    ofxAbletonLiveDevice *device3 = trackVoice->getDevice("Reaktor 6 FX");
    ofxAbletonLiveParameter *synth = device3->getParameter(2);
    ofxAbletonLiveParameter *fx = device3->getParameter(5);
    synth->setValue(voiceRate01);
    fx->setValue(voiceRate01);
}


//--------------------------------------------------------------
void ofApp::refreshData(bool allText) {
    if ( dataSetsClone.size() <= 1 ) {
        dataSetsClone = dataSets;
    } else {
        int r = int(ofRandom(dataSetsClone.size()));
        currentFileName = dataSetsClone[r].fileName;
        if (allText) {
            currentPoem = dataSetsClone[r].poem;
        } else {
            vector <string> sentences = ofSplitString(dataSetsClone[r].poem, ".", true, true);
            if ( sentences.size()>1 ) {
                int a = ofRandom(sentences.size());
                currentPoem = sentences[a] + ".";
                //            int a = ofRandom(sentences.size()-1);
                //            currentPoem = sentences[a] + ".\n" + sentences[a+1] + ".";
            } else {
                currentPoem = dataSetsClone[r].poem;
            }
        }
        dataSetsClone.erase(dataSetsClone.begin() + r);
        cout << dataSetsClone.size() << endl;
    }

    //
    movie.closeMovie();
    cout << "loading: " + currentFileName << endl;
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
        live.setVolume(0.8);
        live.stop();
        live.play();
        live.setTempo(90);
    }
}

//--------------------------------------------------------------
void ofApp::setupGUI(){
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
    s1->setSpeed(.5);
    s2->setSpeed(.5);
    s3->setSpeed(.5);
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
    f3->addToggle("wordByWord");
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
    f4->addSlider(volChords);
    f4->addSlider(volBass);
    f4->addSlider(volVoice);
    f4->addSlider(melodyRate01);
    f4->addSlider(melodyRate02);
    f4->addSlider(chordsRate01);
    f4->addSlider(chordsRate02);
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
}

//--------------------------------------------------------------
void ofApp::setParameters(){
    // NOISE
    // Animations values - Shader
    animationVariables["strength"].set(35,10);
    animationVariables["learningRate"].set(.2,.4);
    animationVariables["blurAmount"].set(1,30);
    animationVariables["windowSize"].set(12,3);
    // Animations values - Voice
    animationVariables["pbas"].set(30,150);
    animationVariables["pmod"].set(1,220);
    animationVariables["rate"].set(140,140);
    animationVariables["volm"].set(1,1);
    // Animations values - Live sound
    animationVariables["volMelody"].set(.1,.5);
    animationVariables["volChords"].set(.2,1);
    animationVariables["volBass"].set(.2,1);
    animationVariables["volVoice"].set(.6,.7);
    animationVariables["melodyRate01"].set(.1,.9);
    animationVariables["melodyRate02"].set(0,.9);
    animationVariables["chordsRate01"].set(0,.9);
    animationVariables["chordsRate02"].set(0,.78);
    animationVariables["bassRate01"].set(0,.5);
    animationVariables["bassRate02"].set(0,1);
    animationVariables["voiceRate01"].set(0,1);
    animationVariables["voiceRate02"].set(0,1);
    
    //    // SIN
    //    // Animations values - Shader
    //    animationVariables["strength"].set(5,35);
    //    animationVariables["learningRate"].set(.4,.2);
    //    animationVariables["blurAmount"].set(2,30);
    //    animationVariables["windowSize"].set(8,4);
    //    // Animations values - Voice
    //    animationVariables["pbas"].set(130,130);
    //    animationVariables["pmod"].set(100,200);
    //    animationVariables["rate"].set(140,140);
    //    animationVariables["volm"].set(1,1);
    //    // Animations values - Live sound
    //    animationVariables["volMelody"].set(.1,.5);
    //    animationVariables["volChords"].set(.1,.7);
    //    animationVariables["volBass"].set(.1,.8);
    //    animationVariables["volVoice"].set(.5,.6);
    //    animationVariables["melodyRate01"].set(.2,.7);
    //    animationVariables["melodyRate02"].set(0,.8);
    //    animationVariables["chordsRate01"].set(0.02,.8);
    //    animationVariables["chordsRate02"].set(0,.75);
    //    animationVariables["bassRate01"].set(0,.5);
    //    animationVariables["bassRate02"].set(0,1);
    //    animationVariables["voiceRate01"].set(0,.7);
    //    animationVariables["voiceRate02"].set(0,.6);
    
    /////////
    //ofParameters
    sinFreq01.set("sinFreq01", 18*1, 1, 60);
    sinFreq02.set("sinFreq02", 24*1, 1, 60);
    sinFreq03.set("sinFreq03", 30*1, 1, 60);
    strength.set("Strength", animationVariables["strength"].x, -50, 50);
    learningRate.set("Learning Rate", animationVariables["learningRate"].x, 0, 2);
    blurAmount.set("Blur Amount", animationVariables["blurAmount"].x, 0, 50);
    windowSize.set("Window Size", animationVariables["windowSize"].x, 1, 50);
    // Speech
    pbas.set("pbas",animationVariables["pbas"].x,-500,500);
    pmod.set("pmod",animationVariables["pmod"].x,-500,500);
    rate.set("rate",animationVariables["rate"].x,1,600);
    volm.set("Speech",animationVariables["volm"].x,0,2);
    // Sound
    volMelody.set("volMelody",animationVariables["volMelody"].x,0,1);
    volChords.set("volChords",animationVariables["volChords"].x,0,1);
    volBass.set("volBass",animationVariables["volBass"].x,0,1);
    volVoice.set("volVoice",animationVariables["volVoice"].x,0,1);
    melodyRate01.set("melodyRate01",animationVariables["melodyRate01"].x,0,1);
    melodyRate02.set("melodyRate02",animationVariables["melodyRate02"].x,0,1);
    chordsRate01.set("chordsRate01",animationVariables["chordsRate01"].x,0,1);
    chordsRate02.set("chordsRate02",animationVariables["chordsRate02"].x,0,1);
    bassRate01.set("bassRate01",animationVariables["bassRate01"].x,0,1);
    bassRate02.set("bassRate02",animationVariables["bassRate02"].x,0,1);
    voiceRate01.set("voiceRate01",animationVariables["voiceRate01"].x,0,1);
    voiceRate02.set("voiceRate02",animationVariables["voiceRate02"].x,0,1);
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
        int stringwidth = helveticaScrolling.stringWidth(tempString);
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
void ofApp::exit(){
    live.stop();
}
