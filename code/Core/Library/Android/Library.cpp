#include <dlfcn.h>
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Library", Library, Object)

bool Library::open(const Path& libraryName)
{
	return open(libraryName, std::vector< Path >(), true);
}

bool Library::open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths)
{
#if !defined(_DEBUG)
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L".so";
#else
	std::wstring resolved = std::wstring(L"lib") + libraryName.getPathName() + L"_d.so";
#endif

    // Prefer executable path first.
    {
        std::wstring library = /*mbstows(dirname(path)) + L"/"*/ L"$ORIGIN/" + resolved;
        m_handle = dlopen(wstombs(library).c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (m_handle)
        {
            T_DEBUG(L"Library \"" << library << L"\" loaded");
            return true;
        }
        else
            log::error << L"Failed to load library \"" << library << L"\"; " << mbstows(dlerror()) << L" (1)" << Endl;
    }

    // Try default search paths.
    {
        std::wstring library = resolved;
        m_handle = dlopen(wstombs(library).c_str(), RTLD_LAZY | RTLD_GLOBAL);
        if (m_handle)
        {
            T_DEBUG(L"Library \"" << library << L"\" loaded");
            return true;
        }
        else
             log::error << L"Failed to load library \"" << library << L"\"; " << mbstows(dlerror()) << L" (2)" << Endl;
   }

	log::error << L"Failed to load library \"" << libraryName.getPathName() << L"\"" << Endl;
	return false;
}

void Library::close()
{
	dlclose(m_handle);
}

void Library::detach()
{
}

void* Library::find(const std::wstring& symbol)
{
	std::string symb = wstombs(symbol);
	return dlsym(m_handle, symb.c_str());
}

Path Library::getPath() const
{
    return Path();
}

}
