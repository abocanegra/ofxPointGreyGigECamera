#include "ofApp.h"

using namespace ofxCv;
using namespace cv;
//--------------------------------------------------------------
void ofApp::setup()
{
    //Needed for full frame speed
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetVerticalSync(false);
	//ofSetFrameRate(240);
    //Base Setup
    ofBackground(20,20,20);
	ofSetWindowTitle("ofxPointGreyGigECamera Example");

    loader =  ofGetElapsedTimeMillis();

    //List Devices and Instantiate Devices
    totalNumCameras = 0;
    camerasAvailable = checkAvailableCameras();
    setFrameRate = 15;
    selectedCamera = 0;
    camZoom = 1;
    calibration = false;

}

//--------------------------------------------------------------
void ofApp::update()
{
    if(!camerasAvailable){
        if(initialized){
            listCam.listGigEDevicesUpdate();
            camerasAvailable = checkAvailableCameras();
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    //Delay initialization To allow everything to get up to speed
    if(!initialized){
        initialized = initDelay();
    }else if(!camerasAvailable && initialized){
            listCam.listDraw(); //Print to screen the loading message.
    }else if(camerasAvailable >0 && initialized){
        for(uint64_t i=0; i<totalNumCameras;i++){//
            if(camThread[i].isThreadRunning()){
               camThread[i].draw();
            }
        }
    }
    ofSetHexColor(0xffffff);
    char reportStr[1024];
    sprintf(reportStr, "FPS: %3.2f", ofGetFrameRate() );
    ofDrawBitmapString(reportStr, ofGetWidth()-100, ofGetHeight() - 25);

   // cout << "Time: " << ofGetTimestampString("%h:%M;%S %A") << endl;
   // ofLog(OF_LOG_NOTICE,"Elapsed: %u:%u:%u\n", ((ofGetElapsedTimeMillis()/1000)/60)/60,((ofGetElapsedTimeMillis()/1000)/60)%60,  (ofGetElapsedTimeMillis()/1000)%60);

}

//--------------------------------------------------------------
bool ofApp::initDelay(){
    loader =  ofGetElapsedTimeMillis();
    int loadTime = ofGetElapsedTimeMillis()%500;
    if(loader < 5000){
        ofSetHexColor(0xffffff);
        string loadChar= ".";
         if(loadTime <=100){
            loadChar += "";
        }else if(loadTime <=200 && loadTime > 100){
            loadChar += ".";
        }else if(loadTime <=300 && loadTime > 200){
            loadChar += "..";
        }else if(loadTime <= 400 && loadTime > 300){
            loadChar += "...";
        }else if(loadTime <= 500 && loadTime >400){
            loadChar +="....";
        }
        ofDrawBitmapString("Initializing" + loadChar,100, 100);
        return false;
   }
   return true;
}

//--------------------------------------------------------------
bool ofApp::checkAvailableCameras(){
    totalNumCameras = listCam.listGigEDevices();
    if(totalNumCameras == 1){
        packetSize = 1500;//9000
        packetDelay = 0;//0
        setFrameRate = 30;//0
    }else if(totalNumCameras == 2){
        packetSize = 1480;//1751;
        packetDelay = 100;//900
        setFrameRate = 16;
    }else if(totalNumCameras == 3){
        packetSize = 821;
        packetDelay = 1090;
        setFrameRate = 16;
    }else if(totalNumCameras ==4){
        packetSize = 674;
        packetDelay = 1199;
        setFrameRate = 12;
    }

    if(totalNumCameras > 0 && !camerasAvailable){
        listCam.~ofxPointGreyGigECamera();
        for(uint64_t i=0; i<totalNumCameras;i++){
            //instantiate cameras to PGRCamera cam[4] array
            ofLog(OF_LOG_NOTICE,"Insantiating Camera %i\n", i);
            //PIXEL_FORMAT_MONO8 || "PIXEL_FORMAT_RAW8" - default
            camThread[i].Tcalibration =     calibration;
            camThread[i].TdeviceID =        i;
            camThread[i].TsetToColor =      false;
            camThread[i].TbUseTexture =     true;
            camThread[i].T_pixelFmt=        "PIXEL_FORMAT_MONO8";
            camThread[i].TabsFrameRate =    setFrameRate;
            camThread[i].TtotalNumCameras = totalNumCameras;
            camThread[i].TsetCamWidth =     1536;//1184;
            camThread[i].TsetCamHeight =    1200;//990;
            camThread[i].TxOffset =         176;//352;
            camThread[i].TyOffset =         0;//90;
            camThread[i].TpacketSize =      packetSize;//1947;//625;
            camThread[i].TpacketDelay =     packetDelay;//509;//1090;

            if(totalNumCameras==1){
                camZoom = sqrt(totalNumCameras);
            }else if(totalNumCameras >1 && totalNumCameras <=4){
                camZoom = 2;
            }else if(totalNumCameras>4 && totalNumCameras <=9){
                camZoom = 3;
            }else if(totalNumCameras>9 && totalNumCameras<=16){
                camZoom = 4;
            }
            camThread[i].TcamZoom = camZoom;
            camThread[i].start();
      }
        return true;
    }
    return false;
}


//--------------------------------------------------------------
/* Special Keys
OF_KEY_BACKSPACE, OF_KEY_RETURN, OF_KEY_PRINTSCR, OF_KEY_F1 - OF_KEY_F12, OF_KEY_LEFT, OF_KEY_UP, OF_KEY_RIGHT, OF_KEY_DOWN, OF_KEY_PAGE_UP, OF_KEY_PAGE_DOWN, OF_KEY_HOME, OF_KEY_END, OF_KEY_INSERT
*/
void ofApp::keyPressed(int key)
{
    if(key == 't' || key == 'T'){
        for(uint64_t i=0; i<totalNumCameras;i++){
            camThread[i].stop();
            camThread[i].TbUseTexture=!camThread[i].TbUseTexture;
            camThread[i].start();
        }
    }

    if(key == OF_KEY_F1){
        selectedCamera = 0;
    }

    if(key == OF_KEY_F2){
        selectedCamera = 1;
    }

    if(key == OF_KEY_F3){
        selectedCamera = 2;
    }

    if(key == OF_KEY_F4){
        selectedCamera = 3;
    }

    if(key == '-' || key == '_'){
        if(camZoom<4 && camZoom >=1){
            camZoom++;
            for(uint64_t i=0; i<totalNumCameras;i++){
                camThread[i].stop();
                camThread[i].TcamZoom = camZoom;
                camThread[i].TtotalNumCameras = pow(camZoom, 2);
                camThread[i].start();
            }
        }
    }

    if(key == '=' || key == '+'){
        if(camZoom>1 && camZoom <=4){
            camZoom--;
            for(uint64_t i=0; i<totalNumCameras;i++){
                camThread[i].stop();
                camThread[i].TcamZoom = camZoom;
                camThread[i].TtotalNumCameras = pow(camZoom, 2);
                camThread[i].start();
            }
        }
    }
    if(key == '0' ){
        if(camZoom !=1){
            camZoom = 1;
            for(uint64_t i=0; i<totalNumCameras;i++){
                camThread[i].stop();
                camThread[i].TcamZoom = camZoom;
                camThread[i].TtotalNumCameras = totalNumCameras;
                camThread[i].start();
            }
        }
    }

    if(key == 'c' || key == 'C'){
      if(calibration){
            calibration = false;
            for(uint64_t i=0; i<totalNumCameras;i++){
                camThread[i].stop();
                camThread[i].TcalActive = false;
                camThread[i].Tcalibration = calibration;
                camThread[i].start();
            }
        }else{
            calibration = true;
            for(uint64_t i=0; i<totalNumCameras;i++){
                camThread[i].stop();
                if(i == selectedCamera){
                   camThread[i].TcalActive = true;
                   camThread[i].Tcalibration = calibration;
                   camThread[i].start();
                }
            }
        }
        ofLog(OF_LOG_NOTICE, "Calibration switched to: %i\n", calibration);
    }

    if(key == 'q' || key == 'Q'){

        for(uint64_t i=0; i<totalNumCameras;i++){
            camThread[i].stop();
        }
        ofLog(OF_LOG_NOTICE, "Video Grab threads have been stopped. You may close the program");
    }

    if(key == 'a' || key == 'A'){
        if(calActive){
            calActive = false;
            camThread[selectedCamera].TcalActive = false;
        }else{
            calActive = true;
            camThread[selectedCamera].TcalActive= true;
        }
    }

    if(key == 'r' || key =='R'){
        camThread[selectedCamera].TresetCalibration = true;
    }

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
