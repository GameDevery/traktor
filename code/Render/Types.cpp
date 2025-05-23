/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/Types.h"

#include "Core/Containers/StaticMap.h"
#include "Core/Misc/String.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"

#include <algorithm>

namespace traktor::render
{
namespace
{

class HandleRegistry
{
public:
	HandleRegistry()
		: m_nextUnusedHandle(1)
	{
	}

	handle_t getHandle(const std::wstring& name)
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		const auto it = m_handles.find(name);
		if (it != m_handles.end())
			return it->second;
		const handle_t handle = m_nextUnusedHandle++;
		m_handles.insert(std::make_pair(name, handle));
		return handle;
	}

	std::wstring getName(handle_t handle) const
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		for (const auto it : m_handles)
			if (it.second == handle)
				return it.first;
		return L"";
	}

private:
	mutable Semaphore m_lock;
	StaticMap< std::wstring, handle_t, 32768 > m_handles;
	handle_t m_nextUnusedHandle;
};

HandleRegistry* s_handleRegistry = nullptr;

struct TextureFormatInfo
{
	const wchar_t* name;
	uint32_t blockSize;
	uint32_t blockDenom;
} c_textureFormatInfo[] = {
	{ L"TfInvalid", 0, 0 },

	{ L"TfR8", 1, 1 },
	{ L"TfR8G8B8A8", 4, 1 },
	{ L"TfR5G6B5", 2, 1 },
	{ L"TfR5G5B5A1", 2, 1 },
	{ L"TfR4G4B4A4", 2, 1 },

	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },

	{ L"TfR16G16B16A16F", 8, 1 },
	{ L"TfR32G32B32A32F", 16, 1 },
	{ L"TfR16G16F", 4, 1 },
	{ L"TfR32G32F", 8, 1 },
	{ L"TfR16F", 2, 1 },
	{ L"TfR32F", 4, 1 },
	{ L"TfR11G11B10F", 4, 1 },

	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },

	{ L"TfDXT1", 8, 4 },
	{ L"TfDXT2", 16, 4 },
	{ L"TfDXT3", 16, 4 },
	{ L"TfDXT4", 16, 4 },
	{ L"TfDXT5", 16, 4 },
	{ L"TfBC6HU", 16, 4 },
	{ L"TfBC6HS", 16, 4 },

	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },
	{ L"TfInvalid", 0, 0 },

	{ L"TfPVRTC1", 2, 2 },
	{ L"TfPVRTC2", 2, 4 },
	{ L"TfPVRTC3", 2, 2 },
	{ L"TfPVRTC4", 2, 4 },

	{ L"TfETC1", 8, 4 },

	{ L"TfASTC4x4", 16, 4 },
	{ L"TfASTC8x8", 16, 8 },
	{ L"TfASTC10x10", 16, 10 },
	{ L"TfASTC12x12", 16, 12 },
	{ L"TfASTC4x4F", 16, 4 },
	{ L"TfASTC8x8F", 16, 8 },
	{ L"TfASTC10x10F", 16, 10 },
	{ L"TfASTC12x12F", 16, 12 }
};

}

handle_t getParameterHandle(const std::wstring& name)
{
	if (s_handleRegistry)
		return s_handleRegistry->getHandle(name);
	else
	{
		s_handleRegistry = new HandleRegistry();
		return s_handleRegistry->getHandle(name);
	}
}

std::wstring getParameterName(handle_t handle)
{
	if (s_handleRegistry)
		return s_handleRegistry->getName(handle);
	else
	{
		s_handleRegistry = new HandleRegistry();
		return s_handleRegistry->getName(handle);
	}
}

std::wstring getParameterNameFromTextureReferenceIndex(int32_t index)
{
	return std::wstring(L"linkage_texRef") + toString(index);
}

handle_t getParameterHandleFromTextureReferenceIndex(int32_t index)
{
	return getParameterHandle(getParameterNameFromTextureReferenceIndex(index));
}

std::wstring getDataUsageName(DataUsage usage)
{
	const wchar_t* c_names[] = {
		L"DuPosition",
		L"DuNormal",
		L"DuTangent",
		L"DuBinormal",
		L"DuColor",
		L"DuCustom"
	};
	T_ASSERT(int(usage) < sizeof_array(c_names));
	return c_names[int(usage)];
}

std::wstring getDataTypeName(DataType dataType)
{
	const wchar_t* c_names[] = {
		L"DtFloat1",
		L"DtFloat2",
		L"DtFloat3",
		L"DtFloat4",
		L"DtByte4",
		L"DtByte4N",
		L"DtShort2",
		L"DtShort4",
		L"DtShort2N",
		L"DtShort4N",
		L"DtHalf2",
		L"DtHalf4",
		L"DtInteger1",
		L"DtInteger2",
		L"DtInteger3",
		L"DtInteger4"
	};
	T_ASSERT(int(dataType) < sizeof_array(c_names));
	return c_names[int(dataType)];
}

uint32_t getDataElementCount(DataType dataType)
{
	const uint32_t c_elementCounts[] = { 1, 2, 3, 4, 4, 4, 2, 4, 2, 4, 2, 4, 1, 2, 3, 4 };
	T_ASSERT(int(dataType) < sizeof_array(c_elementCounts));
	return c_elementCounts[int(dataType)];
}

std::wstring getTextureFormatName(TextureFormat format)
{
	T_ASSERT(int(format) < sizeof_array(c_textureFormatInfo));
	return c_textureFormatInfo[int(format)].name;
}

uint32_t getTextureBlockSize(TextureFormat format)
{
	T_ASSERT(int(format) < sizeof_array(c_textureFormatInfo));
	return c_textureFormatInfo[int(format)].blockSize;
}

uint32_t getTextureBlockDenom(TextureFormat format)
{
	T_ASSERT(int(format) < sizeof_array(c_textureFormatInfo));
	return c_textureFormatInfo[int(format)].blockDenom;
}

uint32_t getTextureMipSize(uint32_t textureSize, uint32_t mipLevel)
{
	const uint32_t mipSize = textureSize >> mipLevel;
	return mipSize > 0 ? mipSize : 1;
}

uint32_t getTextureRowPitch(TextureFormat format, uint32_t textureWidth)
{
	const uint32_t blockDenom = getTextureBlockDenom(format);
	const uint32_t blockWidth = (textureWidth + blockDenom - 1) / blockDenom;
	return getTextureBlockSize(format) * blockWidth;
}

uint32_t getTextureRowPitch(TextureFormat format, uint32_t textureWidth, uint32_t mipLevel)
{
	return getTextureRowPitch(format, getTextureMipSize(textureWidth, mipLevel));
}

uint32_t getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight)
{
	if (format == TfPVRTC1 || format == TfPVRTC3)
	{
		textureWidth = std::max< uint32_t >(textureWidth, 8);
		textureHeight = std::max< uint32_t >(textureHeight, 8);
	}
	else if (format == TfPVRTC2 || format == TfPVRTC4)
	{
		textureWidth = std::max< uint32_t >(textureWidth, 16);
		textureHeight = std::max< uint32_t >(textureHeight, 8);
	}

	const uint32_t blockDenom = getTextureBlockDenom(format);
	const uint32_t blockWidth = (textureWidth + blockDenom - 1) / blockDenom;
	const uint32_t blockHeight = (textureHeight + blockDenom - 1) / blockDenom;
	const uint32_t blockCount = blockWidth * blockHeight;

	return getTextureBlockSize(format) * blockCount;
}

uint32_t getTextureMipPitch(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevel)
{
	return getTextureMipPitch(format, getTextureMipSize(textureWidth, mipLevel), getTextureMipSize(textureHeight, mipLevel));
}

uint32_t getTextureSize(TextureFormat format, uint32_t textureWidth, uint32_t textureHeight, uint32_t mipLevels)
{
	uint32_t textureSize = 0;
	for (uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
		textureSize += getTextureMipPitch(format, textureWidth, textureHeight, mipLevel);
	return textureSize;
}

}
