#pragma once

#include <atomic>
#include <functional>
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Event.h"
#include "Core/Thread/Thread.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Thread pool manager.
 * \ingroup Core
 */
class T_DLLCLASS ThreadPool : public ISingleton
{
public:
	typedef std::function< void() > threadPoolFn_t;

	static ThreadPool& getInstance();

	bool spawn(const threadPoolFn_t& fn, Thread*& outThread, Thread::Priority priority = Thread::Normal);

	bool join(Thread* thread);

	bool stop(Thread* thread);

protected:
	virtual void destroy();

private:
	struct Worker
	{
		Thread* thread = nullptr;
		Event eventAttachWork;
		Event eventFinishedWork;
		threadPoolFn_t fn;
		std::atomic< int32_t > alive;
		std::atomic< int32_t > busy;

		Worker();
	};

	Worker m_workerThreads[256];
};

}

