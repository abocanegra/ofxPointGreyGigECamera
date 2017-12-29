#pragma once
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"

using namespace ofxCv;
using namespace cv;

class ofxNTSBlobTrack{

    public:
        ofxNTSBlobTrack();
        virtual ~ofxNTSBlobTrack();
        void setup();
        void update(ofImage& cam);
        void draw();
        void drawTracked();
        void exit();
        void bUseTextureEnablePressed(const void * sender, bool & pressed);
		void bgEnablePressed(const void * sender, bool & pressed);
		void blurEnablePressed(const void * sender, bool & pressed);
		void hpEnablePressed(const void * sender, bool & pressed);
		void velocityEnablePressed(const void * sender, bool & pressed);
		void smRectEnablePressed(const void * sender, bool & pressed);
		void bfEllipseEnablePressed(const void * sender, bool & pressed);
		void mnCircleEnablePressed(const void * sender, bool & pressed);
		void cnvxEnablePressed(const void * sender, bool & pressed);
		void blobApplyPressed();
		void loadSettingsPressed();
		void saveSettingsPressed();
		void backgroundSamplePressed();
		void minBGSamplePressed();
		void minTextureSwitchPressed(const void * sender, bool & pressed);

        int   posX;
        int   posY;
        float scaleCams;
        uint64_t camWidth;
        uint64_t camHeight;
        float scaledWidth;
        float scaledHeight;
        uint64_t cameraId;

        ofImage             source;
        ofImage             bgImg;
        ofxCvGrayscaleImage tmp;
        ofxCvGrayscaleImage tmpBlur;

        ofxCvGrayscaleImage blurImg;
        ofxCvGrayscaleImage hpTemp;
        ofxCvGrayscaleImage highpass;
        ofImage             trackedImg;

        ofxCv::RunningBackground background;

        protected:
		bool gHide;

        ofxIntSlider    thresholdValue;
        ofxIntSlider    learnSpeed;
        ofxIntSlider    blur;
        ofxIntSlider    hpblur;
        ofxIntSlider    blobThreshold;
        ofxIntSlider    minAreaRadius;
        ofxIntSlider    maxAreaRadius;
        ofxIntSlider    maxDistance;

        ofxFloatSlider  hpBright;
        ofxFloatSlider  hpCont;

        ofxLabel        screenSize;
        ofxLabel        fpsLabel;
        ofxLabel        btCamNumber;
        ofxLabel        minBtCamNumber;

        ofxToggle       bUseTexture;
        ofxToggle       bgEnable;
        ofxToggle       blurEnable;
        ofxToggle       hpEnable;
        ofxToggle       velocity;
        ofxToggle       smRect;
        ofxToggle       bfEllipse;
        ofxToggle       mnCircle;
        ofxToggle       cnvxHull;
        ofxToggle       minTextureSwitch;

        ofxButton       minBGSample;
        ofxButton       loadSettings;
        ofxButton       saveSettings;
        ofxButton       blobApply;
        ofxButton       backgroundSample;

        ofxPanel                blobTrackGui;
        ofxPanel                bgGui;
        ofxPanel                blurGui;
        ofxPanel                hpGui;
        ofxPanel                blobGui;
        ofxPanel                blobTrackOptions;
        ofxPanel                minBlobTrackingMenu;



        float threshold;
        ofxCv::TrackingColorMode trackingColorMode;
        ofColor targetColor;
        ofxCv::ContourFinder contourFinder;
};
