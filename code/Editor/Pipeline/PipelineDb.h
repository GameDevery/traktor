#ifndef traktor_editor_PipelineDb_H
#define traktor_editor_PipelineDb_H

#include "Editor/IPipelineDb.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sql
	{

class IConnection;

	}

	namespace editor
	{

class T_DLLCLASS PipelineDb : public IPipelineDb
{
	T_RTTI_CLASS;

public:
	bool open(const std::wstring& connectionString);

	void close();

	virtual void setDependency(const Guid& guid, const DependencyHash& hash);

	virtual bool getDependency(const Guid& guid, DependencyHash& outHash) const;

	virtual void setFile(const Path& path, const FileHash& file);

	virtual bool getFile(const Path& path, FileHash& outFile);

	virtual Ref< IPipelineReport > createReport(const std::wstring& name, const Guid& guid);

private:
	Ref< sql::IConnection > m_connection;
};

	}
}

#endif	// traktor_editor_PipelineDb_H
