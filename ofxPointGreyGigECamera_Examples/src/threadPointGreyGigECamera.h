#pragma once

#include "ofMain.h"
#include "ofThread.h"
#include "ofxCv.h"
#include "ofxOpenCv.h"
#include "ofxPointGreyGigECamera.h"
#include "calibration.h"
#include "CalibrationExtended.h"
#include "ofxNTSBlobTrack.h"

using namespace ofxCv;
using namespace cv;

class threadPointGreyGigECamera : public ofThread {

    public:
        threadPointGreyGigECamera() {
            TdeviceID        = 0;
            TsetToColor      = false;
            TbUseTexture     = true;
            T_pixelFmt       = "PIXEL_FORMAT_MONO8";
            TabsFrameRate    = 30;
            TcamZoom         = 4;
            camSetup         = false;
            TtotalNumCameras = 1;
            Tcalibration     = 0;
            TcalActive       = false;
            TsetCamWidth     = 1920;
            TsetCamHeight    = 1200;
            TxOffset         = 0;
            TyOffset         = 0;
            TpacketSize      = 9000;
            TpacketDelay     = 0;
            TresetCalibration = false;
            TcalibrateTracker = false;
            TblobTrack        = true;
            camIsCalibrated  = false;
        }

         /// Start the thread.
        void start()
        {
            if(!camSetup){
                setupCamera();
            }
            startThread();
        }

        bool setupCamera(){
            if(lock()){
                cam.deviceID =      TdeviceID;
                cam.setToColor =    TsetToColor;
                cam.bUseTexture =   TbUseTexture;
                cam._pixelFmt =     T_pixelFmt;
                cam.absFrameRate =  TabsFrameRate;
                cam.camWidth =      TsetCamWidth;
                cam.camHeight =     TsetCamHeight;
                cam.xOffset =       TxOffset;
                cam.yOffset =       TyOffset;
                cam.packetSize=     TpacketSize;
                cam.packetDelay=    TpacketDelay;
                cam.setup();
                TcamWidth =         cam.camWidth;
                TcamHeight =        cam.camHeight;
                TcamWidthRatio =    ofGetWidth()/TcamWidth;
                TcamHeightRatio =   ofGetHeight()/TcamHeight;
                camIsCalibrated =   false;


                cam.isCamConnected = cam.checkCameraConnected();
                if(TcamWidthRatio < 1 || TcamHeightRatio < 1){
                    if(TcamWidthRatio < TcamHeightRatio){
                        TfullScaledWidth = (TcamWidth * TcamWidthRatio);
                        TfullScaledHeight = (TcamHeight * TcamWidthRatio);
                        TselectedRatio = TcamWidthRatio;
                    }else{
                        TfullScaledWidth = (TcamWidth * TcamHeightRatio);
                        TfullScaledHeight = (TcamHeight * TcamHeightRatio);
                        TselectedRatio = TcamHeightRatio;
                    }
                }else{
                    TfullScaledWidth = TcamWidth;
                    TfullScaledHeight = TcamHeight;
                    TselectedRatio = 1;
                }

                /*Set up matrix for Feed Locations at different zoom levels*/
                for(uint64_t i=1; i<=16; i++){
                    if(i==1){
                        TzL = 1;
                    }else if(i >1 && i <=4){
                        TzL = 2;
                    }else if(i>4 && i <=9){
                        TzL = 3;
                    }else if(i>9 && i<=16){
                        TzL = 4;
                    }
                    //By Camera
                    for(int j=0; j<i; j++){
                        int cols = ofGetWidth()/(TfullScaledWidth/TzL);
                        int rows = ofGetHeight()/(TfullScaledHeight/TzL);
                        int currCol = ((j)*(TfullScaledWidth/TzL))/TfullScaledWidth;
                        int currRow = ((j)*(TfullScaledHeight/TzL))/TfullScaledHeight;
                        camCoord[i][j][0]=TfullScaledWidth/TzL;//width
                        camCoord[i][j][1]=TfullScaledHeight/TzL;//height
                        camCoord[i][j][2]=(j%cols)*camCoord[i][j][0];//x
                        camCoord[i][j][3]=currRow*camCoord[i][j][1];//y
                   }
                }
                camIsColor  = cam.isSetToColor();
                if(camIsColor){
                    camImage.allocate(TcamWidth, TcamHeight,OF_IMAGE_COLOR);
                    undistortedImage.allocate(TcamWidth, TcamHeight, OF_IMAGE_COLOR);
                }else{
                    undistortedImage.allocate(TcamWidth, TcamHeight, OF_IMAGE_GRAYSCALE);
                    camImage.allocate(TcamWidth, TcamHeight,OF_IMAGE_GRAYSCALE);
                }

                camSetup = true;

                if(Tcalibration){
                    TcalActive = true;
                    camCal.camera = cam.SerialNumber;
                    camCal.active = TcalActive;
                    camCal.setup(camImage);
                }
            }

           camCal.camera = cam.SerialNumber;

            sprintf(
                calibrationFile,
                "calibration-cam_%u.yml",
                cam.SerialNumber
            );

            camIsCalibrated = ofFile::doesFileExist(calibrationFile);
            calibSetup = false;
            camCalibration.setFillFrame(false);

            undistortedImage.setUseTexture(TbUseTexture);
            camImage.setUseTexture(TbUseTexture);

            //Setup BlobTracker
            blobTracker.cameraId = TdeviceID;
            blobTracker.scaleCams = TselectedRatio;
            blobTracker.camWidth = TcamWidth;
            blobTracker.camHeight = TcamHeight;
            blobTracker.posX = camCoord[TtotalNumCameras][TdeviceID][2];
            blobTracker.posY = camCoord[TtotalNumCameras][TdeviceID][3];
            blobTracker.scaledWidth = camCoord[TtotalNumCameras][TdeviceID][0];
            blobTracker.scaledHeight =camCoord[TtotalNumCameras][TdeviceID][1];
            if(!blobTracker.source.isAllocated()){
                blobTracker.setup();
            }

            unlock();
            return camSetup;
        }
       /// Signal the thread to stop.  After calling this method,
        /// isThreadRunning() will return false and the while loop will stop
        /// next time it has the chance to.
        void stop()
        {
            camSetup = false;
            stopThread();
        }

        /// Our implementation of threadedFunction.
        void threadedFunction()
        {
            while(isThreadRunning())
            {
                // Attempt to lock the mutex.  If blocking is turned on,
                if(lock())
                {
                   cam.isCamConnected = cam.checkCameraConnected();
                   if(cam.isCamConnected){
                        cam.update();
                        camCal.active = TcalActive;
                        if(TresetCalibration){
                            camCal.reset();
                            TresetCalibration = false;
                        }

                        copy(cam.drawImage().getPixelsRef(),camImage);
                        camImage.update();
                        if(calibSetup && camIsCalibrated){
                            camCalibration.undistort(ofxCv::toCv(camImage), ofxCv::toCv(undistortedImage));
                        }
                        unlock();
                }
                 // called if lock() returned true above.
              }
                else
                {
                    // If we reach this else statement, it means that we could not
                    // lock our mutex, and so we do not need to call unlock().
                    // Calling unlock without locking will lead to problems.
                    //ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
                }
            }
       }

        void draw()
        {
            if(lock())
            {
               if(cam.isCamConnected){
                    camImage.update();
                    unlock();
                    if(Tcalibration){
                        camCal.update(camImage);
                        unlock();
                        camCal.draw(TsetCamWidth, TsetCamHeight);
                    }else{
                       if(calibSetup && camIsCalibrated){
                          undistortedImage.update();
                       }
                      if(TbUseTexture){
                           if(calibSetup && camIsCalibrated){
                                if(TblobTrack){
                                    unlock();
                                     if(TblobTrack || TcalibrateTracker){
                                            blobTracker.update(undistortedImage);
                                       }

                                    blobTracker.drawTracked();
                                }else if(TcalibrateTracker){
                                    unlock();
                                     if(TblobTrack || TcalibrateTracker){
                                            blobTracker.update(undistortedImage);
                                       }
                                    blobTracker.draw();
                                }else{
                                    unlock();
                                    undistortedImage.draw(camCoord[TtotalNumCameras][TdeviceID][2], camCoord[TtotalNumCameras][TdeviceID][3], camCoord[TtotalNumCameras][TdeviceID][0],  camCoord[TtotalNumCameras][TdeviceID][1]);
                                }
                            }else{
                               if(TblobTrack || TcalibrateTracker){
                                    blobTracker.update(camImage);
                               }
                                if(TblobTrack){
                                    unlock();
                                    blobTracker.drawTracked();
                                }else if(TcalibrateTracker){
                                    unlock();
                                    blobTracker.draw();
                                }else{
                                    unlock();
                                    camImage.draw(camCoord[TtotalNumCameras][TdeviceID][2], camCoord[TtotalNumCameras][TdeviceID][3], camCoord[TtotalNumCameras][TdeviceID][0],  camCoord[TtotalNumCameras][TdeviceID][1]);
                                }
                            }
                       }else{
                            unlock();
                       }
                        ofSetHexColor(0xffffff);
                        ofDrawBitmapString(TcamInfo, camCoord[TtotalNumCameras][TdeviceID][2] + 15,camCoord[TtotalNumCameras][TdeviceID][3] +25);
                        TcamInfo = cam.drawInfo();

                        if(!calibSetup && camIsCalibrated){
                            camCalibration.load(calibrationFile);
                            ofxCv::imitate(undistortedImage, camImage);
                            camImage.update();
                            calibSetup = true;
                            blobTracker.background.reset();
                        }
                    }
                }else{
                   unlock();
                   string bitmapString = "No Signal Detected at Camera with IP Address--" + string(cam.ipAddress);
                   ofDrawBitmapString( bitmapString,camCoord[TtotalNumCameras][TdeviceID][2] + 100,camCoord[TtotalNumCameras][TdeviceID][3] +100);
               }
            }else
            {
                // If we reach this else statement, it means that we could not
                // lock our mutex, and so we do not need to call unlock().
                // Calling unlock without locking will lead to problems.
                //ofLogWarning("threadedFunction()") << "Unable to lock mutex.";
            }
        }

        bool        Tcalibration;
        bool        TcalActive;
        bool        TresetCalibration;
        uint64_t	TdeviceID;
        bool        TsetToColor;
        bool        TbUseTexture;
        string      T_pixelFmt;
        float       TcamWidth;
        float       TcamHeight;
        uint64_t    TabsFrameRate;
        uint64_t    TcamZoom;
        uint64_t    TzL;
        bool        camSetup;
        uint64_t    TtotalNumCameras;
        uint64_t    TpacketSize;
        uint64_t    TpacketDelay;
        string      TcamInfo;
        uint64_t    TsetCamWidth;
        uint64_t    TsetCamHeight;
        uint64_t    TxOffset;
        uint64_t    TyOffset;
        char        calibrationFile[30];
        bool        camIsCalibrated;
        bool        calibSetup;
        bool        TcalibrateTracker;
        bool        TblobTrack;
        ofImage                camImage;
        ofImage                undistortedImage;
        ofxCvExtend::Calibration camCalibration;
        int        camCoord[16][16][4];
        FlyCapture2::ofxPointGreyGigECamera cam;
        ofxNTSBlobTrack blobTracker;
   protected:
        bool       camIsColor;
        uint64_t   TcamCornerX;
        uint64_t   TcamCornerY;
        float      TcamWidthRatio;
        float      TcamHeightRatio;
        float      TfullScaledWidth;
        float      TfullScaledHeight;
        float      TselectedRatio;

        GigECalibration        camCal;
};
