#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Math/Const.h"
#include "Core/Timer/Timer.h"
#include "Flash/FlashMoviePlayer.h"
#include "Flash/FlashMovieRenderer.h"
#include "Flash/FlashMovie.h"
#include "Flash/FlashSoundPlayer.h"
#include "Flash/FlashSprite.h"
#include "Flash/FlashSpriteInstance.h"
#include "Flash/GC.h"
#include "Flash/IDisplayRenderer.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Avm1/Classes/AsKey.h"
#include "Flash/Action/Avm1/Classes/AsMouse.h"
#include "Flash/Action/Avm1/Classes/AsSound.h"
#include "Flash/Action/Avm1/Classes/AsStage.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const int32_t c_framesBetweenCollections = 100;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashMoviePlayer", FlashMoviePlayer, Object)

FlashMoviePlayer::FlashMoviePlayer(IDisplayRenderer* displayRenderer, ISoundRenderer* soundRenderer)
:	m_displayRenderer(displayRenderer)
,	m_soundRenderer(soundRenderer)
,	m_movieRenderer(new FlashMovieRenderer(displayRenderer))
,	m_soundPlayer(new FlashSoundPlayer(soundRenderer))
,	m_intervalNextId(1)
,	m_timeCurrent(0.0f)
,	m_timeNext(0.0f)
,	m_timeNextFrame(0.0f)
,	m_framesUntilCollection(c_framesBetweenCollections)
{
}

FlashMoviePlayer::~FlashMoviePlayer()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

bool FlashMoviePlayer::create(FlashMovie* movie, int32_t width, int32_t height)
{
	ActionValue memberValue;

	m_movie = movie;
	m_movieInstance = m_movie->createMovieClipInstance();

	Ref< ActionContext > context = m_movieInstance->getContext();
	Ref< ActionObject > global = context->getGlobal();

	// Override some global methods.
	setGlobal("getUrl", ActionValue(createNativeFunction(context, this, &FlashMoviePlayer::Global_getUrl)));
	setGlobal("setInterval", ActionValue(createNativeFunction(context, this, &FlashMoviePlayer::Global_setInterval)));
	setGlobal("clearInterval", ActionValue(createNativeFunction(context, this, &FlashMoviePlayer::Global_clearInterval)));

	// Create sound prototype.
	setGlobal("Sound", ActionValue(new AsSound(context, m_soundPlayer)));

	// Get references to key and mouse singletons.
	if (global->getMember("Key", memberValue))
		m_key = memberValue.getObject< AsKey >();
	if (global->getMember("Mouse", memberValue))
		m_mouse = memberValue.getObject< AsMouse >();
	if (global->getMember("Stage", memberValue))
		m_stage = memberValue.getObject< AsStage >();
	
	// Ensure stage are properly initialized.
	if (m_stage)
		m_stage->eventResize(width, height);

	// Preload resources into display renderer.
	m_displayRenderer->preload(*m_movie);
	return true;
}

void FlashMoviePlayer::destroy()
{
	m_displayRenderer = 0;
	m_soundRenderer = 0;
	m_movieRenderer = 0;
	m_soundPlayer = 0;
	m_actionVM = 0;
	m_key = 0;
	m_mouse = 0;
	m_movie = 0;
	
	if (m_movieInstance)
	{
		m_movieInstance->destroy();
		m_movieInstance = 0;
	}
	
	m_events.clear();
	m_fsCommands.clear();
	m_interval.clear();

	GC::getInstance().collectCycles(true);
}

void FlashMoviePlayer::gotoAndPlay(uint32_t frame)
{
	m_movieInstance->setPlaying(true);
	m_movieInstance->gotoFrame(frame);
}

void FlashMoviePlayer::gotoAndStop(uint32_t frame)
{
	m_movieInstance->setPlaying(false);
	m_movieInstance->gotoFrame(frame);
}

bool FlashMoviePlayer::gotoAndPlay(const std::string& frameLabel)
{
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame < 0)
		return false;

	m_movieInstance->setPlaying(true);
	m_movieInstance->gotoFrame(frame);
	
	return true;
}

bool FlashMoviePlayer::gotoAndStop(const std::string& frameLabel)
{
	int frame = m_movie->getMovieClip()->findFrame(frameLabel);
	if (frame < 0)
		return false;

	m_movieInstance->setPlaying(false);
	m_movieInstance->gotoFrame(frame);

	return true;
}

uint32_t FlashMoviePlayer::getFrameCount() const
{
	return m_movie->getMovieClip()->getFrameCount();
}

void FlashMoviePlayer::renderFrame()
{
	m_movieRenderer->renderFrame(
		m_movie,
		m_movieInstance,
		m_stage->getViewWidth(),
		m_stage->getViewHeight(),
		m_stage->getViewOffset()
	);
}

void FlashMoviePlayer::executeFrame()
{
	ActionContext* context = m_movieInstance->getContext();

	Ref< FlashSpriteInstance > current = context->getMovieClip();
	context->setMovieClip(m_movieInstance);

	// Issue interval functions.
	ActionValueArray argv;
	for (std::map< uint32_t, Interval >::iterator i = m_interval.begin(); i != m_interval.end(); ++i)
	{
		if (i->second.count++ >= i->second.interval)
		{
			i->second.function->call(i->second.target, argv);
			i->second.count = 0;
		}
	}

	// Issue all events in sequence as each event possibly update
	// the play head and other aspects of the movie.

	m_movieInstance->updateDisplayList();

	m_movieInstance->preDispatchEvents();

	while (!m_events.empty())
	{
		const Event& evt = m_events.front();
		switch (evt.eventType)
		{
		case EvtKeyDown:
			if (m_key)
				m_key->eventKeyDown(evt.keyCode);
			m_movieInstance->eventKeyDown(evt.keyCode);
			break;

		case EvtKeyUp:
			if (m_key)
				m_key->eventKeyUp(evt.keyCode);
			m_movieInstance->eventKeyUp(evt.keyCode);
			break;

		case EvtMouseDown:
			if (m_mouse)
				m_mouse->eventMouseDown(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			m_movieInstance->eventMouseDown(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtMouseUp:
			if (m_mouse)
				m_mouse->eventMouseUp(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			m_movieInstance->eventMouseUp(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtMouseMove:
			if (m_mouse)
				m_mouse->eventMouseMove(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			m_movieInstance->eventMouseMove(evt.mouse.x, evt.mouse.y, evt.mouse.button);
			break;

		case EvtViewResize:
			if (m_stage)
				m_stage->eventResize(evt.view.width, evt.view.height);

			break;
		}
		m_events.pop_front();
	}

	// Finally issue the frame event.
	m_movieInstance->eventFrame();

	// Notify frame listeners.
	context->notifyFrameListeners(avm_number_t(m_timeCurrent));

	m_movieInstance->postDispatchEvents();
	context->setMovieClip(current);

	// Flush pool memory; release all lingering object references etc.
	context->getPool().flush();

	// Collect reference cycles.
	if (--m_framesUntilCollection <= 0)
	{
		GC::getInstance().collectCycles(false);
		m_framesUntilCollection = c_framesBetweenCollections;
	}
}

bool FlashMoviePlayer::progressFrame(float deltaTime)
{
	bool executed = false;
	if (m_timeNext >= m_timeNextFrame)
	{
		m_timeCurrent = m_timeNext;
		m_timeNextFrame += 1.0f / m_movie->getMovieClip()->getFrameRate();
		executeFrame();
		executed = true;
	}
	m_timeNext += deltaTime;
	return executed;
}

void FlashMoviePlayer::postKeyDown(int32_t keyCode)
{
	Event evt;
	evt.eventType = EvtKeyDown;
	evt.keyCode = keyCode;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postKeyUp(int32_t keyCode)
{
	Event evt;
	evt.eventType = EvtKeyUp;
	evt.keyCode = keyCode;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postMouseDown(int32_t x, int32_t y, int32_t button)
{
	Vector2 xy = m_stage->toStage(Vector2(x, y));

	Event evt;
	evt.eventType = EvtMouseDown;
	evt.mouse.x = xy.x;
	evt.mouse.y = xy.y;
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postMouseUp(int32_t x, int32_t y, int32_t button)
{
	Vector2 xy = m_stage->toStage(Vector2(x, y));

	Event evt;
	evt.eventType = EvtMouseUp;
	evt.mouse.x = xy.x;
	evt.mouse.y = xy.y;
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postMouseMove(int32_t x, int32_t y, int32_t button)
{
	Vector2 xy = m_stage->toStage(Vector2(x, y));

	Event evt;
	evt.eventType = EvtMouseMove;
	evt.mouse.x = xy.x;
	evt.mouse.y = xy.y;
	evt.mouse.button = button;
	m_events.push_back(evt);
}

void FlashMoviePlayer::postViewResize(int32_t width, int32_t height)
{
	Event evt;
	evt.eventType = EvtViewResize;
	evt.view.width = width;
	evt.view.height = height;
	m_events.push_back(evt);
}

bool FlashMoviePlayer::getFsCommand(std::wstring& outCommand, std::wstring& outArgs)
{
	if (m_fsCommands.empty())
		return false;

	outCommand = m_fsCommands.front().first;
	outArgs = m_fsCommands.front().second;

	m_fsCommands.pop_front();
	return true;
}

FlashSpriteInstance* FlashMoviePlayer::getMovieInstance() const
{
	return m_movieInstance;
}

IActionVM* FlashMoviePlayer::getVM() const
{
	return m_actionVM;
}

void FlashMoviePlayer::setGlobal(const std::string& name, const ActionValue& value)
{
	ActionContext* actionContext = m_movieInstance->getContext();
	T_ASSERT (actionContext);

	ActionObject* global = actionContext->getGlobal();
	T_ASSERT (global);

	global->setMember(name, value);
}

ActionValue FlashMoviePlayer::getGlobal(const std::string& name) const
{
	ActionContext* actionContext = m_movieInstance->getContext();
	T_ASSERT (actionContext);

	ActionObject* global = actionContext->getGlobal();
	T_ASSERT (global);

	ActionValue value;
	global->getMember(name, value);

	return value;
}

void FlashMoviePlayer::Global_getUrl(CallArgs& ca)
{
	std::string url = ca.args[0].getString();
	if (startsWith< std::string >(url, "FSCommand:"))
	{
		m_fsCommands.push_back(std::make_pair(
			mbstows(url.substr(10)),
			ca.args[1].getWideString()
		));
	}
}

void FlashMoviePlayer::Global_setInterval(CallArgs& ca)
{
	Ref< ActionObject > target;
	Ref< ActionFunction > function;
	ActionValue functionValue;
	uint32_t interval;

	if (ca.args[1].isString())
	{
		// (objectReference:Object, methodName:String, interval:Number, [param1:Object, param2, ..., paramN])
		target = ca.args[0].getObjectAlways(ca.context);
		if (!target->getMember(ca.args[1].getString(), functionValue))
			return;
		function = functionValue.getObject< ActionFunction >();
		interval = uint32_t(ca.args[2].getNumber());
	}
	else
	{
		// (functionReference:Function, interval:Number, [param1:Object, param2, ..., paramN])
		target = ca.self;
		function = ca.args[0].getObject< ActionFunction >();
		interval = uint32_t(ca.args[1].getNumber());
	}

	if (!function)
		return;

	uint32_t id = m_intervalNextId++;

	Interval& iv = m_interval[id];
	iv.count = 0;
	iv.interval = interval / m_movie->getMovieClip()->getFrameRate();
	iv.target = target;
	iv.function = function;

	ca.ret = ActionValue(avm_number_t(id));
}

void FlashMoviePlayer::Global_clearInterval(CallArgs& ca)
{
	uint32_t id = uint32_t(ca.args[0].getNumber());
	std::map< uint32_t, Interval >::iterator i = m_interval.find(id);
	if (i != m_interval.end())
		m_interval.erase(i);
}

	}
}
