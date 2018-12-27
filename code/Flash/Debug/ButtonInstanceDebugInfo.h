/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ButtonInstanceDebugInfo_H
#define traktor_flash_ButtonInstanceDebugInfo_H

#include "Flash/Debug/InstanceDebugInfo.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ButtonInstance;
	
/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS ButtonInstanceDebugInfo : public InstanceDebugInfo
{
	T_RTTI_CLASS;

public:
	ButtonInstanceDebugInfo();

	ButtonInstanceDebugInfo(const ButtonInstance* instance);

	virtual void serialize(ISerializer& s) override final;
};
	
	}
}

#endif	// traktor_flash_ButtonInstanceDebugInfo_H

