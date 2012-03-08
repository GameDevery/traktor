#ifndef traktor_amalgam_TargetConfiguration_H
#define traktor_amalgam_TargetConfiguration_H

#include <list>
#include "Core/Guid.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace amalgam
	{

class TargetConfiguration : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void setPlatform(const Guid& platform);

	const Guid& getPlatform() const;

	void setExecutable(const std::wstring& executable);

	const std::wstring& getExecutable() const;

	void setIcon(const std::wstring& icon);

	const std::wstring& getIcon() const;

	void addFeature(const Guid& feature);

	void removeFeature(const Guid& feature);

	bool haveFeature(const Guid& feature) const;

	const std::list< Guid >& getFeatures() const;

	void setBuildRoot(const Guid& buildRoot);

	const Guid& getRoot() const;

	void setStartup(const Guid& startup);

	const Guid& getStartup() const;

	void setDefaultInput(const Guid& defaultInput);

	const Guid& getDefaultInput() const;

	void setOnlineConfig(const Guid& onlineConfig);

	const Guid& getOnlineConfig() const;

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_name;
	Guid m_platform;
	std::wstring m_executable;
	std::wstring m_icon;
	std::list< Guid > m_features;
	Guid m_root;
	Guid m_startup;
	Guid m_defaultInput;
	Guid m_onlineConfig;
};

	}
}

#endif	// traktor_amalgam_TargetConfiguration_H
