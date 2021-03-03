#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/System/Win32/ProcessWin32.h"

namespace traktor
{
	namespace
	{

class PipeStream : public IStream
{
public:
	PipeStream(HANDLE hProcess, HANDLE hPipe)
	:	m_hProcess(hProcess)
	,	m_hPipe(hPipe)
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

	virtual int64_t tell() const
	{
		return 0;
	}

	virtual int64_t available() const
	{
		return 0;
	}

	virtual int64_t seek(SeekOriginType origin, int64_t offset)
	{
		return 0;
	}

	virtual int64_t read(void* block, int64_t nbytes)
	{
		DWORD npending = 0;
		if (!PeekNamedPipe(m_hPipe, nullptr, 0, nullptr, &npending, nullptr))
			npending = 0;

		if (npending == 0)
		{
			bool processTerminated = (WaitForSingleObject(m_hProcess, 0) == WAIT_OBJECT_0);
			return processTerminated ? -1 : 0;
		}

		DWORD nread = 0;
		if (!ReadFile(m_hPipe, block, (DWORD)min(npending, nbytes), &nread, NULL))
			return -1;

		return int64_t(nread);
	}

	virtual int64_t write(const void* block, int64_t nbytes)
	{
		return 0;
	}

	virtual void flush()
	{
	}

private:
	HANDLE m_hProcess;
	HANDLE m_hPipe;
};

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
	m_pipeStdOut = new PipeStream(m_hProcess, m_hStdOutRead);
	m_pipeStdErr = new PipeStream(m_hProcess, m_hStdErrRead);
}

ProcessWin32::~ProcessWin32()
{
	CloseHandle(m_hProcess);
	CloseHandle(m_hThread);
	CloseHandle(m_hStdInRead);
	CloseHandle(m_hStdInWrite);
	CloseHandle(m_hStdOutRead);
	CloseHandle(m_hStdOutWrite);
	CloseHandle(m_hStdErrRead);
	CloseHandle(m_hStdErrWrite);
}

bool ProcessWin32::setPriority(Priority priority)
{
	const DWORD c_priorityClasses[] =
	{
		IDLE_PRIORITY_CLASS,
		BELOW_NORMAL_PRIORITY_CLASS,
		NORMAL_PRIORITY_CLASS,
		ABOVE_NORMAL_PRIORITY_CLASS,
		HIGH_PRIORITY_CLASS
	};
	return SetPriorityClass(m_hProcess, c_priorityClasses[priority]) != 0;
}

bool ProcessWin32::wait(int32_t timeout)
{
	return WaitForSingleObject(m_hProcess, timeout >= 0 ? timeout : INFINITE) == WAIT_OBJECT_0;
}

IStream* ProcessWin32::getPipeStream(StdPipe pipe)
{
	if (pipe == SpStdOut)
		return m_pipeStdOut;
	else if (pipe == SpStdErr)
		return m_pipeStdErr;
	else
		return nullptr;
}

IStream* ProcessWin32::waitPipeStream(int32_t timeout)
{
	HANDLE hs[] = { m_hStdOutRead, m_hStdErrRead, m_hProcess };

	DWORD result = WaitForMultipleObjects(3, hs, FALSE, timeout);
	if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + 3)
	{
		if (result == WAIT_OBJECT_0)
			return m_pipeStdOut;
		else if (result == WAIT_OBJECT_0 + 1)
			return m_pipeStdErr;
		else
		{
			// Process signal, probably terminated.
			return nullptr;
		}
	}

	// Timeout.
	return nullptr;
}

bool ProcessWin32::signal(SignalType signalType)
{
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
}

bool ProcessWin32::terminate(int32_t exitCode)
{
	return TerminateProcess(m_hProcess, 0) == TRUE;
}

int32_t ProcessWin32::exitCode() const
{
	DWORD code = 0;

	if (!GetExitCodeProcess(m_hProcess, &code))
		return 0;

	return (int32_t)code;
}

}
