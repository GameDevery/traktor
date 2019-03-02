#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Functor/Functor.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/System/OS.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Drawing/Image.h"
#include "Flash/Movie.h"
#include "Flash/MovieFactory.h"
#include "Flash/MovieLoader.h"
#include "Flash/Optimizer.h"
#include "Flash/Sprite.h"
#include "Flash/SwfReader.h"
#include "Net/UrlConnection.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

class MovieLoaderHandle : public IMovieLoader::IHandle
{
public:
	MovieLoaderHandle(const std::wstring& url, const std::wstring& cacheDirectory, bool merge, bool triangulate, bool includeAS)
	:	m_url(url)
	,	m_cacheDirectory(cacheDirectory)
	,	m_merge(merge)
	,	m_triangulate(triangulate)
	,	m_includeAS(includeAS)
	{
		m_job = JobManager::getInstance().add(makeFunctor< MovieLoaderHandle >(this, &MovieLoaderHandle::loader));
	}

	virtual bool wait() override final
	{
		return m_job ? m_job->wait() : true;
	}

	virtual bool ready() override final
	{
		return m_job ? m_job->stopped() : true;
	}

	virtual bool succeeded() override final
	{
		return wait() ? (m_movie != 0) : false;
	}

	virtual Ref< Movie > get() override final
	{
		return wait() ? m_movie : 0;
	}

private:
	std::wstring m_url;
	std::wstring m_cacheDirectory;
	bool m_merge;
	bool m_triangulate;
	bool m_includeAS;
	Ref< Job > m_job;
	Ref< Movie > m_movie;

	void loader()
	{
		std::wstring cacheFileName = net::Url::encode(m_url);

		if (m_merge || m_triangulate || m_includeAS)
		{
			cacheFileName += L"_";
			if (m_merge)
				cacheFileName += L"m";
			if (m_triangulate)
				cacheFileName += L"t";
			if (m_includeAS)
				cacheFileName += L"i";
		}

		if (!m_cacheDirectory.empty())
		{
			Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmRead);
			if (f)
			{
				compress::InflateStreamLzf is(f);
				BufferedStream bs(&is);
				m_movie = BinarySerializer(&bs).readObject< Movie >();
				bs.close();
			}
			if (m_movie)
				return;
		}

		Ref< net::UrlConnection > connection = net::UrlConnection::open(m_url);
		if (!connection)
			return;

		Ref< IStream > s = connection->getStream();
		T_ASSERT (s);

		std::wstring tempFile;
		Ref< IStream > d;

		for (int32_t i = 0; i < 10; ++i)
		{
			tempFile = OS::getInstance().getWritableFolderPath() + L"/" + cacheFileName + L"_" + toString(i);
			if ((d = FileSystem::getInstance().open(tempFile, File::FmWrite)) != 0)
				break;
		}
		if (!d)
			return;

		if (!StreamCopy(d, s).execute())
			return;

		d->close();
		s->close();

		d = FileSystem::getInstance().open(tempFile, File::FmRead);
		if (!d)
			return;

		std::wstring ext = toLower(traktor::Path(m_url).getExtension());

		// Try to load image and embedd into a movie first, if extension
		// not supported then this fail quickly.
		Ref< drawing::Image > image = drawing::Image::load(d, ext);
		if (image)
			m_movie = MovieFactory(false).createMovieFromImage(image);
		else
		{
			SwfReader swfReader(d);
			m_movie = MovieFactory(m_includeAS).createMovie(&swfReader);
		}

		d->close();
		FileSystem::getInstance().remove(tempFile);

		if (!m_movie)
			return;

		if (m_merge)
			m_movie = Optimizer().merge(m_movie);

		if (m_triangulate)
			Optimizer().triangulate(m_movie, false);

		if (!m_cacheDirectory.empty())
		{
			Ref< IStream > f = FileSystem::getInstance().open(m_cacheDirectory + L"/" + cacheFileName, File::FmWrite);
			if (f)
			{
				compress::DeflateStreamLzf ds(f);
				BufferedStream bs(&ds);
				BinarySerializer(&bs).writeObject(m_movie);
				bs.close();
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.MovieLoader", MovieLoader, IMovieLoader)

MovieLoader::MovieLoader()
:	m_merge(false)
,	m_triangulate(false)
,	m_includeAS(true)
{
}

void MovieLoader::setCacheDirectory(const std::wstring& cacheDirectory)
{
	m_cacheDirectory = cacheDirectory;
}

void MovieLoader::setMerge(bool merge)
{
	m_merge = merge;
}

void MovieLoader::setTriangulate(bool triangulate)
{
	m_triangulate = triangulate;
}

void MovieLoader::setIncludeAS(bool includeAS)
{
	m_includeAS = includeAS;
}

Ref< IMovieLoader::IHandle > MovieLoader::loadAsync(const std::wstring& url) const
{
	return new MovieLoaderHandle(url, m_cacheDirectory, m_merge, m_triangulate, m_includeAS);
}

Ref< Movie > MovieLoader::load(const std::wstring& url) const
{
	Ref< IHandle > handle = loadAsync(url);
	return handle ? handle->get() : 0;
}

	}
}
