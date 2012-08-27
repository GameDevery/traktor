#ifndef traktor_online_LocalUser_H
#define traktor_online_LocalUser_H

#include "Online/Provider/IUserProvider.h"

namespace traktor
{
	namespace online
	{

class LocalUser : public IUserProvider
{
	T_RTTI_CLASS;

public:
	virtual bool getName(uint64_t userHandle, std::wstring& outName);

	virtual bool isFriend(uint64_t userHandle);

	virtual bool invite(uint64_t userHandle);

	virtual bool setPresenceValue(uint64_t userHandle, const std::wstring& key, const std::wstring& value);

	virtual bool getPresenceValue(uint64_t userHandle, const std::wstring& key, std::wstring& outValue);

	virtual bool sendP2PData(uint64_t userHandle, const void* data, size_t size, bool reliable);
};

	}
}

#endif	// traktor_online_LocalUser_H
