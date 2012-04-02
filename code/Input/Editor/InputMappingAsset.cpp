#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/InputMappingResource.h"
#include "Input/Binding/InputMappingSourceData.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Editor/InputMappingAsset.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingAsset", 0, InputMappingAsset, editor::ITypedAsset)

const TypeInfo* InputMappingAsset::getOutputType() const
{
	return &type_of< InputMappingResource >();
}

bool InputMappingAsset::serialize(ISerializer& s)
{
	s >> MemberRef< InputMappingSourceData >(L"sourceData", m_sourceData);
	s >> MemberRef< InputMappingStateData >(L"stateData", m_stateData);
	s >> MemberStlList< Guid >(L"dependencies", m_dependencies);
	return true;
}

	}
}
