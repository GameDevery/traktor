/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AsSound_H
#define traktor_flash_AsSound_H

#include "Flash/Action/ActionClass.h"

namespace traktor
{
	namespace flash
	{

struct CallArgs;
class ISoundRenderer;

/*! \brief Sound class.
 * \ingroup Flash
 */
class AsSound : public ActionClass
{
	T_RTTI_CLASS;

public:
	AsSound(ActionContext* context, ISoundRenderer* soundRenderer);

	virtual void initialize(ActionObject* self) override final;

	virtual void construct(ActionObject* self, const ActionValueArray& args) override final;

	virtual ActionValue xplicit(const ActionValueArray& args) override final;

private:
	Ref< ISoundRenderer > m_soundRenderer;

	void Sound_attachSound(ActionObject* self, const std::string& exportName) const;

	void Sound_getBytesLoaded(CallArgs& ca);

	void Sound_getBytesTotal(CallArgs& ca);

	void Sound_getPan(CallArgs& ca);

	void Sound_getTransform(CallArgs& ca);

	void Sound_getVolume(CallArgs& ca);

	void Sound_loadSound(CallArgs& ca);

	void Sound_setPan(CallArgs& ca);

	void Sound_setTransform(CallArgs& ca);

	void Sound_setVolume(CallArgs& ca);

	void Sound_start(ActionObject* self) const;

	void Sound_stop(ActionObject* self) const;

	void Sound_get_checkPolicyFile(CallArgs& ca);

	void Sound_set_checkPolicyFile(CallArgs& ca);

	void Sound_get_duration(CallArgs& ca);

	void Sound_get_id3(CallArgs& ca);

	void Sound_get_position(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsSound_H
