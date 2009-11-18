#ifndef traktor_ui_custom_SyntaxLanguageJs_H
#define traktor_ui_custom_SyntaxLanguageJs_H

#include "Ui/Custom/SyntaxRichEdit/SyntaxLanguage.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

/*! \brief Syntax highlight JavaScript language.
 * \ingroup UIC
 */
class T_DLLCLASS SyntaxLanguageJs : public SyntaxLanguage
{
	T_RTTI_CLASS;

public:
	virtual void begin();

	virtual bool consume(const std::wstring& text, State& outState, int& outConsumedChars);

	virtual void newLine();
};

		}
	}
}

#endif	// traktor_ui_custom_SyntaxLanguageJs_H
