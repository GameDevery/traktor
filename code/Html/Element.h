#pragma once

#include "Html/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace html
	{

class Attribute;

/*! \brief HTML DOM Element.
 * \ingroup HTML
 */
class T_DLLCLASS Element : public Node
{
	T_RTTI_CLASS;

public:
	Element(const std::wstring& name);

	virtual std::wstring getName() const override final;

	virtual std::wstring getValue() const override final;

	bool hasAttribute(const std::wstring& name) const;

	void setAttribute(const std::wstring& name, const std::wstring& value);

	Attribute* getFirstAttribute() const;

	Attribute* getLastAttribute() const;

	Attribute* getAttribute(const std::wstring& name) const;

	Element* getPreviousElementSibling() const;

	Element* getNextElementSibling() const;

	Element* getFirstElementChild() const;

	Element* getLastElementChild() const;

	void innerHTML(OutputStream& os) const;

	virtual void toString(OutputStream& os) const override final;

private:
	std::wstring m_name;
	Ref< Attribute > m_firstAttribute;
	Ref< Attribute > m_lastAttribute;
};

	}
}

