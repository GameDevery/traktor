#include "Core/Platform.h"
#include "Core/Debug/CallStack.h"

namespace traktor
{

uint32_t getCallStack(uint32_t ncs, void** outCs)
{
	return CaptureStackBackTrace(
		1,
		ncs,
		outCs,
		0
	);
}

}
