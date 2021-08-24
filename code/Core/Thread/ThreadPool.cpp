#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadPool.h"

namespace traktor
{
	namespace
	{

void threadPoolDispatcher(
	Event& eventAttachWork,
	Event& eventFinishedWork,
	const ThreadPool::threadPoolFn_t& fn,
	const std::atomic< int32_t >& alive
)
{
	while (alive)
	{
		// Wait until work has been attached.
		if (!eventAttachWork.wait(100))
			continue;

		// Execute work.
		fn();

		// Signal work has finished.
		eventFinishedWork.broadcast();
	}
}

	}

ThreadPool& ThreadPool::getInstance()
{
	static ThreadPool* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new ThreadPool();
		SingletonManager::getInstance().addBefore(s_instance, &ThreadManager::getInstance());
	}
	return *s_instance;
}

bool ThreadPool::spawn(const threadPoolFn_t& fn, Thread*& outThread, Thread::Priority priority)
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];

		int32_t expected = 0;
		if (!worker.busy.compare_exchange_strong(expected, 1))
			continue;

		if (worker.thread == nullptr)
		{
			worker.thread = ThreadManager::getInstance().create([&](){ threadPoolDispatcher(worker.eventAttachWork, worker.eventFinishedWork, worker.fn, worker.alive); }, L"Thread pool worker");
			if (!worker.thread)
			{
				worker.busy = 0;
				continue;
			}
			worker.thread->start(priority);
		}

		outThread = worker.thread;
		outThread->resume(priority);

		worker.fn = fn;
		worker.eventFinishedWork.reset();
		worker.eventAttachWork.broadcast();
		return true;
	}

	// No more slots available.
	outThread = nullptr;
	return false;
}

bool ThreadPool::join(Thread* thread)
{
	if (!thread)
		return true;

	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.thread == thread)
		{
			worker.eventFinishedWork.wait();
			worker.thread->resume(Thread::Normal);
			worker.busy = 0;
			return true;
		}
	}

	return false;
}

bool ThreadPool::stop(Thread* thread)
{
	if (!thread)
		return true;

	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		if (worker.thread == thread)
		{
			worker.thread->stop(0);
			worker.eventFinishedWork.wait();
			worker.thread->resume(Thread::Normal);
			worker.busy = 0;
			return true;
		}
	}

	return false;
}

void ThreadPool::destroy()
{
	for (uint32_t i = 0; i < sizeof_array(m_workerThreads); ++i)
	{
		Worker& worker = m_workerThreads[i];
		worker.alive = 0;
		if (worker.thread != nullptr)
		{
			worker.thread->stop();
			ThreadManager::getInstance().destroy(worker.thread);
			worker.thread = nullptr;
		}
		worker.busy = 0;
	}
}

ThreadPool::Worker::Worker()
:	alive(1)
,	busy(0)
{
}

}
