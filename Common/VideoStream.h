/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        VideoStream.h

  Description: Helper class for recording video files. Utilizes libav

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

#ifndef AVT_VMBAPI_VIDEOSTREAM_H
#define AVT_VMBAPI_VIDEOSTREAM_H

#include "Lib/Include/libavcodec/avcodec.h"
#include "Lib/Include/libavformat/avformat.h"

#include "VimbaCPP/Include/VimbaCPP.h"

namespace AVT {
namespace VmbAPI {

class VideoStream
{
  public:
    // Purpose:
    //  C'tor
    VideoStream();

    // Purpose
    //  Dtor
    //
    // Details:
    //  Performs cleanup if necessary
    ~VideoStream();

    // Purpose:
    //  Prepare the video encoding by setting all necessary libav parameters and allocating resources
    //
    // Parameters:
    //  [ in]   FileName        The path of the resulting video file. Will be overwritten if already existing
    //  [ in]   Width           The width of the video in- and output
    //  [ in]   Height          The height of the video in- and output
    //  [ in]   FPS             The frame rate of the video in- and output
    //
    // Returns:
    //  VmbErrorSuccess         Everything is OK
    //  VmbErrorBadParameter    Bad input parameter, e.g. NULL
    //  VmbErrorInvalidCall     Already initialized
    //  VmbErrorNotFound        Codec not found in libAV
    //  VmbErrorResources       Not enough resources for allocation
    //  VmbErrorInternalFault   Libav could not prepare the encoding
    VmbErrorType Initialize( const char* FileName, const uint32_t Width, const uint32_t Height, const uint32_t FPS );

    // Purpose:
    //  Encodes a given frame with the previously set up codec parameters and muxes it to the output file
    //
    // Parameters:
    //  [ in]   pFrame          A shared pointer to a Vimba frame
    //
    // Returns:
    //  VmbErrorSuccess         Everything is OK
    //  VmbErrorInvalidCall     VideoStream::Initialize() was not called before
    //  VmbErrorBadParameter    Bad input parameter, e.g. NULL
    //  VmbErrorResources       Not enough resources for allocation
    //
    // Details:
    //  VideoStream::Initialize() has to be called before
    //  If the input frame's parameters have been changed after VideoStream::Initialize() was called,
    //  the behavior is undefined
    VmbErrorType Encode( const FramePtr pFrame );

    // Purpose:
    //  Finalize the encoded video by closing the video file and freeing libav resources
    //
    // Returns:
    //  VmbErrorSuccess         Everything is OK
    //  VmbErrorInvalidCall     Not initialized or already finalized
    VmbErrorType Finalize();


  private:
    typedef struct
    {
        const char*         CodecString;
        const AVCodecID     CodecID;
        const AVPixelFormat PixelFormat;
        const int           BitRate;
        const int           BiteRateTolerance;
    } VideoStreamSettings;

    static VideoStreamSettings  m_Settings;             // Basic settings for all instances
    bool                        m_IsVirgin;             // Did we initialize everything correctly?
    AVStream*                   m_pStream;              // Our video stream with all information about codec, format, bit rate ...
    AVFormatContext*            m_pFormatCtx;           // Our container context for muxing
    int64_t                     m_NextTimeStamp;        // The presentation time stamp in relation to the fps
    AVFrame*                    m_pFrame;               // A single reusable frame
    VmbUchar_t*                 m_pConvBuffer;          // A reusable buffer for pixel format conversion

    // Purpose:
    //  Converts an libav error to an Vimba error
    //
    // Parameters:
    //  ErrNo       The libav error code to convert
    //
    // Returns:
    //  The Vimba error code
    VmbErrorType LibavToVimbaError ( const int ErrNo );

    VideoStream( const VideoStream& );
    VideoStream& operator=( const VideoStream& );
}; // VideoStream

}} // AVT::Vimba
#endif