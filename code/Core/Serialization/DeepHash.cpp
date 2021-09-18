#include "Core/Guid.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Core/Math/Color4f.h"
#include "Core/Math/Color4ub.h"
#include "Core/Math/Matrix33.h"
#include "Core/Math/Matrix44.h"
#include "Core/Math/Quaternion.h"
#include "Core/Math/Vector2.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Serialization/DeepHash.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberArray.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace
	{

class HashSerializer : public Serializer
{
public:
	HashSerializer(Murmur3& hasher)
	:	m_hasher(hasher)
	,	m_writtenCount(0)
	{
	}

	virtual Direction getDirection() const override final
	{
		return Direction::Write;
	}

	virtual void operator >> (const Member< bool >& m) override final
	{
		m_hasher.feed< uint8_t >(m ? 0xff : 0x00);
	}

	virtual void operator >> (const Member< int8_t >& m) override final
	{
		m_hasher.feed< int8_t >(m);
	}

	virtual void operator >> (const Member< uint8_t >& m) override final
	{
		m_hasher.feed< uint8_t >(m);
	}

	virtual void operator >> (const Member< int16_t >& m) override final
	{
		m_hasher.feed< int64_t >(m);
	}

	virtual void operator >> (const Member< uint16_t >& m) override final
	{
		m_hasher.feed< uint16_t >(m);
	}

	virtual void operator >> (const Member< int32_t >& m) override final
	{
		m_hasher.feed< int32_t >(m);
	}

	virtual void operator >> (const Member< uint32_t >& m) override final
	{
		m_hasher.feed< uint32_t >(m);
	}

	virtual void operator >> (const Member< int64_t >& m) override final
	{
		m_hasher.feed< int64_t >(m);
	}

	virtual void operator >> (const Member< uint64_t >& m) override final
	{
		m_hasher.feed< uint64_t >(m);
	}

	virtual void operator >> (const Member< float >& m) override final
	{
		m_hasher.feed< float >(m);
	}

	virtual void operator >> (const Member< double >& m) override final
	{
		m_hasher.feed< double >(m);
	}

	virtual void operator >> (const Member< std::string >& m) override final
	{
		const std::string& str = m;
		m_hasher.feed(str.data(), str.size());
	}

	virtual void operator >> (const Member< std::wstring >& m) override final
	{
		const std::wstring& str = m;
		for (size_t i = 0; i < str.length(); ++i)
		{
			uint32_t ch = (uint32_t)str[i];
			m_hasher.feed(ch);
		}
	}

	virtual void operator >> (const Member< Guid >& m) override final
	{
		const Guid& guid = m;
		m_hasher.feed((const uint8_t*)guid, 16);
	}

	virtual void operator >> (const Member< Path >& m) override final
	{
		std::wstring path = m->getOriginal();
		for (size_t i = 0; i < path.length(); ++i)
		{
			uint32_t ch = (uint32_t)path[i];
			m_hasher.feed(ch);
		}
	}

	virtual void operator >> (const Member< Color4ub >& m) override final
	{
		m_hasher.feed< uint8_t >(m->r);
		m_hasher.feed< uint8_t >(m->g);
		m_hasher.feed< uint8_t >(m->b);
		m_hasher.feed< uint8_t >(m->a);
	}

	virtual void operator >> (const Member< Color4f >& m) override final
	{
		float T_MATH_ALIGN16 e[4];
		m->storeAligned(e);
		m_hasher.feed(e, sizeof(e));
	}

	virtual void operator >> (const Member< Scalar >& m) override final
	{
		float v = (float)(const Scalar&)m;
		m_hasher.feed< float >(v);
	}

	virtual void operator >> (const Member< Vector2 >& m) override final
	{
		m_hasher.feed< float >(m->x);
		m_hasher.feed< float >(m->y);
	}

	virtual void operator >> (const Member< Vector4 >& m) override final
	{
		float T_MATH_ALIGN16 e[4];
		m->storeAligned(e);
		m_hasher.feed(e, sizeof(e));
	}

	virtual void operator >> (const Member< Matrix33 >& m) override final
	{
		for (int i = 0; i < 3 * 3; ++i)
			m_hasher.feed< float >(m->m[i]);
	}

	virtual void operator >> (const Member< Matrix44 >& m) override final
	{
		float T_MATH_ALIGN16 e[16];
		m->storeAligned(e);
		m_hasher.feed(e, sizeof(e));
	}

	virtual void operator >> (const Member< Quaternion >& m) override final
	{
		float T_MATH_ALIGN16 e[4];
		m->e.storeAligned(e);
		m_hasher.feed(e, sizeof(e));
	}

	virtual void operator >> (const Member< ISerializable* >& m) override final
	{
		ISerializable* object = *m;
		if (object)
		{
			auto it = m_written.find(object);
			if (it == m_written.end())
			{
				m_written.insert(std::make_pair(object, ++m_writtenCount));

				const wchar_t* const typeName = type_of(object).getName();
				for (size_t i = 0; typeName[i]; ++i)
				{
					uint32_t ch = (uint32_t)typeName[i];
					m_hasher.feed(ch);
				}

				serialize(object);
			}
			else
				m_hasher.feed< uint32_t >(it->second);
		}
		else
			m_hasher.feed< uint32_t >(0);
	}

	virtual void operator >> (const Member< void* >& m) override final
	{
		m_hasher.feed(m.getBlob(), m.getBlobSize());
	}

	virtual void operator >> (const MemberArray& m) override final
	{
		size_t size = m.size();
		for (size_t i = 0; i < size; ++i)
			m.write(*this);
	}

	virtual void operator >> (const MemberComplex& m) override final
	{
		m.serialize(*this);
	}

	virtual void operator >> (const MemberEnumBase& m) override final
	{
		m.serialize(*this);
	}

private:
	Murmur3& m_hasher;
	SmallMap< ISerializable*, uint32_t > m_written;
	uint32_t m_writtenCount;
};

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.DeepHash", DeepHash, Object)

DeepHash::DeepHash(const ISerializable* object)
:	m_hash(0)
{
	if (object)
	{
		Murmur3 a;
		a.begin();
		HashSerializer(a).writeObject(object);
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
