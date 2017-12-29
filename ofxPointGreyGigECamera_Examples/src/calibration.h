#include "ofApp.h"
#include "CalibrationExtended.h"

using namespace ofxCvExtend;
using namespace cv;

class GigECalibration{

    public:
        GigECalibration(){
            active = true;
            camera = 0;
        }

        virtual ~GigECalibration(){

        }

        void setup(ofImage cam) {
            camImage = cam;
            calibration.setCalibFlags(CV_CALIB_FIX_PRINCIPAL_POINT | CV_CALIB_ZERO_TANGENT_DIST | CV_CALIB_FIX_FOCAL_LENGTH | CV_CALIB_FIX_ASPECT_RATIO | CV_CALIB_FIX_K3 | CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5 | CV_CALIB_FIX_K6
);
            calibration.setFillFrame(false);
            FileStorage settings(ofToDataPath("settings_assymetricalCircles.yml"), FileStorage::READ);
            if(settings.isOpened()) {
                int xCount = settings["xCount"], yCount = settings["yCount"];
                calibration.setPatternSize(xCount, yCount);
                float squareSize = settings["squareSize"];
                calibration.setSquareSize(squareSize);
                CalibrationPattern patternType;
                switch(settings["patternType"]) {
                    case 0: patternType = CHESSBOARD; break;
                    case 1: patternType = CIRCLES_GRID; break;
                    case 2: patternType = ASYMMETRIC_CIRCLES_GRID; break;
                }
                calibration.setPatternType(patternType);
                sprintf(
                        calibrationFile,
                        "calibration-cam_%u.yml",
                        camera
                    );

                if(ofFile::doesFileExist(calibrationFile)){
                    sprintf(
                        backupCalibrationFile,
                        "calibration-cam_%u-%d-%d-%d_%d-%d-%d.yml",
                        camera,ofGetMonth(), ofGetDay(),ofGetYear(), ofGetHours(), ofGetMinutes(), ofGetSeconds()
                    );
                   ofFile::copyFromTo(calibrationFile,backupCalibrationFile);
                }

            }

            ofxCv::imitate(undistorted, camImage);
            ofxCv::imitate(previous, camImage);
            ofxCv::imitate(diff, camImage);

            lastTime = 0;

            //active = true;
        }

            void update(ofImage cam) {
                camImage = cam;
                Mat camMat = ofxCv::toCv(camImage);
                Mat prevMat = ofxCv::toCv(previous);
                Mat diffMat = ofxCv::toCv(diff);

                absdiff(prevMat, camMat, diffMat);
                camMat.copyTo(prevMat);

                diffMean = mean(Mat(mean(diffMat)))[0];

                float curTime = ofGetElapsedTimef();
                if(active && curTime - lastTime > timeThreshold && diffMean < diffThreshold) {
                    if(calibration.add(camMat)) {
                        cout << "re-calibrating" << endl;
                        calibration.calibrate();
                        if(calibration.size() > startCleaning) {
                            calibration.clean();
                        }
                        calibration.save(calibrationFile);
                        lastTime = curTime;
                    }
                }

                if(calibration.size() > 0) {
                    calibration.undistort(ofxCv::toCv(camImage), ofxCv::toCv(undistorted));
                    camImage.update();
                    undistorted.update();
                }

        }

        void draw(int camW, int camH) {
            ofSetColor(255);
            camImage.setUseTexture(true);
            camImage.update();
            camImage.draw(100, 100,camW/2, camH/2);
            undistorted.draw(camW/2+100, 100, camW/2,camH/2);

            if(active){
                ofDrawBitmapString("Calibration Active",ofGetWidth()-200,20);
            }else{
                ofDrawBitmapString("Calibration Inactive",ofGetWidth()-200,20);
            }
            ofDrawBitmapString(calibrationFile,ofGetWidth()-250,35);

            stringstream intrinsics;
            intrinsics << "fov: " << ofxCv::toOf(calibration.getDistortedIntrinsics().getFov()) << " distCoeffs: " << calibration.getDistCoeffs();
            ofxCv::drawHighlightString(intrinsics.str(), 10, 20, ofxCv::yellowPrint, ofColor(0));
            ofxCv::drawHighlightString("movement: " + ofToString(diffMean), 10, 40, ofxCv::cyanPrint);
            ofxCv::drawHighlightString("reproj error: " + ofToString(calibration.getReprojectionError()) + " from " + ofToString(calibration.size()), 10, 60, ofxCv::magentaPrint);
            for(int i = 0; i < calibration.size(); i++) {
                ofxCv::drawHighlightString(ofToString(i) + ": " + ofToString(calibration.getReprojectionError(i)), 10, 80 + 16 * i, ofxCv::magentaPrint);
            }
        }

        bool reset(){
            calibration.reset();
            ofLog(OF_LOG_NOTICE,"Calibration Reset");
            return true;
        }
        bool active;
        uint64_t camera;
        char calibrationFile[30];
        char backupCalibrationFile[60];
   protected:
        const float diffThreshold = 2.5; // maximum amount of movement
        const float timeThreshold = 1; // minimum time between snapshots
        const int startCleaning = 10; // start cleaning outliers after this many samples

        ofImage camImage;
        ofImage undistorted;
        ofPixels previous;
        ofPixels diff;
        float diffMean;

        float lastTime;
        ofxCvExtend::Calibration calibration;
};
