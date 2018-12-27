/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_drawing_ImageFormatJpeg_H
#define traktor_drawing_ImageFormatJpeg_H

#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

class ImageFormatJpegImpl;

/*! \brief JPEG format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatJpeg : public IImageFormat
{
	T_RTTI_CLASS;

public:
	ImageFormatJpeg();

	virtual ~ImageFormatJpeg();

	virtual Ref< Image > read(IStream* stream) override final;

	virtual bool write(IStream* stream, Image* image) override final;

	bool readJpegHeader(IStream* stream);

	Ref< Image > readJpegImage(IStream* stream);

private:
	ImageFormatJpegImpl* m_impl;
};

	}
}

#endif	// traktor_drawing_ImageFormatJpeg_H
