#include "Model/Operations/WeldHoles.h"

namespace traktor
{
	namespace model
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.WeldHoles", WeldHoles, IModelOperation)

bool WeldHoles::apply(Model& model) const
{
	return false;
}

	}
}
