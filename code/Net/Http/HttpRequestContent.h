/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_net_HttpRequestContent_H
#define traktor_net_HttpRequestContent_H

#include "Net/Http/IHttpRequestContent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

/*! \brief
 * \ingroup Net
 */
class T_DLLCLASS HttpRequestContent : public IHttpRequestContent
{
	T_RTTI_CLASS;

public:
	HttpRequestContent();

	HttpRequestContent(const std::wstring& content);

	void set(const std::wstring& content);

	virtual std::wstring getUrlEncodedContent() const override final;

private:
	std::wstring m_content;
};

	}
}

#endif	// traktor_net_HttpRequestContent_H
