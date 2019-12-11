#pragma once

#include <string>
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

class Pose;

/*! Animation
 * \ingroup Model
 */
class T_DLLCLASS Animation : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	void insertKeyFrame(float time, const Pose* pose);

	uint32_t getKeyFrameCount() const;

	float getKeyFrameTime(uint32_t keyFrame) const;

	void setKeyFramePose(uint32_t keyFrame, const Pose* pose);

	const Pose* getKeyFramePose(uint32_t keyFrame) const;

	virtual void serialize(ISerializer& s) override final;

private:
	struct KeyFrame
	{
		float time;
		Ref< const Pose > pose;

		void serialize(ISerializer& s);
	};

	std::wstring m_name;
	AlignedVector< KeyFrame > m_keyFrames;
};

	}
}
