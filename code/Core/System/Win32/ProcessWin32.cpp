#include "Core/Io/IStream.h"
#include "Core/System/Win32/ProcessWin32.h"

namespace traktor
{
	namespace
	{

#if !defined(WINCE)

class PipeStream : public IStream
{
public:
	PipeStream(HANDLE hPipe)
	:	m_hPipe(hPipe)
	{
	}

	virtual void close()
	{
	}

	virtual bool canRead() const
	{
		return true;
	}

	virtual bool canWrite() const
	{
		return false;
	}

	virtual bool canSeek() const
	{
		return false;
	}

	virtual int tell() const
	{
		return 0;
	}

	virtual int available() const
	{
		DWORD dwAvail = 0;
		PeekNamedPipe(m_hPipe, NULL, 0, NULL, &dwAvail, NULL);
		return int(dwAvail);
	}

	virtual int seek(SeekOriginType origin, int offset)
	{
		return 0;
	}

	virtual int read(void* block, int nbytes)
	{
		int navail = available();
		if (navail < 0)
			return -1;

		int nread = std::min< int >(nbytes, navail);
		if (nread == 0)
			return 0;

		DWORD dwRead = 0;
		if (!ReadFile(m_hPipe, block, nread, &dwRead, NULL))
			return -1;

		return int(dwRead);
	}

	virtual int write(const void* block, int nbytes)
	{
		return 0;
	}

	virtual void flush()
	{
	}

private:
	HANDLE m_hPipe;
};

#endif

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ProcessWin32", ProcessWin32, IProcess)

ProcessWin32::ProcessWin32(
	HANDLE hProcess,
	DWORD dwProcessId,
	HANDLE hThread,
	HANDLE hStdInRead,
	HANDLE hStdInWrite,
	HANDLE hStdOutRead,
	HANDLE hStdOutWrite,
	HANDLE hStdErrRead,
	HANDLE hStdErrWrite
)
:	m_hProcess(hProcess)
,	m_dwProcessId(dwProcessId)
,	m_hThread(hThread)
,	m_hStdInRead(hStdInRead)
,	m_hStdInWrite(hStdInWrite)
,	m_hStdOutRead(hStdOutRead)
,	m_hStdOutWrite(hStdOutWrite)
,	m_hStdErrRead(hStdErrRead)
,	m_hStdErrWrite(hStdErrWrite)
{
#if !defined(WINCE)
	m_pipeStdOut = new PipeStream(m_hStdOutRead);
	m_pipeStdErr = new PipeStream(m_hStdErrRead);
#endif
}

ProcessWin32::~ProcessWin32()
{
	CloseHandle(m_hProcess);
	CloseHandle(m_hThread);
#if !defined(WINCE)
	CloseHandle(m_hStdInRead);
	CloseHandle(m_hStdInWrite);
	CloseHandle(m_hStdOutRead);
	CloseHandle(m_hStdOutWrite);
	CloseHandle(m_hStdErrRead);
	CloseHandle(m_hStdErrWrite);
#endif
}

bool ProcessWin32::wait(int32_t timeout)
{
	return WaitForSingleObject(m_hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0;
}

Ref< IStream > ProcessWin32::getPipeStream(StdPipe pipe)
{
#if !defined(WINCE)
	if (pipe == SpStdOut)
		return m_pipeStdOut;
	else if (pipe == SpStdErr)
		return m_pipeStdErr;
	else
#endif
		return 0;
}

bool ProcessWin32::signal(SignalType signalType)
{
#if !defined(WINCE)
	switch (signalType)
	{
	case StCtrlC:
		if (!GenerateConsoleCtrlEvent(CTRL_C_EVENT, m_dwProcessId))
			return false;
		break;
	case StCtrlBreak:
		if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, m_dwProcessId))
			return false;
		break;
	default:
		return false;
	}
	return true;
#else
	return false;
#endif
}

int32_t ProcessWin32::exitCode() const
{
	DWORD code;

	if (!GetExitCodeProcess(m_hProcess, &code))
		return 0;

	return (int32_t)code;
}

}
