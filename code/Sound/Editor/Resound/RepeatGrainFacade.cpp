#include "I18N/Format.h"
#include "I18N/Text.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RepeatGrainFacade", RepeatGrainFacade, IGrainFacade)

int32_t RepeatGrainFacade::getImage(const IGrainData* grain) const
{
	return 2;
}

std::wstring RepeatGrainFacade::getText(const IGrainData* grain) const
{
	const RepeatGrainData* repeatGrain = static_cast< const RepeatGrainData* >(grain);
	if (repeatGrain->getCount() != 0)
		return i18n::Format(L"RESOUND_REPEAT_GRAIN_TEXT", int32_t(repeatGrain->getCount()));
	else
		return i18n::Text(L"RESOUND_REPEAT_GRAIN_INFINITE_TEXT");
}

bool RepeatGrainFacade::canHaveChildren() const
{
	return false;
}

bool RepeatGrainFacade::addChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool RepeatGrainFacade::removeChild(IGrainData* parentGrain, IGrainData* childGrain)
{
	return false;
}

bool RepeatGrainFacade::getChildren(IGrainData* grain, RefArray< IGrainData >& outChildren)
{
	RepeatGrainData* repeatGrain = checked_type_cast< RepeatGrainData*, false >(grain);
	if (repeatGrain->getGrain())
		outChildren.push_back(repeatGrain->getGrain());
	return true;
}

	}
}
