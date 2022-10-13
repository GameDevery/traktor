#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Io/StringOutputStream.h"
#include "Xml/Attribute.h"
#include "Xml/Document.h"
#include "Xml/Element.h"
#include "Xml/Text.h"
#include "Xml/XmlClassFactory.h"

namespace traktor::xml
{
	namespace
	{

std::wstring xml_Node_write(xml::Node* node)
{
	StringOutputStream ss;
	node->write(ss);
	return ss.str();
}

RefArray< xml::Element > xml_Element_get(xml::Element* element, const std::wstring& path)
{
	RefArray< xml::Element > elements;
	element->get(path, elements);
	return elements;
}

Ref< xml::Attribute > xml_Element_getAttribute_1(xml::Element* element, const std::wstring& name)
{
	return element->getAttribute(name);
}

Ref< xml::Attribute > xml_Element_getAttribute_2(xml::Element* element, const std::wstring& name, const std::wstring& defaultValue)
{
	return element->getAttribute(name, defaultValue);
}

RefArray< xml::Element > xml_Document_get(xml::Document* document, const std::wstring& path)
{
	RefArray< xml::Element > elements;
	document->get(path, elements);
	return elements;
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.xml.XmlClassFactory", 0, XmlClassFactory, IRuntimeClassFactory)

void XmlClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< xml::Attribute > > classAttribute = new AutoRuntimeClass< xml::Attribute >();
	classAttribute->addConstructor< const std::wstring&, const std::wstring& >();
	classAttribute->addProperty("name", &xml::Attribute::setName, &xml::Attribute::getName);
	classAttribute->addProperty("value", &xml::Attribute::setValue, &xml::Attribute::getValue);
	classAttribute->addProperty("previous", &xml::Attribute::getPrevious);
	classAttribute->addProperty("next", &xml::Attribute::getNext);
	registrar->registerClass(classAttribute);

	Ref< AutoRuntimeClass< xml::Node > > classNode = new AutoRuntimeClass< xml::Node >();
	classNode->addProperty("name", &xml::Node::setName, &xml::Node::getName);
	classNode->addProperty("value", &xml::Node::setValue, &xml::Node::getValue);
	classNode->addProperty("parent", &xml::Node::getParent);
	classNode->addProperty("previousSibling", &xml::Node::getPreviousSibling);
	classNode->addProperty("nextSibling", &xml::Node::getNextSibling);
	classNode->addProperty("firstChild", &xml::Node::getFirstChild);
	classNode->addProperty("lastChild", &xml::Node::getLastChild);
	classNode->addMethod("write", &xml_Node_write);
	classNode->addMethod("unlink", &xml::Node::unlink);
	classNode->addMethod("addChild", &xml::Node::addChild);
	classNode->addMethod("removeChild", &xml::Node::removeChild);
	classNode->addMethod("removeAllChildren", &xml::Node::removeAllChildren);
	classNode->addMethod("insertBefore", &xml::Node::insertBefore);
	classNode->addMethod("insertAfter", &xml::Node::insertAfter);
	registrar->registerClass(classNode);

	Ref< AutoRuntimeClass< xml::Text > > classText = new AutoRuntimeClass< xml::Text >();
	classText->addConstructor< const std::wstring& >();
	classText->addMethod("clone", &xml::Text::clone);
	registrar->registerClass(classText);

	Ref< AutoRuntimeClass< xml::Element > > classElement = new AutoRuntimeClass< xml::Element >();
	classElement->addConstructor< const std::wstring& >();
	classElement->addProperty("firstAttribute", &xml::Element::getFirstAttribute);
	classElement->addProperty("lastAttribute", &xml::Element::getLastAttribute);
	classElement->addMethod("get", &xml_Element_get);
	classElement->addMethod("getSingle", &xml::Element::getSingle);
	classElement->addMethod("getPath", &xml::Element::getPath);
	classElement->addMethod("match", &xml::Element::match);
	classElement->addMethod("hasAttribute", &xml::Element::hasAttribute);
	classElement->addMethod("setAttribute", &xml::Element::setAttribute);
	classElement->addMethod("getAttribute", &xml_Element_getAttribute_1);
	classElement->addMethod("getAttribute", &xml_Element_getAttribute_2);
	classElement->addMethod("getChildElementByName", &xml::Element::getChildElementByName);
	classElement->addMethod("clone", &xml::Element::clone);
	registrar->registerClass(classElement);

	Ref< AutoRuntimeClass< xml::Document > > classDocument = new AutoRuntimeClass< xml::Document >();
	classDocument->addConstructor();
	classDocument->addProperty("documentElement", &xml::Document::setDocumentElement, &xml::Document::getDocumentElement);
	classDocument->addMethod("loadFromFile", &xml::Document::loadFromFile);
	classDocument->addMethod("loadFromStream", &xml::Document::loadFromStream);
	classDocument->addMethod("loadFromText", &xml::Document::loadFromText);
	classDocument->addMethod("saveAsFile", &xml::Document::saveAsFile);
	classDocument->addMethod("saveIntoStream", &xml::Document::saveIntoStream);
	classDocument->addMethod("get", &xml_Document_get);
	classDocument->addMethod("getSingle", &xml::Document::getSingle);
	classDocument->addMethod("clone", &xml::Document::clone);
	registrar->registerClass(classDocument);
}

}
