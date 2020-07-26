// Contains an Image Event Handler that prints a message for each event method call.

#ifndef INCLUDED_IMAGEEVENTPRINTER_H_7884943
#define INCLUDED_IMAGEEVENTPRINTER_H_7884943

#include <pylon/ImageEventHandler.h>
#include <pylon/GrabResultPtr.h>
#include <iostream>
#include "opencv2/opencv.hpp"
#include <sstream>

namespace Pylon
{
    class CInstantCamera;

    class CImageEventPrinter : public CImageEventHandler
    {
    public:
        int frameNumber = 0;
        virtual void OnImagesSkipped( CInstantCamera& camera, size_t countOfSkippedImages)
        {
            std::cout << "OnImagesSkipped event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;
            std::cout << countOfSkippedImages  << " images have been skipped." << std::endl;
            std::cout << std::endl;
        }


        virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
        {
            std::cout << "OnImageGrabbed event for device " << camera.GetDeviceInfo().GetModelName() << std::endl;

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;
                std::cout << "SizeY: " << ptrGrabResult->GetHeight() << std::endl;
                const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();
                std::cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0] << std::endl;
                std::cout << std::endl;

                CPylonImage pylonImage;//me
                CImageFormatConverter formatConverter;//me
                formatConverter.OutputPixelFormat = PixelType_BGR8packed;//me
                
                formatConverter.Convert(pylonImage, ptrGrabResult);//me
                // Create an OpenCV image out of pylon image
                cv::Mat openCvImage;//me
                openCvImage = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *)pylonImage.GetBuffer());//me
                cv::imwrite("frames/image_"+std::to_string(frameNumber)+".jpg", openCvImage);
                cv::imshow("left camera", openCvImage);
                frameNumber++;

            }
            else
            {
                std::cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << std::endl;
            }
        }
    };
}

#endif /* INCLUDED_IMAGEEVENTPRINTER_H_7884943 */
