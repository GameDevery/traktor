#ifndef traktor_model_Vertex_H
#define traktor_model_Vertex_H

#include <vector>
#include "Core/Config.h"
#include "Model/Types.h"

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

/*! \brief Vertex
 * \ingroup Model
 */
class T_DLLCLASS Vertex
{
public:
	Vertex();

	explicit Vertex(uint32_t position);

	void setPosition(uint32_t position);

	uint32_t getPosition() const;

	void setColor(uint32_t color);

	uint32_t getColor() const;

	void setNormal(uint32_t normal);

	uint32_t getNormal() const;

	void setTangent(uint32_t tangent);

	uint32_t getTangent() const;

	void setBinormal(uint32_t binormal);

	uint32_t getBinormal() const;

	void clearTexCoords();

	void setTexCoord(uint32_t channel, uint32_t texCoord);

	uint32_t getTexCoord(uint32_t channel) const;

	uint32_t getTexCoordCount() const;

	void clearJointInfluences();

	void setJointInfluence(uint32_t jointIndex, float influence);

	float getJointInfluence(uint32_t jointIndex) const;

	uint32_t getJointInfluenceCount() const;

	uint32_t getHash() const;

	bool operator == (const Vertex& r) const;

private:
	uint32_t m_position;
	uint32_t m_color;
	uint32_t m_normal;
	uint32_t m_tangent;
	uint32_t m_binormal;
	std::vector< uint32_t > m_texCoords;
	std::vector< float > m_jointInfluences;
};

	}
}

#endif	// traktor_model_Vertex_H
