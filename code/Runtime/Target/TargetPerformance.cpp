#include "Runtime/Target/TargetPerformance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

using namespace traktor;

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetPerformance", 0, TargetPerformance, ISerializable)

TargetPerformance::TargetPerformance()
:	time(0.0f)
,	fps(0.0f)
,	update(0.0f)
,	build(0.0f)
,	render(0.0f)
,	physics(0.0f)
,	input(0.0f)
,	garbageCollect(0.0f)
,	steps(0.0f)
,	interval(0.0f)
,	collisions(0)
,	memInUse(0)
,	memInUseScript(0)
,	memCount(0)
,	memDeltaCount(0)
,	heapObjects(0)
,	gpuMemInUse(0)
,	passCount(0)
,	drawCalls(0)
,	primitiveCount(0)
,	residentResourcesCount(0)
,	exclusiveResourcesCount(0)
,	bodyCount(0)
,	activeBodyCount(0)
,	manifoldCount(0)
,	queryCount(0)
,	activeSoundChannels(0)
{
}

void TargetPerformance::serialize(ISerializer& s)
{
	s >> Member< float >(L"time", time);
	s >> Member< float >(L"fps", fps);
	s >> Member< float >(L"update", update);
	s >> Member< float >(L"build", build);
	s >> Member< float >(L"render", render);
	s >> Member< float >(L"physics", physics);
	s >> Member< float >(L"input", input);
	s >> Member< float >(L"garbageCollect", garbageCollect);
	s >> Member< float >(L"steps", steps);
	s >> Member< float >(L"interval", interval);
	s >> Member< uint32_t >(L"collisions", collisions);
	s >> Member< uint32_t >(L"memInUse", memInUse);
	s >> Member< uint32_t >(L"memInUseScript", memInUseScript);
	s >> Member< int32_t >(L"memCount", memCount);
	s >> Member< int32_t >(L"memDeltaCount", memDeltaCount);
	s >> Member< uint32_t >(L"heapObjects", heapObjects);
	s >> Member< uint32_t >(L"gpuMemInUse", gpuMemInUse);
	s >> Member< uint32_t >(L"passCount", passCount);
	s >> Member< uint32_t >(L"drawCalls", drawCalls);
	s >> Member< uint32_t >(L"primitiveCount", primitiveCount);
	s >> Member< uint32_t >(L"residentResourcesCount", residentResourcesCount);
	s >> Member< uint32_t >(L"exclusiveResourcesCount", exclusiveResourcesCount);
	s >> Member< uint32_t >(L"bodyCount", bodyCount);
	s >> Member< uint32_t >(L"activeBodyCount", activeBodyCount);
	s >> Member< uint32_t >(L"manifoldCount", manifoldCount);
	s >> Member< uint32_t >(L"queryCount", queryCount);
	s >> Member< uint32_t >(L"activeSoundChannels", activeSoundChannels);
}

	}
}
