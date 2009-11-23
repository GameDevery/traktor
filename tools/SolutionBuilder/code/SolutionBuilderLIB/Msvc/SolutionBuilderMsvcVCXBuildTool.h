#ifndef SolutionBuilderMsvcVCXBuildTool_H
#define SolutionBuilderMsvcVCXBuildTool_H

#include <Core/Serialization/ISerializable.h>
#include <Core/Io/Path.h>

class GeneratorContext;
class Solution;
class Project;

class SolutionBuilderMsvcVCXBuildTool : public traktor::ISerializable
{
	T_RTTI_CLASS;

public:
	virtual bool generate(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const traktor::Path& fileName,
		traktor::OutputStream& os
	) const;

	virtual bool serialize(traktor::ISerializer& s);

protected:
	std::wstring m_name;
	std::wstring m_fileType;
};

#endif	// SolutionBuilderMsvcVCXBuildTool_H
