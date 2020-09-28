#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <limits.h>
#include <spawn.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/TString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/System/Environment.h"
#include "Core/System/ResolveEnv.h"
#include "Core/System/Linux/ProcessLinux.h"
#include "Core/System/Linux/SharedMemoryLinux.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.OS", OS, Object)

OS& OS::getInstance()
{
	static OS* s_instance = 0;
	if (!s_instance)
	{
		s_instance = new OS();
		s_instance->addRef(0);
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

uint32_t OS::getCPUCoreCount() const
{
	return (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);
}

Path OS::getExecutable() const
{
	char result[PATH_MAX] = { 0 };
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	if (count > 0)
		return Path(mbstows(result));
	else
		return Path();
}

std::wstring OS::getCommandLine() const
{
	char cmdLine[1024] = { '\0' };
	FILE* fp = fopen("/proc/self/cmdline", "r");
	if (fp)
	{
		fgets(cmdLine, sizeof(cmdLine), fp);
		fclose(fp);
	}
	return mbstows(cmdLine);
}

std::wstring OS::getComputerName() const
{
	char name[MAXHOSTNAMELEN];

	if (gethostname(name, sizeof_array(name)) != -1)
		return mbstows(name);

	return L"Unavailable";
}

std::wstring OS::getCurrentUser() const
{
	passwd* pwd = getpwuid(geteuid());
	if (!pwd)
		return L"Unavailable";

	const char* who = pwd->pw_name;
	if (!who)
		return L"Unavailable";

	return mbstows(who);
}

std::wstring OS::getUserHomePath() const
{
	std::wstring home;
	if (getEnvironment(L"HOME", home))
		return home;
	else
		return L"~";
}

std::wstring OS::getUserApplicationDataPath() const
{
	return getUserHomePath() + L"/.traktor";
}

std::wstring OS::getWritableFolderPath() const
{
	return getUserHomePath() + L"/.traktor";
}

bool OS::openFile(const std::wstring& file) const
{
	system(("xdg-open " + wstombs(file)).c_str());
	return true;
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
	return bool(setenv(
		wstombs(name).c_str(),
		wstombs(value).c_str(),
		1
	) == 0);
}

Ref< Environment > OS::getEnvironment() const
{
	Ref< Environment > env = new Environment();
	for (char** e = environ; *e; ++e)
	{
		char* sep = strchr(*e, '=');
		if (sep)
		{
			char* val = sep + 1;
			env->set(
				mbstows(std::string(*e, sep)),
				mbstows(val)
			);
		}
	}
	return env;
}

bool OS::getEnvironment(const std::wstring& name, std::wstring& outValue) const
{
	const char* value = getenv(wstombs(name).c_str());
	if (value)
	{
		outValue = mbstows(value);
		return true;
	}
	else
		return false;
}

Ref< IProcess > OS::execute(
	const std::wstring& commandLine,
	const Path& workingDirectory,
	const Environment* env,
	uint32_t flags
) const
{
	posix_spawn_file_actions_t* fileActions = 0;
	char* envv[4096];
	char* argv[1024];
	int envc = 0;
	int argc = 0;
	int err;
	pid_t pid;
	int childStdOut[2] = { 0 };
	int childStdErr[2] = { 0 };
	std::wstring executable;
	std::wstring arguments;

	// Resolve entire command line.
	std::wstring resolvedCommandLine = resolveEnv(commandLine, env);

	// Extract executable file from command line.
	if (resolvedCommandLine.empty())
		return nullptr;
	if (resolvedCommandLine[0] == L'\"')
	{
		size_t i = resolvedCommandLine.find(L'\"', 1);
		if (i == resolvedCommandLine.npos)
			return nullptr;
		executable = resolvedCommandLine.substr(1, i - 1);
		arguments = trim(resolvedCommandLine.substr(i + 1));
	}
	else
	{
		size_t i = resolvedCommandLine.find(L' ');
		if (i != resolvedCommandLine.npos)
		{
			executable = resolvedCommandLine.substr(0, i);
			arguments = trim(resolvedCommandLine.substr(i + 1));
		}
		else
			executable = resolvedCommandLine;
	}

	// Convert all arguments; append bash if executing shell script.
	if (endsWith(executable, L".sh"))
		argv[argc++] = strdup("/bin/sh");
	else
	{
		// Ensure file has executable permission.
		struct stat st;
		if (stat(wstombs(executable).c_str(), &st) == 0)
			chmod(wstombs(executable).c_str(), st.st_mode | S_IXUSR);
	}

	argv[argc++] = strdup(wstombs(executable).c_str());
	size_t i = 0;
	while (i < arguments.length())
	{
		if (arguments[i] == L'\"')
		{
			size_t j = arguments.find(L'\"', i + 1);
			if (j != arguments.npos)
			{
				argv[argc++] = strdup(wstombs(arguments.substr(i + 1, j - i - 1)).c_str());
				i = j + 1;
			}
			else
				return nullptr;
		}
		else
		{
			size_t j = arguments.find(L' ', i + 1);
			if (j != arguments.npos)
			{
				argv[argc++] = strdup(wstombs(arguments.substr(i, j - i)).c_str());
				i = j + 1;
			}
			else
			{
				argv[argc++] = strdup(wstombs(arguments.substr(i)).c_str());
				break;
			}
		}
	}

	// Convert environment variables; don't pass "DYLIB_LIBRARY_PATH" along as we
	// don't want child process searching our products by default.
	if (env)
	{
		const std::map< std::wstring, std::wstring >& v = env->get();
		for (auto i = v.begin(); i != v.end(); ++i)
			envv[envc++] = strdup(wstombs(i->first + L"=" + i->second).c_str());
	}
	else
	{
		Ref< Environment > env2 = getEnvironment();
		const std::map< std::wstring, std::wstring >& v = env2->get();
		for (auto i = v.begin(); i != v.end(); ++i)
			envv[envc++] = strdup(wstombs(i->first + L"=" + i->second).c_str());
	}

	// Terminate argument and environment vectors.
	envv[envc] = nullptr;
	argv[argc] = nullptr;

	char wd[512];
	Path awd = FileSystem::getInstance().getAbsolutePath(workingDirectory);
	strcpy(wd, wstombs(awd.getPathNameNoVolume()).c_str());

	// Redirect standard IO.
	if ((flags & EfRedirectStdIO) != 0)
	{
		if ((flags & EfMute) == 0)
		{
			pipe(childStdOut);
			pipe(childStdErr);

			fileActions = new posix_spawn_file_actions_t;
			posix_spawn_file_actions_init(fileActions);
#if !defined(__RPI__)
			posix_spawn_file_actions_addchdir_np(fileActions, wd);
#endif
			posix_spawn_file_actions_adddup2(fileActions, childStdOut[1], STDOUT_FILENO);
			posix_spawn_file_actions_addclose(fileActions, childStdOut[0]);
			posix_spawn_file_actions_adddup2(fileActions, childStdErr[1], STDERR_FILENO);
			posix_spawn_file_actions_addclose(fileActions, childStdErr[0]);
		}
		else
		{
			fileActions = new posix_spawn_file_actions_t;
			posix_spawn_file_actions_init(fileActions);
#if !defined(__RPI__)
			posix_spawn_file_actions_addchdir_np(fileActions, wd);
#endif
			posix_spawn_file_actions_addopen(fileActions, STDOUT_FILENO, "/dev/null", O_RDONLY, 0);
			posix_spawn_file_actions_addopen(fileActions, STDERR_FILENO, "/dev/null", O_RDONLY, 0);
		}

		// Spawn process.
		err = posix_spawn(&pid, argv[0], fileActions, 0, argv, envv);
	}
	else
	{
		fileActions = new posix_spawn_file_actions_t;
		posix_spawn_file_actions_init(fileActions);
#if !defined(__RPI__)
		posix_spawn_file_actions_addchdir_np(fileActions, wd);
#endif
	
		// Spawn process.
		err = posix_spawn(&pid, argv[0], fileActions, 0, argv, envv);
	}

	// Free arguments.
	for (char** arg = argv; *arg != nullptr; ++arg)
		free(*arg);
	for (char** env = envv; *env != nullptr; ++env)
		free(*env);

	if (err != 0)
		return nullptr;

	return new ProcessLinux(pid, fileActions, childStdOut[0], childStdErr[0]);
}

Ref< ISharedMemory > OS::createSharedMemory(const std::wstring& name, uint32_t size) const
{
	Ref< SharedMemoryLinux > shm = new SharedMemoryLinux();
	if (shm->create(name, size))
		return shm;
	else
		return nullptr;
}

bool OS::setOwnProcessPriorityBias(int32_t priorityBias)
{
	return false;
}

bool OS::whereIs(const std::wstring& executable, Path& outPath) const
{
	std::wstring paths;

	// Get system "PATH" environment variable.
	if (!getEnvironment(L"PATH", paths))
		return false;

	// Try to locate binary in any of the paths specified in "PATH".
	for (auto path : StringSplit< std::wstring >(paths, L";:,"))
	{
		Ref< File > file = FileSystem::getInstance().get(path + L"/" + executable);
		if (file)
		{
			outPath = file->getPath();
			return true;
		}
	}

	return false;
}

OS::OS()
{
}

OS::~OS()
{
}

void OS::destroy()
{
	T_SAFE_RELEASE(this);
}

}
