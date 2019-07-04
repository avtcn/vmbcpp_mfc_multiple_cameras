/*=============================================================================
  Copyright (C) 2012 - 2016 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AsynchronousGrabDlg.h

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

#pragma once
#include <afxwin.h>
#include <vector>
#include <atlimage.h>
#include <VimbaCPP/Include/VimbaCPP.h>
#include <ApiController.h>
#include "afxcmn.h"
using AVT::VmbAPI::Examples::ApiController;

class CAsynchronousGrabDlg : public CDialog
{
public:
    CAsynchronousGrabDlg( CWnd* pParent = NULL );

    enum { IDD = IDD_ASYNCHRONOUSGRAB_DIALOG };

protected:
    virtual void DoDataExchange( CDataExchange* pDX );

protected:
    HICON m_hIcon;

    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand( UINT nID, LPARAM lParam );
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedButtonStartstop();
    DECLARE_MESSAGE_MAP()

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
	
    afx_msg LRESULT OnFrameReady( WPARAM status, LPARAM lParam );
    afx_msg LRESULT OnFrameReady2( WPARAM status, LPARAM lParam );
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
    afx_msg LRESULT OnCameraListChanged( WPARAM reason, LPARAM lParam );

private:
    // Our controller that wraps API access
    ApiController m_ApiController;
    // A list of known camera IDs
    std::vector<std::string> m_cameras;
    // Are we streaming?
   // bool m_bIsStreaming;
	 // Are we streaming?
    //bool m_bIsStreaming2;
    // Our MFC image to display
    CImage m_Image;
	CImage m_Image2;
    // on first call we clear back
    bool m_ClearBackground;
	bool m_ClearBackground2;
    //
    // Queries and lists all known camera
    //
    void UpdateCameraListBox();
    void UpdateContronls();
    //
    // Prints out a given logging string, error code and the descriptive representation of that error code
    //
    // Parameters:
    //  [in]    strMsg          A given message to be printed out
    //  [in]    eErr            The API status code
    //
    void Log( string_type strMsg, VmbErrorType eErr );
    
    //
    // Prints out a given logging string
    //
    // Parameters:
    //  [in]    strMsg          A given message to be printed out
    //
    void Log( string_type strMsg);
    
    //
    // Copies the content of a byte buffer to a MFC image with respect to the image's alignment
    //
    // Parameters:
    //  [in]    pInbuffer       The byte buffer as received from the cam
    //  [in]    ePixelFormat    The pixel format of the frame
    //  [out]   OutImage        The filled MFC image
    //
    void CopyToImage( VmbUchar_t *pInBuffer, VmbPixelFormat_t PixelFormat, CImage &pOutImage );
	void CopyToImage2( VmbUchar_t *pInBuffer, VmbPixelFormat_t PixelFormat, CImage &pOutImage );
    // MFC Controls
    CListBox m_ListBoxCameras;
    CListBox m_ListLog;
    CButton m_ButtonStartStop;
    CStatic m_PictureBoxStream;
public:
	CStatic m_PictureBoxStream2;
	CButton m_ButtonStartStop2;
	afx_msg void OnBnClickedButtonStartstop2();
	CSliderCtrl m_Slider1;
	
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CSliderCtrl m_Slider2;
	int packetSize1;
	afx_msg void OnEnChangeEdit1();
	CEdit m_packageEidt1;
	CButton m_btOpencam1;
	afx_msg void OnBnClickedBtOpencam1();
	CButton m_btOpencam2;
	CEdit m_packageEidt2;
	int packetSize2;
	afx_msg void OnBnClickedBtOpencam2();
	afx_msg void OnEnChangeEdit2();
};

