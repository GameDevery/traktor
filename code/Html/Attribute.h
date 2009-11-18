#ifndef traktor_html_Attribute_H
#define traktor_html_Attribute_H

#include <string>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HTML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace html
	{

/*! \brief HTML DOM Attribute.
 * \ingroup HTML
 */
class T_DLLCLASS Attribute : public Object
{
	T_RTTI_CLASS;

public:
	Attribute(const std::wstring& name, const std::wstring& value = L"");
	
	const std::wstring& getName() const;

	void setValue(const std::wstring& value);

	const std::wstring& getValue() const;
	
	Ref< Attribute > getPrevious() const;
	
	Ref< Attribute > getNext() const;

private:
	friend class Element;

	std::wstring m_name;
	std::wstring m_value;
	Ref< Attribute > m_previous;
	Ref< Attribute > m_next;
};

	}
}

#endif	// traktor_html_Attribute_H
