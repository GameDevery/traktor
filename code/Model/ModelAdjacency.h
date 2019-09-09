#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/StaticVector.h"

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

class Model;

/*! Model adjacency inspector.
 * \ingroup Model
 */
class T_DLLCLASS ModelAdjacency : public Object
{
	T_RTTI_CLASS;

public:
	enum Mode
	{
		MdByVertex,
		MdByPosition,
		MdByTexCoord
	};

	typedef StaticVector< uint32_t, 8 > share_vector_t;

	ModelAdjacency(const Model* model, Mode mode, uint32_t channel = 0);

	ModelAdjacency(const Model* model, const AlignedVector< uint32_t >& polygons, Mode mode, uint32_t channel = 0);

	/*! Insert a new polygon into adjacency structure. */
	void add(uint32_t polygon);

	/*! Remove a polygon from adjacency structure. */
	void remove(uint32_t polygon);

	/*! Update polygon in adjacency structure. */
	void update(uint32_t polygon);

	/*! Return half edge from polygon edge. */
	uint32_t getEdge(uint32_t polygon, uint32_t polygonEdge) const;

	/*! Get edges "entering" given vertex. */
	void getEnteringEdges(uint32_t vertexId, share_vector_t& outEnteringEdges) const;

	/*! Get edges "leaving" given vertex. */
	void getLeavingEdges(uint32_t vertexId, share_vector_t& outLeavingEdges) const;

	/*! Get sharing edges; ie opposite edges. */
	void getSharedEdges(uint32_t edge,share_vector_t& outSharedEdges) const;

	/*! Get sharing edges; ie opposite edges. */
	void getSharedEdges(uint32_t polygon, uint32_t polygonEdge, share_vector_t& outSharedEdges) const;

	/*! Count number of sharing edges. */
	uint32_t getSharedEdgeCount(uint32_t edge) const;

	/*! Count number of sharing edges. */
	uint32_t getSharedEdgeCount(uint32_t polygon, uint32_t polygonEdge) const;

	/*! Get polygon owning edge. */
	uint32_t getPolygon(uint32_t edge) const;

	/*! Get polygon edge number from half-edge. */
	uint32_t getPolygonEdge(uint32_t edge) const;

	/*! Get total number of half-edges. */
	uint32_t getEdgeCount() const;

	/*! Get edge matching indices. */
	void getEdgeIndices(uint32_t edge, uint32_t& outIndex0, uint32_t& outIndex1) const;

private:
	struct Edge
	{
		uint32_t polygon;
		uint32_t index;
		share_vector_t share;
	};

	Ref< const Model > m_model;
	Mode m_mode;
	uint32_t m_channel;
	AlignedVector< Edge > m_edges;
};

	}
}

