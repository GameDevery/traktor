#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMappingSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingSourceData", 0, InputMappingSourceData, ISerializable)

void InputMappingSourceData::setSourceData(const std::wstring& id, IInputSourceData* data)
{
	if (data != nullptr)
		m_sourceData[id] = data;
	else
		m_sourceData.remove(id);
}

IInputSourceData* InputMappingSourceData::getSourceData(const std::wstring& id)
{
	auto it = m_sourceData.find(id);
	return it != m_sourceData.end() ? it->second : nullptr;
}

const SmallMap< std::wstring, Ref< IInputSourceData > >& InputMappingSourceData::getSourceData() const
{
	return m_sourceData;
}

void InputMappingSourceData::serialize(ISerializer& s)
{
	s >> MemberSmallMap<
		std::wstring,
		Ref< IInputSourceData >,
		Member< std::wstring >,
		MemberRef< IInputSourceData >
	>(L"sourceData", m_sourceData);
}

	}
}
