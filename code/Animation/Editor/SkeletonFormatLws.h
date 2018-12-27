/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#pragma once

#include "Animation/Editor/SkeletonFormat.h"

namespace traktor
{
	namespace animation
	{

class LwsDocument;

/*! \brief
 * \ingroup Animation
 */
class SkeletonFormatLws : public SkeletonFormat
{
	T_RTTI_CLASS;

public:
	Ref< Skeleton > create(LwsDocument* document) const;

	virtual void getExtensions(std::wstring& outDescription, std::vector< std::wstring >& outExtensions) const override final;

	virtual bool supportFormat(const std::wstring& extension) const override final;

	virtual Ref< Skeleton > read(IStream* stream, const Vector4& offset, float scale, float radius, bool invertX, bool invertZ) const override final;
};

	}
}

