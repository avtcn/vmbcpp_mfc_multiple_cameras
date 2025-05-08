/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        ApiController.cpp

  Description: Implementation file for the ApiController helper class that
               demonstrates how to implement an asynchronous, continuous image
               acquisition with VimbaCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/
#include <afxwin.h>
#include <ApiController.h>
#include "Common/StreamSystemInfo.h"
#include "Common/ErrorCodeToMessage.h"
namespace AVT {
namespace VmbAPI {
namespace Examples {

//enum { NUM_FRAMES = 3, };
enum { NUM_FRAMES = 10, };

ApiController::ApiController()
// Get a reference to the Vimba singleton
    : m_system( VimbaSystem::GetInstance() )
{
	VmbErrorType res = VmbErrorSuccess;
	VmbVersionInfo_t version;
	res = m_system.QueryVersion(version);
	res = res;
}

ApiController::~ApiController()
{
}

//
// Translates Vimba error codes to readable error messages
//
// Parameters:
//  [in]    eErr        The error code to be converted to string
//
// Returns:
//  A descriptive string representation of the error code
//
string_type ApiController::ErrorCodeToMessage( VmbErrorType eErr ) const
{
    return AVT::VmbAPI::Examples::ErrorCodeToMessage( eErr );
}

//
// Starts the Vimba API and loads all transport layers
//
// Returns:
//  An API status code
//
VmbErrorType ApiController::StartUp()
{
    VmbErrorType res;

    // Start Vimba
    res = m_system.Startup();
    if( VmbErrorSuccess == res )
    {
        // Register an observer whose callback routine gets triggered whenever a camera is plugged in or out
        res = m_system.RegisterCameraListObserver( ICameraListObserverPtr( new CameraObserver() ) );
    }
	CameraIsOpen1=false;
	CameraIsAcq1=false;
	CameraIsOpen2=false;
	CameraIsAcq2=false;

    return res;
}

//
// Shuts down the API
//
void ApiController::ShutDown()
{
    // Release Vimba
    m_system.Shutdown();
}

/** read an integer feature from camera.
*/
 inline VmbErrorType GetFeatureIntValue( const CameraPtr &camera, const std::string &featureName, VmbInt64_t & value )
{
    if( SP_ISNULL( camera ) )
    {
        return VmbErrorBadParameter;
    }
    FeaturePtr      pFeature;
    VmbErrorType    result;
    result = SP_ACCESS( camera )->GetFeatureByName( featureName.c_str(), pFeature );
    if( VmbErrorSuccess == result )
    {
        result = SP_ACCESS( pFeature )->GetValue( value );
    }
    return result;
}
/** write an integer feature from camera.
*/
 inline VmbErrorType SetFeatureIntValue( const CameraPtr &camera, const std::string &featureName, VmbInt64_t value )
{
    if( SP_ISNULL( camera ) )
    {
        return VmbErrorBadParameter;
    }
    FeaturePtr      pFeature;
    VmbErrorType    result;
    result = SP_ACCESS( camera )->GetFeatureByName( featureName.c_str(), pFeature );
    if( VmbErrorSuccess == result )
    {
        result = SP_ACCESS( pFeature )->SetValue( value );
    }
    return result;
}

//
// Opens the given camera
// Sets the maximum possible Ethernet packet size
// Adjusts the image format
// Sets up the observer that will be notified on every incoming frame
// Calls the API convenience function to start image acquisition
// Closes the camera in case of failure
//
// Parameters:
//  [in]    rStrCameraID    The ID of the camera to open as reported by Vimba
//
// Returns:
//  An API status code
//

VmbErrorType ApiController::OpenCloseCamera1(const std::string &rStrCameraID)
{
	VmbErrorType res;
	res=VmbErrorSuccess;
	if (!CameraIsOpen1)
	{
		res = m_system.OpenCameraByID( rStrCameraID.c_str(), VmbAccessModeFull, m_pCamera );
		if( VmbErrorSuccess == res )
		{
			// Set the GeV packet size to the highest possible value
			// (In this example we do not test whether this cam actually is a GigE cam)
			CameraIsOpen1=true;
			FeaturePtr pCommandFeature;
			if( VmbErrorSuccess == m_pCamera->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ) )
			{
				if( VmbErrorSuccess == pCommandFeature->RunCommand() )
				{
					bool bIsCommandDone = false;
					do
					{
						if( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ) )
						{
							break;
						}
					} while( false == bIsCommandDone );
				}
			}

			// Save the current width
			res = GetFeatureIntValue( m_pCamera, "Width", m_nWidth );
			if( VmbErrorSuccess == res )
			{
				// Save current height
				res = GetFeatureIntValue( m_pCamera, "Height", m_nHeight );
				if( VmbErrorSuccess == res )
				{
					// Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
					// Try to set BGR
					// res = SetFeatureIntValue( m_pCamera, "PixelFormat", VmbPixelFormatBgr8 );//此处需要更改为RGB
					res = SetFeatureIntValue( m_pCamera, "PixelFormat", VmbPixelFormatRgb8 );//此处需要更改为RGB
					if( VmbErrorSuccess != res )
					{
						// Fall back to Mono
						res = SetFeatureIntValue( m_pCamera,"PixelFormat", VmbPixelFormatMono8 );
					}
					// Read back the currently selected pixel format
					res =  GetFeatureIntValue( m_pCamera, "PixelFormat", m_nPixelFormat );
				}
			}
		}
	}
	else
	{
		res = SP_ACCESS( m_pCamera )->Close();
		CameraIsOpen1=false;
	    CameraIsAcq1=false;
	}
	return res;
}

VmbErrorType ApiController::StartContinuousImageAcquisition()
{
    VmbErrorType  res;
	res=VmbErrorSuccess;
    if( CameraIsOpen1 )
    {
        // Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
        SP_SET( m_pFrameObserver,new FrameObserver( m_pCamera ) );
        // Start streaming
        res = SP_ACCESS( m_pCamera )->StartContinuousImageAcquisition( NUM_FRAMES, m_pFrameObserver );
		CameraIsAcq1=true;
    }            
    return res;
}

VmbErrorType ApiController::OpenCloseCamera2(const std::string &rStrCameraID)
{
	VmbErrorType res;
	res=VmbErrorSuccess;
	if (!CameraIsOpen2)
	{
		res = m_system.OpenCameraByID( rStrCameraID.c_str(), VmbAccessModeFull, m_pCamera2 );
		if( VmbErrorSuccess == res )
		{
			// Set the GeV packet size to the highest possible value
			// (In this example we do not test whether this cam actually is a GigE cam)
			CameraIsOpen2=true;
			FeaturePtr pCommandFeature;
			if( VmbErrorSuccess == m_pCamera2->GetFeatureByName( "GVSPAdjustPacketSize", pCommandFeature ) )
			{
				if( VmbErrorSuccess == pCommandFeature->RunCommand() )
				{
					bool bIsCommandDone = false;
					do
					{
						if( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ) )
						{
							break;
						}
					} while( false == bIsCommandDone );
				}
			}
			// Save the current width
			res = GetFeatureIntValue( m_pCamera2, "Width", m_nWidth2 );
			if( VmbErrorSuccess == res )
			{
				// Save current height
				res = GetFeatureIntValue( m_pCamera2, "Height", m_nHeight2 );
				if( VmbErrorSuccess == res )
				{
					// Set pixel format. For the sake of simplicity we only support Mono and BGR in this example.
					// Try to set BGR
					//res = SetFeatureIntValue( m_pCamera2, "PixelFormat", VmbPixelFormatBgr8 );//此处需要更改为RGB
					res = SetFeatureIntValue( m_pCamera2, "PixelFormat", VmbPixelFormatRgb8 );//此处需要更改为RGB
					if( VmbErrorSuccess != res )
					{
						// Fall back to Mono
						res = SetFeatureIntValue( m_pCamera2,"PixelFormat", VmbPixelFormatMono8 );
					}
					// Read back the currently selected pixel format
					res =  GetFeatureIntValue( m_pCamera2, "PixelFormat", m_nPixelFormat2 );
				}
			}
		}
	}
	else
	{
		SP_ACCESS( m_pCamera2 )->Close();
		CameraIsOpen2=false;
	    CameraIsAcq2=false;
	}
	return res;
}
VmbErrorType ApiController::StartContinuousImageAcquisition2()
{    
    VmbErrorType  res;
	res=VmbErrorSuccess;
    if( CameraIsOpen2 )
    {
        // Create a frame observer for this camera (This will be wrapped in a shared_ptr so we don't delete it)
        SP_SET( m_pFrameObserver2,new FrameObserver2( m_pCamera2 ) );
        // Start streaming
        res = SP_ACCESS( m_pCamera2 )->StartContinuousImageAcquisition( NUM_FRAMES, m_pFrameObserver2 );
		CameraIsAcq2=true;
    }        
    return res;
}
//
// Calls the API convenience function to stop image acquisition
// Closes the camera
//
// Returns:
//  An API status code
//

VmbErrorType ApiController::SetCameraFloatFeature(int cameraIndex,const std::string &featureName, float value)
{
	VmbErrorType res;
	FeaturePtr feature;
	if (cameraIndex<2)
	{

	
     res = m_pCamera ->GetFeatureByName( featureName.c_str(), feature );
     res = feature ->SetValue(value) ;

	}
	else
	{
     res = m_pCamera2 ->GetFeatureByName( featureName.c_str(), feature );
     res = feature ->SetValue(value) ;
	}
	 return res;
}
VmbErrorType ApiController::SetCameraIntFeature(int cameraIndex,const std::string &featureName, int value)
{
	VmbErrorType res;
	FeaturePtr feature;
	if (cameraIndex<2)
	{	
     res = m_pCamera ->GetFeatureByName( featureName.c_str(), feature );
     res = feature ->SetValue(value) ;
	}
	else
	{
     res = m_pCamera2 ->GetFeatureByName( featureName.c_str(), feature );
     res = feature ->SetValue(value) ;
	}
	 return res;
}
VmbErrorType ApiController::StopContinuousImageAcquisition()
{
    // Stop streaming
	CameraIsAcq1=false;
    return SP_ACCESS( m_pCamera )->StopContinuousImageAcquisition();

    // Close camera
    //return  SP_ACCESS( m_pCamera )->Close();
}
VmbErrorType ApiController::StopContinuousImageAcquisition2()
{

    // Stop streaming
   CameraIsAcq2=false;
   return SP_ACCESS( m_pCamera2 )->StopContinuousImageAcquisition();

    // Close camera
    //return  SP_ACCESS( m_pCamera2 )->Close();
}
//
// Gets all cameras known to Vimba
//
// Returns:
//  A vector of camera shared pointers
//
CameraPtrVector ApiController::GetCameraList()
{
    CameraPtrVector cameras;
    // Get all known cameras
    if( VmbErrorSuccess == m_system.GetCameras( cameras ) )
    {
        // And return them
        return cameras;
    }
    return CameraPtrVector();
}

//
// Gets the width of a frame
//
// Returns:
//  The width as integer
//
int ApiController::GetWidth()
{
    return static_cast<int>( m_nWidth );
}
int ApiController::GetWidth2()
{
    return static_cast<int>( m_nWidth2 );
}
//
// Gets the height of a frame
//
// Returns:
//  The height as integer
//
int ApiController::GetHeight()
{
    return static_cast<int>( m_nHeight );
}
int ApiController::GetHeight2()
{
    return static_cast<int>( m_nHeight2 );
}

//
// Gets the pixel format of a frame
//
// Returns:
//  The pixel format as enum
//
VmbPixelFormatType ApiController::GetPixelFormat()
{
    return static_cast<VmbPixelFormatType>( m_nPixelFormat );
}
VmbPixelFormatType ApiController::GetPixelFormat2()
{
    return static_cast<VmbPixelFormatType>( m_nPixelFormat2 );
}
//
// Gets the oldest frame that has not been picked up yet
//
// Returns:
//  A frame shared pointer
//
FramePtr ApiController::GetFrame()
{
    return SP_DYN_CAST( m_pFrameObserver,FrameObserver )->GetFrame();
}
FramePtr ApiController::GetFrame2()
{
    return SP_DYN_CAST( m_pFrameObserver2,FrameObserver2 )->GetFrame();
}
//
// Clears all remaining frames that have not been picked up
//
void ApiController::ClearFrameQueue()
{
    SP_DYN_CAST( m_pFrameObserver,FrameObserver )->ClearFrameQueue();
}
void ApiController::ClearFrameQueue2()
{
    SP_DYN_CAST( m_pFrameObserver2,FrameObserver2 )->ClearFrameQueue();
}
//
// Queues a given frame to be filled by the API
//
// Parameters:
//  [in]    pFrame          The frame to queue
//
// Returns:
//  An API status code
//
VmbErrorType ApiController::QueueFrame( FramePtr pFrame )
{
    return SP_ACCESS( m_pCamera )->QueueFrame( pFrame );
}

VmbErrorType ApiController::QueueFrame2( FramePtr pFrame )
{
    return SP_ACCESS( m_pCamera2 )->QueueFrame( pFrame );
}
//
// Gets the version of the Vimba API
//
// Returns:
//  The version as string
//
string_type ApiController::GetVersion() const
{
    string_stream_type os;
    os << m_system;
    return os.str();
}
}}} // namespace AVT::VmbAPI::Examples
