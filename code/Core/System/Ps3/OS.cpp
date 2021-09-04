#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_gamecontent.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/OS.h"

namespace traktor
{
	namespace
	{

struct OSData
{
	std::wstring contentPath;
	std::wstring usrdirPath;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new OS();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

std::wstring OS::getName() const
{
	return L"PS3";	
}

std::wstring OS::getIdentifier() const
{
	return L"ps3";
}

uint32_t OS::getCPUCoreCount() const
{
	return 2;
}

Path OS::getExecutable() const
{
	return Path();
}

std::wstring OS::getCommandLine() const
{
	return L"";
}

std::wstring OS::getComputerName() const
{
	return L"PS3";
}

std::wstring OS::getCurrentUser() const
{
	char user[256];
	if (cellSysutilGetSystemParamString(CELL_SYSUTIL_SYSTEMPARAM_ID_CURRENT_USERNAME, user, sizeof(user)) == CELL_OK)
		return mbstows(user);
	else
		return L"";
}

std::wstring OS::getUserHomePath() const
{
	OSData* data = static_cast< OSData* >(m_handle);
	return data->usrdirPath;
}

std::wstring OS::getUserApplicationDataPath() const
{
	OSData* data = static_cast< OSData* >(m_handle);
	return data->contentPath;
}

std::wstring OS::getWritableFolderPath() const
{
	OSData* data = static_cast< OSData* >(m_handle);
	return data->contentPath;
}

bool OS::openFile(const std::wstring& file) const
{
	return false;
}

bool OS::editFile(const std::wstring& file) const
{
	return false;
}

bool OS::exploreFile(const std::wstring& file) const
{
	return false;
}

bool OS::setEnvironment(const std::wstring& name, const std::wstring& value) const
{
	return false;
}

Ref< Environment > OS::getEnvironment() const
{
	OSData* data = static_cast< OSData* >(m_handle);

	Ref< Environment > env = new Environment();
	env->set(L"CONTENT_PATH", data->contentPath);
	env->set(L"USRDIR_PATH", data->usrdirPath);

	return env;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	OSData* data = static_cast< OSData* >(m_handle);

	if (name == L"CONTENT_PATH")
	{
		outValue = data->contentPath;
		return true;
	}
	else if (name == L"USRDIR_PATH")
	{
		outValue = data->usrdirPath;
		return true;
	}

	return false;
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const Environment* env,
	uint32_t flags
) const
{
	return 0;
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	return 0;
}

bool OS::setOwnProcessPriorityBias(int32_t priorityBias)
{
	return false;
}

OS::OS()
:	m_handle(0)
{
	uint32_t type;
	uint32_t attributes;
	CellGameContentSize size;
	char dirName[256];
	char contentInfoPath[256];
	char usrdirPath[256];

	cellSysmoduleLoadModule(CELL_SYSMODULE_SYSUTIL_GAME);

	cellGameBootCheck(&type, &attributes, &size, dirName);
	cellGameContentPermit(contentInfoPath, usrdirPath);

	OSData* data = new OSData();
	data->contentPath = mbstows(contentInfoPath);
	data->usrdirPath = mbstows(usrdirPath);

#if defined(_DEBUG)
	log::debug << L"contentPath \"" << data->contentPath << L"\"" << Endl;
	log::debug << L"usrdirPath \"" << data->usrdirPath << L"\"" << Endl;
#endif

	m_handle = data;
}

OS::~OS()
{
	delete (OSData*)m_handle;
}

void OS::destroy()
{
	delete this;
}

}
