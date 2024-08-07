/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

namespace traktor::jungle
{

enum RMessageId
{
	RmiPing	= 0xa0,
	RmiPong = 0xa1,
	RmiState = 0xb0,
	RmiEvent0 = 0xc0,
	RmiEvent0Ack = 0xc1,
	RmiEvent1 = 0xd0,
	RmiEvent1Ack = 0xd1
};

#pragma pack(1)
struct RMessage
{
	uint8_t id;
	uint32_t time;
	union
	{
		struct
		{
			uint32_t time0;
			uint8_t status;
		} ping;

		struct
		{
			uint32_t time0;
			uint32_t rtime0;
			uint32_t latency;
			uint32_t latencySpread;
		} pong;

		struct
		{
			uint8_t data[1];
		} state;

		struct
		{
			uint8_t sequence;
			uint8_t data[1];
		} event;

		struct
		{
			uint8_t sequence;
		} eventAck;

		uint8_t reserved[1024 - sizeof(uint8_t) - sizeof(uint32_t)];
	};
};
#pragma pack()

T_FORCE_INLINE int32_t RMessage_HeaderSize()				{ return sizeof(uint8_t) + sizeof(uint32_t); }

T_FORCE_INLINE int32_t RmiPing_NetSize()					{ return RMessage_HeaderSize() + sizeof(uint32_t) + sizeof(uint8_t); }
T_FORCE_INLINE int32_t RmiPong_NetSize()					{ return RMessage_HeaderSize() + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t); }

T_FORCE_INLINE int32_t RmiState_NetSize(int32_t stateSize)	{ return RMessage_HeaderSize() + stateSize; }
T_FORCE_INLINE int32_t RmiState_StateSize(int32_t netSize)	{ return netSize - RMessage_HeaderSize(); }
T_FORCE_INLINE int32_t RmiState_MaxStateSize()				{ return RmiState_StateSize(1024); }

T_FORCE_INLINE int32_t RmiEvent_NetSize(int32_t eventSize)	{ return RMessage_HeaderSize() + sizeof(uint8_t) + eventSize; }
T_FORCE_INLINE int32_t RmiEvent_EventSize(int32_t netSize)	{ return netSize - RMessage_HeaderSize() - sizeof(uint8_t); }
T_FORCE_INLINE int32_t RmiEvent_MaxEventSize()				{ return RmiEvent_EventSize(1024); }

T_FORCE_INLINE int32_t RmiEventAck_NetSize()				{ return RMessage_HeaderSize() + sizeof(uint8_t); }

T_FORCE_INLINE uint32_t time2net(double time)	{ return uint32_t(time * 1000.0 + 0.5); }
T_FORCE_INLINE double net2time(uint32_t time)	{ return time / 1000.0; }

}
