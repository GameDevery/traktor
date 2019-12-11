#pragma once

#include <cell/spurs.h>
#include "Core/Ref.h"
#include "Core/Singleton/ISingleton.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class SpursJobQueue;

/*! Spurs job manager.
 * \ingroup Core
 */
class T_DLLCLASS SpursManager : public ISingleton
{
public:
	enum Priority
	{
		Lowest = 0,
		Below = 1,
		Normal = 2,
		Above = 3,
		Highest = 4
	};

	static SpursManager& getInstance();

	CellSpurs* getSpurs() const;

	uint32_t getSpuCount() const;

	Ref< SpursJobQueue > createJobQueue(uint32_t descriptorSize, uint32_t submitCount, Priority priority);

protected:
	virtual void destroy();

private:
	CellSpurs* m_spurs;

	SpursManager();

	virtual ~SpursManager();
};

}

