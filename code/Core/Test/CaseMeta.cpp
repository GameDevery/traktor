#include "Core/Test/CaseMeta.h"
#include "Core/Meta/TypeList.h"
#include "Core/Meta/Equal.h"

namespace traktor
{
	namespace test
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.test.CaseMeta", 0, CaseMeta, Case)

void CaseMeta::run()
{
	typedef Get< TypeList< bool, int, float >, 0 >::type_t first_type_t;
	typedef Get< TypeList< bool, int, float >, 1 >::type_t second_type_t;
	typedef Get< TypeList< bool, int, float >, 2 >::type_t third_type_t;

	CASE_ASSERT((Equal< first_type_t, bool >::value));
	CASE_ASSERT((Equal< second_type_t, int >::value));
	CASE_ASSERT((Equal< third_type_t, float >::value));
}

	}
}
