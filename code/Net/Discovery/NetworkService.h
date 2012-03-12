#ifndef traktor_net_NetworkService_H
#define traktor_net_NetworkService_H

#include "Core/Settings/PropertyGroup.h"
#include "Net/Discovery/IService.h"

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

/*! \brief Generic network service.
 * \ingroup Net
 */
class T_DLLCLASS NetworkService : public IService
{
	T_RTTI_CLASS;

public:
	NetworkService();

	NetworkService(
		const std::wstring& type,
		const PropertyGroup* properties
	);

	const std::wstring& getType() const;

	const PropertyGroup* getProperties() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_type;
	Ref< const PropertyGroup > m_properties;
};

	}
}

#endif	// traktor_net_NetworkService_H
