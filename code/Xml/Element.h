#ifndef traktor_xml_Element_H
#define traktor_xml_Element_H

#include <vector>
#include "Core/RefArray.h"
#include "Xml/Node.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{

class OutputStream;

	namespace xml
	{
	
class Attribute;

/*! \brief XML Element.
 * \ingroup XML
 */
class T_DLLCLASS Element : public Node
{
	T_RTTI_CLASS;

public:
	Element(const std::wstring& name);
	
	virtual std::wstring getName() const;

	virtual void setName(const std::wstring& name);

	virtual std::wstring getValue() const;

	virtual void write(OutputStream& os) const;

	int get(const std::wstring& path, RefArray< Element >& elements);
	
	Ref< Element > getSingle(const std::wstring& path);

	std::wstring getPath() const;

	bool match(const std::wstring& condition);
	
	bool hasAttribute(const std::wstring& name) const;
	
	void setAttribute(const std::wstring& name, const std::wstring& value);
	
	Ref< Attribute > getFirstAttribute() const;
	
	Ref< Attribute > getLastAttribute() const;
	
	Ref< Attribute > getAttribute(const std::wstring& name) const;

	/*! \brief Get attribute by name, will always return an attribute.
	 * This method will always return an attribute, if named attribute
	 * doesn't exist a temporary one will be created with given value.
	 *
	 * \note
	 * In case of an temporary attribute it will not get linked
	 * with other attributes in the element thus getPrevious and getNext
	 * will always return null.
	 * 
	 * \param name Name of attribute.
	 * \return Attribute object.
	 */
	Ref< Attribute > getAttribute(const std::wstring& name, const std::wstring& defaultValue) const;
	
	Ref< Element > getChildElementByName(const std::wstring& name);

private:
	std::wstring m_name;
	Ref< Attribute > m_firstAttribute;
	Ref< Attribute > m_lastAttribute;
};
	
	}
}

#endif	// traktor_xml_Element_H
