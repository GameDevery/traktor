/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_IndexRange_H
#define traktor_mesh_IndexRange_H

#include "Core/Config.h"
#include "Core/Containers/AlignedVector.h"

namespace traktor
{
	namespace mesh
	{

struct IndexRange
{
	int32_t offsetFirst;
	int32_t offsetLast;
	int32_t minIndex;
	int32_t maxIndex;
	bool opaque;

	IndexRange();

	void mergeInto(AlignedVector< IndexRange >& ranges) const;

	bool operator < (const IndexRange& rh) const;
};

	}
}

#endif	// traktor_mesh_IndexRange_H
