#ifndef traktor_scene_ISceneRenderControl_H
#define traktor_scene_ISceneRenderControl_H

#include "Core/Object.h"
#include "Core/Math/Vector4.h"

namespace traktor
{
	namespace ui
	{

class Command;
class Point;

	}

	namespace scene
	{

class ISceneRenderControl : public Object
{
	T_RTTI_CLASS;

public:
	enum MoveCameraMode
	{
		McmRotate = 0,
		McmMoveXZ = 1,
		McmMoveXY = 2
	};

	virtual void destroy() = 0;

	virtual void updateWorldRenderer() = 0;

	virtual void setAspect(float aspect) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;

	virtual bool hitTest(const ui::Point& position) const = 0;

	virtual bool calculateRay(const ui::Point& position, Vector4& outWorldRayOrigin, Vector4& outWorldRayDirection) const = 0;

	virtual void moveCamera(MoveCameraMode mode, const Vector4& mouseDelta, const Vector4& viewDelta) = 0;
};

	}
}

#endif	// traktor_scene_ISceneRenderControl_H
