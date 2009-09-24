#include <cstring>
#include <pthread.h>
#include "Core/Thread/Mutex.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

struct InternalData
{
	pthread_mutex_t outer;
};

	}

Mutex::Mutex()
:	m_existing(false)
,	m_handle(0)
{
	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

	int rc = pthread_mutex_init(&data->outer, &ma);
	T_ASSERT (rc == 0);

	m_handle = data;
}

Mutex::Mutex(const Guid& id)
:	m_existing(false)
,	m_handle(0)
{
	// @fixme Currently we just create an unnamed local mutex as
	// pthreads doesn't seem to support system wide mutexes.

	InternalData* data = new InternalData();
	std::memset(data, 0, sizeof(InternalData));

	pthread_mutexattr_t ma;
	pthread_mutexattr_init(&ma);
	pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);

	int rc = pthread_mutex_init(&data->outer, &ma);
	T_ASSERT (rc == 0);

	m_handle = data;
}

Mutex::~Mutex()
{
	delete reinterpret_cast< InternalData* >(m_handle);
}

bool Mutex::acquire(int timeout)
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);

	int rc = pthread_mutex_lock(&data->outer);
	T_ASSERT(rc == 0);

	return true;
}

void Mutex::release()
{
	InternalData* data = reinterpret_cast< InternalData* >(m_handle);

	int rc = pthread_mutex_unlock(&data->outer);
	T_ASSERT(rc == 0);
}

bool Mutex::existing() const
{
	return m_existing;
}

}
