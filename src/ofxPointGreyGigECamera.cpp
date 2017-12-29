#include "ofxPointGreyGigECamera.h"
#include <string>

namespace FlyCapture2{
    ofxPointGreyGigECamera::ofxPointGreyGigECamera()
    {
        deviceID        = 0;
        setToColor      = false;
        bUseTexture     = false;
        _pixelFmt       = "PIXEL_FORMAT_RAW8";
        absFrameRate    = 30;
        camWidth        = 1920;
        camHeight       = 1200;
        xOffset         = 0;
        yOffset         = 0;
        packetSize      = 9000;
        packetDelay     = 0;
    }

    ofxPointGreyGigECamera::~ofxPointGreyGigECamera()
    {
        if(mCamera->IsConnected()){
            mCamera->StopCapture();
            mCamera->Disconnect();
        }

        delete mCamera;
        delete mCamImage;
    }

    //-------------------------------------------------------------------------------------
    uint64_t ofxPointGreyGigECamera::listGigEDevices(){

        if(numCameras >0){
            ofLog(OF_LOG_NOTICE,"BEGIN LIST Cameras\n");

            for(uint64_t i=0; i<numCameras; i++){
                //Get First Connected Camera
                catchError("Problem linking selected camera to the buffer!", busMgr.GetCameraFromIndex(i,&guid));

                //Connect to Camera
                mCamera = new GigECamera();
                catchError("Failed to Connect to Camera!" , mCamera->Connect(&guid));

                //Get Camera Information
                catchError("Failed to get Camera Info!" , mCamera->GetCameraInfo(&camInfo));
                ofLog(OF_LOG_NOTICE,"Camera %i",i);
                printInfo(camInfo);
                catchError("Failed to Disconnect the Camera!", mCamera->Disconnect());
            }
            ofLog(OF_LOG_NOTICE,"END LIST Cameras\n");
        }else{
            ofLog(OF_LOG_ERROR,"No Cameras Found | Searching >>>>>>\n");
        }
        return numCameras;
    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::catchError(const string& prefix, Error error)
    {
        if(error == PGRERROR_TIMEOUT){
            /* With exception.h header included in Flycapture2.h
            throw CameraTimeoutException("PointGreyCamera: Failed to retrieve buffer within timeout.");
            */
            cout << "\n" << endl;
             ofLog(OF_LOG_ERROR, "PointGreyCamera: Failed to retrieve buffer within timeout.\n");
            // lock();
            catchError("Failed to Rescan Bus", busMgr.RescanBus());
            // unlock();
       }else if ((error == PGRERROR_IIDC_FAILED || error == PGRERROR_BUS_MASTER_FAILED || error==PGRERROR_FAILED_BUS_MASTER_CONNECTION ||
                  error == PGRERROR_NOT_IMPLEMENTED || error== PGRERROR_REGISTER_FAILED || error== PGRERROR_ISOCH_NOT_STARTED)
                  && isConnected && ofGetElapsedTimeMillis() >4000){
            /*
            Do as you need here
            */
            /*
            mCamera->StopCapture();
            mCamera->Disconnect();
            isConnected = false;
            lostSignal = true;

            catchError("Failed to Rescan Bus", busMgr.RescanBus());
            */
          //  isCamConnected = checkCameraConnected();
        }else if(error != PGRERROR_OK){
            ofLog(OF_LOG_ERROR, "\n%s\n", prefix.c_str());
            error.PrintErrorTrace();
            cout << "\nError Type: " << error.GetType() << "\n" << endl;
            /* run_time error : Must include exceptions.h in flycapture2.h"
            string start(" | FlyCapture2::ErrorType ");
            stringstream out;
            out << error.GetType();
            string desc(error.GetDescription());
            throw runtime_error(prefix + start + out.str() + desc);
            */
        }
    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::printInfo(const CameraInfo& cameraInfo)
    {
        MAC_Address = cameraInfo.macAddress;
        sprintf(
            macAddress,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            cameraInfo.macAddress.octets[0],
            cameraInfo.macAddress.octets[1],
            cameraInfo.macAddress.octets[2],
            cameraInfo.macAddress.octets[3],
            cameraInfo.macAddress.octets[4],
            cameraInfo.macAddress.octets[5]);

        IP_Address =cameraInfo.ipAddress;
        sprintf(
            ipAddress,
            "%u.%u.%u.%u",
            cameraInfo.ipAddress.octets[0],
            cameraInfo.ipAddress.octets[1],
            cameraInfo.ipAddress.octets[2],
            cameraInfo.ipAddress.octets[3]);

        Subnet_Mask = cameraInfo.subnetMask;
        sprintf(
            subnetMask,
            "%u.%u.%u.%u",
            cameraInfo.subnetMask.octets[0],
            cameraInfo.subnetMask.octets[1],
            cameraInfo.subnetMask.octets[2],
            cameraInfo.subnetMask.octets[3]);

        Default_Gateway = cameraInfo.defaultGateway;
        sprintf(
            defaultGateway,
            "%u.%u.%u.%u",
            cameraInfo.defaultGateway.octets[0],
            cameraInfo.defaultGateway.octets[1],
            cameraInfo.defaultGateway.octets[2],
            cameraInfo.defaultGateway.octets[3]);

        SerialNumber = cameraInfo.serialNumber;

        ofLog(OF_LOG_NOTICE,
            "\n*** CAMERA INFORMATION ***\n"
            "Serial number - %u\n"
            "Mac Address - %s\n"
            "IP Address - %s\n"
            "Subnet Mask - %s\n"
            "Default Gateway - %s\n"
            "Camera model - %s\n"
            "Camera vendor - %s\n"
            "Sensor - %s\n"
            "Resolution - %s\n"
            "Is Color - %d\n"
            "Firmware version - %s\n"
            "Firmware build time - %s\n\n",
            cameraInfo.serialNumber,
            macAddress,
            ipAddress,
            subnetMask,
            defaultGateway,
            cameraInfo.modelName,
            cameraInfo.vendorName,
            cameraInfo.sensorInfo,
            cameraInfo.sensorResolution,
            cameraInfo.isColorCamera,
            cameraInfo.firmwareVersion,
            cameraInfo.firmwareBuildTime);
    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::printBuildInfo()
    {
        FC2Version fc2Version;

        Utilities::GetLibraryVersion( &fc2Version );

        char version[128];
        sprintf(version,"FlyCapture2 library version: %d.%d.%d.%d\n",fc2Version.major, fc2Version.minor, fc2Version.type, fc2Version.build );

        printf( "%s", version );

        char timeStamp[512];
        sprintf( timeStamp, "Application build date: %s %s\n\n", __DATE__, __TIME__ );

        printf( "%s", timeStamp );

    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::printGigECapabilities()
    {
        catchError("Could Not Get GigEImageSettingsInfo", mCamera->GetGigEImageSettingsInfo(&gigEImageSettingsInfo));
        //camWidth = gigEImageSettingsInfo.maxWidth;
        //camHeight = gigEImageSettingsInfo.maxHeight;
        printf(
            "Max image pixels: (%u, %u)\n"
            "Image Unit size: (%u, %u)\n"
            "Offset Unit size: (%u, %u)\n"
            "Pixel format bitfield: 0x%08x\n\n",
            gigEImageSettingsInfo.maxWidth,
            gigEImageSettingsInfo.maxHeight,
            gigEImageSettingsInfo.imageHStepSize,
            gigEImageSettingsInfo.imageVStepSize,
            gigEImageSettingsInfo.offsetHStepSize,
            gigEImageSettingsInfo.offsetVStepSize,
            gigEImageSettingsInfo.pixelFormatBitField );
    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::setup()//uint64_t _deviceID,string _pixelFmt
    {
        //Get The Selected Camera
        catchError("Failed to Link Selected Camera to the Bus Manager!", busMgr.GetCameraFromIndex(deviceID,&guid));
        sprintf(camGuid,"%u",guid);

            delete mCamera;
            //Connect to Camera
            mCamera = new GigECamera();

            if(mCamera->IsConnected()){
                mCamera->StopCapture();
                mCamera->Disconnect();
            }

            catchError("Failed to Connect to the Selected Camera", mCamera->Connect(&guid));

            isConnected = mCamera->IsConnected();

            //Get Active Camera Information
            catchError("Failed to get Camera Info from the selected Camera", mCamera->GetCameraInfo(&camInfo));

            ofLog(OF_LOG_NOTICE,"ACTIVE CAMERA INFO \n");
            printInfo(camInfo);
            ofLog(OF_LOG_NOTICE,"END ACTIVE CAMERA INFO\n");

            // Enable metadata
            info.timestamp.onOff = true;
            info.gain.onOff = true;
            info.shutter.onOff = true;
            info.brightness.onOff = true;
            info.exposure.onOff = true;
            info.whiteBalance.onOff = true;
            info.frameCounter.onOff = true;
            info.ROIPosition.onOff = true;

            catchError("Failed to Enable Metadata!", mCamera->SetEmbeddedImageInfo(&info));
            //Show Capabilities for this Camera
            printGigECapabilities();

            // Set GigE Image Settings
            if(_pixelFmt == "PIXEL_FORMAT_MONO8"){
                pixelFormat = PIXEL_FORMAT_MONO8;
            }else if(_pixelFmt == "PIXEL_FORMAT_MONO12"){
                pixelFormat = PIXEL_FORMAT_MONO12;
            }else if(_pixelFmt == "PIXEL_FORMAT_MONO16"){
                pixelFormat = PIXEL_FORMAT_MONO16;
            }else if(_pixelFmt == "PIXEL_FORMAT_RGB"){
                pixelFormat = PIXEL_FORMAT_RGB;
            }else if(_pixelFmt == "PIXEL_FORMAT_RGB8"){
                pixelFormat = PIXEL_FORMAT_RGB8;
            }else if(_pixelFmt == "PIXEL_FORMAT_RGB16"){
                pixelFormat = PIXEL_FORMAT_RGB16;
            }else if(_pixelFmt == "PIXEL_FORMAT_RAW8"){
                pixelFormat = PIXEL_FORMAT_RAW8;
            }else if(_pixelFmt == "PIXEL_FORMAT_RAW12"){
                pixelFormat = PIXEL_FORMAT_RAW12;
            }else if(_pixelFmt == "PIXEL_FORMAT_RAW16"){
                pixelFormat = PIXEL_FORMAT_RAW16;
            }else if(_pixelFmt == "PIXEL_FORMAT_411YUV8"){
                pixelFormat = PIXEL_FORMAT_411YUV8;
            }else if(_pixelFmt == "PIXEL_FORMAT_422YUV8"){
                pixelFormat = PIXEL_FORMAT_422YUV8;
            }else if(_pixelFmt == "PIXEL_FORMAT_422YUV8_JPEG"){
                pixelFormat = PIXEL_FORMAT_422YUV8_JPEG;
            }else if(_pixelFmt == "PIXEL_FORMAT_444YUV8"){
                pixelFormat = PIXEL_FORMAT_444YUV8;
            }else if(_pixelFmt == "PIXEL_FORMAT_BGR"){
                pixelFormat = PIXEL_FORMAT_BGR;
            }else if(_pixelFmt == "PIXEL_FORMAT_BGR16"){
                pixelFormat = PIXEL_FORMAT_BGR16;
            }else if(_pixelFmt == "PIXEL_FORMAT_RGBU"){
                pixelFormat = PIXEL_FORMAT_RGBU;
            }else if(_pixelFmt == "PIXEL_FORMAT_S_MONO16"){
                pixelFormat = PIXEL_FORMAT_S_MONO16;
            }else if(_pixelFmt == "PIXEL_FORMAT_S_RGB16"){
                pixelFormat = PIXEL_FORMAT_S_RGB16;
            }
            gigEImageSettings.width= camWidth;
            gigEImageSettings.height= camHeight;
            gigEImageSettings.pixelFormat = pixelFormat;
            gigEImageSettings.offsetX = xOffset;
            gigEImageSettings.offsetY = yOffset;

            catchError("Failed to set GigEImageSettings",mCamera->SetGigEImageSettings(&gigEImageSettings));
            //Get GigE Image Settings
            catchError("Failed to get GigEImageSettings",mCamera->GetGigEImageSettings(&gigEImageSettings));

             //Set GigE Packet Size
            setupGigEPacketSize(mCamera,packetSize);

            //set Frame Rate
            frmRate.type =           FRAME_RATE;
            frmRate.onePush =        false;
            frmRate.autoManualMode = false;
            frmRate.onOff =          true;
            frmRate.absControl =     true;
            frmRate.absValue=        absFrameRate;
            frmRate.valueA =         FRAMERATE_7_5;
            frmRate.valueB =         FRAMERATE_30;

            catchError("Cannot Set Framewrate", mCamera->SetProperty(&frmRate));

            // Start Capture
            catchError("Failed to Start Capture", mCamera->StartCapture());

            isConnected = mCamera->IsConnected();
            // Retrieve frame rate property
            //frmRate.type = FRAME_RATE;
            catchError("Cannot receive Frame Rate!", mCamera->GetProperty( &frmRate ));

            printf( "Frame rate is %3.2f fps\n", frmRate.absValue );
            if(gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO8 ||  gigEImageSettings.pixelFormat == PIXEL_FORMAT_BGR || gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO12 ||
               gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO16 || gigEImageSettings.pixelFormat == PIXEL_FORMAT_S_MONO16){
                mCamImage = new ofImage();
                mCamImage->setUseTexture(bUseTexture);
                mCamImage->allocate(camWidth,camHeight, OF_IMAGE_GRAYSCALE);
                setToColor = false;
           }else{
                //ofImage
                mCamImage = new ofImage();
                mCamImage->setUseTexture(bUseTexture);
                mCamImage->allocate(camWidth,camHeight, OF_IMAGE_COLOR);

                setToColor = true;
               cout << "SETUP COMPLETE" << endl;
            }
    }

   //-------------------------------------------------------------------------------------
   bool ofxPointGreyGigECamera::isSetToColor(){
        return setToColor;
   }
     //-------------------------------------------------------------------------------------
   void ofxPointGreyGigECamera::setupGigEPacketSize(GigECamera* camera, unsigned int _packetSize){
        //Discover Packet Size
        catchError("Could Not Discover Preferred Packet Size!", camera->DiscoverGigEPacketSize( &_packetSize ));

        ofLog(OF_LOG_NOTICE, "Camera %u Preferred Packet Size: %i\n", deviceID, _packetSize);

        prop.propType = PACKET_SIZE;

        catchError("Could not set Packet Size Property", camera->GetGigEProperty(&prop));

        if(packetSize == 9000){
            prop.value = _packetSize;
        }else{
            prop.value = packetSize;
        }

        catchError("Failed to set Packet Size", camera->SetGigEProperty(&prop));
        catchError("Could not confirm Packet Size Set", camera->GetGigEProperty(&prop));


        ofLog(OF_LOG_NOTICE, "Camera Packet Size Property Current Value: %u\n", prop.value);

        //Discover Packet Delay
        prop.propType = PACKET_DELAY;

        catchError("Could not set Packet Size Property", camera->GetGigEProperty(&prop));

        prop.value = packetDelay;

        catchError("Failed to set Packet Delay", camera->SetGigEProperty(&prop));
        catchError("Could not confirm Packet Size Delay", camera->GetGigEProperty(&prop));

        ofLog(OF_LOG_NOTICE, "Camera Packet Delay Property Current Value: %u\n", prop.value);

    }

     //-------------------------------------------------------------------------------------
    bool ofxPointGreyGigECamera::checkCameraConnected(){
        /***** Make sure signal has not been lost from the Camera ****/
        isConnected = mCamera->IsConnected();

        CameraStats camStats;
        catchError("Failed to get camera Stats-Signal Lost!",mCamera->GetStats(&camStats));
        //Check for Signal
        if(!camStats.cameraPowerUp){
            cout << "ERROR: Signal Lost to the Camera at: " << ipAddress << " | Time: " << ofGetTimestampString("%h:%M;%S %A") << " | elapsed time: " <<  ofGetElapsedTimeMillis()  << " | isCamConnected " << isCamConnected << endl;
            lostSignal = true;
            cout<< "Lost Signal Check: " << lostSignal <<endl;
            catchError("Failed to Rescan Bus", busMgr.RescanBus());
            PGRGuid testGuid;
            catchError("Active Camera Signal Lost!", busMgr.GetCameraFromIPAddress(IP_Address, &guid));
            if(guid != testGuid){
                ofLog(OF_LOG_ERROR,"Camera with IP Address %s is connected!", ipAddress);
                //Rescan Bus
                catchError("Failed to Rescan Bus", busMgr.RescanBus());
                catchError("Active Camera Signal Lost!", busMgr.GetCameraFromIPAddress(IP_Address, &guid));


                //Connect to Camera
                catchError("Failed to Connect to the Selected Camera", mCamera->Connect(&guid));
                    setupGigEPacketSize(mCamera, packetSize);

                //Start Capture
                catchError("Failed to Start Capture", mCamera->StartCapture());
                isConnected = mCamera->IsConnected();

                //Get Camera Stat and Info
                catchError("Failed to get camera Stats",mCamera->GetStats(&camStats));
                catchError("Failed to get Camera Info-Signal Lost!" , mCamera->GetCameraInfo(&camInfo));
                //Allocate Image
                if(gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO8 ||  gigEImageSettings.pixelFormat == PIXEL_FORMAT_BGR || gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO12 ||
                   gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO16 || gigEImageSettings.pixelFormat == PIXEL_FORMAT_S_MONO16){
                    //ofImage
                   mCamImage->allocate(camWidth,camHeight, OF_IMAGE_GRAYSCALE);
                }else{
                    //ofImage
                    mCamImage->allocate(camWidth,camHeight, OF_IMAGE_COLOR);
                }
                return true;
            }else{
                ofLog(OF_LOG_ERROR,"Camera with IP Address %s is disconnected!", ipAddress);
                return false;
            }
        }else{
            lostSignal = false;
            return true;
        }
        return true;
    }

     //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::update()
    {
        // Retrieve Images -----------------------------------
            catchError("Buffer Failed to Be Retrieved from the Camera", mCamera->WaitForBufferEvent(&rawImage,waitForBuffer));
            while(waitForBuffer != 0){
                catchError("Buffer Failed to Be Retrieved from the Camera", mCamera->WaitForBufferEvent(&rawImage,waitForBuffer));
            }
           if(gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO8){
                memcpy(mCamImage->getPixels(),rawImage.GetData(),rawImage.GetDataSize());
           }else if(gigEImageSettings.pixelFormat == PIXEL_FORMAT_RGB || gigEImageSettings.pixelFormat == PIXEL_FORMAT_RGB8){
                memcpy(mCamImage->getPixels(),rawImage.GetData(),rawImage.GetDataSize());
          }else if(gigEImageSettings.pixelFormat == PIXEL_FORMAT_BGR || gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO12 ||
                    gigEImageSettings.pixelFormat == PIXEL_FORMAT_MONO16 || gigEImageSettings.pixelFormat == PIXEL_FORMAT_S_MONO16){
                Image imageConv;
                PixelFormat pxConv = PIXEL_FORMAT_MONO8;
                rawImage.Convert( pxConv, &imageConv);
                memcpy(mCamImage->getPixels(),imageConv.GetData() ,imageConv.GetDataSize());
           }else{
                Image imageConv;
                PixelFormat pxConv = PIXEL_FORMAT_RGB;
                rawImage.Convert( pxConv, &imageConv);
                memcpy(mCamImage->getPixels(),imageConv.GetData() ,imageConv.GetDataSize());
            }
            //Flip Image
            //mCamImage->mirror(false,false);

            mCamImage->update();
    }

    //ofImage-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::draw()
    {
        if(bUseTexture){
            mCamImage->draw(0,0);
        }
    }
    //ofImage-------------------------------------------------------------------------------------
    ofImage ofxPointGreyGigECamera::drawImage()
    {
        return *mCamImage;
    }

    //-------------------------------------------------------------------------------------
    string ofxPointGreyGigECamera::drawInfo(){
            char reportStr[1024];
            sprintf(reportStr, "Camera: %d | IP: %s | Draw Texture %d", deviceID, ipAddress, bUseTexture);
            return reportStr;
    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::listDraw(){
        ofSetHexColor(0xffffff);
        string counterString = ".";
        int counter = ofGetElapsedTimeMillis()%4000;
        if(counter <=2000 && counter > 1000){
            counterString += ".";
        }else if(counter >2000 && counter <= 3000){
             counterString += "..";
        }else if(counter >3000 && counter <=4000){
            counterString += "...";
        }
        ofDrawBitmapString("Searching for Cameras" + counterString,100, 100);
    }

    //-------------------------------------------------------------------------------------
    void ofxPointGreyGigECamera::listGigEDevicesUpdate(){
        catchError("Failed to rescan the bus", busMgr.RescanBus());
        err = busMgr.GetNumOfCameras(&numCameras);
        ofLog(OF_LOG_NOTICE, "Number of Cameras Detected: %u\n", numCameras);
    }
}
