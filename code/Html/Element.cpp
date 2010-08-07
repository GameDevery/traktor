#include <sstream>
#include "Html/Element.h"
#include "Html/Attribute.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace html
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.html.Element", Element, Node)

Element::Element(const std::wstring& name)
:	m_name(name)
{
}

std::wstring Element::getName() const
{
	return m_name;
}

std::wstring Element::getValue() const
{
	std::wstringstream ss;
	for (Ref< Node > child = getFirstChild(); child; child = child->getNextSibling())
		ss << child->getValue();
	return ss.str();
}

bool Element::hasAttribute(const std::wstring& name) const
{
	return bool(getAttribute(name) != 0);
}

void Element::setAttribute(const std::wstring& name, const std::wstring& value)
{
	Ref< Attribute > attr = getAttribute(name);
	if (attr == 0)
	{
		attr = new Attribute(name);
		attr->m_previous = 0;
		attr->m_next = m_firstAttribute;
		
		if (m_firstAttribute != 0)
			m_firstAttribute->m_previous = attr;

		m_firstAttribute = attr;
	}
	attr->setValue(value);
}

Ref< Attribute > Element::getFirstAttribute() const
{
	return m_firstAttribute;
}

Ref< Attribute > Element::getLastAttribute() const
{
	return m_lastAttribute;
}

Ref< Attribute > Element::getAttribute(const std::wstring& name) const
{
	Ref< Attribute > attr;
	for (attr = m_firstAttribute; attr != 0; attr = attr->getNext())
	{
		if (attr->getName() == name)
			break;
	}
	return attr;
}

Ref< Element > Element::getPreviousElementSibling() const
{
	Ref< Node > node = getPreviousSibling();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getPreviousSibling();
	}
	return static_cast< Element* >(node.ptr());
}

Ref< Element > Element::getNextElementSibling() const
{
	Ref< Node > node = getNextSibling();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getNextSibling();
	}
	return static_cast< Element* >(node.ptr());
}

Ref< Element > Element::getFirstElementChild() const
{
	Ref< Node > node = getFirstChild();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getNextSibling();
	}
	return static_cast< Element* >(node.ptr());
}

Ref< Element > Element::getLastElementChild() const
{
	Ref< Node > node = getLastChild();
	while (node)
	{
		if (is_a< Element >(node))
			break;
		node = node->getPreviousSibling();
	}
	return static_cast< Element* >(node.ptr());
}

void Element::writeHtml(IStream* stream)
{
	static int depth = 0; depth++;
	std::wstringstream ss;

	for (int i = 0; i < depth - 1; ++i)
		ss << L"\t";

	ss << L"<" << m_name;
	for (Ref< Attribute > attribute = m_firstAttribute; attribute; attribute = attribute->getNext())
		ss << L" " << attribute->getName() << L"=\"" << attribute->getValue() << L"\"";

	if (getFirstChild())
	{
		ss << L">";
		if (is_a< Element >(getFirstChild()))
			ss << std::endl;

		std::wstring tmp = ss.str(); ss.str(L"");
		stream->write(tmp.c_str(), int(tmp.length()));

		for (Ref< Node > child = getFirstChild(); child; child = child->getNextSibling())
			child->writeHtml(stream);

		if (is_a< Element >(getFirstChild()))
		{
			for (int i = 0; i < depth - 1; ++i)
				ss << L"\t";
			ss << std::endl;
		}
		ss << L"</" << m_name << L">";
	}
	else
		ss << L"/>" << std::endl;

	std::wstring tmp = ss.str();
	stream->write(tmp.c_str(), int(tmp.length()));

	depth--;
}

	}
}
