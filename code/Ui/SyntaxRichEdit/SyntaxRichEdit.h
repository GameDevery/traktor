#pragma once

#include <list>
#include "Ui/RichEdit/RichEdit.h"
#include "Ui/SyntaxRichEdit/SyntaxTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class SyntaxLanguage;

/*! RichEdit control with automatic syntax highlighting.
 * \ingroup UI
 */
class T_DLLCLASS SyntaxRichEdit : public RichEdit
{
	T_RTTI_CLASS;

public:
	SyntaxRichEdit();

	bool create(Widget* parent, const std::wstring& text = L"", int32_t style = WsDoubleBuffer);

	void setLanguage(const SyntaxLanguage* language);

	const SyntaxLanguage* getLanguage() const;

	void setErrorHighlight(int32_t line);

	void getOutline(std::list< SyntaxOutline >& outOutline) const;

	void updateLanguage(int32_t fromLine, int32_t toLine);

	virtual void setText(const std::wstring& text) override;

private:
	Ref< const SyntaxLanguage > m_language;
	int32_t m_attributeDefault[2];
	int32_t m_attributeString[2];
	int32_t m_attributeNumber[2];
	int32_t m_attributeSelf[2];
	int32_t m_attributeComment[2];
	int32_t m_attributeFunction[2];
	int32_t m_attributeType[2];
	int32_t m_attributeKeyword[2];
	int32_t m_attributeSpecial[2];
	int32_t m_attributePreprocessor[2];
	int32_t m_attributeError[2];

	void eventChange(ContentChangeEvent* event);
};

	}
}

