#ifndef traktor_ui_custom_Tick_H
#define traktor_ui_custom_Tick_H

#include "Ui/Custom/Sequencer/Key.h"

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

/*! \brief Sequencer tick.
 * \ingroup UIC
 */
class T_DLLCLASS Tick : public Key
{
	T_RTTI_CLASS(Tick)

public:
	Tick(int time);

	void setTime(int time);

	int getTime() const;

	virtual void getRange(const Sequence* sequence, int& outLeft, int& outRight) const;

	virtual void paint(ui::Canvas& canvas, const Sequence* sequence, const Rect& rcClient, int scrollOffset);

private:
	int m_time;
};

		}
	}
}

#endif	// traktor_ui_custom_Tick_H
