/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        AsynchronousGrab.cpp

  Description: CAsynchronousGrabApp is the main entry class for the
               AsynchronousGrab example for VimbaCPP.

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

#include "stdafx.h"
#include "AsynchronousGrab.h"
#include "AsynchronousGrabDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAsynchronousGrabApp

BEGIN_MESSAGE_MAP( CAsynchronousGrabApp, CWinApp )
    ON_COMMAND( ID_HELP, &CWinApp::OnHelp )
END_MESSAGE_MAP()


// CAsynchronousGrabApp construction

CAsynchronousGrabApp::CAsynchronousGrabApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}


// The one and only CAsynchronousGrabApp object

CAsynchronousGrabApp theApp;


// CAsynchronousGrabApp initialization

BOOL CAsynchronousGrabApp::InitInstance()
{
    // InitCommonControlsEx() is required on Windows XP if an application
    // manifest specifies use of ComCtl32.dll version 6 or later to enable
    // visual styles.  Otherwise, any window creation will fail.
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof( InitCtrls );
    // Set this to include all the common control classes you want to use
    // in your application.
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx( &InitCtrls );

    CWinApp::InitInstance();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    // of your final executable, you should remove from the following
    // the specific initialization routines you do not need
    // Change the registry key under which our settings are stored
    // TODO: You should modify this string to be something appropriate
    // such as the name of your company or organization
    SetRegistryKey( _T( "Local AppWizard-Generated Applications" ) );

    CAsynchronousGrabDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if( nResponse == IDOK )
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with OK
    }
    else if( nResponse == IDCANCEL )
    {
        // TODO: Place code here to handle when the dialog is
        //  dismissed with Cancel
    }

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}
