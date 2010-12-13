#ifndef traktor_drone_DroneToolUpdate_H
#define traktor_drone_DroneToolUpdate_H

#include "App/DroneTool.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class BackgroundWorkerStatus;

		}
	}

	namespace drone
	{

class UpdateBundle;

class DroneToolUpdate : public DroneTool
{
	T_RTTI_CLASS

public:
	DroneToolUpdate();

	virtual ~DroneToolUpdate();

	virtual void getMenuItems(RefArray< ui::MenuItem >& outItems);

	virtual bool execute(ui::Widget* parent, ui::MenuItem* menuItem);

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_url;
	std::wstring m_rootPath;

	void updateThread(ui::Widget* parent, UpdateBundle* bundle, ui::custom::BackgroundWorkerStatus* status);
};

	}
}

#endif	// traktor_drone_DroneToolUpdate_H
