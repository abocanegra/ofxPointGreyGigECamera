#pragma once

#include "ofMain.h"
#include "FlyCapture2.h"

namespace FlyCapture2{
    class ofxPointGreyGigECamera : public ofVideoGrabber
    {
        public:

            ofxPointGreyGigECamera();
            virtual ~ofxPointGreyGigECamera();

            uint64_t                camWidth;
            uint64_t                camHeight;
            uint64_t                xOffset;
            uint64_t                yOffset;
            /* Becoming Deprecated In Future use ofPixels& ofPixels();*/
            unsigned char * 		ofPixels();

            uint64_t	        	listGigEDevices();
            void                    printInfo(const CameraInfo& cameraInfo);
            void                    printBuildInfo();
            void                    setup();
            void                    update();
            void                    draw();
            ofImage                 drawImage();
            string                  drawInfo();
            void                    listDraw();
            void                    listGigEDevicesUpdate();
            bool                    isSetToColor();

            float test;

            ofImage*                mCamImage;

            uint64_t				deviceID;
            bool                    setToColor;
            bool                    bUseTexture;
            string                  _pixelFmt;
            uint64_t                absFrameRate;

            uint64_t                SerialNumber;
            bool                    checkCameraConnected();
            bool                    isConnected;
            bool                    isCamConnected;
            unsigned int            waitForBuffer;
            bool                    lostSignal;
            char                    ipAddress[32];
            char                    camGuid[22];
            unsigned int            packetSize;
            unsigned int            packetDelay;

        protected:
            unsigned int            numCameras;
            bool                    isColor;


            char                    subnetMask[32];
            char                    defaultGateway[32];
            char                    macAddress[64];
            //--------------------------------- flyCapture
            GigECamera*             mCamera;
            Error                   err;
            BusManager		        busMgr;
            PGRGuid                 guid;
            Camera          		cam;
            CameraInfo              camInfo;
            GigEImageSettingsInfo   gigEImageSettingsInfo;
            Property                frmRate;
            PixelFormat             pixelFormat;
            GigEImageSettings       gigEImageSettings;
            Image			        rawImage;
            IPAddress               IP_Address;
            MACAddress              MAC_Address;
            IPAddress               Subnet_Mask;
            IPAddress               Default_Gateway;
            GigEProperty            prop;
            PropertyInfo            propInfo;
            EmbeddedImageInfo       info;


            void                    printGigECapabilities();
            void                    catchError(const string &prefix, Error error);
            void                    setupGigEPacketSize(GigECamera* camera, unsigned int packetSize);

        private:

    };
}
