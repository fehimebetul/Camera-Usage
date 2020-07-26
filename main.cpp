// Grab_UsingGrabLoopThread.cpp
/*
    Note: Before getting started, Basler recommends reading the Programmer's Guide topic
    in the pylon C++ API documentation that gets installed with pylon.
    If you are upgrading to a higher major version of pylon, Basler also
    strongly recommends reading the Migration topic in the pylon C++ API documentation.
    This sample illustrates how to grab and process images using the grab loop thread
    provided by the Instant Camera class.
*/
// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#include "opencv2/opencv.hpp"
#include <pylon/ImageEventHandler.h>
#include <pylon/GrabResultPtr.h>
#define USE_GIGE 1
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
// Include files used by samples.
#include "./include/ConfigurationEventPrinter.h"
#include "./include/ImageEventPrinter.h"
// Namespace for using pylon objects.
using namespace Pylon;
#if defined ( USE_GIGE )
// Settings for using Basler GigE Vision cameras.
#include <pylon/gige/BaslerGigEInstantCamera.h>
typedef Pylon::CBaslerGigEInstantCamera Camera_t;
using namespace Basler_GigECameraParams;
using namespace Basler_GigEStreamParams;
#else
#error Camera type is not specified. For example, define USE_GIGE for using GigE cameras.
#endif
// Namespace for using cout.
using namespace std;
//Example of an image event handler.
class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
    {
#ifdef PYLON_WIN_BUILD
        // Display the image
        Pylon::DisplayImage(1, ptrGrabResult);
#endif
        cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
        cout << std::endl;
        cout << std::endl;
    }
};
int main(int argc, char* argv[])
{

    double exp_0 = 3000;
    double exp_1 = exp_0*3;
    double exp_2 = exp_1*3;

    // The exit code of the sample application.
    int exitCode = 0;
    // Before using any pylon methods, the pylon runtime must be initialized. 
    PylonInitialize();
    try
    {
        // Only look for cameras supported by Camera_t.
        // GiGe camerayı aradı buldu
        CDeviceInfo info;
        info.SetDeviceClass( Camera_t::DeviceClass());
        // Create an instant camera object with the first found camera device that matches the specified device class.
        Camera_t camera( CTlFactory::GetInstance().CreateFirstDevice( info));
        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        // Register the standard configuration event handler for enabling software triggering.
        // The software trigger configuration handler replaces the default configuration
        // as all currently registered configuration handlers are removed by setting the registration mode to RegistrationMode_ReplaceAll.
        camera.RegisterConfiguration( new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
        // For demonstration purposes only, add a sample configuration event handler to print out information
        // about camera use.
        camera.RegisterConfiguration( new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete);
        // The image event printer serves as sample image processing.
        // When using the grab loop thread provided by the Instant Camera object, an image event handler processing the grab
        // results must be created and registered.
        camera.RegisterImageEventHandler( new CImageEventPrinter, RegistrationMode_Append, Cleanup_Delete);
        // For demonstration purposes only, register another image event handler.
        camera.RegisterImageEventHandler( new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);
        // Open the camera device.
        camera.Open();

        // Can the camera device be queried whether it is ready to accept the next frame trigger?
        if (camera.CanWaitForFrameTriggerReady())
        {
            if (IsWritable(camera.SequenceEnable))
            {

                camera.GammaEnable.SetValue(true);
                camera.Gamma.SetValue(0.46); 

                // Disable the sequencer before changing parameters.
                // The parameters under control of the sequencer are locked
                // when the sequencer is enabled. For a list of parameters
                // controlled by the sequencer, see the camera User's Manual.
                camera.SequenceEnable.SetValue(false);
                // Turn configuration mode on
                if (IsWritable(camera.SequenceConfigurationMode))
                {
                    camera.SequenceConfigurationMode.SetValue(SequenceConfigurationMode_On );
                }
                // Maximize the image area of interest (Image AOI).
                if (IsWritable(camera.OffsetX))
                {
                    camera.OffsetX.SetValue(camera.OffsetX.GetMin());
                }
                if (IsWritable(camera.OffsetY))
                {
                    camera.OffsetY.SetValue(camera.OffsetY.GetMin());
                }
                camera.Width.SetValue(camera.Width.GetMax());
                camera.Height.SetValue(camera.Height.GetMax());
                // Set the pixel data format.
                camera.PixelFormat.SetValue(PixelFormat_Mono8);
                // Set up sequence sets.
                // Configure how the sequence will advance.
                // 'Auto' refers to the auto sequence advance mode.
                // The advance from one sequence set to the next will occur automatically with each image acquired.
                // After the end of the sequence set cycle was reached a new sequence set cycle will start.
                camera.SequenceAdvanceMode = SequenceAdvanceMode_Auto;
                // Our sequence sets relate to three steps (0..2).
                // In each step we will increase the height of the Image AOI by one increment.
                camera.SequenceSetTotalNumber = 3;
                int64_t increments = (camera.Height.GetMax() - camera.Height.GetMin()) / camera.Height.GetInc();
                // Set the parameters for step 0; quarter height image.
                camera.SequenceSetIndex = 0;
                //camera.Height.SetValue( camera.Height.GetInc() * (increments / 4));
                camera.ExposureTimeAbs.SetValue(exp_0);
                camera.SequenceSetStore.Execute();
                // Set the parameters for step 1; half height image.
                camera.SequenceSetIndex = 1;
                //camera.Height.SetValue( camera.Height.GetInc() * (increments / 2));
                camera.ExposureTimeAbs.SetValue(exp_1);
                camera.SequenceSetStore.Execute();
                // Set the parameters for step 2; full height image.
                camera.SequenceSetIndex = 2;
                //camera.Height.SetValue( camera.Height.GetInc() * increments);
                camera.ExposureTimeAbs.SetValue( exp_2);
                camera.SequenceSetStore.Execute();
                // Finish configuration
                if (IsWritable(camera.SequenceConfigurationMode))
                {
                    camera.SequenceConfigurationMode.SetValue(SequenceConfigurationMode_Off);
                }
                // Enable the sequencer feature.
                // From here on you cannot change the sequencer settings anymore.
                camera.SequenceEnable.SetValue(true);

                // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
                // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
                // The GrabStrategy_OneByOne default grab strategy is used.
                camera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
        
                cerr << endl << "Enter \"t\" to trigger the camera or \"e\" to exit and press enter? (t/e)" << endl << endl;
        
                // Wait for user input to trigger the camera or exit the program.
                // The grabbing is stopped, the device is closed and destroyed automatically when the camera object goes out of scope.
                char key;
                do
                {
                    //cin.get(key);
                    //if ( (key == 't' || key == 'T'))
                    //{
                        // Execute the software trigger. Wait up to 500 ms for the camera to be ready for trigger.
                        if ( camera.WaitForFrameTriggerReady( 500, TimeoutHandling_ThrowException))
                        {
                            camera.ExecuteSoftwareTrigger();
                        }
                    //}
                    //cin.get(key);
                    key = (char) cv::waitKey(5);
                }
                while ( (key != 'e') && (key != 'E'));

                // Disable the sequencer.
                camera.SequenceEnable.SetValue(false);
            }
        }
        else
        {
            // See the documentation of CInstantCamera::CanWaitForFrameTriggerReady() for more information.
            cout << endl;
            cout << "This sample can only be used with cameras that can be queried whether they are ready to accept the next frame trigger.";
            cout << endl; 
            cout << endl;
        }
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
        << e.GetDescription() << endl;
        exitCode = 1;
        // Remove left over characters from input buffer.
        cin.ignore(cin.rdbuf()->in_avail());
    }
    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press Enter to exit." << endl;
    while( cin.get() != '\n');
    // Releases all pylon resources. 
    PylonTerminate(); 
    return exitCode;
}