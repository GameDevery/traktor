#ifndef SolutionBuilderMsvcSettings_H
#define SolutionBuilderMsvcSettings_H

#include <Core/Serialization/ISerializable.h>

class SolutionBuilderMsvcProject;

/*! Visual Studio solution settings. */
class SolutionBuilderMsvcSettings : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool serialize(traktor::ISerializer& s);

	inline const std::wstring& getSLNVersion() const { return m_slnVersion; }

	inline const std::wstring& getVSVersion() const { return m_vsVersion; }

	inline SolutionBuilderMsvcProject* getProject() const { return m_project; }

private:
	std::wstring m_slnVersion;
	std::wstring m_vsVersion;
	traktor::Ref< SolutionBuilderMsvcProject > m_project;
};

#endif	// SolutionBuilderMsvcSettings_H
