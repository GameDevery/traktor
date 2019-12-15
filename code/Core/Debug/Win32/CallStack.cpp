#include "Core/System.h"
#include "Core/Debug/CallStack.h"

namespace traktor
{

uint32_t getCallStack(uint32_t ncs, void** outCs, uint32_t skip)
{
	return CaptureStackBackTrace(
		skip + 1,
		ncs,
		outCs,
		0
	);
}

}
