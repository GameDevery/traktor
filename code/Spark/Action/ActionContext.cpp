#include "Core/Misc/StringSplit.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Action/ActionContext.h"
#include "Spark/Action/ActionFunction.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionContext", ActionContext, Collectable)

ActionContext::ActionContext(const Movie* movie, const ICharacterFactory* characterFactory, const IMovieLoader* movieLoader)
:	m_movie(movie)
,	m_characterFactory(characterFactory)
,	m_movieLoader(movieLoader)
{
	m_strings[""];	// 0
	m_strings["__proto__"];
	m_strings["prototype"];
	m_strings["__ctor__"];
	m_strings["constructor"];
	m_strings["this"];
	m_strings["super"];
	m_strings["_global"];
	m_strings["arguments"];
	m_strings["_root"];
	m_strings["_parent"];
	m_strings["apply"];
	m_strings["call"];
	m_strings["toString"];
	m_strings["_target"];
	m_strings["_targetProperty"];
	m_strings["_function"];
	m_strings["onLoad"];
	m_strings["onEnterFrame"];
	m_strings["onKeyDown"];
	m_strings["onKeyUp"];
	m_strings["onMouseDown"];
	m_strings["onPress"];
	m_strings["onMouseUp"];
	m_strings["onRelease"];
	m_strings["onMouseMove"];
	m_strings["onRollOver"];
	m_strings["onRollOut"];
	m_strings["onFrame"];
	m_strings["onSetFocus"];
	m_strings["onKillFocus"];
	m_strings["onMouseWheel"];
	m_strings["onChanged"];
	m_strings["onScroller"];
	m_strings["x"];
	m_strings["y"];
	m_strings["width"];
	m_strings["height"];
}

void ActionContext::setGlobal(ActionObject* global)
{
	m_global = global;
}

void ActionContext::setMovieClip(SpriteInstance* movieClip)
{
	m_movieClip = movieClip;
}

void ActionContext::setFocus(CharacterInstance* focus)
{
	if (m_focus)
		m_focus->eventKillFocus();

	m_focus = focus;

	if (m_focus)
		m_focus->eventSetFocus();
}

void ActionContext::setPressed(CharacterInstance* pressed)
{
	m_pressed = pressed;
}

void ActionContext::setRolledOver(CharacterInstance* rolledOver)
{
	m_rolledOver = rolledOver;
}

void ActionContext::addFrameListener(ActionObject* frameListener)
{
	ActionValue memberValue;
	if (frameListener->getLocalMember(IdOnFrame, memberValue))
	{
		Ref< ActionFunction > listenerFunction = memberValue.getObject< ActionFunction >();
		if (listenerFunction)
		{
			FrameListener fl = { frameListener, listenerFunction };
			m_frameListeners.push_back(fl);
		}
	}
}

void ActionContext::removeFrameListener(ActionObject* frameListener)
{
	for (AlignedVector< FrameListener >::iterator i = m_frameListeners.begin(); i != m_frameListeners.end(); ++i)
	{
		if (i->listenerTarget == frameListener)
		{
			m_frameListeners.erase(i);
			break;
		}
	}
}

void ActionContext::notifyFrameListeners(float time)
{
	if (m_frameListeners.empty())
		return;

	ActionValueArray argv(m_pool, 1);
	argv[0] = ActionValue(time);

	AlignedVector< FrameListener > frameListeners = m_frameListeners;
	for (AlignedVector< FrameListener >::iterator i = frameListeners.begin(); i != frameListeners.end(); ++i)
		i->listenerFunction->call(i->listenerTarget, argv);
}

ActionObject* ActionContext::lookupClass(const std::string& className)
{
	Ref< ActionObject > clazz = m_global;

	StringSplit< std::string > classNameSplit(className, ".");
	for (StringSplit< std::string >::const_iterator i = classNameSplit.begin(); clazz != nullptr && i != classNameSplit.end(); ++i)
	{
		ActionValue clazzMember;
		if (!clazz->getLocalMember(m_strings[*i], clazzMember) || !clazzMember.isObject())
			return nullptr;

		clazz = clazzMember.getObject();
	}

	if (!clazz)
		return nullptr;

	ActionValue prototypeMember;
	if (!clazz->getLocalMember(IdPrototype, prototypeMember) || !prototypeMember.isObject())
		return nullptr;

	return prototypeMember.getObject();
}

uint32_t ActionContext::getString(const std::string& str)
{
	return m_strings[str];
}

std::string ActionContext::getString(uint32_t id)
{
	return m_strings[id];
}

void ActionContext::trace(visitor_t visitor) const
{
	T_ASSERT(m_pool.offset() == 0);

	visitor(m_global);
	visitor(m_movieClip);
	visitor(m_focus);
	visitor(m_pressed);
	visitor(m_rolledOver);

	for (AlignedVector< FrameListener >::const_iterator i = m_frameListeners.begin(); i != m_frameListeners.end(); ++i)
	{
		visitor(i->listenerTarget);
		visitor(i->listenerFunction);
	}
}

void ActionContext::dereference()
{
	m_global = nullptr;
	m_movieClip = nullptr;
	m_focus = nullptr;
	m_pressed = nullptr;
	m_rolledOver = nullptr;
	m_frameListeners.clear();
}

	}
}
