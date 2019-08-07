#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Telemetry/Telemetry.h"
#include "Telemetry/TelemetryClassFactory.h"

namespace traktor
{
	namespace telemetry
	{
		namespace
		{

Telemetry* Telemetry_getInstance()
{
	return &Telemetry::getInstance();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.telemetry.TelemetryClassFactory", 0, TelemetryClassFactory, IRuntimeClassFactory)

void TelemetryClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classTelemetry = new AutoRuntimeClass< Telemetry >();
	classTelemetry->addProperty("lastSequenceNumber", &Telemetry::getLastSequenceNumber);
	classTelemetry->addStaticMethod("getInstance", &Telemetry_getInstance);
	classTelemetry->addMethod("create", &Telemetry::create);
	classTelemetry->addMethod("event", &Telemetry::event);
	classTelemetry->addMethod("set", &Telemetry::set);
	classTelemetry->addMethod("add", &Telemetry::add);
	classTelemetry->addMethod("flush", &Telemetry::flush);
	registrar->registerClass(classTelemetry);
}

	}
}
