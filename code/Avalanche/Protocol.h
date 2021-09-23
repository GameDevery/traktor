#pragma once

#include "Core/Config.h"

namespace traktor
{
	namespace avalanche
	{

constexpr static uint8_t c_replyOk				= 0x01;
constexpr static uint8_t c_replyFailure			= 0x80;

constexpr static uint8_t c_commandPing			= 0x01;
constexpr static uint8_t c_commandStat			= 0x02;
constexpr static uint8_t c_commandGet			= 0x03;
constexpr static uint8_t c_commandPut			= 0x04;
constexpr static uint8_t c_commandStats			= 0x05;

constexpr static uint8_t c_subCommandPutAppend	= 0x41;
constexpr static uint8_t c_subCommandPutCommit	= 0x42;
constexpr static uint8_t c_subCommandPutDiscard	= 0x43;

	}
}
