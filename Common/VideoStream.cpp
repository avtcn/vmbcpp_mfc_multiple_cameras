/*=============================================================================
  Copyright (C) 2015 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        VideoStream.cpp

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

#include "VmbTransform.h"

#include "VideoStream.h"

using namespace AVT::VmbAPI;

// Set basic codec settings
VideoStream::VideoStreamSettings VideoStream::m_Settings = {    "MPEG2VIDEO",               // Codec string
                                                                AV_CODEC_ID_MPEG2VIDEO,     // Codec ID
                                                                AV_PIX_FMT_YUV422P,         // Pixel format
                                                                400000,                     // Bitrate
                                                                1000000 };                  // Bitrate tolerance

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
//
// Details:
//  When encoding has finished, call VideoStream::Finalize() to free resources and close the output file
VmbErrorType VideoStream::Initialize( const char* FileName, const uint32_t Width, const uint32_t Height, const uint32_t FPS )
{
    VmbErrorType res = VmbErrorSuccess;
    int err = 0;

    // Sanity checks
    if ( !m_IsVirgin )
    {
        return VmbErrorInvalidCall;
    }
    if (    NULL == FileName
         || 0 >= FPS )
    {
        return VmbErrorBadParameter;
    }

    // Init
    m_pFormatCtx = NULL;
    m_pStream = NULL;
    m_pFrame = NULL;
    m_pConvBuffer = NULL;
    m_NextTimeStamp = 0;

    // Alloc output container
    res = LibavToVimbaError( avformat_alloc_output_context2( &m_pFormatCtx, NULL, m_Settings.CodecString, FileName ));
    if (    VmbErrorSuccess <= res
         && m_pFormatCtx )
    {
        // Find the video encoder
        AVCodec* pCodec = avcodec_find_encoder( m_Settings.CodecID );
        if ( pCodec )
        {
            // Register it for use
            avcodec_register( pCodec );
            // Alloc new stream
            m_pStream = avformat_new_stream( m_pFormatCtx, pCodec );
            if ( m_pStream )
            {
                // Set stream ID to number of streams - 1
                m_pStream->id = m_pFormatCtx->nb_streams - 1;
                // Set the stream's codec
                m_pStream->codec->codec_id = m_Settings.CodecID;
                // Set pixel format
                m_pStream->codec->pix_fmt = m_Settings.PixelFormat;
                // Set sample parameters
                // TODO: calculate bitrate
                m_pStream->codec->bit_rate = m_Settings.BitRate;
                m_pStream->codec->bit_rate_tolerance = m_Settings.BiteRateTolerance;
                // Resolution must be a multiple of two
                m_pStream->codec->width = Width;
                m_pStream->codec->height = Height;
                // Set the fps in notation 1 / frames per second
                m_pStream->time_base.num = 1;
                m_pStream->time_base.den = FPS;
                m_pStream->codec->time_base = m_pStream->time_base;
                // Group of pictures = 10
                m_pStream->codec->gop_size = 10;
                // Number of max. consecutive bidirectionally predicted frames
                m_pStream->codec->max_b_frames = 2;
                // Some formats want stream headers to be separate
                if ( m_pFormatCtx->oformat->flags & AVFMT_GLOBALHEADER )
                {
                    m_pStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                }
                // Open the codec
                res = LibavToVimbaError( avcodec_open2( m_pStream->codec, pCodec, NULL ));
                if ( VmbErrorSuccess == res )
                {
                    // Allocate and init a re-usable frame
                    m_pFrame = av_frame_alloc();
                    if ( m_pFrame )
                    {
                        m_pFrame->format = m_pStream->codec->pix_fmt;
                        m_pFrame->width = m_pStream->codec->width;
                        m_pFrame->height = m_pStream->codec->height;
                        // Allocate some buffer we might reuse later. We assume a bit depth of 16 bpp
                        try
                        {
                            m_pConvBuffer = new VmbUchar_t[m_pFrame->width * m_pFrame->height * 2];
                        }
                        catch ( ... )
                        {
                            res = VmbErrorResources;
                        }
                        if ( VmbErrorSuccess == res )
                        {
                            // Allocate the video buffer
                            res = LibavToVimbaError( av_frame_get_buffer( m_pFrame, 32 ));
                            if ( VmbErrorSuccess == res )
                            {
                                // Print output format
                                av_dump_format( m_pFormatCtx, 0, FileName, 1 );
                                // TODO: remove?
                                if ( !(m_pFormatCtx->flags & AVFMT_NOFILE) )
                                {
                                    // Open the output file
                                    res = LibavToVimbaError( avio_open( &m_pFormatCtx->pb, FileName, AVIO_FLAG_WRITE ));
                                    if ( VmbErrorSuccess == res )
                                    {
                                        // Write the stream header, if any
                                        res = LibavToVimbaError( avformat_write_header( m_pFormatCtx, NULL ));
                                        if ( VmbErrorSuccess == res )
                                        {
                                            // Initialization successfully completed
                                            m_IsVirgin = false;
                                        }
                                    }
                                }
                                else
                                {
                                    res = VmbErrorResources;
                                }
                            }
                        }
                    }
                    else
                    {
                        res = VmbErrorResources;
                    }
                }
            }
            else
            {
                res = VmbErrorResources;
            }
        }
        else
        {
            res = VmbErrorNotFound;
        }
    }

    return res;
}


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
VmbErrorType VideoStream::Encode( const FramePtr pFrame )
{
    VmbErrorType res = VmbErrorSuccess;

    // Sanity checks
    if ( m_IsVirgin )
    {
        return VmbErrorInvalidCall;
    }
    if ( SP_ISNULL( pFrame ))
    {
        return VmbErrorBadParameter;
    }

    // Get the input pixel format
    VmbPixelFormatType SrcPixelFormat;
    res = SP_ACCESS( pFrame )->GetPixelFormat( SrcPixelFormat );
    if ( VmbErrorSuccess == res )
    {
        // A pointer to loop over the input buffer
        VmbUchar_t* pImageBuffer;
        res = pFrame->GetImage( pImageBuffer );
        if ( VmbErrorSuccess == res )
        {
            // The source represented as VmbImage
            VmbImage SrcImage;
            SrcImage.Data = pImageBuffer;
            // Temp image needed for intermediate pixel format transformation
            VmbImage IntermImage;
            IntermImage.Data = m_pConvBuffer;
            SrcImage.Size = IntermImage.Size = sizeof VmbImage;

            // Define the pixel format of the source image (sent from the camera)
            res = (VmbErrorType)VmbSetImageInfoFromPixelFormat(	SrcPixelFormat,
                                                                m_pFrame->width,
                                                                m_pFrame->height,
                                                                &SrcImage );
            if ( VmbErrorSuccess == res )
            {
                // The size in Bytes of a single line
                VmbUint64_t LineSize = 0;
                // The output pixel format will be planar YUV422 ( YYYY ... UU ... VV ... )
                // The best matching input format we have is IIDC YUV422 ( UYV | Y | UYV | Y )
                // Perform a first conversion to IIDC YUV422 if necessary
                if ( VmbPixelFormatYuv422 != SrcPixelFormat )
                {
                    // Define the pixel format of the intermediate image
                    res = (VmbErrorType)VmbSetImageInfoFromPixelFormat( VmbPixelFormatYuv422,
                                                                        m_pFrame->width,
                                                                        m_pFrame->height,
                                                                        &IntermImage );
                    if ( VmbErrorSuccess == res )
                    {
                        LineSize = IntermImage.ImageInfo.Stride * IntermImage.ImageInfo.PixelInfo.BitsPerPixel / 8;

                        if ( VmbErrorSuccess == res )
                        {
                            // Convert to IIDC YUV422
                            res = (VmbErrorType)VmbImageTransform( &SrcImage, &IntermImage, NULL, 0 );
                            pImageBuffer = (VmbUchar_t*)IntermImage.Data;
                        }
                    }
                }
                else
                {
                    LineSize = SrcImage.ImageInfo.Stride * SrcImage.ImageInfo.PixelInfo.BitsPerPixel / 8;
                }

                if ( VmbErrorSuccess == res )
                {
                    // Assure the frame is writable
                    res = LibavToVimbaError( av_frame_make_writable( m_pFrame ));
                    if ( VmbErrorSuccess == res )
                    {
                        // Convert to planar YUV422
                        // Y
                        for ( int y=0; y<m_pFrame->height; y++ )
                        {
                            for ( int x=0; x<m_pFrame->width; x++ )
                            {
                                m_pFrame->data[0][y * m_pFrame->linesize[0] + x] = pImageBuffer[y * LineSize + x*2 + 1];
                            }
                        }

                        // Cb and Cr
                        // Monochrome requires blue == red == 128
                        if ( VmbPixelFormatMono8 == SrcPixelFormat )
                        {
                            memset( m_pFrame->data[1], 128, m_pFrame->height * m_pFrame->linesize[1] );
                            memset( m_pFrame->data[2], 128, m_pFrame->height * m_pFrame->linesize[2] );
                        }
                        else
                        {
                            for ( int y=0; y<m_pFrame->height; y++ )
                            {
                                for( int x=0; x<m_pFrame->width; x++ )
                                {
                                    if ( !(x % 2) )
                                    {
                                        m_pFrame->data[1][y * m_pFrame->linesize[1] + x/2] = pImageBuffer[y * LineSize + x*2];
                                        m_pFrame->data[2][y * m_pFrame->linesize[2] + x/2] = pImageBuffer[y * LineSize + x*2 + 2];
                                    }
                                }
                            }
                        }

                        // Increase the presentation time stamp
                        m_pFrame->pts = m_NextTimeStamp++;

                        // Encode the image
                        int PacketConverted = 0;
                        AVPacket Packet = { 0 };
                        av_init_packet( &Packet );
                        res = LibavToVimbaError( avcodec_encode_video2( m_pStream->codec, &Packet, m_pFrame, &PacketConverted ));
                        if (    VmbErrorSuccess == res
                             && PacketConverted )
                        {
                            // Rescale the packet timestamp value from codec to stream timebase
                            av_packet_rescale_ts( &Packet, m_pStream->codec->time_base, m_pStream->time_base );
                            Packet.stream_index = m_pStream->index;
                            // Write the compressed frame to the media file
                            res = LibavToVimbaError( av_interleaved_write_frame( m_pFormatCtx, &Packet ));
                        }
                    }
                }
            }
        }
    }

    return res;
}


// Purpose:
//  Finalize the encoded video by closing the video file and freeing libav resources
//
// Returns:
//  VmbErrorSuccess         Everything is OK
//  VmbErrorInvalidCall     Not initialized or already finalized
VmbErrorType VideoStream::Finalize()
{
    VmbErrorType res = VmbErrorSuccess;
    int err = 0;

    // Sanity checks
    if ( m_IsVirgin )
    {
        return VmbErrorInvalidCall;
    }

    // Write the trailer, if any
    err = av_write_trailer( m_pFormatCtx );
    if ( err )
    {
        // We report the last error to the user
        res = LibavToVimbaError( err );
    }
    // Close the codec
    avcodec_close( m_pStream->codec );
    // Free the codec context
    // TOOD: remove?
    avcodec_free_context( &m_pStream->codec );
    // Free the frame
    av_frame_free( &m_pFrame );
    // Free the conversion buffer
    if ( m_pConvBuffer )
    {
        delete m_pConvBuffer;
    }
    if ( !(m_pFormatCtx->oformat->flags & AVFMT_NOFILE ))
    {
        // Close the output file
        err = avio_closep( &m_pFormatCtx->pb );
        if ( err )
        {
            // We report the last error to the user
            res = LibavToVimbaError( err );
        }
    }
    // Free the format context
    //avformat_free_context( m_pFormatCtx );
    m_IsVirgin = true;

    return res;
}


// Purpose:
//  Converts an libav error to an Vimba error
//
// Parameters:
//  ErrNo       The libav error code to convert
//
// Returns:
//  The Vimba error code
VmbErrorType VideoStream::LibavToVimbaError ( const int ErrNo )
{
    if ( 0 <= ErrNo )
    {
        return VmbErrorSuccess;
    }
    else
    {
        switch ( AVERROR( ErrNo ))
        {
        case EIO:
        case ENOMEM:    
        case EADDRINUSE:
        case EADDRNOTAVAIL:
            return VmbErrorResources;
            break;
        case EINVAL:
            return VmbErrorBadParameter;
            break;
        case EACCES:
            return VmbErrorInvalidAccess;
            break;
        case EBUSY:
        case EDEADLK:
        case EWOULDBLOCK:
            return VmbErrorInvalidCall;
            break;
        case ERANGE:
            return VmbErrorInvalidValue;
            break;
        case ETIMEDOUT:
            return VmbErrorTimeout;
            break;
        default:
            return VmbErrorInternalFault;
            break;
        }
    }
}


// Purpose:
//  Ctor
VideoStream::VideoStream()
    :   m_IsVirgin( true )
{
    // Register all known muxers
    av_register_all();
}


// Purpose
//  Dtor
//
// Details:
//  Performs cleanup if necessary
VideoStream::~VideoStream()
{
    Finalize();
}


VideoStream::VideoStream( const VideoStream& )
{
    // No copy ctor
}


VideoStream& VideoStream::operator=( const VideoStream& )
{
    // No assignment operator
    return *this;
}