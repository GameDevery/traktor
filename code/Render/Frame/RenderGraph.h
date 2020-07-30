#pragma once

#include <functional>
#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/StaticVector.h"
#include "Render/Frame/RenderGraphTypes.h"
#include "Render/Frame/RenderPass.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class RenderContext;
class RenderGraphTargetSetPool;

/*! Render graph.
 * \ingroup Render
 * 
 * A render graph describe all passes which is required
 * when rendering a frame. Since all passes is registered
 * beforehand passes can be organized to reduce
 * transitions of targets between passes.
 * 
 * External handlers are registered which
 * are called at appropriate times to render each pass.
 */
class T_DLLCLASS RenderGraph : public Object
{
	T_RTTI_CLASS;

public:
	struct Target
	{
		const wchar_t* name;
		handle_t persistentHandle;
		RenderGraphTargetSetDesc targetSetDesc;
		Ref< IRenderTargetSet > sharedDepthStencilTargetSet;
		Ref< IRenderTargetSet > rts;
		handle_t sizeReferenceTargetSetId;
		int32_t referenceCount;
		bool storeDepth;
		bool external;

		Target()
		:	name(nullptr)
		,	persistentHandle(0)
		,	sizeReferenceTargetSetId(0)
		,	referenceCount(0)
		,	storeDepth(false)
		,	external(false)
		{
		}
	};

	/*! */
	explicit RenderGraph(IRenderSystem* renderSystem);

	/*! */
	virtual ~RenderGraph();

	/*! */
	void destroy();

	/*! Add transient target set.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencil Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque handle of transient target set.
	 */
	handle_t addTransientTargetSet(
		const wchar_t* const name,
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencil = nullptr,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add persistent target set.
	 *
	 * A persistent target set is a target set which is reused
	 * for multiple frames, such as last frame etc.
	 * The persistent handle is used to track target so
	 * same target is reused (if possible).
	 * First time target is requested it's created thus
	 * algorithms should expect target's content to not always be valid.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param persistentHandle Unique handle to track persistent targets.
	 * \param targetSetDesc Render target set create description.
	 * \param sharedDepthStencil Share depth/stencil with target set.
	 * \param sizeReferenceTargetSetId Target to get reference size from when determine target set.
	 * \return Opaque handle of transient target set.
	 */
	handle_t addPersistentTargetSet(
		const wchar_t* const name,
		handle_t persistentHandle,
		const RenderGraphTargetSetDesc& targetSetDesc,
		IRenderTargetSet* sharedDepthStencil = nullptr,
		handle_t sizeReferenceTargetSetId = 0
	);

	/*! Add external target set.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \param targetSet Render target set.
	 * \return Opaque handle of target set.
	 */
	handle_t addExternalTargetSet(const wchar_t* const name, IRenderTargetSet* targetSet);

	/*! Find target ID by name.
	 *
	 * \param name Name of target set, used for debugging only.
	 * \return ID of target set.
	 */
	handle_t findTargetByName(const wchar_t* const name) const;

	/*! Get transient target set from target identifier.
	 *
	 * \param targetSetId Unique identifier of target.
	 * \return Render target set.
	 */
	IRenderTargetSet* getTargetSet(handle_t targetSetId) const;

	/*! Add render pass to graph.
	 *
	 * \param pass Render pass to add.
	 */
	void addPass(const RenderPass* pass);

	/*! Validate render graph.
	 *
	 * Walks through all registered passes to determine
	 * which order they should be rendered and which
	 * targets needs to be acquired etc.
	 *
	 * This must be called before build since order
	 * of passes is calculated when validation.
	 *
	 * \return True if validation succeeded and graph is ready to be rendered.
	 */
	bool validate();

	/*! */
	bool build(RenderContext* renderContext, int32_t width, int32_t height);

	/*! */
	const SmallMap< handle_t, Target >& getTargets() const { return m_targets; }

	/*! */
	const RefArray< const RenderPass >& getPasses() const { return m_passes; }

	/*! */
	const StaticVector< uint32_t, 512 >& getOrder() const { return m_order; }

private:
	RenderGraphTargetSetPool* m_pool;
	SmallMap< handle_t, Target > m_targets;
	RefArray< const RenderPass > m_passes;
	StaticVector< uint32_t, 512 > m_order;
	handle_t m_nextTargetSetId;

	bool acquire(int32_t width, int32_t height, Target& outTarget);
};

	}
}