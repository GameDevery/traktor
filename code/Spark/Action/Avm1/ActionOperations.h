#pragma once

#include "Core/Config.h"
#include "Spark/Action/Avm1/ActionOpcodes.h"

namespace traktor
{

class OutputStream;
class Timer;

	namespace spark
	{

class ActionContext;
class ActionFrame;
class ActionObject;
class ActionVMImage1;
class SpriteInstance;

struct PreparationState
{
	ActionVMImage1* image;
	uint8_t* pc;
	uint8_t* npc;
	uint8_t* data;
	uint16_t length;
};

struct ExecutionState
{
	const ActionVMImage1* image;
	ActionFrame* frame;
	const uint8_t* pc;
	const uint8_t* npc;
	const uint8_t* data;
	uint16_t length;
	const Timer* timer;

	// Scope instance.
	Ref< ActionObject > with;

	// Cached instances.
	ActionContext* context;
	ActionObject* self;
	ActionObject* global;
	SpriteInstance* movieClip;

	// Trace instances.
	OutputStream* trace;
};

struct OperationInfo
{
	ActionOpcode op;
	const char* name;
	void (*prepare)(PreparationState& state);
	void (*execute)(ExecutionState& state);
};

extern const OperationInfo c_operationInfos[];

	}
}

