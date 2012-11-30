#ifndef traktor_online_IMatchMaking_H
#define traktor_online_IMatchMaking_H

#include "Online/LobbyResult.h"
#include "Online/LobbyArrayResult.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class ILobby;
class LobbyFilter;

class T_DLLCLASS IMatchMaking : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool ready() const = 0;

	virtual Ref< LobbyArrayResult > findMatchingLobbies(const LobbyFilter* filter) = 0;

	virtual Ref< LobbyResult > createLobby(uint32_t maxUsers) = 0;

	virtual Ref< ILobby > acceptLobby() = 0;
};

	}
}


#endif	// traktor_online_IMatchMaking_H
