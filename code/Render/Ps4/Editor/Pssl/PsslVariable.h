#pragma once

#include <string>
#include "Core/Object.h"
#include "Render/Ps4/Editor/Pssl/PsslType.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
class PsslVariable : public Object
{
public:
	PsslVariable();

	PsslVariable(const std::wstring& name, PsslType type);

	const std::wstring& getName() const { return m_name; }

	void setType(PsslType type) { m_type = type; }

	PsslType getType() const { return m_type; }

	std::wstring cast(PsslType to) const;

private:
	std::wstring m_name;
	PsslType m_type;
};

	}
}
