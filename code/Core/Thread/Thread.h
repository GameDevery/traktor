#pragma once

#include "Core/Ref.h"
#include "Core/Thread/IWaitable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Functor;

/*! OS thread class.
 * \ingroup Core
 */
class T_DLLCLASS Thread : public IWaitable
{
public:
	enum Priority
	{
		Lowest = -2,
		Below = -1,
		Normal = 0,
		Above = 1,
		Highest = 2
	};

	bool start(Priority priority = Normal);

	bool stop(int32_t timeout = -1);

	bool pause();

	bool resume();

	bool resume(Priority priority);

	void sleep(int32_t duration);

	void yield();

	bool current() const;

	bool stopped() const;

	bool finished() const;

	virtual bool wait(int32_t timeout = -1) override final;

	uint32_t id() const { return m_id; }

	const wchar_t* name() const { return m_name; }

	int32_t hardwareCore() const { return m_hardwareCore; }

private:
	friend class ThreadManager;

	void* m_handle;
	uint32_t m_id;
	bool m_stopped;
	Ref< Functor > m_functor;
	const wchar_t* const m_name;
	int32_t m_hardwareCore;

	Thread(Functor* functor, const wchar_t* const name, int32_t hardwareCore);

	~Thread();
};

}
