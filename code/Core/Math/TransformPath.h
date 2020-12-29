#pragma once

#include <vector>
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/ISpline.h"
#include "Core/Math/Transform.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Transformation path.
 * \ingroup Core
 */
class T_DLLCLASS TransformPath : public ISerializable
{
	T_RTTI_CLASS;

public:
	struct T_DLLCLASS Key
	{
		float T = 0.0f;
		Vector4 tcb = Vector4::zero();
		Vector4 position = Vector4::origo();
		Vector4 orientation = Vector4::zero();
		float values[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		Key() = default;

		virtual ~Key() {}

		Transform transform() const;

		void serialize(ISerializer& s);
	};

	TransformPath() = default;

	explicit TransformPath(const TransformPath& path);

	/*! Insert key into path.
	 *
	 * \param key New key frame.
	 * \return Index where key was inserted.
	 */
	size_t insert(const Key& key);

	/*! Evaluate frame.
	 *
	 * \param at Time to evaluate.
	 * \return Evaluated frame.
	 */
	Key evaluate(float at) const;

	/*! Evaluate frame.
	 *
	 * \param at Time to evaluate.
	 * \param end End time of path.
	 * \return Evaluated frame.
	 */
	Key evaluate(float at, float end) const;

	/*! Evaluate frame.
	 *
	 * \param at Time to evaluate.
	 * \param end End time of path.
	 * \param loop Loop time point.
	 * \return Evaluated frame.
	 */
	Key evaluate(float at, float end, float loop) const;

	/*! Get closest key frame from time.
	 *
	 * \param at Closest to time.
	 * \return Index of closest key frame, -1 if no key frame found.
	 */
	int32_t getClosestKey(float at) const;

	/*! Get closest previous key frame from time.
	 *
	 * \param at Closest to time.
	 * \return Index of closest key frame, -1 if no key frame found.
	 */
	int32_t getClosestPreviousKey(float at) const;

	/*! Get closest next key frame from time.
	 *
	 * \param at Closest to time.
	 * \return Index of closest key frame, -1 if no key frame found.
	 */
	int32_t getClosestNextKey(float at) const;

	/*! Split path into two paths at given time.
	 *
	 * \param at Split at time.
	 * \param outPath1 Path before split point.
	 * \param outPath2 Path after split point.
	 */
	void split(float at, TransformPath& outPath1, TransformPath& outPath2) const;

	virtual void serialize(ISerializer& s) override final;

	/*! Get time of first key frame.
	 *
	 * \return Path's first key frame time.
	 */
	float getStartTime() const { return !m_keys.empty() ? m_keys.front().T : 0.0f; }

	/*! Get time of last key frame.
	 *
	 * \return Path's last key frame time.
	 */
	float getEndTime() const { return !m_keys.empty() ? m_keys.back().T : 0.0f; }

	/*! Get key frames.
	 *
	 * \return Key frames.
	 */
	const AlignedVector< Key >& getKeys() const { return m_keys; }

	/*! Get key frames.
	 *
	 * \return Key frames.
	 */
	AlignedVector< Key >& getKeys() { return m_keys; }

	/*! Get number of key frames.
	 *
	 * \return Number of key frames.
	 */
	size_t size() const { return m_keys.size(); }

	/*! Get key frame at index.
	 *
	 * \param at Index.
	 * \return Key frame at index.
	 */
	const Key& operator [] (size_t at) const { return m_keys[at]; }

	/*! Get key frame at index.
	 *
	 * \param at Index.
	 * \return Key frame at index.
	 */
	Key& operator [] (size_t at) { return m_keys[at]; }

	TransformPath& operator = (const TransformPath& path);

private:
	AlignedVector< Key > m_keys;
	mutable AutoPtr< ISpline< Key > > m_spline;
};

}

