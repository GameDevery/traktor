#ifndef traktor_flash_AsXMLNode_H
#define traktor_flash_AsXMLNode_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;
class XMLNode;

/*! \brief XML node class.
 * \ingroup Flash
 */
class AsXMLNode : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsXMLNode(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);

private:
	ActionObject* XMLNode_get_attributes(XMLNode* self) const;

	void XMLNode_set_attributes(XMLNode* self, ActionObject* attributes) const;

	void XMLNode_get_childNodes(CallArgs& ca);

	XMLNode* XMLNode_get_firstChild(XMLNode* self) const;

	void XMLNode_set_firstChild(XMLNode* self, XMLNode* firstChild) const;

	XMLNode* XMLNode_get_lastChild(XMLNode* self) const;

	void XMLNode_set_lastChild(XMLNode* self, XMLNode* lastChild) const;

	std::wstring XMLNode_get_localName(XMLNode* self) const;

	std::wstring XMLNode_get_namespaceURI(XMLNode* self) const;

	XMLNode* XMLNode_get_nextSibling(XMLNode* self) const;

	void XMLNode_set_nextSibling(XMLNode* self, XMLNode* nextSibling) const;

	std::wstring XMLNode_get_nodeName(XMLNode* self) const;

	void XMLNode_set_nodeName(XMLNode* self, const std::wstring& nodeName) const;

	avm_number_t XMLNode_get_nodeType(XMLNode* self) const;

	void XMLNode_set_nodeType(XMLNode* self, avm_number_t nodeType) const;

	std::wstring XMLNode_get_nodeValue(XMLNode* self) const;

	void XMLNode_set_nodeValue(XMLNode* self, const std::wstring& nodeValue) const;

	XMLNode* XMLNode_get_parentNode(XMLNode* self) const;

	void XMLNode_set_parentNode(XMLNode* self, XMLNode* parentNode) const;

	std::wstring XMLNode_get_prefix(XMLNode* self) const;

	XMLNode* XMLNode_get_previousSibling(XMLNode* self) const;

	void XMLNode_set_previousSibling(XMLNode* self, XMLNode* previousSibling) const;

	void XMLNode_appendChild(CallArgs& ca);

	void XMLNode_cloneNode(CallArgs& ca);

	void XMLNode_getNamespaceForPrefix(CallArgs& ca);

	void XMLNode_getPrefixForNamespace(CallArgs& ca);

	bool XMLNode_hasChildNodes(XMLNode* self) const;

	void XMLNode_insertBefore(CallArgs& ca);

	void XMLNode_removeNode(CallArgs& ca);

	void XMLNode_toString(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsXMLNode_H
