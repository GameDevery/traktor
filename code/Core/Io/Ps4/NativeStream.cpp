#include <kernel.h>
#include "Core/Io/File.h"
#include "Core/Io/Ps4/NativeStream.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.NativeStream", NativeStream, IStream)

NativeStream::NativeStream(int fd, uint32_t mode)
:	m_fd(fd)
,	m_mode(mode)
,	m_fileSize(0)
{
	if ((m_mode & File::FmRead) != 0)
	{
		SceKernelStat sb = { 0 };
		sceKernelFstat(m_fd, &sb);
		m_fileSize = (size_t)sb.st_size;
	}
}

NativeStream::~NativeStream()
{
	close();
}

void NativeStream::close()
{
	if (m_fd)
	{
		sceKernelClose(m_fd);
		m_fd = 0;
	}
}

bool NativeStream::canRead() const
{
	return (m_fd != 0 && ((m_mode & File::FmRead) != 0));
}

bool NativeStream::canWrite() const
{
	return (m_fd != 0 && ((m_mode & (File::FmWrite | File::FmAppend)) != 0));
}

bool NativeStream::canSeek() const
{
	return (m_fd != 0);
}

int64_t NativeStream::tell() const
{
	return (m_fd != 0) ? (int64_t)sceKernelLseek(m_fd, 0, SCE_KERNEL_SEEK_CUR) : 0;
}

int64_t NativeStream::available() const
{
	return (m_fd != 0) ? ((int64_t)m_fileSize - tell()) : 0;
}

int64_t NativeStream::seek(SeekOriginType origin, int64_t offset)
{
	if (m_fd == 0)
		return 0;

	const int whence[] = { SCE_KERNEL_SEEK_CUR, SCE_KERNEL_SEEK_END, SCE_KERNEL_SEEK_SET };
	return (int64_t)sceKernelLseek(m_fd, offset, whence[origin]);
}

int64_t NativeStream::read(void* block, int64_t nbytes)
{
	if (m_fd == 0)
		return 0;

	return int64_t(sceKernelRead(m_fd, block, nbytes));
}

int64_t NativeStream::write(const void* block, int64_t nbytes)
{
	if (m_fd == 0)
		return 0;

	return int64_t(sceKernelWrite(m_fd, block, nbytes));
}

void NativeStream::flush()
{
	if (m_fd != 0)
		sceKernelFsync(m_fd);
}

}