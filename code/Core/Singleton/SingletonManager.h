#pragma once

#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISingleton;

/*! \brief Singleton manager.
 * \ingroup Core
 *
 * The singleton manager is responsible of maintaining a list
 * of singletons and which order they should be brought down
 * when the application terminates.
 */
class T_DLLCLASS SingletonManager
{
public:
	static SingletonManager& getInstance();

	/*! \brief Add singleton. */
	void add(ISingleton* singleton);

	/*! \brief Add singleton before a dependent singleton. */
	void addBefore(ISingleton* singleton, ISingleton* dependency);

	/*! \brief Add singleton after a dependent singleton. */
	void addAfter(ISingleton* singleton, ISingleton* dependency);

	/*! \brief Explicit destroy all singletons. */
	void destroy();

private:
	AlignedVector< ISingleton* > m_singletons;

	virtual ~SingletonManager();
};

}

