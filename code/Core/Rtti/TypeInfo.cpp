#if !defined(_WIN32)
#	include <alloca.h>
#endif
#include <cstdlib>
#include <string>
#include "Core/Rtti/TypeInfo.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace
	{

static uint32_t s_typeInfoCount = 0;
static uint32_t s_typeInfoRegistrySize = 0;
static const TypeInfo** s_typeInfoRegistry = 0;

	}

void __registerTypeInfo(const TypeInfo* typeInfo)
{
	if (!s_typeInfoRegistry)
	{
		s_typeInfoRegistrySize = 1024;
		s_typeInfoRegistry = reinterpret_cast< const TypeInfo** >(std::malloc(s_typeInfoRegistrySize * sizeof(const TypeInfo*)));
		T_ASSERT (s_typeInfoRegistry);
	}

	std::wstring typeName(typeInfo->getName());

	uint32_t index = 0;
	while (index < s_typeInfoCount)
	{
		const TypeInfo* typeInfo2 = s_typeInfoRegistry[index];
		T_ASSERT (typeInfo2);

		std::wstring typeName2(typeInfo2->getName());
		int32_t res = typeName2.compare(typeName);
		if (res > 0)
			break;

		T_ASSERT_M (res, L"Type already defined");
		index++;
	}

	if (index < s_typeInfoCount)
	{
		for (uint32_t i = s_typeInfoCount; i > index; --i)
			s_typeInfoRegistry[i] = s_typeInfoRegistry[i - 1];
	}

	s_typeInfoRegistry[index] = typeInfo;

	if (++s_typeInfoCount >= s_typeInfoRegistrySize)
	{
		s_typeInfoRegistrySize += 1024;
		s_typeInfoRegistry = reinterpret_cast< const TypeInfo** >(std::realloc(s_typeInfoRegistry, s_typeInfoRegistrySize * sizeof(const TypeInfo*)));
		T_ASSERT (s_typeInfoRegistry);
	}
}

TypeInfo::TypeInfo(
	const wchar_t* name,
	size_t size,
	int32_t version,
	bool editable,
	const TypeInfo* super,
	const IInstanceFactory* factory
)
:	m_name(name)
,	m_size(size)
,	m_version(version)
,	m_editable(editable)
,	m_super(super)
,	m_factory(factory)
{
	__registerTypeInfo(this);
}

const wchar_t* TypeInfo::getName() const
{
	return m_name;
}

size_t TypeInfo::getSize() const
{
	return m_size;
}

int32_t TypeInfo::getVersion() const
{
	return m_version;
}

bool TypeInfo::isEditable() const
{
	return m_editable;
}

const TypeInfo* TypeInfo::getSuper() const
{
	return m_super;
}

bool TypeInfo::isInstantiable() const
{
	return m_factory != 0;
}

ITypedObject* TypeInfo::createInstance() const
{
	if (m_factory)
		return m_factory->createInstance();
	else
		return 0;
}

const TypeInfo* TypeInfo::find(const std::wstring& name)
{
	uint32_t first = 0;
	uint32_t last = s_typeInfoCount;

	while (first < last)
	{
		uint32_t index = (last + first) >> 1;

		int32_t res = std::wstring(s_typeInfoRegistry[index]->getName()).compare(name);
		if (res == 0)
			return s_typeInfoRegistry[index];
		else if (res > 0)
			last = index;
		else if (res < 0)
			first = index + 1;
	}

	return 0;
}

void TypeInfo::findAllOf(std::vector< const TypeInfo* >& outTypes, bool inclusive) const
{
	for (size_t i = 0; i < s_typeInfoCount; ++i)
	{
		if (is_type_of(*this, *s_typeInfoRegistry[i]))
		{
			if (inclusive || s_typeInfoRegistry[i] != this)
				outTypes.push_back(s_typeInfoRegistry[i]);
		}
	}
}

void __forceLinkReference(const TypeInfo& type)
{
	wchar_t* dummy = static_cast< wchar_t* >(alloca(256 * sizeof(wchar_t)));
#if defined(_WIN32)
	wcscpy_s(dummy, 256, type.getName());
#else
	wcscpy(dummy, type.getName());
#endif
}

}
