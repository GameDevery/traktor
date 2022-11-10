#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Editor/Pipeline/File/FilePipelinePutStream.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.FilePipelinePutStream", FilePipelinePutStream, IStream)

FilePipelinePutStream::FilePipelinePutStream(IStream* file, const std::wstring& path)
:	m_file(file)
,	m_path(path)
{
}

void FilePipelinePutStream::close()
{
	if (m_file)
	{
		m_file->close();
		m_file = nullptr;

		if (!FileSystem::getInstance().move(
			m_path,
			m_path + L"~"
		))
			log::error << L"File pipeline cache failed; unable to commit cache entry " << m_path << L"." << Endl;
	}
}

bool FilePipelinePutStream::canRead() const
{
	return m_file->canRead();
}

bool FilePipelinePutStream::canWrite() const
{
	return m_file->canWrite();
}

bool FilePipelinePutStream::canSeek() const
{
	return m_file->canSeek();
}

int64_t FilePipelinePutStream::tell() const
{
	return m_file->tell();
}

int64_t FilePipelinePutStream::available() const
{
	return m_file->available();
}

int64_t FilePipelinePutStream::seek(SeekOriginType origin, int64_t offset)
{
	return m_file->seek(origin, offset);
}

int64_t FilePipelinePutStream::read(void* block, int64_t nbytes)
{
	return m_file->read(block, nbytes);
}

int64_t FilePipelinePutStream::write(const void* block, int64_t nbytes)
{
	return m_file->write(block, nbytes);
}

void FilePipelinePutStream::flush()
{
	return m_file->flush();
}

}
