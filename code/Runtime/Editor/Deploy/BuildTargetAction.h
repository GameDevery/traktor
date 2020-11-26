#pragma once

#include <string>
#include "Runtime/Editor/Deploy/ITargetAction.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_DEPLOY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class PropertyGroup;

	namespace db
	{

class Database;

	}

	namespace runtime
	{

class Target;
class TargetConfiguration;

/*! Build target action.
 * \ingroup Runtime
 */
class T_DLLCLASS BuildTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	BuildTargetAction(
		db::Database* database,
		const PropertyGroup* globalSettings,
		const PropertyGroup* defaultPipelineSettings,
		const Target* target,
		const TargetConfiguration* targetConfiguration,
		const std::wstring& outputPath,
		const PropertyGroup* tweakSettings,
		bool standAlone
	);

	virtual bool execute(IProgressListener* progressListener) override final;

private:
	Ref< db::Database > m_database;
	Ref< const PropertyGroup > m_globalSettings;
	Ref< const PropertyGroup > m_defaultPipelineSettings;
	Ref< const Target > m_target;
	Ref< const TargetConfiguration > m_targetConfiguration;
	std::wstring m_outputPath;
	Ref< const PropertyGroup > m_tweakSettings;
	bool m_standAlone;
};

	}
}

