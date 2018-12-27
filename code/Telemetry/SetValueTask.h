/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_telemetry_SetValueTask_H
#define traktor_telemetry_SetValueTask_H

#include "Telemetry/ITask.h"

namespace traktor
{
	namespace telemetry
	{

/*! \brief
 * \ingroup Telemetry
 */
class SetValueTask : public ITask
{
	T_RTTI_CLASS;

public:
	SetValueTask(const std::wstring& serverHost, const std::wstring& client, const std::wstring& symbol, int32_t value);

	virtual TaskResult execute() override final;

private:
	std::wstring m_serverHost;
	std::wstring m_client;
	std::wstring m_symbol;
	int32_t m_value;
	uint64_t m_timeStamp;
};

	}
}

#endif	// traktor_telemetry_SetValueTask_H
