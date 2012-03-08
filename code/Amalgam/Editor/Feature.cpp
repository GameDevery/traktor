#include "Amalgam/Editor/Feature.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.amalgam.Feature", 0, Feature, ISerializable)

bool Feature::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"description", m_description);
	s >> MemberComposite< Platforms >(L"platforms", m_platforms);
	s >> MemberRef< PropertyGroup >(L"pipelineProperties", m_pipelineProperties);
	s >> MemberRef< PropertyGroup >(L"runtimeProperties", m_runtimeProperties);
	return true;
}

Feature::Platforms::Platforms()
:	ios(false)
,	linux(false)
,	osx(false)
,	ps3(false)
,	win32(false)
,	win64(false)
,	xbox360(false)
{
}

bool Feature::Platforms::serialize(ISerializer& s)
{
	s >> Member< bool >(L"ios", ios);
	s >> Member< bool >(L"linux", linux);
	s >> Member< bool >(L"osx", osx);
	s >> Member< bool >(L"ps3", ps3);
	s >> Member< bool >(L"win32", win32);
	s >> Member< bool >(L"win64", win64);
	s >> Member< bool >(L"xbox360", xbox360);
	return true;
}

	}
}
