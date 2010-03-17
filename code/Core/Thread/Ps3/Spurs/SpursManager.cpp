#include <spu_printf.h>
#include "Core/Memory/Alloc.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#include "Core/Thread/Ps3/Spurs/SpursManager.h"

#define SM_SPURS_PREFIX	"Traktor Spurs"
#define SM_SPURS_SPU_COUNT 4
#define	SM_SPURS_SPU_THREAD_GROUP_PRIORITY 250
#define	SM_SPURS_PPU_THREAD_PRIORITY 100

namespace traktor
{

SpursManager& SpursManager::getInstance()
{
	static SpursManager* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new SpursManager();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

Ref< SpursJobQueue > SpursManager::createJobQueue(uint32_t descriptorSize, uint32_t submitCount)
{
	Ref< SpursJobQueue > jobQueue = new SpursJobQueue(m_spurs);
	if (jobQueue->create(descriptorSize, submitCount))
		return jobQueue;
	else
		return 0;
}

void SpursManager::destroy()
{
	delete this;
}

SpursManager::SpursManager()
:	m_spurs(0)
{
	spu_printf_initialize(1535, 0);

	m_spurs = (CellSpurs*)Alloc::acquireAlign(CELL_SPURS_SIZE, CELL_SPURS_ALIGN);
	T_FATAL_ASSERT (m_spurs);

	CellSpursAttribute attributeSpurs;
	cellSpursAttributeInitialize(
		&attributeSpurs,
		SM_SPURS_SPU_COUNT,
		SM_SPURS_SPU_THREAD_GROUP_PRIORITY,
		SM_SPURS_PPU_THREAD_PRIORITY,
		true
	);
	cellSpursAttributeSetNamePrefix(&attributeSpurs, SM_SPURS_PREFIX, strlen(SM_SPURS_PREFIX));
	cellSpursAttributeEnableSpuPrintfIfAvailable(&attributeSpurs);
	cellSpursInitializeWithAttribute(m_spurs, &attributeSpurs);

	cellSysmoduleLoadModule(CELL_SYSMODULE_SPURS_JQ);
}

SpursManager::~SpursManager()
{
	if (m_spurs)
	{
		cellSysmoduleUnloadModule(CELL_SYSMODULE_SPURS_JQ);
		cellSpursFinalize(m_spurs);
		Alloc::freeAlign(m_spurs);
		m_spurs = 0;
	}

	spu_printf_finalize();
}

}
