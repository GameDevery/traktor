#pragma once

#include <functional>
#include <list>
#include "Ui/Widget.h"

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

class IBitmap;
class ScrollBar;
class SearchControl;
class SearchEvent;

/*! \brief Rich, multi-line, text editor.
 * \ingroup UI
 */
class T_DLLCLASS RichEdit : public Widget
{
	T_RTTI_CLASS;

public:
	enum
	{
		WsNoClipboard = WsUser
	};

	struct ISpecialCharacter : public IRefCount
	{
		virtual int32_t measureWidth(const RichEdit* richEdit) const = 0;

		virtual void draw(Canvas& canvas, const Rect& rc) const = 0;

		virtual void mouseButtonDown(MouseButtonDownEvent* event) const = 0;

		virtual void mouseButtonUp(MouseButtonUpEvent* event) const = 0;

		virtual void mouseDoubleClick(MouseDoubleClickEvent* event) const = 0;
	};

	RichEdit();

	bool create(Widget* parent, const std::wstring& text = L"", int32_t style = WsClientBorder | WsDoubleBuffer);

	/*! \brief Set text content of text editor. */
	virtual void setText(const std::wstring& text) override;

	/*! \brief Get textual content of text editor. */
	virtual std::wstring getText() const override;

	/*! \brief Get textual content of text editor. */
	std::wstring getText(std::function< std::wstring (wchar_t) > cfn, std::function< std::wstring (const ISpecialCharacter*) > scfn) const;

	/*! \brief Set font. */
	virtual void setFont(const Font& font) override;

	/*! \brief Define a new text attribute. */
	int32_t addTextAttribute(const Color4ub& textColor, bool bold, bool italic, bool underline);

	/*! \brief Define a new background attribute. */
	int32_t addBackgroundAttribute(const Color4ub& backColor);

	/*! \brief Use text attribute on a selection of the text. */
	void setTextAttribute(int32_t start, int32_t length, int32_t attribute);

	/*! \brief Use background attribute on a selection of the text. */
	void setBackgroundAttribute(int32_t start, int32_t length, int32_t attribute);

	/*! \brief Use background attribute on an entire line. */
	void setBackgroundAttribute(int32_t line, int32_t attribute);

	/*! \brief Use attributes on a selection of the text. */
	void setAttributes(int32_t start, int32_t length, int32_t textAttribute, int32_t backgroundAttribute);

	/*! \brief Define new image. */
	int32_t addImage(IBitmap* image, uint32_t imageCount);

	/*! \brief Use image on a line of text. */
	void setImage(int32_t line, int32_t image);

	/*! \brief Add special character with substiute value. */
	wchar_t addSpecialCharacter(const ISpecialCharacter* specialCharacter);

	/*! \brief Clear attributes, images or content of text. */
	void clear(bool attributes, bool images, bool content);

	/*! \brief Delete selected text. */
	void deleteSelection();

	/*! \brief Insert text at caret's current position. */
	void insert(const std::wstring& text);

	/*! \brief Get offset from client position. */
	int32_t getOffsetFromPosition(const Point& position);

	/*! \brief Get caret's current position. */
	int32_t getCaretOffset() const;

	/*! \brief Get line from client Y position. */
	int32_t getLineFromPosition(int32_t position);

	/*! \brief Get line index of position. */
	int32_t getLineFromOffset(int32_t offset) const;

	/*! \brief Get number of textual lines. */
	int32_t getLineCount() const;

	/*! \brief Get starting offset of line. */
	int32_t getLineOffset(int32_t line) const;

	/*! \brief Get length of line. */
	int32_t getLineLength(int32_t line) const;

	/*! \brief Replace text at a specific line. */
	void setLine(int32_t line, const std::wstring& text);

	/*! \brief Get a single line of text. */
	std::wstring getLine(int32_t line) const;

	/*! \brief Set user object with specific line. */
	void setLineData(int32_t line, Object* data);

	/*! \brief Get user object of specific line. */
	Object* getLineData(int32_t line) const;

	/*! \brief Get selection start offset. */
	int32_t getSelectionStartOffset() const;

	/*! \brief Get selection end offset. */
	int32_t getSelectionStopOffset() const;

	/*! \brief Get selected text. */
	std::wstring getSelectedText() const;

	/*! \brief Get selected text. */
	std::wstring getSelectedText(std::function< std::wstring (wchar_t) > cfn, std::function< std::wstring (const ISpecialCharacter*) > scfn) const;

	/*! \brief Scroll text view to a certain line. */
	bool scrollToLine(int32_t line);

	/*! \brief Get current scrolled offset. */
	int32_t getScrollLine() const;

	/*! \brief Ensure line is visible. */
	bool showLine(int32_t line);

	/*! \brief Place caret at a specified offset. */
	void placeCaret(int32_t offset);

	/*! \brief Select all. */
	void selectAll();

	/*! \brief Copy selected text into clipboard. */
	bool copy();

	/*! \brief Cut selected text into clipboard. */
	bool cut();

	/*! \brief Paste text from clipboard at caret position. */
	bool paste();

	/*! \brief Show and focus find. */
	bool find();

	/*! \brief Find next occurance of entered search term. */
	bool findNext();

	/*! \brief Show and focus replace. */
	bool replace();

	/*! \brief Replace all occurances of entered replace terms. */
	bool replaceAll();

	/*! \brief Get text inner rectangle. */
	Rect getEditRect() const;

	/*! \brief Get margin width. */
	int32_t getMarginWidth() const;

private:
	struct TextAttribute
	{
		Color4ub textColor;
		bool bold;
		bool italic;
		bool underline;

		TextAttribute()
		:	bold(false)
		,	italic(false)
		,	underline(false)
		{
		}
	};

	struct BackgroundAttribute
	{
		Color4ub backColor;
	};

	struct Line
	{
		int32_t start;
		int32_t stop;
		int32_t image;
		uint16_t attrib;
		Ref< Object > data;

		Line()
		:	start(0)
		,	stop(0)
		,	image(-1)
		,	attrib(0xffff)
		{
		}
	};

	struct Character
	{
		wchar_t ch;
		uint16_t tai;
		uint16_t bgai;
		uint16_t width;

		Character()
		:	ch(0)
		,	tai(0)
		,	bgai(0)
		,	width(0)
		{
		}

		explicit Character(wchar_t _ch)
		:	ch(_ch)
		,	tai(0)
		,	bgai(0)
		,	width(0)
		{
		}
	};

	struct FindResult
	{
		int32_t line;
		int32_t offset;
	};

	Ref< ScrollBar > m_scrollBarV;
	Ref< ScrollBar > m_scrollBarH;
	Ref< SearchControl > m_searchControl;
	std::vector< TextAttribute > m_textAttributes;
	std::vector< BackgroundAttribute > m_backgroundAttributes;
	Ref< IBitmap > m_image;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	uint32_t m_imageCount;
	std::vector< Line > m_lines;
	std::vector< Character > m_text;
	std::map< wchar_t, Ref< const ISpecialCharacter > > m_specialCharacters;
	bool m_clipboard;
	int32_t m_caret;
	bool m_caretBlink;
	int32_t m_selectionStart;
	int32_t m_selectionStop;
	int32_t m_lineMargin;
	int32_t m_lineOffsetH;
	int32_t m_widestLineWidth;
	int32_t m_fromCaret;
	wchar_t m_nextSpecialCharacter;
	int32_t m_foundLineAttribute;

	void updateScrollBars();

	void updateCharacterWidths();

	void deleteCharacters();

	void insertCharacter(wchar_t ch, bool issueEvents);

	void insertAt(int32_t offset, wchar_t ch);

	void scrollToCaret();

	void santiyCheck();

	void updateFindPreview() const;

	bool findFirstLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const;

	bool findNextLine(int32_t currentLine, const std::wstring& needle, bool caseSensitive, bool wholeWord, bool wildcard, FindResult& outResult) const;

	void eventKeyDown(KeyDownEvent* event);

	void eventKey(KeyEvent* event);

	void eventButtonDown(MouseButtonDownEvent* event);

	void eventButtonUp(MouseButtonUpEvent* event);

	void eventMouseMove(MouseMoveEvent* event);

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventMouseWheel(MouseWheelEvent* event);

	void eventPaint(PaintEvent* event);

	void eventSize(SizeEvent* event);

	void eventTimer(TimerEvent* event);

	void eventScroll(ScrollEvent* event);

	void eventSearch(SearchEvent* event);
};

	}
}

