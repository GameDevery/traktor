#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberEnum.h>
#include <Core/Serialization/MemberStl.h>
#include "SolutionBuilderLIB/Configuration.h"

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Configuration", 1, Configuration, traktor::ISerializable)

Configuration::Configuration()
:	m_targetFormat(TfStaticLibrary)
,	m_targetProfile(TpDebug)
{
}

void Configuration::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Configuration::getName() const
{
	return m_name;
}

void Configuration::setTargetFormat(TargetFormat targetFormat)
{
	m_targetFormat = targetFormat;
}

Configuration::TargetFormat Configuration::getTargetFormat() const
{
	return m_targetFormat;
}

void Configuration::setTargetProfile(TargetProfile targetProfile)
{
	m_targetProfile = targetProfile;
}

Configuration::TargetProfile Configuration::getTargetProfile() const
{
	return m_targetProfile;
}

void Configuration::setPrecompiledHeader(const std::wstring& precompiledHeader)
{
	m_precompiledHeader = precompiledHeader;
}

const std::wstring& Configuration::getPrecompiledHeader() const
{
	return m_precompiledHeader;
}

void Configuration::addIncludePath(const std::wstring& includePath)
{
	m_includePaths.push_back(includePath);
}

void Configuration::setIncludePaths(const std::vector< std::wstring >& includePaths)
{
	m_includePaths = includePaths;
}

const std::vector< std::wstring >& Configuration::getIncludePaths() const
{
	return m_includePaths;
}

void Configuration::addDefinition(const std::wstring& definition)
{
	m_definitions.push_back(definition);
}

void Configuration::setDefinitions(const std::vector< std::wstring >& definitions)
{
	m_definitions = definitions;
}

const std::vector< std::wstring >& Configuration::getDefinitions() const
{
	return m_definitions;
}

void Configuration::setLibraryPaths(const std::vector< std::wstring >& libraryPaths)
{
	m_libraryPaths = libraryPaths;
}

const std::vector< std::wstring >& Configuration::getLibraryPaths() const
{
	return m_libraryPaths;
}

void Configuration::setLibraries(const std::vector< std::wstring >& libraries)
{
	m_libraries = libraries;
}

const std::vector< std::wstring >& Configuration::getLibraries() const
{
	return m_libraries;
}

void Configuration::setAdditionalCompilerOptions(const std::wstring& additionalCompilerOptions)
{
	m_additionalCompilerOptions = additionalCompilerOptions;
}

const std::wstring& Configuration::getAdditionalCompilerOptions() const
{
	return m_additionalCompilerOptions;
}

void Configuration::setAdditionalLinkerOptions(const std::wstring& additionalLinkerOptions)
{
	m_additionalLinkerOptions = additionalLinkerOptions;
}

const std::wstring& Configuration::getAdditionalLinkerOptions() const
{
	return m_additionalLinkerOptions;
}

bool Configuration::serialize(traktor::ISerializer& s)
{
	traktor::MemberEnum< TargetFormat >::Key kTargetFormat[] =
	{
		{ L"TfStaticLibrary", TfStaticLibrary },
		{ L"TfSharedLibrary", TfSharedLibrary },
		{ L"TfExecutable", TfExecutable },
		{ L"TfExecutableConsole", TfExecutableConsole },
		{ 0, 0 }
	};

	traktor::MemberEnum< TargetProfile >::Key kTargetProfile[] =
	{
		{ L"TpDebug", TpDebug },
		{ L"TpRelease", TpRelease },
		{ 0, 0 }
	};

	s >> traktor::Member< std::wstring >(L"name", m_name);
	s >> traktor::MemberEnum< TargetFormat >(L"targetFormat", m_targetFormat, kTargetFormat);
	s >> traktor::MemberEnum< TargetProfile >(L"targetProfile", m_targetProfile, kTargetProfile);
	s >> traktor::Member< std::wstring >(L"precompiledHeader", m_precompiledHeader);
	s >> traktor::MemberStlVector< std::wstring >(L"includePaths", m_includePaths);
	s >> traktor::MemberStlVector< std::wstring >(L"definitions", m_definitions);
	s >> traktor::MemberStlVector< std::wstring >(L"libraryPaths", m_libraryPaths);
	s >> traktor::MemberStlVector< std::wstring >(L"libraries", m_libraries);

	if (s.getVersion() >= 1)
	{
		s >> traktor::Member< std::wstring >(L"additionalCompilerOptions", m_additionalCompilerOptions);
		s >> traktor::Member< std::wstring >(L"additionalLinkerOptions", m_additionalLinkerOptions);
	}

	return true;
}
