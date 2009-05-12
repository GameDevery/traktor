#ifndef traktor_net_HttpResponse_H
#define traktor_net_HttpResponse_H

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
	
class Stream;
	
	namespace net
	{

class T_DLLCLASS HttpResponse : public Object
{
	T_RTTI_CLASS(HttpResponse)

public:
	HttpResponse();
	
	bool parse(Stream* stream);

	int32_t getStatusCode() const;
	
	const std::wstring& getStatusMessage() const;

	void set(const std::wstring& name, const std::wstring& value);
	
	std::wstring get(const std::wstring& name);
	
private:
	int32_t m_statusCode;
	std::wstring m_statusMessage;
	std::map< std::wstring, std::wstring > m_values;
};
	
	}
}

#endif	// traktor_net_HttpResponse_H
