#pragma once

#include "ofMain.h"
#include "ofThread.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "threadPointGreyGigECamera.h"
#include "ofxPointGreyGigECamera.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp
{
	public:

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		bool initDelay();
		bool checkAvailableCameras();

        ofxPanel        mFPS;
        ofxPanel        mActiveCamera;

        ofxToggle       mLensCalibrate;
        ofxToggle       mTrackingCalibrate;

        ofxButton       minBGSample;


    protected:
        threadPointGreyGigECamera camThread[4];
        FlyCapture2::ofxPointGreyGigECamera listCam;

        unsigned int loader;
        bool        calibration;
        bool         initialized;
        bool        calActive;
        uint64_t    setFrameRate;
        uint64_t    totalNumCameras;
        uint64_t    selectedCamera;
        uint64_t    packetDelay;
        uint64_t    packetSize;
        bool        camerasAvailable;
        uint64_t    camZoom;


    private:

};
