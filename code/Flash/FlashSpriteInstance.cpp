#include <limits>
#include "Flash/FlashCanvas.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/FlashFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunction.h"
#include "Flash/Action/IActionVM.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashSpriteInstance", FlashSpriteInstance, FlashCharacterInstance)

FlashSpriteInstance::FlashSpriteInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashSprite* sprite)
:	FlashCharacterInstance(context, "MovieClip", parent)
,	m_sprite(sprite)
,	m_displayList(context)
,	m_currentFrame(0)
,	m_nextFrame(0)
,	m_lastUpdateFrame(~0U)
,	m_lastExecutedFrame(~0U)
,	m_skipEnterFrame(0)
,	m_initialized(false)
,	m_removed(false)
,	m_playing(true)
,	m_visible(false)
,	m_enabled(true)
,	m_inside(false)
,	m_inDispatch(false)
,	m_gotoIssued(false)
,	m_press(false)
,	m_mouseX(0)
,	m_mouseY(0)
,	m_maskCount(0)
{
	T_ASSERT (m_sprite->getFrameCount() > 0);
}

FlashSpriteInstance::~FlashSpriteInstance()
{
	destroy();
}

void FlashSpriteInstance::destroy()
{
	m_sprite = 0;
	m_mask = 0;
	m_canvas = 0;
	m_playing = false;

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.instance)
			i->second.instance->destroy();
	}
	m_displayList.reset();
	m_visibleCharacters.clear();

	FlashCharacterInstance::destroy();
}

const FlashSprite* FlashSpriteInstance::getSprite() const
{
	return m_sprite;
}

void FlashSpriteInstance::gotoFrame(uint32_t frameId)
{
	frameId = min(frameId, m_sprite->getFrameCount() - 1);
	if (!m_inDispatch)
	{
		m_currentFrame =
		m_nextFrame = frameId;
	}
	else
	{
		m_nextFrame = frameId;
		m_skipEnterFrame = 1;
		m_gotoIssued = true;
	}
}

void FlashSpriteInstance::gotoPrevious()
{
	if (!m_inDispatch)
	{
		if (m_currentFrame > 0)
			m_currentFrame--;
	}
	else
	{
		if (m_currentFrame > 0)
		{
			m_nextFrame = m_currentFrame - 1;
			m_gotoIssued = true;
		}
	}
}

void FlashSpriteInstance::gotoNext()
{
	if (!m_inDispatch)
	{
		if (m_currentFrame < m_sprite->getFrameCount() - 1)
			m_currentFrame++;
	}
	else
	{
		if (m_currentFrame < m_sprite->getFrameCount() - 1)
		{
			m_nextFrame = m_currentFrame + 1;
			m_gotoIssued = true;
		}
	}
}

uint32_t FlashSpriteInstance::getCurrentFrame() const
{
	return m_currentFrame;
}

void FlashSpriteInstance::setPlaying(bool playing)
{
	m_playing = playing;
}

bool FlashSpriteInstance::getPlaying() const
{
	return m_playing;
}

void FlashSpriteInstance::updateDisplayList()
{
	// Update sprite instance's display list.
	if (m_currentFrame < m_lastUpdateFrame)
	{
		m_displayList.updateBegin(true);
		for (uint32_t i = 0; i <= m_currentFrame; ++i)
		{
			FlashFrame* frame = m_sprite->getFrame(i);
			T_ASSERT (frame);

			m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}
	else if (m_currentFrame > m_lastUpdateFrame)
	{
		m_displayList.updateBegin(false);
		for (uint32_t i = m_lastUpdateFrame; i <= m_currentFrame; ++i)
		{
			FlashFrame* frame = m_sprite->getFrame(i);
			T_ASSERT (frame);

			m_displayList.updateFrame(this, frame);
		}
		m_displayList.updateEnd();
	}

	m_lastUpdateFrame = m_currentFrame;

	// Recursive update of child sprite instances as well.
	m_displayList.getVisibleObjects(m_visibleCharacters);
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
	{
		if (&type_of(*i) == &type_of< FlashSpriteInstance >())
			static_cast< FlashSpriteInstance* >(*i)->updateDisplayList();
	}
	m_visibleCharacters.resize(0);
}

FlashDisplayList& FlashSpriteInstance::getDisplayList()
{
	return m_displayList;
}

void FlashSpriteInstance::removeMovieClip()
{
	if (FlashCharacterInstance::getFocus() == this)
		FlashCharacterInstance::setFocus(0);

	FlashSpriteInstance* parentClipInstance = checked_type_cast< FlashSpriteInstance*, true >(getParent());
	if (parentClipInstance)
	{
		FlashDisplayList& parentDisplayList = parentClipInstance->getDisplayList();
		parentDisplayList.removeObject(this);
	}

	m_displayList.reset();
	m_mask = 0;
	m_canvas = 0;
	m_removed = true;

	setParent(0);
}

Ref< FlashSpriteInstance > FlashSpriteInstance::clone() const
{
	const SmallMap< uint32_t, Ref< const IActionVMImage > >& events = getEvents();
	Ref< FlashSpriteInstance > cloneInstance = checked_type_cast< FlashSpriteInstance* >(m_sprite->createInstance(
		getContext(),
		getParent(),
		"",
		0,
		&events
	));
	return cloneInstance;
}

SwfRect FlashSpriteInstance::getLocalBounds() const
{
	SwfRect bounds =
	{
		Vector2( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max()),
		Vector2(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max())
	};

	if (m_canvas)
		bounds = m_canvas->getBounds();

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		T_ASSERT (i->second.instance);

		SwfRect layerBounds = i->second.instance->getBounds();
		bounds.min.x = std::min(bounds.min.x, layerBounds.min.x);
		bounds.min.y = std::min(bounds.min.y, layerBounds.min.y);
		bounds.max.x = std::max(bounds.max.x, layerBounds.max.x);
		bounds.max.y = std::max(bounds.max.y, layerBounds.max.y);
	}

	return bounds;
}

void FlashSpriteInstance::setMask(FlashSpriteInstance* mask)
{
	if (m_mask)
		m_mask->m_maskCount--;
	if ((m_mask = mask) != 0)
		m_mask->m_maskCount++;
	if (m_mask)
		m_mask->setVisible(false);
}

FlashSpriteInstance* FlashSpriteInstance::getMask()
{
	return m_mask;
}

FlashCanvas* FlashSpriteInstance::createCanvas()
{
	if (!m_canvas)
		m_canvas = new FlashCanvas();
	return m_canvas;
}

bool FlashSpriteInstance::enumerateMembers(std::vector< uint32_t >& outMemberNames) const
{
	// Visible named character in display list.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		outMemberNames.push_back(i->second.name);
	return true;
}

bool FlashSpriteInstance::getMember(ActionContext* context, uint32_t memberName, ActionValue& outMemberValue)
{
	// Find visible named character in display list.
	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		if (i->second.name == memberName)
		{
			outMemberValue = ActionValue(i->second.instance->getAsObject(context));
			return true;
		}
	}

	// No character, propagate to base class.
	return FlashCharacterInstance::getMember(context, memberName, outMemberValue);
}

void FlashSpriteInstance::preDispatchEvents()
{
	T_ASSERT (!m_inDispatch);
	m_inDispatch = true;
	m_gotoIssued = false;

	// Initialize sprite instance.
	if (!m_initialized)
	{
		eventLoad();
		m_initialized = true;
	}

	// Set initial next frame index, this might change during execution of events.
	if (m_playing)
		m_nextFrame = (m_currentFrame + 1) % m_sprite->getFrameCount();
	else
		m_nextFrame = m_currentFrame;

	// Issue dispatch event on visible child instances.
	m_displayList.getVisibleObjects(m_visibleCharacters);
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->preDispatchEvents();
}

void FlashSpriteInstance::postDispatchEvents()
{
	if (!m_inDispatch)
		return;

	// Update current frame index.
	if (m_playing || m_gotoIssued)
	{
		T_ASSERT (m_nextFrame < m_sprite->getFrameCount());
		m_currentFrame = m_nextFrame;
	}

	// Issue post dispatch event on child sprite instances.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->postDispatchEvents();
	m_visibleCharacters.resize(0);

	m_inDispatch = false;
}

void FlashSpriteInstance::eventInit()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	ActionObject* self = getAsObject(context);
	T_ASSERT (self);

	Ref< ActionObject > super = self->getSuper();

	const RefArray< const IActionVMImage >& initActionScripts = m_sprite->getInitActionScripts();
	for (RefArray< const IActionVMImage >::const_iterator i = initActionScripts.begin(); i != initActionScripts.end(); ++i)
	{
		ActionFrame callFrame(
			context,
			self,
			*i,
			4,
			0,
			0
		);

		callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
		callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
		callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

		context->getVM()->execute(&callFrame);
	}

	FlashCharacterInstance::eventInit();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventConstruct()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	FlashCharacterInstance::eventConstruct();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventLoad()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnLoad, ActionValue());

	FlashCharacterInstance::eventLoad();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventFrame()
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	FlashFrame* frame = m_sprite->getFrame(m_currentFrame);
	T_ASSERT (frame);

	// Issue script assigned event; hack to skip events when using goto methods.
	if (!m_skipEnterFrame)
		executeScriptEvent(ActionContext::IdOnEnterFrame, ActionValue());
	else
		--m_skipEnterFrame;

	// Execute frame scripts.
	if (m_lastExecutedFrame != m_currentFrame)
	{
		ActionObject* self = getAsObject(context);
		T_ASSERT (self);

		Ref< ActionObject > super = self->getSuper();

		const RefArray< const IActionVMImage >& scripts = frame->getActionScripts();
		for (RefArray< const IActionVMImage >::const_iterator i = scripts.begin(); i != scripts.end(); ++i)
		{
			ActionFrame callFrame(
				context,
				self,
				*i,
				4,
				0,
				0
			);

			callFrame.setVariable(ActionContext::IdThis, ActionValue(self));
			callFrame.setVariable(ActionContext::IdSuper, ActionValue(super));
			callFrame.setVariable(ActionContext::IdGlobal, ActionValue(context->getGlobal()));

			context->getVM()->execute(&callFrame);
		}

		m_lastExecutedFrame = m_currentFrame;
	}

	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventFrame();

	FlashCharacterInstance::eventFrame();

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventKey(wchar_t unicode)
{
	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventKey(unicode);

	FlashCharacterInstance::eventKey(unicode);
}

void FlashSpriteInstance::eventKeyDown(int32_t keyCode)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventKeyDown(keyCode);

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnKeyDown, ActionValue());

	FlashCharacterInstance::eventKeyDown(keyCode);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventKeyUp(int32_t keyCode)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Issue events on "visible" characters.
	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		(*i)->eventKeyUp(keyCode);

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnKeyUp, ActionValue());

	FlashCharacterInstance::eventKeyUp(keyCode);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(x, y);
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
			(*i)->eventMouseDown(x, y, button);
	}

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseDown, ActionValue());

	// Check if we're inside then issue press events.
	SwfRect bounds = getLocalBounds();
	bool inside = (xy.x >= bounds.min.x && xy.y >= bounds.min.y && xy.x <= bounds.max.x && xy.y <= bounds.max.y);
	if (inside)
	{
		executeScriptEvent(ActionContext::IdOnPress, ActionValue());
		m_press = true;
	}

	// Call base class event function.
	FlashCharacterInstance::eventMouseDown(x, y, button);

	context->setMovieClip(current);
}

void FlashSpriteInstance::eventMouseUp(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(x, y);
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
			(*i)->eventMouseUp(x, y, button);
	}

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseUp, ActionValue());

	// Check if we're inside then issue press events.
	SwfRect bounds = getLocalBounds();
	bool inside = (xy.x >= bounds.min.x && xy.y >= bounds.min.y && xy.x <= bounds.max.x && xy.y <= bounds.max.y);
	if (inside && m_press)
		executeScriptEvent(ActionContext::IdOnRelease, ActionValue());

	FlashCharacterInstance::eventMouseUp(x, y, button);

	context->setMovieClip(current);
	m_press = false;
}

void FlashSpriteInstance::eventMouseMove(int32_t x, int32_t y, int32_t button)
{
	ActionContext* context = getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(this);

	// Transform coordinates into local.
	Vector2 xy = getFullTransform().inverse() * Vector2(x, y);
	m_mouseX = int32_t(xy.x / 20.0f);
	m_mouseY = int32_t(xy.y / 20.0f);

	// Issue events on "visible" characters.
	if (!m_visibleCharacters.empty())
	{
		for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
			(*i)->eventMouseMove(x, y, button);
	}

	// Issue script assigned event.
	executeScriptEvent(ActionContext::IdOnMouseMove, ActionValue());

	// Roll over and out event handling.
	SwfRect bounds = getLocalBounds();
	bool inside = (xy.x >= bounds.min.x && xy.y >= bounds.min.y && xy.x <= bounds.max.x && xy.y <= bounds.max.y);
	if (inside != m_inside)
	{
		if (inside)
			executeScriptEvent(ActionContext::IdOnRollOver, ActionValue());
		else
			executeScriptEvent(ActionContext::IdOnRollOut, ActionValue());

		m_inside = inside;
	}

	FlashCharacterInstance::eventMouseMove(x, y, button);

	context->setMovieClip(current);
}

SwfRect FlashSpriteInstance::getBounds() const
{
	SwfRect bounds = getLocalBounds();

	Matrix33 transform = getTransform();
	bounds.min = transform * bounds.min;
	bounds.max = transform * bounds.max;

	return bounds;
}

void FlashSpriteInstance::trace(const IVisitor& visitor) const
{
	visitor(m_mask);

	for (RefArray< FlashCharacterInstance >::const_iterator i = m_visibleCharacters.begin(); i != m_visibleCharacters.end(); ++i)
		visitor(*i);

	const FlashDisplayList::layer_map_t& layers = m_displayList.getLayers();
	for (FlashDisplayList::layer_map_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
		visitor(i->second.instance);

	FlashCharacterInstance::trace(visitor);
}

void FlashSpriteInstance::dereference()
{
	m_mask = 0;
	m_visibleCharacters.resize(0);
	m_displayList.reset();

	FlashCharacterInstance::dereference();
}

	}
}
