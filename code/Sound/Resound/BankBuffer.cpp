#include "Core/Thread/Acquire.h"
#include "Sound/Sound.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/IGrain.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

struct BankBufferCursor : public RefCountImpl< ISoundBufferCursor >
{
	int32_t m_grainIndex;
	Ref< ISoundBufferCursor > m_grainCursor;

	virtual void reset()
	{
		m_grainIndex = 0;
		m_grainCursor->reset();
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.BankBuffer", BankBuffer, ISoundBuffer)

BankBuffer::BankBuffer(const RefArray< IGrain >& grains)
:	m_grains(grains)
{
}

const IGrain* BankBuffer::getCurrentGrain(ISoundBufferCursor* cursor) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	BankBufferCursor* bankCursor = static_cast< BankBufferCursor* >(cursor);
	const IGrain* currentGrain = m_grains[bankCursor->m_grainIndex];
	return currentGrain ? currentGrain->getCurrentGrain(bankCursor->m_grainCursor) : 0;
}

void BankBuffer::updateCursor(ISoundBufferCursor* cursor) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	BankBufferCursor* bankCursor = static_cast< BankBufferCursor* >(cursor);
	const IGrain* currentGrain = m_grains[bankCursor->m_grainIndex];
	currentGrain->updateCursor(bankCursor->m_grainCursor);
}

Ref< ISoundBufferCursor > BankBuffer::createCursor() const
{
	Ref< BankBufferCursor > bankCursor = new BankBufferCursor();

	bankCursor->m_grainIndex = 0;
	bankCursor->m_grainCursor = m_grains[0]->createCursor();

	return bankCursor->m_grainCursor ? bankCursor : 0;
}

bool BankBuffer::getBlock(ISoundBufferCursor* cursor, SoundBlock& outBlock) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	BankBufferCursor* bankCursor = static_cast< BankBufferCursor* >(cursor);

	int32_t ngrains = int32_t(m_grains.size());
	if (bankCursor->m_grainIndex >= ngrains)
		return false;

	Ref< IGrain > grain = m_grains[bankCursor->m_grainIndex];

	for (;;)
	{
		if (grain->getBlock(
			bankCursor->m_grainCursor,
			outBlock
		))
			break;

		if (++bankCursor->m_grainIndex >= ngrains)
			return false;

		grain = m_grains[bankCursor->m_grainIndex];

		bankCursor->m_grainCursor = grain->createCursor();
	}

	return true;
}

	}
}
