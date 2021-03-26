#pragma once

#include "Render/IRenderTargetSet.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker;
class SimpleTextureVrfy;

/*!
 * \ingroup Vrfy
 */
class RenderTargetSetVrfy : public IRenderTargetSet
{
	T_RTTI_CLASS;

public:
	explicit RenderTargetSetVrfy(ResourceTracker* resourceTracker, const RenderTargetSetCreateDesc& setDesc, IRenderTargetSet* renderTargetSet);

	virtual ~RenderTargetSetVrfy();

	virtual void destroy() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int32_t index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int32_t index, void* buffer) const override final;

	IRenderTargetSet* getRenderTargetSet() const { return m_renderTargetSet; }

	bool haveColorTexture(int32_t index) const;

	bool haveDepthTexture() const;

	bool usingPrimaryDepthStencil() const;

	uint32_t getMultiSample() const;

private:
	Ref< ResourceTracker > m_resourceTracker;
	RenderTargetSetCreateDesc m_setDesc;
	Ref< IRenderTargetSet > m_renderTargetSet;
	mutable Ref< SimpleTextureVrfy > m_colorTextures[4];
	mutable Ref< SimpleTextureVrfy > m_depthTexture;
};

	}
}

