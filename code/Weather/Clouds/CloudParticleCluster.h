#pragma once

#include "Core/Math/Aabb3.h"
#include "Core/Containers/AlignedVector.h"
#include "Weather/Clouds/CloudParticle.h"

namespace traktor
{
	namespace weather
	{

class CloudParticleData;

class CloudParticleCluster
{
public:
	bool create(const CloudParticleData& particleData);

	void update(const CloudParticleData& particleData, float deltaTime);

	const Aabb3& getBoundingBox() const;

	const AlignedVector< CloudParticle >& getParticles() const;

private:
	Aabb3 m_boundingBox;
	AlignedVector< CloudParticle > m_particles;
};

	}
}

