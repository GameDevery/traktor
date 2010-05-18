#include "Core/Io/IStream.h"
#include "Core/Misc/Adler32.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/BinarySerializer.h"

namespace traktor
{
	namespace
	{

/*! \brief Feed stream directly into Adler32 checksum. */
class Adler32Stream : public IStream
{
public:
	Adler32Stream(Adler32& adler)
	:	m_adler(adler)
	{
	}

	virtual void close() {}
	virtual bool canRead() const { return false; }
	virtual bool canWrite() const { return true; }
	virtual bool canSeek() const { return false; }
	virtual int tell() const { return 0; }
	virtual int available() const { return 0; }
	virtual int seek(SeekOriginType origin, int offset) { return 0; }
	virtual int read(void* block, int nbytes) { return 0; }

	virtual int write(const void* block, int nbytes)
	{
		m_adler.feed(block, nbytes);
		return nbytes;
	}

	virtual void flush() {}

private:
	Adler32& m_adler;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepHash", DeepHash, Object)

DeepHash::DeepHash(const ISerializable* object)
:	m_hash(0)
{
	if (object)
	{
		Adler32 a;
		Adler32Stream as(a);

		a.begin();
		BinarySerializer(&as).writeObject(object);
		a.end();

		m_hash = a.get();
	}
}

uint32_t DeepHash::get() const
{
	return m_hash;
}

bool DeepHash::operator == (const DeepHash& hash) const
{
	return m_hash == hash.m_hash;
}

bool DeepHash::operator != (const DeepHash& hash) const
{
	return m_hash != hash.m_hash;
}

bool DeepHash::operator == (const DeepHash* hash) const
{
	return m_hash == hash->m_hash;
}

bool DeepHash::operator != (const DeepHash* hash) const
{
	return m_hash != hash->m_hash;
}

bool DeepHash::operator == (const ISerializable* object) const
{
	DeepHash hash(object);
	return m_hash == hash.m_hash;
}

bool DeepHash::operator != (const ISerializable* object) const
{
	DeepHash hash(object);
	return m_hash != hash.m_hash;
}

bool DeepHash::operator == (uint32_t hash) const
{
	return m_hash == hash;
}

bool DeepHash::operator != (uint32_t hash) const
{
	return m_hash != hash;
}

}
