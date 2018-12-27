/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_ActionVM1_H
#define traktor_flash_ActionVM1_H

#include "Flash/Action/IActionVM.h"

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

/*! \brief ActionScript virtual machine.
 * \ingroup Flash
 *
 * Virtual machine to execute blocks of ActionScript
 * byte code.
 */
class T_DLLCLASS ActionVM1 : public IActionVM
{
	T_RTTI_CLASS;

public:
	virtual Ref< const IActionVMImage > load(SwfReader& swf) const override final;
};

	}
}

#endif	// traktor_flash_ActionVM1_H
