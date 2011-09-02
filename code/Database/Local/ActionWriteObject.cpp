#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionWriteObject", ActionWriteObject, Action)

ActionWriteObject::ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName, DynamicMemoryStream* objectStream)
:	m_instancePath(instancePath)
,	m_primaryTypeName(primaryTypeName)
,	m_objectStream(objectStream)
,	m_editObject(false)
,	m_editMeta(false)
{
}

bool ActionWriteObject::execute(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	m_editObject = fileStore->edit(instanceObjectPath);
	if (!m_editObject)
	{
		log::error << L"Unable to edit instance object" << Endl;
		return false;
	}

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Unable to read instance meta data" << Endl;
		return false;
	}

	Ref< IStream > instanceStream = FileSystem::getInstance().open(instanceObjectPath, File::FmWrite);
	if (!instanceStream)
	{
		log::error << L"Unable to open instance object stream; \"" << instanceObjectPath.getPathName() << L"\"" << Endl;
		return false;
	}

	const std::vector< uint8_t >& objectBuffer = m_objectStream->getBuffer();
	if (instanceStream->write(&objectBuffer[0], objectBuffer.size()) != objectBuffer.size())
	{
		log::error << L"Failed to write instance object" << Endl;
		instanceStream->close();
		return false;
	}

	instanceStream->close();

	if (instanceMeta->getPrimaryType() != m_primaryTypeName)
	{
		m_editMeta = fileStore->edit(instanceMetaPath);
		if (!m_editMeta)
		{
			log::error << L"Unable to edit instance meta" << Endl;
			return false;
		}

		instanceMeta->setPrimaryType(m_primaryTypeName);

		if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
		{
			log::error << L"Unable to write instance meta data" << Endl;
			return false;
		}
	}

	return true;
}

bool ActionWriteObject::undo(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editObject)
		fileStore->rollback(instanceObjectPath);
	if (m_editMeta)
		fileStore->rollback(instanceMetaPath);

	m_editObject =
	m_editMeta = false;

	return true;
}

void ActionWriteObject::clean(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editObject)
		fileStore->clean(instanceObjectPath);
	if (m_editMeta)
		fileStore->clean(instanceMetaPath);
}

	}
}
