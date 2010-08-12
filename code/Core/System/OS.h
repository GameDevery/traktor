#ifndef traktor_OS_H
#define traktor_OS_H

#include <map>
#include "Core/Ref.h"
#include "Core/Singleton/ISingleton.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;
class IProcess;
class ISharedMemory;

/*! \brief Operative System information.
 * \ingroup Core
 */
class T_DLLCLASS OS : public ISingleton
{
public:
	typedef std::map< std::wstring, std::wstring > envmap_t;

	static OS& getInstance();

	/*! \brief Get number of CPU cores.
	 *
	 * \return Number of CPU cores.
	 */
	uint32_t getCPUCoreCount() const;

	/*! \brief Get network name of computer.
	 *
	 * \return Computer network name.
	 */
	std::wstring getComputerName() const;

	/*! \brief Get name of currently logged in user.
	 *
	 * \return User name.
	 */
	std::wstring getCurrentUser() const;

	/*! \brief Get path to user home.
	 *
	 * \return User home path.
	 */
	std::wstring getUserHomePath() const;

	/*! \brief Get path to user application data.
	 *
	 * \return User application data path.
	 */
	std::wstring getUserApplicationDataPath() const;

	/*! \brief Get path to writable folder.
	 *
	 * \return Writable folder.
	 */
	std::wstring getWritableFolderPath() const;

	/*! \brief Launch associate editor of given file.
	 *
	 * \return True if associate editor opened successfully.
	 */
	bool editFile(const Path& file) const;

	/*! \brief Open file explorer to given file.
	 *
	 * \return True if explorer opened successfully.
	 */
	bool exploreFile(const Path& file) const;

	/*! \brief Get process's environment variables.
	 *
	 * \return Environment variables.
	 */
	envmap_t getEnvironment() const;
	
	/*! \brief Get environment variable value.
	 *
	 * \param name Name of variable.
	 * \param outValue Value of variable if found.
	 * \return True if variable found.
	 */
	bool getEnvironment(const std::wstring& name, std::wstring& outValue) const;

	/*! \brief Execute command.
	 *
	 * \param file Path to executable.
	 * \param commandLine Execute command line.
	 * \param workingDirectory Process's initial working directory.
	 * \param envmap Optional environment variables.
	 * \param redirect Redirect standard IO.
	 * \param mute Mute spawn process's output.
	 * \return Process instance, null if unable to execute.
	 */
	Ref< IProcess > execute(
		const Path& file,
		const std::wstring& commandLine,
		const Path& workingDirectory,
		const envmap_t* envmap,
		bool redirect,
		bool mute
	) const;

	/*! \brief Create shared memory object.
	 *
	 * \param name Name of shared memory object.
	 * \param size Size of shared memory.
	 * \return Shared memory object.
	 */
	Ref< ISharedMemory > createSharedMemory(const std::wstring& name, uint32_t size) const;

protected:
	OS();

	virtual ~OS();

	virtual void destroy();
};

}

#endif	// traktor_OS_H
