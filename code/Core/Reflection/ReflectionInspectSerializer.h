#pragma once

#include "Core/RefArray.h"
#include "Core/Serialization/Serializer.h"

namespace traktor
{

class ReflectionMember;
class RfmCompound;

/*! Internal reflection inspection serializer.
 * \ingroup Core
 */
class ReflectionInspectSerializer : public Serializer
{
	T_RTTI_CLASS;

public:
	ReflectionInspectSerializer(RfmCompound* compound);

	virtual Direction getDirection() const override final;

	virtual void operator >> (const Member< bool >& m) override final;

	virtual void operator >> (const Member< int8_t >& m) override final;

	virtual void operator >> (const Member< uint8_t >& m) override final;

	virtual void operator >> (const Member< int16_t >& m) override final;

	virtual void operator >> (const Member< uint16_t >& m) override final;

	virtual void operator >> (const Member< int32_t >& m) override final;

	virtual void operator >> (const Member< uint32_t >& m) override final;

	virtual void operator >> (const Member< int64_t >& m) override final;

	virtual void operator >> (const Member< uint64_t >& m) override final;

	virtual void operator >> (const Member< float >& m) override final;

	virtual void operator >> (const Member< double >& m) override final;

	virtual void operator >> (const Member< std::string >& m) override final;

	virtual void operator >> (const Member< std::wstring >& m) override final;

	virtual void operator >> (const Member< Guid >& m) override final;

	virtual void operator >> (const Member< Path >& m) override final;

	virtual void operator >> (const Member< Color4ub >& m) override final;

	virtual void operator >> (const Member< Color4f >& m) override final;

	virtual void operator >> (const Member< Scalar >& m) override final;

	virtual void operator >> (const Member< Vector2 >& m) override final;

	virtual void operator >> (const Member< Vector4 >& m) override final;

	virtual void operator >> (const Member< Matrix33 >& m) override final;

	virtual void operator >> (const Member< Matrix44 >& m) override final;

	virtual void operator >> (const Member< Quaternion >& m) override final;

	virtual void operator >> (const Member< ISerializable* >& m) override final;

	virtual void operator >> (const Member< void* >& m) override final;

	virtual void operator >> (const MemberArray& m) override final;

	virtual void operator >> (const MemberComplex& m) override final;

	virtual void operator >> (const MemberEnumBase& m) override final;

private:
	friend class Reflection;

	Ref< RfmCompound > m_compoundMember;

	bool addMember(ReflectionMember* member);
};

}

