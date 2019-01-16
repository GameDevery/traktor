#pragma once

#include <string>
#include <map>
#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Point.h"
#include "Ui/Rect.h"

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

class Node;

/*! \brief Graph node pin.
 * \ingroup UI
 */
class T_DLLCLASS Pin
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	enum Direction
	{
		DrInput,
		DrOutput
	};

	explicit Pin(Node* node, const std::wstring& name, const std::wstring& label, Direction direction, bool mandatory);

	Node* getNode() const;

	const std::wstring& getName() const;

	const std::wstring& getLabel() const;

	Direction getDirection() const;

	bool isMandatory() const;

	Point getPosition() const;

private:
	Node* m_node;
	std::wstring m_name;
	std::wstring m_label;
	Direction m_direction;
	bool m_mandatory;
};

	}
}
