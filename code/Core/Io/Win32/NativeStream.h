/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Platform.h"
#include "Core/Io/File.h"
#include "Core/Io/IStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class T_DLLCLASS NativeStream : public IStream
{
	T_RTTI_CLASS;

public:
	NativeStream(void* hFile, uint32_t mode);

	virtual ~NativeStream();

	virtual void close();

	virtual bool canRead() const;

	virtual bool canWrite() const;

	virtual bool canSeek() const;

	virtual int64_t tell() const;

	virtual int64_t available() const;

	virtual int64_t seek(SeekOriginType origin, int64_t offset);

	virtual int64_t read(void* block, int64_t nbytes);

	virtual int64_t write(const void* block, int64_t nbytes);

	virtual void flush();

private:
	void* m_hFile;
	uint32_t m_mode;
	mutable size_t m_fileSize;
};

}

