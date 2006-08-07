/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006 Mathieu Malaterre
  Copyright (c) 1993-2005 CREATIS
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmJPEGCodec.h"
#include "gdcmTS.h"
#include "gdcmTrace.h"
#include "gdcmIStream.h"
#include "gdcmStringStream.h"

#include "gdcmJPEG8Codec.h"
#include "gdcmJPEG12Codec.h"
#include "gdcmJPEG16Codec.h"

namespace gdcm
{

JPEGCodec::JPEGCodec():BitSample(0)
{
  Internal = NULL;
}

JPEGCodec::~JPEGCodec()
{
  delete Internal;
}

bool JPEGCodec::CanDecode(TS const &ts)
{
  return ts.GetCompressionType() == Compression::JPEG;
}

void JPEGCodec::SetPixelType(PixelType const &pt)
{
  //SetBitSample( pt.GetBitsAllocated() );
  SetBitSample( pt.GetBitsStored() );
  ImageCodec::SetPixelType(pt);
}

void JPEGCodec::SetBitSample(int bit)
{
  BitSample = bit;
  assert( Internal == NULL );
  if ( BitSample <= 8 )
    {
    Internal = new JPEG8Codec;
    }
  else if ( BitSample <= 12 )
    {
    Internal = new JPEG12Codec;
    }
  else if ( BitSample <= 16 )
    {
    Internal = new JPEG16Codec;
    }
  else
    {
    gdcmWarningMacro( "bla" );
    // Clearly make sure Internal will not be used
    delete Internal;
    Internal = NULL;
    }
}

bool JPEGCodec::Decode(IStream &is, OStream &os)
{
  StringStream tmpos;
  if ( !Internal->Decode(is,tmpos) )
    {
#ifdef GDCM_SUPPORT_BROKEN_IMPLEMENTATION
    // let's check if this is one of those buggy lossless JPEG
    if( this->BitSample != Internal->BitSample )
      {
      // MARCONI_MxTWin-12-MONO2-JpegLossless-ZeroLengthSQ.dcm
      // PHILIPS_Gyroscan-12-MONO2-Jpeg_Lossless.dcm
      gdcmWarningMacro( "DICOM header said it was " << this->BitSample <<
        " but JPEG header says it's: " << Internal->BitSample );
      delete Internal;
      is.Seekg(0, std::ios::beg);
      switch( Internal->BitSample )
        {
      case 8:
        Internal = new JPEG8Codec;
        break;
      case 12:
        Internal = new JPEG12Codec;
        break;
      case 16:
        Internal = new JPEG16Codec;
        break;
      default:
        abort();
        }
      if( Internal->Decode(is,tmpos) )
        {
        return ImageCodec::Decode(tmpos,os);
        }
      }
#endif
    return false;
    }

  return ImageCodec::Decode(tmpos,os);
}

} // end namespace gdcm
