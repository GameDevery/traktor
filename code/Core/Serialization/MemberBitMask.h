#ifndef traktor_MemberBitMask_H
#define traktor_MemberBitMask_H

#include "Core/Serialization/MemberComplex.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Bit mask member.
 * \ingroup Core
 */
class T_DLLCLASS MemberBitMask : public MemberComplex
{
public:
	struct Bit
	{
		const wchar_t* const id;
		uint32_t mask;
	};

	MemberBitMask(const wchar_t* const name, uint32_t& bm, const Bit* bits);

	virtual bool serialize(ISerializer& s) const;

private:
	const Bit* m_bits;
	uint32_t& m_bm;
};

}

#endif	// traktor_MemberBitMask_H
