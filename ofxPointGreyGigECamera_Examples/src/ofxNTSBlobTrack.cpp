#include "ofxNTSBlobTrack.h"



ofxNTSBlobTrack::ofxNTSBlobTrack(){
    gHide = false;
    scaleCams   = 0.5;
    camWidth    = 1920;
    camHeight   = 1080;
    scaledWidth = 1920;
    scaledHeight = 1080;
    posX        = 0;
    posY        = 0;
    cameraId      = 0;
}

ofxNTSBlobTrack::~ofxNTSBlobTrack(){
    if(source.isAllocated()){
        //source.clear();
        //bgImg.clear();bgIm
        highpass.clear();
        blurImg.clear();
        tmp.clear();
        tmpBlur.clear();
        //trackedImg.clear();
    }
}

//---------------------------------------------------------------
void ofxNTSBlobTrack::setup(){
    if(!source.isAllocated()){
        source.allocate(camWidth, camHeight, OF_IMAGE_GRAYSCALE);
        bgImg.allocate(camWidth,camHeight,OF_IMAGE_GRAYSCALE);
    }
    highpass.allocate(camWidth,camHeight);
    blurImg.allocate(camWidth,camHeight);
    tmp.allocate(camWidth,camHeight);
    tmpBlur.allocate(camWidth,camHeight);
    if(!trackedImg.isAllocated()){
        trackedImg.allocate(camWidth,camHeight, OF_IMAGE_GRAYSCALE);
    }

    //GUI
    ofColor bgColor;
    bgColor.set(10,20,100,180);
    ofColor bgFillColor;
    bgFillColor.set(30,110,160,220);

    ofColor infoBgColor;
    infoBgColor.set(25,35,75,180);

    //Event Listeners
	bUseTexture.addListener(this, &ofxNTSBlobTrack::bUseTextureEnablePressed);
	bgEnable.addListener(this, &ofxNTSBlobTrack::bgEnablePressed);
	blurEnable.addListener(this, &ofxNTSBlobTrack::blurEnablePressed);
	hpEnable.addListener(this, &ofxNTSBlobTrack::hpEnablePressed);
	blobApply.addListener(this,&ofxNTSBlobTrack::blobApplyPressed);
	smRect.addListener(this, &ofxNTSBlobTrack::smRectEnablePressed);
	backgroundSample.addListener(this,&ofxNTSBlobTrack::backgroundSamplePressed);
	velocity.addListener(this, &ofxNTSBlobTrack::velocityEnablePressed);
	bfEllipse.addListener(this, &ofxNTSBlobTrack::bfEllipseEnablePressed);
	mnCircle.addListener(this, &ofxNTSBlobTrack::mnCircleEnablePressed);
	cnvxHull.addListener(this, &ofxNTSBlobTrack::cnvxEnablePressed);
	loadSettings.addListener(this,&ofxNTSBlobTrack::loadSettingsPressed);
	saveSettings.addListener(this,&ofxNTSBlobTrack::saveSettingsPressed);
    minTextureSwitch.addListener(this,&ofxNTSBlobTrack::minTextureSwitchPressed);
    minBGSample.addListener(this,&ofxNTSBlobTrack::minBGSamplePressed);

    blobTrackGui.setup(); // most of the time you don't need a name
    blobTrackGui.setName("Blob Track Gui");
    blobTrackGui.setHeaderBackgroundColor(infoBgColor);
    screenSize = ofToString(ofGetWidth()) + "x" + ofToString(ofGetHeight());
    fpsLabel = ofToString(ofGetFrameRate(),2);
	blobTrackGui.add(bUseTexture.setup("Texture",1,120,20));
	blobTrackGui.add(fpsLabel.setup("FPS", fpsLabel));
	blobTrackGui.add(screenSize.setup("Screen Size", screenSize));
	blobTrackGui.add(loadSettings.setup("Load Settings"));
	blobTrackGui.add(saveSettings.setup("Save Settings"));

    fpsLabel.setBackgroundColor(infoBgColor);
    screenSize.setBackgroundColor(infoBgColor);
    screenSize.setFillColor(infoBgColor);
    loadSettings.setFillColor(infoBgColor);
    loadSettings.setBackgroundColor(infoBgColor);
    saveSettings.setFillColor(infoBgColor);
    saveSettings.setBackgroundColor(infoBgColor);

	bgGui.setup();
	bgGui.setName("Background");
	bgGui.setPosition(100, (camHeight/2)+100+(camHeight/4));
	bgGui.setSize(camWidth/4,20);
	bgGui.setBorderColor(bgFillColor);
	bgGui.setHeaderBackgroundColor(bgColor);
	bgGui.add(bgEnable.setup("Background",1,120,20));
	bgGui.add(backgroundSample.setup("Background Reset"));
	bgGui.add(learnSpeed.setup("Learn Speed", 1000, 0, 2000, camWidth/4,20.0));
	bgGui.add(thresholdValue.setup("Threshold", 20, 0, 255, camWidth/4,20.0));

    bgEnable.setBorderColor(bgFillColor);
    backgroundSample.setBorderColor(bgFillColor);
    learnSpeed.setBackgroundColor(bgColor);
    learnSpeed.setFillColor(bgFillColor);
    learnSpeed.setBorderColor(bgFillColor);
    thresholdValue.setBackgroundColor(bgColor);
    thresholdValue.setFillColor(bgFillColor);
    learnSpeed.setBorderColor(bgFillColor);

	blurGui.setup();
	blurGui.setName("De-Noise (Gaussian)");
	blurGui.setPosition((camWidth/4)+120, (camHeight/2)+100+(camHeight/4));
	blurGui.setSize(camWidth/4,20);
	blurGui.setBorderColor(bgFillColor);
	blurGui.setHeaderBackgroundColor(bgColor);
	blurGui.add(blurEnable.setup("Smooth",1,120,20));
	blurGui.add(blur.setup("Amount", 15, 0, 19,camWidth/4,20.0));

    blur.setBackgroundColor(bgColor);
    blur.setFillColor(bgFillColor);
    blur.setBorderColor(bgFillColor);

	hpGui.setup();
	hpGui.setName("Highpass");
	hpGui.setPosition(((camWidth/4)*2)+140, (camHeight/2)+100+(camHeight/4));
	hpGui.setSize(camWidth/4,20);
	hpGui.setBorderColor(bgFillColor);
	hpGui.setHeaderBackgroundColor(bgColor);
	hpGui.add(hpEnable.setup("Highpass",1,120,20));
	hpGui.add(hpblur.setup("Blur", 1,0, 300,camWidth/4,20.0));
	hpGui.add(hpBright.setup("Brightness", 1,0, 1.0,camWidth/4,20.0));
	hpGui.add(hpCont.setup("Contrast", 1,0, 1.0,camWidth/4,20.0));

    hpblur.setBackgroundColor(bgColor);
    hpblur.setFillColor(bgFillColor);
    hpblur.setBorderColor(bgFillColor);

    hpBright.setBackgroundColor(bgColor);
    hpBright.setFillColor(bgFillColor);
    hpBright.setBorderColor(bgFillColor);
    hpCont.setBackgroundColor(bgColor);
    hpCont.setFillColor(bgFillColor);
    hpCont.setBorderColor(bgFillColor);

	blobGui.setup();
	blobGui.setName("Tracking");
	blobGui.setPosition(5, (camHeight/2)+5);
	blobGui.setSize(ofGetWidth()-10,20);
	blobGui.setBorderColor(bgFillColor);
	blobGui.setHeaderBackgroundColor(bgColor);

	blobGui.add(blobApply.setup("Apply Changes",camWidth/4,20));
	blobGui.add(blobThreshold.setup("Threshold", 15, 0, 255,camWidth/4,20.0));
	blobGui.add(minAreaRadius.setup("Min Area Radius", 1, 1, camWidth/2,camWidth/4,20.0));
	blobGui.add(maxAreaRadius.setup("Max Area Radius", 50, 1, camWidth,camWidth/4,20.0));
	blobGui.add(maxDistance.setup("Max Distance", 32, 1, 300,camWidth/4,20.0));


    minAreaRadius.setPosition(5,(camHeight/2)+50);
    maxAreaRadius.setPosition(5,(camHeight/2)+70);
    blobThreshold.setPosition(camWidth/4+20,(camHeight/2)+30);
    maxDistance.setPosition(camWidth/4+20,(camHeight/2)+50);
    blobThreshold.setBackgroundColor(bgColor);
    blobThreshold.setFillColor(bgFillColor);
    blobThreshold.setBorderColor(bgFillColor);
    minAreaRadius.setBackgroundColor(bgColor);
    minAreaRadius.setFillColor(bgFillColor);
    minAreaRadius.setBorderColor(bgFillColor);
    maxAreaRadius.setBackgroundColor(bgColor);
    maxAreaRadius.setFillColor(bgFillColor);
    maxAreaRadius.setBorderColor(bgFillColor);
    maxDistance.setBackgroundColor(bgColor);
    maxDistance.setFillColor(bgFillColor);
    maxDistance.setBorderColor(bgFillColor);

	blobTrackOptions.setup();
	blobTrackOptions.setName("Blob Tracking Options");
	blobTrackOptions.setPosition(ofGetWidth()-320, (camHeight/2)+90);
	blobTrackOptions.setSize(300,20);
	blobTrackOptions.setBorderColor(bgFillColor);
	blobTrackOptions.setHeaderBackgroundColor(bgColor);

	blobTrackOptions.add(velocity.setup("Velocity",1,300,20));
	blobTrackOptions.add(smRect.setup("Smallest Rectangle",1,300,20));
	blobTrackOptions.add(bfEllipse.setup("Best Fit Ellipse",1,300,20));
	blobTrackOptions.add(mnCircle.setup("Minimum Circle",1,300,20));
	blobTrackOptions.add(cnvxHull.setup("Convex Hull",1,300,20));

	minBlobTrackingMenu.setup();
	minBlobTrackingMenu.setName("Basic Tracking Options");
	minBlobTrackingMenu.setPosition((posX+scaledWidth)-300, (posY+scaledHeight)-125);
	minBlobTrackingMenu.setSize(300,20);
	minBlobTrackingMenu.setBorderColor(bgFillColor);
	minBlobTrackingMenu.setHeaderBackgroundColor(bgColor);

    string cameraIdstr = ofToString(cameraId,0);
    minBlobTrackingMenu.add(minBtCamNumber.setup("Tracking Camera ",cameraIdstr,300));
	minBlobTrackingMenu.add(minTextureSwitch.setup("Switch Texture",0,300,20));
	minBlobTrackingMenu.add(minBGSample.setup("Resample Background",300,20));

    minBtCamNumber.setBackgroundColor(bgColor);
    minBtCamNumber.setFillColor(bgFillColor);
    minBtCamNumber.setBorderColor(bgFillColor);
    minTextureSwitch.setBackgroundColor(bgColor);
    minTextureSwitch.setFillColor(bgFillColor);
    minTextureSwitch.setBorderColor(bgFillColor);
    minBGSample.setBackgroundColor(bgColor);
    minBGSample.setFillColor(bgFillColor);
    minBGSample.setBorderColor(bgFillColor);

	if(ofFile::doesFileExist("settings.xml")){
		blobTrackGui.loadFromFile("settings.xml");
		bgGui.loadFromFile("settings.xml");
		blurGui.loadFromFile("settings.xml");
 		hpGui.loadFromFile("settings.xml");
 		blobGui.loadFromFile("settings.xml");
 		blobTrackOptions.loadFromFile("settings.xml");
 		minBlobTrackingMenu.loadFromFile("settings.xml");
   }
   	background.setLearningTime(learnSpeed);
   	background.setThresholdValue(thresholdValue);

   	bgImg.setUseTexture(bUseTexture);
   	blurImg.setUseTexture(bUseTexture);
   	highpass.setUseTexture(bUseTexture);


	contourFinder.setMinAreaRadius(minAreaRadius);
	contourFinder.setMaxAreaRadius(maxAreaRadius);
	contourFinder.setThreshold(blobThreshold);
	// wait for half a frame before forgetting something
	contourFinder.getTracker().setPersistence(15);
	// an object can move up to 32 pixels per frame
	contourFinder.getTracker().setMaximumDistance(maxDistance);

}

void ofxNTSBlobTrack::update(ofImage& cam){
    if((ofImageType)cam.type != 0){
        cam.setImageType(OF_IMAGE_GRAYSCALE);
    }
    copy(cam, source);
    source.update();
    copy(source,trackedImg);
    if(bgEnable){
        background.setLearningTime(learnSpeed);
        background.setThresholdValue(thresholdValue);
        background.update(source, bgImg);
        copy(bgImg.getPixelsRef(),trackedImg);
    }

    if(blurEnable){
        blurImg.setFromPixels(trackedImg.getPixelsRef());
        blurImg.erode();
        blurImg.dilate();
        blurImg.blurGaussian(blur);
        copy(blurImg.getPixelsRef(),trackedImg);
    }

    if(hpEnable){
        if(blurEnable && bgEnable){
           tmp.setFromPixels(bgImg.getPixelsRef());
           tmpBlur.setFromPixels(blurImg.getPixelsRef());
        }else if(!blurEnable && bgEnable){
            tmp.setFromPixels(bgImg.getPixelsRef());
            tmpBlur.setFromPixels(bgImg.getPixelsRef());
        }else if(blurEnable && !bgEnable){
            tmp.setFromPixels(source.getPixelsRef());
            tmpBlur.setFromPixels(blurImg.getPixelsRef());
        }else if(!blurEnable && !bgEnable){
            tmp.setFromPixels(source.getPixelsRef());
            tmpBlur.setFromPixels(source.getPixelsRef());
        }
        tmpBlur.blur(hpblur);
        tmp.brightnessContrast(hpBright,hpCont);
        tmp.contrastStretch();

        cvSub(tmpBlur.getCvImage(),tmp.getCvImage(),highpass.getCvImage());
        //highpass.blur(hpblur);
        highpass.contrastStretch();
        highpass.flagImageChanged();
        copy(highpass.getPixelsRef(),trackedImg);
    }
    if(bgEnable){
        bgImg.update();
    }
    trackedImg.update();

    contourFinder.setThreshold(blobThreshold);
    contourFinder.findContours(trackedImg);
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::draw(){
    ofSetColor(ofColor::white);

	RectTracker& tracker = contourFinder.getTracker();
	if(bUseTexture){

        source.draw(0, 0, camWidth*0.5, camHeight*0.5);
        //Background Removal
        if(bgEnable){
            bgImg.draw(100, camHeight*0.5+100, camWidth/4, camHeight/4);
        }
        //Smooth
        if(blurEnable){
            blurImg.draw((camWidth/4)+120,camHeight*0.5+100, camWidth/4, camHeight/4);
        }
        //HP Filter
        if(hpEnable){
            highpass.draw(((camWidth/4)*2)+140,camHeight*0.5+100, camWidth/4, camHeight/4);
        }
        trackedImg.draw(camWidth*0.5, 0, camWidth*0.5, camHeight*0.5);
	}

    ofSetColor(255);
    ofPushMatrix();
    ofScale(0.5, 0.5 );
    ofTranslate(camWidth, 0);
    ofSetLineWidth(2);
    contourFinder.draw();
    int n = contourFinder.size();
    for(int i = 0; i <n; i++) {
        //Labels and Center Markers with Age
        ofSetColor(255);
        ofPoint centerP = toOf(contourFinder.getCenter(i));
        ofPushMatrix();
        ofTranslate(centerP.x, centerP.y);
        int label = contourFinder.getLabel(i);
        string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
        //cout << msg << ":"<< centerP.x<<","<<centerP.y<<endl;
        ofDrawBitmapString(msg, 20, 200);
        //Velocity
        if(velocity){
            ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            ofScale(5, 5);
            ofLine(0, 0, velocity.x, velocity.y);
            ofPopMatrix();
        }

        ofSetLineWidth(2);
        ofNoFill();

        if(smRect){
            // smallest rectangle that fits the contour
            ofSetColor(cyanPrint);
            ofPolyline minAreRect = toOf(contourFinder.getMinAreaRect(i));
            minAreRect.resize(1);
            minAreRect.draw();
        }

        // ellipse that best fits the contour
        if(bfEllipse){
            ofSetColor(magentaPrint);
            cv::RotatedRect ellipse = contourFinder.getFitEllipse(i);
            ofPushMatrix();
            ofVec2f ellipseCenter = toOf(ellipse.center);
            ofVec2f ellipseSize = toOf(ellipse.size);
            ofTranslate(ellipseCenter.x, ellipseCenter.y);
            ofRotate(ellipse.angle);
            ofEllipse(0, 0, ellipseSize.x, ellipseSize.y);
            ofPopMatrix();
            // minimum area circle that encloses the contour
            ofSetColor(cyanPrint);
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            ofCircle(circleCenter, circleRadius);
        }

        // convex hull of the contour
        if(cnvxHull){
            ofSetColor(yellowPrint);
            ofPolyline convexHull = toOf(contourFinder.getConvexHull(i));
            convexHull.resize(1);
            convexHull.draw();

            // defects of the convex hull
            vector<cv::Vec4i> defects = contourFinder.getConvexityDefects(i);
            for(int j = 0; j < defects.size(); j++) {
                ofLine(defects[j][0], defects[j][1], defects[j][2], defects[j][3]);
            }
        }
        // some different styles of contour centers
        ofVec2f centroid = toOf(contourFinder.getCentroid(i));
        ofVec2f average = toOf(contourFinder.getAverage(i));
        ofVec2f center = toOf(contourFinder.getCenter(i));
        ofSetColor(cyanPrint);
        ofCircle(centroid, 1);
        ofSetColor(magentaPrint);
        ofCircle(average, 1);
        ofSetColor(yellowPrint);
        ofCircle(center, 1);

        // you can also get the area and perimeter using ofPolyline:
        // ofPolyline::getArea() and ofPolyline::getPerimeter()
        double area = contourFinder.getContourArea(i);
        double length = contourFinder.getArcLength(i);

        // balance is useful for detecting when a shape has an "arm" sticking out
        // if balance.length() is small, the shape is more symmetric: like I, O, X...
        // if balance.length() is large, the shape is less symmetric: like L, P, F...
        ofVec2f balance = toOf(contourFinder.getBalance(i));
        ofPushMatrix();
        ofTranslate(centroid.x, centroid.y);
        ofScale(5, 5);
        ofLine(0, 0, balance.x, balance.y);
        ofPopMatrix();
   }
    ofPopMatrix();

    // this chunk of code visualizes the creation and destruction of labels
	const vector<unsigned int>& currentLabels = tracker.getCurrentLabels();
	const vector<unsigned int>& previousLabels = tracker.getPreviousLabels();
	const vector<unsigned int>& newLabels = tracker.getNewLabels();
	const vector<unsigned int>& deadLabels = tracker.getDeadLabels();
	ofSetColor(cyanPrint);
	for(int i = 0; i < currentLabels.size(); i++) {
		int j = currentLabels[i];
		ofLine(j, 0, j, 4);
	}
	ofSetColor(magentaPrint);
	for(int i = 0; i < previousLabels.size(); i++) {
		int j = previousLabels[i];
		ofLine(j, 4, j, 8);
	}
	ofSetColor(yellowPrint);
	for(int i = 0; i < newLabels.size(); i++) {
		int j = newLabels[i];
		ofLine(j, 8, j, 12);
	}
	ofSetColor(ofColor::white);
	for(int i = 0; i < deadLabels.size(); i++) {
		int j = deadLabels[i];
		ofLine(j, 12, j, 16);
	}

    if( !gHide ){
        fpsLabel = ofToString(ofGetFrameRate(),2);
        blobTrackGui.draw();
        bgGui.draw();
        blurGui.draw();
        hpGui.draw();
        blobGui.draw();
        blobTrackOptions.draw();
    }
}

void ofxNTSBlobTrack::drawTracked(){
    ofSetColor(ofColor::white);

	RectTracker& tracker = contourFinder.getTracker();
	if(bUseTexture){
        if(minTextureSwitch){
            source.draw(posX,posY,scaledWidth,scaledHeight);
        }else{
            trackedImg.draw(posX,posY, scaledWidth, scaledHeight);
        }
	}

    ofSetColor(255);
    ofPushMatrix();
    ofScale(scaleCams, scaleCams );
    ofTranslate(posX, posY);
    ofSetLineWidth(2);
    contourFinder.draw();
    int n = contourFinder.size();
    for(int i = 0; i <n; i++) {
        //Labels and Center Markers with Age
        ofSetColor(255);
        ofPoint centerP = toOf(contourFinder.getCenter(i));
        ofPushMatrix();
        ofTranslate(centerP.x, centerP.y);
        int label = contourFinder.getLabel(i);
        string msg = ofToString(label) + ":" + ofToString(tracker.getAge(label));
        //cout << msg << ":"<< centerP.x<<","<<centerP.y<<endl;
        ofDrawBitmapString(msg, 20, 200);
        //Velocity
        if(velocity){
            ofVec2f velocity = toOf(contourFinder.getVelocity(i));
            ofScale(5, 5);
            ofLine(0, 0, velocity.x, velocity.y);
            ofPopMatrix();
        }

        ofSetLineWidth(2);
        ofNoFill();

        if(smRect){
            // smallest rectangle that fits the contour
            ofSetColor(cyanPrint);
            ofPolyline minAreRect = toOf(contourFinder.getMinAreaRect(i));
            minAreRect.resize(1);
            minAreRect.draw();
        }

        // ellipse that best fits the contour
        if(bfEllipse){
            ofSetColor(magentaPrint);
            cv::RotatedRect ellipse = contourFinder.getFitEllipse(i);
            ofPushMatrix();
            ofVec2f ellipseCenter = toOf(ellipse.center);
            ofVec2f ellipseSize = toOf(ellipse.size);
            ofTranslate(ellipseCenter.x, ellipseCenter.y);
            ofRotate(ellipse.angle);
            ofEllipse(0, 0, ellipseSize.x, ellipseSize.y);
            ofPopMatrix();
            // minimum area circle that encloses the contour
            ofSetColor(cyanPrint);
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            ofCircle(circleCenter, circleRadius);
        }

        // convex hull of the contour
        if(cnvxHull){
            ofSetColor(yellowPrint);
            ofPolyline convexHull = toOf(contourFinder.getConvexHull(i));
            convexHull.resize(1);
            convexHull.draw();

            // defects of the convex hull
            vector<cv::Vec4i> defects = contourFinder.getConvexityDefects(i);
            for(int j = 0; j < defects.size(); j++) {
                ofLine(defects[j][0], defects[j][1], defects[j][2], defects[j][3]);
            }
        }
        // some different styles of contour centers
        ofVec2f centroid = toOf(contourFinder.getCentroid(i));
        ofVec2f average = toOf(contourFinder.getAverage(i));
        ofVec2f center = toOf(contourFinder.getCenter(i));
        ofSetColor(cyanPrint);
        ofCircle(centroid, 1);
        ofSetColor(magentaPrint);
        ofCircle(average, 1);
        ofSetColor(yellowPrint);
        ofCircle(center, 1);

        // you can also get the area and perimeter using ofPolyline:
        // ofPolyline::getArea() and ofPolyline::getPerimeter()
        double area = contourFinder.getContourArea(i);
        double length = contourFinder.getArcLength(i);

        // balance is useful for detecting when a shape has an "arm" sticking out
        // if balance.length() is small, the shape is more symmetric: like I, O, X...
        // if balance.length() is large, the shape is less symmetric: like L, P, F...
        ofVec2f balance = toOf(contourFinder.getBalance(i));
        ofPushMatrix();
        ofTranslate(centroid.x, centroid.y);
        ofScale(5, 5);
        ofLine(0, 0, balance.x, balance.y);
        ofPopMatrix();
   }
    ofPopMatrix();

    minBlobTrackingMenu.draw();
}
//--------------------------------------------------------------
void ofxNTSBlobTrack::exit(){
	bUseTexture.removeListener(this, &ofxNTSBlobTrack::bUseTextureEnablePressed);
	bgEnable.removeListener(this, &ofxNTSBlobTrack::bgEnablePressed);
	blurEnable.removeListener(this, &ofxNTSBlobTrack::blurEnablePressed);
	hpEnable.removeListener(this,&ofxNTSBlobTrack::hpEnablePressed);
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::bUseTextureEnablePressed(const void * sender, bool & pressed){
    bgImg.setUseTexture(bUseTexture);
    blurImg.setUseTexture(bUseTexture);
    highpass.setUseTexture(bUseTexture);
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::bgEnablePressed(const void * sender, bool & pressed){
   // bgImg.clear();
   if(bgEnable){
       bgImg.allocate(camWidth,camHeight,OF_IMAGE_GRAYSCALE);
   }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::blurEnablePressed(const void * sender, bool & pressed){
    if(blurEnable){
        blurImg.allocate(camWidth,camHeight);
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::hpEnablePressed(const void * sender, bool & pressed){
    if(hpEnable){
        highpass.allocate(camWidth,camHeight);
        tmp.allocate(camWidth,camHeight);
        tmpBlur.allocate(camWidth,camHeight);
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::velocityEnablePressed(const void * sender, bool & pressed){
    if(velocity){
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::smRectEnablePressed(const void * sender, bool & pressed){
    if(smRect){
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::bfEllipseEnablePressed(const void * sender, bool & pressed){
    if(bfEllipse){
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::mnCircleEnablePressed(const void * sender, bool & pressed){
    if(mnCircle){
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::cnvxEnablePressed(const void * sender, bool & pressed){
    if(cnvxHull){
    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::blobApplyPressed(){
	contourFinder.setMinAreaRadius(minAreaRadius);
	contourFinder.setMaxAreaRadius(maxAreaRadius);
	contourFinder.setThreshold(blobThreshold);
	contourFinder.getTracker().setMaximumDistance(maxDistance);
}


//--------------------------------------------------------------
void ofxNTSBlobTrack::backgroundSamplePressed(){
    background.reset();
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::minTextureSwitchPressed(const void * sender, bool & pressed){
    if(minTextureSwitch){

    }
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::minBGSamplePressed(){
    background.reset();
}


//--------------------------------------------------------------
void ofxNTSBlobTrack::loadSettingsPressed(){
		blobTrackGui.loadFromFile("settings.xml");
		bgGui.loadFromFile("settings.xml");
		blurGui.loadFromFile("settings.xml");
		hpGui.loadFromFile("settings.xml");
		blobGui.loadFromFile("settings.xml");
		blobTrackOptions.loadFromFile("settings.xml");
		minBlobTrackingMenu.loadFromFile("settings.xml");
}

//--------------------------------------------------------------
void ofxNTSBlobTrack::saveSettingsPressed(){
		blobTrackGui.saveToFile("settings.xml");
		bgGui.saveToFile("settings.xml");
		blurGui.saveToFile("settings.xml");
		hpGui.saveToFile("settings.xml");
        blobGui.saveToFile("settings.xml");
        blobTrackOptions.saveToFile("settings.xml");
        minBlobTrackingMenu.saveToFile("settings.xml");
}
