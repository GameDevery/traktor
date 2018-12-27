/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSetDx11_H
#define traktor_render_RenderTargetSetDx11_H

#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Render/RenderTargetSet.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
class RenderTargetDepthDx11;
class RenderTargetDx11;
struct RenderTargetSetCreateDesc;

/*!
 * \ingroup DX11
 */
class RenderTargetSetDx11 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetDx11(ContextDx11* context);

	virtual ~RenderTargetSetDx11();

	bool create(const RenderTargetSetCreateDesc& setDesc);

	virtual void destroy() override final;

	virtual int getWidth() const override final;
	
	virtual int getHeight() const override final;

	virtual ISimpleTexture* getColorTexture(int index) const override final;

	virtual ISimpleTexture* getDepthTexture() const override final;

	virtual void swap(int index1, int index2) override final;

	virtual void discard() override final;

	virtual bool isContentValid() const override final;

	virtual bool read(int index, void* buffer) const override final;

	void setContentValid(bool contentValid) {
		m_contentValid = contentValid;
	}

	bool usingPrimaryDepthStencil() const {
		return m_usingPrimaryDepthStencil;
	}

private:
	Ref< ContextDx11 > m_context;
	RefArray< RenderTargetDx11 > m_colorTextures;
	Ref< RenderTargetDepthDx11 > m_depthTexture;
	int m_width;
	int m_height;
	bool m_contentValid;
	bool m_usingPrimaryDepthStencil;
};

	}
}

#endif	// traktor_render_RenderTargetSetDx11_H
