#ifndef traktor_net_HttpRequest_H
#define traktor_net_HttpRequest_H

#include <string>
#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS HttpRequest : public Object
{
	T_RTTI_CLASS(HttpRequest)

public:
	enum Method
	{
		MtGet,
		MtPost
	};

	Method getMethod() const;

	const std::wstring& getResource() const;

	bool hasValue(const std::wstring& key) const;

	void setValue(const std::wstring& key, const std::wstring& value);

	std::wstring getValue(const std::wstring& key) const;

	static HttpRequest* parse(const std::wstring& request);

private:
	Method m_method;
	std::wstring m_resource;
	std::map< std::wstring, std::wstring > m_values;
};

	}
}

#endif	// traktor_net_HttpRequest_H
