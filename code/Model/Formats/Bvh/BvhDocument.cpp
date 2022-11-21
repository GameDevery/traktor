/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/AnsiEncoding.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Model/Formats/Bvh/BvhDocument.h"
#include "Model/Formats/Bvh/BvhJoint.h"

namespace traktor::model
{
	namespace
	{

bool parseGroup(StringReader& sr, int32_t& channelCount, BvhJoint* joint)
{
	std::wstring str;

	if (sr.readLine(str) <= 0 || trim(str) != L"{")
		return false;

	for (;;)
	{
		if (sr.readLine(str) <= 0)
			return false;

		str = trim(str);

		if (startsWith(str, L"OFFSET"))
		{
			std::vector< float > fv;
			if (Split< std::wstring, float >::any(str.substr(7), L" ", fv) >= 3)
			{
				joint->setOffset(Vector4(
					fv[0],
					fv[1],
					fv[2]
				));
			}
		}
		else if (startsWith(str, L"CHANNELS"))
		{
			std::vector< std::wstring > sv;
			if (Split< std::wstring >::any(str.substr(9), L" ", sv) >= 1)
			{
				const int32_t nchannels = parseString< int32_t >(sv[0]);
				if (nchannels != int32_t(sv.size() - 1))
					return false;

				joint->setChannelOffset(channelCount);
				channelCount += nchannels;

				for (int32_t i = 0; i < nchannels; ++i)
					joint->addChannel(sv[i + 1]);
			}
		}
		else if (startsWith(str, L"JOINT"))
		{
			Ref< BvhJoint > childJoint = new BvhJoint(str.substr(6));
			if (!parseGroup(sr, channelCount, childJoint))
				return false;
			joint->addChild(childJoint);
		}
		else if (str == L"End Site")
		{
			Ref< BvhJoint > childJoint = new BvhJoint(L"");
			if (!parseGroup(sr, channelCount, childJoint))
				return false;
			joint->addChild(childJoint);
		}
		else if (str == L"}")
			break;
	}

	return true;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BvhDocument", BvhDocument, Object)

Ref< BvhDocument > BvhDocument::parse(IStream* stream)
{
	BufferedStream bs(stream);
	StringReader sr(&bs, new AnsiEncoding());
	std::wstring str;

	if (sr.readLine(str) <= 0 || str != L"HIERARCHY")
		return nullptr;
	if (sr.readLine(str) <= 0 || !startsWith(str, L"ROOT "))
		return nullptr;

	Ref< BvhDocument > document = new BvhDocument();
	document->m_rootJoint = new BvhJoint(str.substr(5));

	int32_t channelCount = 0;
	if (!parseGroup(sr, channelCount, document->m_rootJoint))
		return nullptr;

	if (sr.readLine(str) <= 0 || str != L"MOTION")
		return nullptr;
	if (sr.readLine(str) <= 0 || !startsWith(str, L"Frames:"))
		return document;
	if (sr.readLine(str) <= 0 || !startsWith(str, L"Frame Time:"))
		return nullptr;

	document->m_frameTime = parseString< float >(trim(str.substr(11)));

	while (sr.readLine(str) >= 0)
	{
		std::vector< float > fv;
		Split< std::wstring, float >::any(str, L" ", fv);

		if (int32_t(fv.size()) != channelCount)
			return nullptr;

		document->m_channelValues.push_back(fv);
	}

	return document;
}

BvhJoint* BvhDocument::getRootJoint() const
{
	return m_rootJoint;
}

float BvhDocument::getFrameTime() const
{
	return m_frameTime;
}

const BvhDocument::cv_list_t& BvhDocument::getChannelValues() const
{
	return m_channelValues;
}

}
