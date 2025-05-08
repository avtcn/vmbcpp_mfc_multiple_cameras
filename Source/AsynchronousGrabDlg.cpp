/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AsynchronousGrabDlg.cpp

  Description: MFC dialog class for the GUI of the AsynchronousGrab example of
               VimbaCPP.

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

#include <stdafx.h>
#include <AsynchronousGrab.h>
#include <AsynchronousGrabDlg.h>
#include "VmbTransform.h"
#include <iostream>
#include <sstream>
#include <string>
#define NUM_COLORS 3
#define BIT_DEPTH 8

using AVT::VmbAPI::FramePtr;
using AVT::VmbAPI::CameraPtrVector;
// Ctor
CAsynchronousGrabDlg::CAsynchronousGrabDlg( CWnd* pParent )
    : CDialog( CAsynchronousGrabDlg::IDD, pParent )   
	, packetSize1(1500)
	, packetSize2(1500)
{
    m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
}

BEGIN_MESSAGE_MAP( CAsynchronousGrabDlg, CDialog )
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()

    // Buttons to start/stop each camera
    ON_BN_CLICKED(IDC_BUTTON_STARTSTOP, &CAsynchronousGrabDlg::OnBnClickedButtonStartstop )
	ON_BN_CLICKED(IDC_BUTTON_STARTSTOP2, &CAsynchronousGrabDlg::OnBnClickedButtonStartstop2)

    // Here we add the event handlers for Vimba events, frame receiving and update images in image boxes
    ON_MESSAGE( WM_FRAME_READY1, OnFrameReady )
	ON_MESSAGE( WM_FRAME_READY2, OnFrameReady2 )

    ON_MESSAGE( WM_CAMERA_LIST_CHANGED, OnCameraListChanged )
	
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EDIT1, &CAsynchronousGrabDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_BT_OPENCAM1, &CAsynchronousGrabDlg::OnBnClickedBtOpencam1)
	ON_BN_CLICKED(IDC_BT_OPENCAM2, &CAsynchronousGrabDlg::OnBnClickedBtOpencam2)
	ON_EN_CHANGE(IDC_EDIT2, &CAsynchronousGrabDlg::OnEnChangeEdit2)
END_MESSAGE_MAP()

BOOL CAsynchronousGrabDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetIcon( m_hIcon, TRUE );
    SetIcon( m_hIcon, FALSE );

	m_Slider1.SetRange(1,200000);//设置滑动范围为1到200ms,可以自定义
    m_Slider1.SetTicFreq(500);//每1个单位画一刻度
    m_Slider1.SetPos(1500);

	m_Slider2.SetRange(1,200000);//设置滑动范围为1到200ms,可以自定义
    m_Slider2.SetTicFreq(500);//每1个单位画一刻度
    m_Slider2.SetPos(1500);

	m_ButtonStartStop.EnableWindow(false);
	m_Slider1.EnableWindow(false);
	m_packageEidt1.EnableWindow(false);

	m_ButtonStartStop2.EnableWindow(false);
	m_Slider2.EnableWindow(false);
	m_packageEidt2.EnableWindow(false);

    // Start Vimba
    VmbErrorType err = m_ApiController.StartUp();
    string_type DialogTitle( _TEXT( "AsynchronousGrab (MFC version) Vimba V" ) );
    SetWindowText( ( DialogTitle+m_ApiController.GetVersion() ).c_str() );
    Log( _TEXT( "Starting Vimba " + m_ApiController.GetVersion()), err);
    if( VmbErrorSuccess == err )
    {
        // Initially get all connected cameras
        UpdateCameraListBox();
        string_stream_type strMsg;
        strMsg << "Cameras found..." << m_cameras.size();
        Log( strMsg.str() );
    }

    return TRUE;
}

void CAsynchronousGrabDlg::OnSysCommand( UINT nID, LPARAM lParam )
{
    if( SC_CLOSE == nID )
    {
        // if we are streaming, must stop streaming first for each camera
        if( true == m_ApiController.CameraIsOpen1 )
            OnBnClickedBtOpencam1();
		if( true == m_ApiController.CameraIsOpen2 )
            OnBnClickedBtOpencam2();

        // Before we close the application we stop Vimba SDK library
        m_ApiController.ShutDown();
    }

    CDialog::OnSysCommand( nID, lParam );
}

// Button operation for #1 camera
void CAsynchronousGrabDlg::OnBnClickedButtonStartstop()
{
    VmbErrorType err;
   
    if( false == m_ApiController.CameraIsAcq1 )
    {        
        // Start acquisition
        err = m_ApiController.StartContinuousImageAcquisition();
        // Set up image for MFC picture box
        if (    VmbErrorSuccess == err
                && NULL == m_Image )
        {
            m_Image.Create(  m_ApiController.GetWidth(),
                            -m_ApiController.GetHeight(),
                            NUM_COLORS * BIT_DEPTH );
            m_ClearBackground = true;
        }
        Log( _TEXT( "Camera 1 Starting Acquisition" ), err );
             
    }
    else
    {
       
        // Stop acquisition
        err = m_ApiController.StopContinuousImageAcquisition();
        m_ApiController.ClearFrameQueue();
        if( NULL != m_Image )
        {
            m_Image.Destroy();
        }
        Log( _TEXT( "Camera 1 Stopping Acquisition" ), err );
    }

   UpdateContronls();
}


// Button operation for #2 camera
void CAsynchronousGrabDlg::OnBnClickedButtonStartstop2()
{
	VmbErrorType err;   
    if( false == m_ApiController.CameraIsAcq2 )
    {     
        // Start acquisition
        err = m_ApiController.StartContinuousImageAcquisition2();
        // Set up image for MFC picture box
        if (    VmbErrorSuccess == err
                && NULL == m_Image2 )
        {
            m_Image2.Create(  m_ApiController.GetWidth2(),
                            -m_ApiController.GetHeight2(),
                            NUM_COLORS * BIT_DEPTH );
            m_ClearBackground2 = true;
        }
        Log( _TEXT( "Camera 2 Starting Acquisition" ), err );
        
      }
       
    else
    {      
        // Stop acquisition
        err = m_ApiController.StopContinuousImageAcquisition2();
        m_ApiController.ClearFrameQueue2();
        if( NULL != m_Image2 )
        {
            m_Image2.Destroy();
        }
        Log( _TEXT( "Camera 2 Stopping Acquisition" ), err );
    }

	UpdateContronls();
   
}

//
// This event handler is triggered through a MFC message posted by the frame observer
//
// Parameters:
//  [in]    status          The frame receive status (complete, incomplete, ...)
//  [in]    lParam          [Unused, demanded by MFC signature]
//
// Returns:
//  Nothing, always returns 0
//
LRESULT CAsynchronousGrabDlg::OnFrameReady( WPARAM status, LPARAM lParam )
{
    if( true == m_ApiController.CameraIsAcq1 )
    {
        // Pick up frame for camera #1
        FramePtr pFrame = m_ApiController.GetFrame();
        if( SP_ISNULL( pFrame) )
        {
            Log( _TEXT("frame ptr is NULL, late call") );
            return 0;
        }
        // See if it is not corrupt
        if( VmbFrameStatusComplete == status )
        {
            VmbUchar_t *pBuffer;
            VmbUchar_t *pColorBuffer = NULL;
            VmbErrorType err = pFrame->GetImage( pBuffer );
            if (VmbErrorSuccess == err)
            {
                // show frame number
                VmbUint64_t nFrameID1;
                err = pFrame->GetFrameID(nFrameID1);
                if (VmbErrorSuccess == err)
                {
                    CString strFrameID1;
                    strFrameID1.Format(L"FrameID: %lld", nFrameID1);
                    SetDlgItemText(IDC_STATIC_FRAME_ID1, strFrameID1);
                }

                // show new frame from camera
                VmbUint32_t nSize;
                err = pFrame->GetImageSize(nSize);
                if (VmbErrorSuccess == err)
                {
                    VmbPixelFormatType ePixelFormat = m_ApiController.GetPixelFormat();
                    CopyToImage(pBuffer, ePixelFormat, m_Image);
                    // Display it
                    RECT rect;
                    m_PictureBoxStream.GetWindowRect(&rect);
                    ScreenToClient(&rect);
                    InvalidateRect(&rect, false);
                }


            }
        }
        else
        {
            // If we receive an incomplete image we do nothing but logging
            Log( _TEXT( "Failure in receiving image of camera #1:" ), VmbErrorOther );
        }

        // And queue it to continue streaming
        m_ApiController.QueueFrame( pFrame );
    }

    return 0;
}

LRESULT CAsynchronousGrabDlg::OnFrameReady2( WPARAM status, LPARAM lParam )
{
    if( true == m_ApiController.CameraIsAcq2 )
    {
        // Pick up frame for camera #2
        FramePtr pFrame = m_ApiController.GetFrame2();
        if( SP_ISNULL( pFrame) )
        {
            Log( _TEXT("frame ptr is NULL, late call") );
            return 0;
        }
        // See if it is not corrupt
        if( VmbFrameStatusComplete == status )
        {
            VmbUchar_t *pBuffer;
            VmbUchar_t *pColorBuffer = NULL;
            VmbErrorType err = pFrame->GetImage( pBuffer );
            if( VmbErrorSuccess == err )
            {
                // show frame number
                VmbUint64_t nFrameID2;
                err = pFrame->GetFrameID(nFrameID2);
                if (VmbErrorSuccess == err)
                {
                    CString strFrameID2;
                    strFrameID2.Format(L"FrameID: %lld", nFrameID2);
                    SetDlgItemText(IDC_STATIC_FRAME_ID2, strFrameID2);
                }

                // show frame in image box 
                VmbUint32_t nSize;
                err = pFrame->GetImageSize( nSize );
                if( VmbErrorSuccess == err )
                {
                    VmbPixelFormatType ePixelFormat = m_ApiController.GetPixelFormat2();
                    CopyToImage2( pBuffer,ePixelFormat, m_Image2 );
                    // Display it
                    RECT rect;
                    m_PictureBoxStream2.GetWindowRect( &rect );
                    ScreenToClient( &rect );
                    InvalidateRect( &rect, false );
                }
            }
        }
        else
        {
            // If we receive an incomplete image we do nothing but logging
            Log( _TEXT( "Failure in receiving image of camera #2:" ), VmbErrorOther );
        }

        // And queue it to continue streaming
        m_ApiController.QueueFrame2( pFrame );
    }

    return 0;
}
//
// This event handler is triggered through a MFC message posted by the camera observer
//
// Parameters:
//  [in]    reason          The reason why the callback of the observer was triggered (plug-in, plug-out, ...)
//  [in]    lParam          [Unused, demanded by MFC signature]
//
// Returns:
//  Nothing, always returns 0
//
LRESULT CAsynchronousGrabDlg::OnCameraListChanged( WPARAM reason, LPARAM lParam )
{
    bool bUpdateList = false;

    // We only react on new cameras being found and known cameras being unplugged
    if( AVT::VmbAPI::UpdateTriggerPluggedIn == reason )
    {
        Log( _TEXT( "Camera list changed. A new camera was discovered by Vimba." ) );
        bUpdateList = true;
    }
    else if( AVT::VmbAPI::UpdateTriggerPluggedOut == reason )
    {
        Log( _TEXT( "Camera list changed. A camera was disconnected from Vimba." ) );
        
        bUpdateList = true;
    }
    // Avoid stopping streaming cameras
    /*
	if (true == m_ApiController.CameraIsOpen1)
	{
		OnBnClickedBtOpencam1();
	}
	if (true == m_ApiController.CameraIsOpen2)
	{
		OnBnClickedBtOpencam2();
	}
    */

    if( true == bUpdateList )
    {
        UpdateCameraListBox();
    }

   // m_btOpencam1.EnableWindow( 0 < m_cameras.size());
	//m_btOpencam2.EnableWindow( 1 < m_cameras.size());
    return 0;
}

//
// Copies the content of a byte buffer to a MFC image with respect to the image's alignment
//
// Parameters:
//  [in]    pInbuffer       The byte buffer as received from the cam
//  [in]    ePixelFormat    The pixel format of the frame
//  [out]   OutImage        The filled MFC image
//
void CAsynchronousGrabDlg::CopyToImage2( VmbUchar_t *pInBuffer, VmbPixelFormat_t ePixelFormat, CImage &OutImage )
{
    const int               nHeight         = m_ApiController.GetHeight2();
    const int               nWidth          = m_ApiController.GetWidth2();
    const int               nStride         = OutImage.GetPitch();
    const int               nBitsPerPixel   = OutImage.GetBPP();
    VmbError_t              Result;
    if( ( nWidth*nBitsPerPixel ) /8 != nStride )
    {
        Log( _TEXT( "Vimba only supports stride that is equal to width." ), VmbErrorWrongType );
        return;
    }
    VmbImage                SourceImage,DestinationImage;
    SourceImage.Size        = sizeof( SourceImage );
    DestinationImage.Size   = sizeof( DestinationImage );

    SourceImage.Data        = pInBuffer;
    DestinationImage.Data   = OutImage.GetBits();

    Result = VmbSetImageInfoFromPixelFormat( ePixelFormat, nWidth, nHeight, &SourceImage );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error setting source image info." ), static_cast<VmbErrorType>( Result ) );
        return;
    }
	static const std::string DisplayFormat( "BGR24" );
    //static const std::string DisplayFormat( "BGR24" );
    Result = VmbSetImageInfoFromString( DisplayFormat.c_str(), (VmbUint32_t)DisplayFormat.size(), nWidth,nHeight, &DestinationImage );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error setting destination image info." ),static_cast<VmbErrorType>( Result ) );
        return;
    }
    Result = VmbImageTransform( &SourceImage, &DestinationImage,NULL,0 );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error transforming image." ), static_cast<VmbErrorType>( Result ) );
    }
}
void CAsynchronousGrabDlg::CopyToImage( VmbUchar_t *pInBuffer, VmbPixelFormat_t ePixelFormat, CImage &OutImage )
{
    const int               nHeight         = m_ApiController.GetHeight();
    const int               nWidth          = m_ApiController.GetWidth();
    const int               nStride         = OutImage.GetPitch();
    const int               nBitsPerPixel   = OutImage.GetBPP();
    VmbError_t              Result;
    if( ( nWidth*nBitsPerPixel ) /8 != nStride )
    {
        Log( _TEXT( "Vimba only supports stride that is equal to width." ), VmbErrorWrongType );
        return;
    }
    VmbImage                SourceImage,DestinationImage;
    SourceImage.Size        = sizeof( SourceImage );
    DestinationImage.Size   = sizeof( DestinationImage );

    SourceImage.Data        = pInBuffer;
    DestinationImage.Data   = OutImage.GetBits();

    Result = VmbSetImageInfoFromPixelFormat( ePixelFormat, nWidth, nHeight, &SourceImage );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error setting source image info." ), static_cast<VmbErrorType>( Result ) );
        return;
    }
    static const std::string DisplayFormat( "RGB24" );
    Result = VmbSetImageInfoFromString( DisplayFormat.c_str(), (VmbUint32_t)DisplayFormat.size(), nWidth,nHeight, &DestinationImage );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error setting destination image info." ),static_cast<VmbErrorType>( Result ) );
        return;
    }
    Result = VmbImageTransform( &SourceImage, &DestinationImage,NULL,0 );
    if( VmbErrorSuccess != Result )
    {
        Log( _TEXT( "Error transforming image." ), static_cast<VmbErrorType>( Result ) );
    }
}

//
// Queries and lists all known camera
//
void CAsynchronousGrabDlg::UpdateCameraListBox()
{
    // Get all cameras currently connected to Vimba
    CameraPtrVector cameras = m_ApiController.GetCameraList();

    // Simply forget about all cameras known so far
    m_ListBoxCameras.ResetContent();
    m_cameras.clear();

    // And query the camera details again
    for(    CameraPtrVector::const_iterator iter = cameras.begin();
            cameras.end() != iter;
            ++iter )
    {
        std::string strCameraName;
        std::string strCameraID;
        if( VmbErrorSuccess != (*iter)->GetName( strCameraName ) )
        {
            strCameraName = "[NoName]";
        }
        // If for any reason we cannot get the ID of a camera we skip it
        if( VmbErrorSuccess == (*iter)->GetID( strCameraID ) )
        {
            
            std::string strInfo = strCameraName + " " + strCameraID;
            m_ListBoxCameras.AddString( CString( strInfo.c_str() ) );
            m_cameras.push_back( strCameraID );
        }
        else
        {
            Log( _TEXT("Could not get camera ID") );
        }
    }

    // Select first cam if none is selected
    if (    -1 == m_ListBoxCameras.GetCurSel()
         && 0 < m_cameras.size() )
    {
        m_ListBoxCameras.SetCurSel( 0 );
    }

    m_btOpencam1.EnableWindow( 0 < m_cameras.size());
	m_btOpencam2.EnableWindow( 1 < m_cameras.size());
}

//
// Prints out a given logging string, error code and the descriptive representation of that error code
//
// Parameters:
//  [in]    strMsg          A given message to be printed out
//  [in]    eErr            The API status code
//
void CAsynchronousGrabDlg::Log( string_type strMsg, VmbErrorType eErr )
{
    strMsg += _TEXT( "..." ) + m_ApiController.ErrorCodeToMessage( eErr );
    m_ListLog.InsertString( 0, strMsg.c_str() );
}

//
// Prints out a given logging string
//
// Parameters:
//  [in]    strMsg          A given message to be printed out
//
void CAsynchronousGrabDlg::Log( string_type strMsg )
{
    m_ListLog.InsertString( 0, strMsg.c_str() );
}

//
// The remaining functions are MFC intrinsic only
//

HCURSOR CAsynchronousGrabDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>( m_hIcon );
}

void CAsynchronousGrabDlg::DoDataExchange( CDataExchange* pDX )
{
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_LIST_CAMERAS, m_ListBoxCameras );
	DDX_Control( pDX, IDC_LIST_LOG, m_ListLog );
	DDX_Control( pDX, IDC_BUTTON_STARTSTOP, m_ButtonStartStop );
	DDX_Control( pDX, IDC_PICTURE_STREAM, m_PictureBoxStream );
	DDX_Control(pDX, IDC_PICTURE_STREAM2, m_PictureBoxStream2);
	DDX_Control(pDX, IDC_BUTTON_STARTSTOP2, m_ButtonStartStop2);
	DDX_Control(pDX, IDC_SLIDER2, m_Slider1);
	DDX_Control(pDX, IDC_SLIDER3, m_Slider2);
	DDX_Text(pDX, IDC_EDIT1, packetSize1);
	DDV_MinMaxInt(pDX, packetSize1, 500, 9973);
	DDX_Control(pDX, IDC_EDIT1, m_packageEidt1);
	DDX_Control(pDX, IDC_BT_OPENCAM1, m_btOpencam1);
	DDX_Control(pDX, IDC_BT_OPENCAM2, m_btOpencam2);
	DDX_Control(pDX, IDC_EDIT2, m_packageEidt2);
	DDX_Text(pDX, IDC_EDIT2, packetSize2);
	DDV_MinMaxInt(pDX, packetSize2, 500, 9973);
}

template <typename T>
CRect fitRect( T w, T h, const CRect &dst)
{
    double sw = static_cast<double>( dst.Width() ) / w;
    double sh = static_cast<double>( dst.Height() ) / h;
    double s = min( sw, sh );
    T new_w = static_cast<T>( w * s );
    T new_h = static_cast<T>( h * s );
    T off_w = (1 + dst.Width() - new_w) /2;
    T off_h = (1 + dst.Height() - new_h) /2;
    return CRect(off_w,off_h, off_w + new_w, off_h + new_h );
}

void CAsynchronousGrabDlg::OnPaint()
{
    if( IsIconic() )
    {
        CPaintDC dc( this );

        SendMessage( WM_ICONERASEBKGND, reinterpret_cast<WPARAM>( dc.GetSafeHdc() ), 0 );

        int cxIcon = GetSystemMetrics( SM_CXICON );
        int cyIcon = GetSystemMetrics( SM_CYICON );
        CRect rect;
        GetClientRect( &rect );
        int x = ( rect.Width() - cxIcon + 1 ) / 2;
        int y = ( rect.Height() - cyIcon + 1 ) / 2;
        dc.DrawIcon( x, y, m_hIcon );
    }
    else
    {
        CDialog::OnPaint();

        if( NULL != m_Image )
        {
            CPaintDC dc( &m_PictureBoxStream );
            CRect rect;
            m_PictureBoxStream.GetClientRect( &rect );
            if( m_ClearBackground)
            {
                m_ClearBackground = false;
                CBrush clearBrush( GetSysColor( COLOR_BTNFACE) );
                dc.FillRect( rect, &clearBrush);
            }
            rect = fitRect( m_Image.GetWidth(), m_Image.GetHeight(), rect );
            // HALFTONE enhances image quality but decreases performance
            dc.SetStretchBltMode( HALFTONE );
            m_Image.StretchBlt( dc.m_hDC, rect );
        }
		if( NULL != m_Image2 )
        {
            CPaintDC dc( &m_PictureBoxStream2 );
            CRect rect;
            m_PictureBoxStream2.GetClientRect( &rect );
            if( m_ClearBackground2)
            {
                m_ClearBackground2 = false;
                CBrush clearBrush( GetSysColor( COLOR_BTNFACE) );
                dc.FillRect( rect, &clearBrush);
            }
            rect = fitRect( m_Image2.GetWidth(), m_Image2.GetHeight(), rect );
            // HALFTONE enhances image quality but decreases performance
            dc.SetStretchBltMode( HALFTONE );
            m_Image2.StretchBlt( dc.m_hDC, rect );
        }
    }
}

void CAsynchronousGrabDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if( pScrollBar->GetDlgCtrlID() == IDC_SLIDER2 ) 
	{
		CSliderCtrl   *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER2);
		//m_int 即为当前滑块的值。
		int m_int =1*pSlidCtrl->GetPos();//取得当前位置值
		float m_float=(float)m_int;
		//m_ApiController.SetCameraFloatFeature(1,"ExposureTimeAbs",m_float);
	}
	else if  (pScrollBar->GetDlgCtrlID() == IDC_SLIDER3 )
	{
		CSliderCtrl   *pSlidCtrl=(CSliderCtrl*)GetDlgItem(IDC_SLIDER3);
		//m_int 即为当前滑块的值。
		int m_int =1*pSlidCtrl->GetPos();//取得当前位置值
		float m_float=(float)m_int;
		//m_ApiController.SetCameraFloatFeature(2,"ExposureTimeAbs",m_float);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CAsynchronousGrabDlg::OnEnChangeEdit1()
{
	UpdateData(TRUE);
	//m_ApiController.SetCameraIntFeature(1,"GevSCPSPacketSize",packetSize1);

}

void CAsynchronousGrabDlg::OnEnChangeEdit2()
{
	UpdateData(TRUE);
	//m_ApiController.SetCameraIntFeature(2,"GevSCPSPacketSize",packetSize2);
	
}

void CAsynchronousGrabDlg::OnBnClickedBtOpencam1()
{
	if (m_cameras.size() >= 1)
	{
		m_ApiController.OpenCloseCamera1(m_cameras[0]);
		UpdateContronls();
	}
    else
    {
		Log(_TEXT("Can not find #1 camera."));
    }
}

void CAsynchronousGrabDlg::OnBnClickedBtOpencam2()
{
	if (m_cameras.size() >= 2)
	{
		m_ApiController.OpenCloseCamera2(m_cameras[1]);
		UpdateContronls();
	}
	else
	{
		Log(_TEXT("Can not find #2 camera."));
	}
}

void CAsynchronousGrabDlg::UpdateContronls()
{
	if (m_ApiController.CameraIsOpen1)//只有相机打开后才能进行相关的参数设置
	{
		m_btOpencam1.SetWindowText( _TEXT( "CloseCamera #1" ) );
		m_ButtonStartStop.EnableWindow(true);		
		
		m_Slider1.EnableWindow(true);
		if (m_ApiController.CameraIsAcq1)//采集状态下PackageSize是不能进行设置的
		{
			m_ButtonStartStop.SetWindowText( _TEXT( "Stop Image Acquisition #1" ) );
			m_packageEidt1.EnableWindow(false);
		}
		else
		{
		    m_ButtonStartStop.SetWindowText( _TEXT( "Start Image Acquisition #1" ) );
			m_packageEidt1.EnableWindow(true);
		}
	}
	else
	{
		m_btOpencam1.SetWindowText( _TEXT( "OpenCamera #1" ) );
		m_ButtonStartStop.SetWindowText( _TEXT( "Start Image Acquisition #1" ) );
	    m_ButtonStartStop.EnableWindow(false);
		m_Slider1.EnableWindow(false);
		m_packageEidt1.EnableWindow(false);
	}

	if (m_ApiController.CameraIsOpen2)//只有相机打开后才能进行相关的参数设置
	{
		m_btOpencam2.SetWindowText( _TEXT( "CloseCamera #2" ) );
		m_ButtonStartStop2.EnableWindow(true);		
		
		m_Slider2.EnableWindow(true);
		if (m_ApiController.CameraIsAcq2)//采集状态下PackageSize是不能进行设置的
		{
			m_ButtonStartStop2.SetWindowText( _TEXT( "Stop Image Acquisition #2" ) );
			m_packageEidt2.EnableWindow(false);
		}
		else
		{
		    m_ButtonStartStop2.SetWindowText( _TEXT( "Start Image Acquisition #2" ) );
			m_packageEidt2.EnableWindow(true);
		}
	}
	else
	{
		m_btOpencam2.SetWindowText( _TEXT( "OpenCamera #2" ) );
		m_ButtonStartStop2.SetWindowText( _TEXT( "Start Image Acquisition #2" ) );
	    m_ButtonStartStop2.EnableWindow(false);
		m_Slider2.EnableWindow(false);
		m_packageEidt2.EnableWindow(false);
	}
}




