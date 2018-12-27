/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_run_StreamInput_H
#define traktor_run_StreamInput_H

#include "Core/Io/StringReader.h"
#include "Run/App/IInput.h"

namespace traktor
{
	namespace run
	{

/*! \brief Stream input reader.
 * \ingroup Run
 */
class StreamInput : public IInput
{
	T_RTTI_CLASS;

public:
	StreamInput(IStream* stream, IEncoding* encoding);

	virtual bool endOfFile() override final;

	virtual std::wstring readChar() override final;

	virtual std::wstring readLn() override final;

private:
	Ref< IStream > m_stream;
	StringReader m_reader;
};

	}
}

#endif	// traktor_run_StreamInput_H
