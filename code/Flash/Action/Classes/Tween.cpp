#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Tween.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Tween", Tween, ActionObjectRelay)

Tween::Tween(ActionContext* context)
:	ActionObjectRelay("mx.transitions.Tween")
,	m_context(context)
,	m_begin(0)
,	m_finish(0)
,	m_duration(0)
,	m_useSeconds(false)
,	m_timeStart(0)
,	m_current(0)
,	m_playing(false)
{
}

void Tween::init(
	ActionObject* target,
	const std::string& propertyName,
	ActionFunction* function,
	avm_number_t begin,
	avm_number_t finish,
	avm_number_t duration,
	bool useSeconds
)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	m_begin = begin;
	m_finish = finish;
	m_duration = duration;
	m_useSeconds = useSeconds;
	m_timeStart = avm_number_t(-1);
	m_current = begin;
	m_playing = false;

	if (target)
	{
		self->setMember(ActionContext::IdTarget, ActionValue(target));

		Ref< ActionFunction > propertySet;
		if (target->getPropertySet(propertyName, propertySet))
			self->setMember(ActionContext::IdTargetProperty, ActionValue(propertySet));
	}
	
	if (function)
		self->setMember(ActionContext::IdFunction, ActionValue(function));

	self->setMember("onFrame", ActionValue(createNativeFunction(m_context, this, &Tween::onFrame)));
	start();
}

void Tween::continueTo(avm_number_t finish, avm_number_t duration)
{
	m_begin = m_current;
	m_finish = finish;
	m_duration = duration;
	m_timeStart = avm_number_t(-1);
	resume();
}

void Tween::fforward()
{
}

void Tween::nextFrame()
{
}

void Tween::prevFrame()
{
}

void Tween::resume()
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	if (!m_playing)
		m_context->addFrameListener(self);

	m_playing = true;
}

void Tween::rewind(avm_number_t t)
{
	m_timeStart = avm_number_t(-1);
}

void Tween::start()
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	m_timeStart = avm_number_t(-1);

	if (!m_playing)
	{
		ActionValue target;
		ActionValue propertySet;
		ActionValue function;

		self->getLocalMember(ActionContext::IdTarget, target);
		self->getLocalMember(ActionContext::IdTargetProperty, propertySet);
		self->getLocalMember(ActionContext::IdFunction, function);

		// Ensure property is set to initial value.
		if (propertySet.isObject< ActionFunction >() && function.isObject< ActionFunction >())
		{
			ActionValue value;

			// Calculate eased value.
			if (m_duration > 0.0f)
			{
				ActionValueArray argv0(m_context->getPool(), 4);
				argv0[0] = ActionValue(avm_number_t(0));
				argv0[1] = ActionValue(m_begin);
				argv0[2] = ActionValue(m_finish - m_begin);
				argv0[3] = ActionValue(m_duration);
				value = function.getObject< ActionFunction >()->call(self, argv0);
			}
			else
				value = ActionValue(m_begin);

			m_current = value.getNumber();

			// Set property value.
			if (!value.isUndefined())
			{
				ActionValueArray argv1(m_context->getPool(), 1);
				argv1[0] = value;
				propertySet.getObject< ActionFunction >()->call(
					target.getObjectAlways(m_context),
					argv1
				);
			}
		}

		m_context->addFrameListener(self);
	}

	m_playing = true;
}

void Tween::stop()
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	if (m_playing)
		m_context->removeFrameListener(self);

	m_playing = false;
}

void Tween::yoyo()
{
	std::swap(m_begin, m_finish);
	start();
}

void Tween::onFrame(CallArgs& ca)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT (self);

	ActionValue target;
	ActionValue propertySet;
	ActionValue function;

	self->getLocalMember(ActionContext::IdTarget, target);
	self->getLocalMember(ActionContext::IdTargetProperty, propertySet);
	self->getLocalMember(ActionContext::IdFunction, function);

	if (
		!function.isObject< ActionFunction >() ||
		!propertySet.isObject< ActionFunction >() ||
		!m_playing ||
		m_duration <= 0.0f
	)
		return;

	ActionValue value;

	avm_number_t time = ca.args[0].getNumber();
	if (m_timeStart < 0)
		m_timeStart = time;

	// Calculate interpolated value.
	avm_number_t t = time - m_timeStart;
	avm_number_t T = clamp(t, avm_number_t(0), m_duration);

	// Calculate eased value.
	if (m_duration > 0.0f)
	{
		ActionValueArray argv0(m_context->getPool(), 4);
		argv0[0] = ActionValue(T);
		argv0[1] = ActionValue(m_begin);
		argv0[2] = ActionValue(m_finish - m_begin);
		argv0[3] = ActionValue(m_duration);
		value = function.getObject< ActionFunction >()->call(self, argv0);
	}
	else
		value = ActionValue(m_begin);

	m_current = value.getNumber();

	// Set property value.
	if (!value.isUndefined())
	{
		ActionValueArray argv1(m_context->getPool(), 1);
		argv1[0] = value;
		propertySet.getObject< ActionFunction >()->call(
			target.getObjectAlways(m_context),
			argv1
		);
	}

	// Stop after duration expired.
	if (t >= m_duration)
	{
		stop();

		// Notify listener when we've reached the end.
		ActionValue memberValue;
		if (self->getLocalMember("onMotionFinished", memberValue))
		{
			Ref< ActionFunction > motionFinished = memberValue.getObject< ActionFunction >();
			if (motionFinished)
				motionFinished->call(self, ActionValueArray());
		}
	}
}

	}
}
