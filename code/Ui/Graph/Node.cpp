#include "Ui/Event.h"
#include "Ui/Graph/Edge.h"
#include "Ui/Graph/Node.h"
#include "Ui/Graph/INodeShape.h"
#include "Ui/Graph/Pin.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Node", Node, Object)

void Node::setTitle(const std::wstring& title)
{
	T_ASSERT(m_shape);
	m_title = title;
	m_size = m_shape->calculateSize(m_owner, this);
}

const std::wstring& Node::getTitle() const
{
	return m_title;
}

void Node::setInfo(const std::wstring& info)
{
	T_ASSERT(m_shape);
	m_info = info;
	m_size = m_shape->calculateSize(m_owner, this);
}

const std::wstring& Node::getInfo() const
{
	return m_info;
}

void Node::setComment(const std::wstring& comment)
{
	T_ASSERT(m_shape);
	m_comment = comment;
	m_size = m_shape->calculateSize(m_owner, this);
}

const std::wstring& Node::getComment() const
{
	return m_comment;
}

void Node::setImage(IBitmap* image)
{
	T_ASSERT(m_shape);
	if (m_image != image)
	{
		m_image = image;
		m_size = m_shape->calculateSize(m_owner, this);
	}
}

IBitmap* Node::getImage() const
{
	return m_image;
}

void Node::setState(int32_t state)
{
	T_ASSERT(m_shape);
	if (m_state != state)
	{
		m_state = state;
		m_size = m_shape->calculateSize(m_owner, this);
	}
}

int32_t Node::getState() const
{
	return m_state;
}

void Node::setPosition(const Point& position)
{
	T_ASSERT(m_shape);
	if (m_position != position)
	{
		m_position = position;
		m_size = m_shape->calculateSize(m_owner, this);
	}
}

Point Node::getPosition() const
{
	return m_position;
}

void Node::setSelected(bool selected)
{
	T_ASSERT(m_shape);
	if (m_selected != selected)
	{
		m_selected = selected;
		m_size = m_shape->calculateSize(m_owner, this);
	}
}

bool Node::isSelected() const
{
	return m_selected;
}

void Node::setShape(const INodeShape* shape)
{
	m_shape = shape;
	m_size = m_shape->calculateSize(m_owner, this);
}

const INodeShape* Node::getShape() const
{
	return m_shape;
}

Pin* Node::createInputPin(const std::wstring& name, bool mandatory)
{
	return createInputPin(name, name, mandatory);
}

Pin* Node::createInputPin(const std::wstring& name, const std::wstring& label, bool mandatory)
{
	T_ASSERT(m_shape);

	Ref< Pin > pin = new Pin(this, name, label, Pin::DrInput, mandatory);
	m_inputPins.push_back(pin);

	m_size = m_shape->calculateSize(m_owner, this);

	return pin;
}

const RefArray< Pin >& Node::getInputPins() const
{
	return m_inputPins;
}

Pin* Node::getInputPin(uint32_t index) const
{
	return m_inputPins[index];
}

Pin* Node::findInputPin(const std::wstring& name) const
{
	for (auto inputPin : m_inputPins)
	{
		if (inputPin->getName() == name)
			return inputPin;
	}
	return nullptr;
}

Pin* Node::createOutputPin(const std::wstring& name)
{
	return createOutputPin(name, name);
}

Pin* Node::createOutputPin(const std::wstring& name, const std::wstring& label)
{
	T_ASSERT(m_shape);

	Ref< Pin > pin = new Pin(this, name, label, Pin::DrOutput, false);
	m_outputPins.push_back(pin);

	m_size = m_shape->calculateSize(m_owner, this);

	return pin;
}

const RefArray< Pin >& Node::getOutputPins() const
{
	return m_outputPins;
}

Pin* Node::getOutputPin(uint32_t index) const
{
	return m_outputPins[index];
}

Pin* Node::findOutputPin(const std::wstring& name) const
{
	for (auto outputPin : m_outputPins)
	{
		if (outputPin->getName() == name)
			return outputPin;
	}
	return nullptr;
}

bool Node::hit(const Point& p) const
{
	return calculateRect().inside(p);
}

Point Node::getPinPosition(const Pin* pin) const
{
	T_ASSERT(m_shape);
	return m_shape->getPinPosition(m_owner, this, pin);
}

Pin* Node::getPinAt(const Point& p) const
{
	T_ASSERT(m_shape);
	return m_shape->getPinAt(m_owner, this, p);
}

void Node::paint(GraphCanvas* canvas, const Pin* hotPin, const Size& offset) const
{
	T_ASSERT(m_shape);
	m_shape->paint(m_owner, this, canvas, hotPin, offset);
}

Node::Node(GraphControl* owner, const std::wstring& title, const std::wstring& info, const Point& position, const INodeShape* shape)
:	m_owner(owner)
,	m_title(title)
,	m_info(info)
,	m_state(0)
,	m_position(position)
,	m_size(0, 0)
,	m_selected(false)
,	m_shape(shape)
{
}

Rect Node::calculateRect() const
{
	return Rect(m_position, m_size);
}

	}
}
