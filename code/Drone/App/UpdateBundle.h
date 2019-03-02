#pragma once

#include <Core/Serialization/ISerializable.h>
#include <Net/Url.h>

namespace traktor
{
	namespace drone
	{

class UpdateBundle : public ISerializable
{
	T_RTTI_CLASS

public:
	struct BundledItem
	{
		std::wstring url;
		std::wstring path;

		void serialize(ISerializer& s);
	};

	UpdateBundle();

	uint32_t getBundleVersion() const;

	const std::wstring& getDescription() const;

	const std::vector< BundledItem >& getItems() const;

	virtual void serialize(ISerializer& s);

private:
	uint32_t m_version;
	std::wstring m_description;
	std::vector< BundledItem > m_items;
};

	}
}

