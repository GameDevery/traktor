#include "Render/Dx9/StateBlockDx9.h"
#include "Render/Dx9/ParameterCache.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Core/Assert.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.StateBlockDx9", 1, StateBlockDx9, ISerializable)

StateBlockDx9::StateBlockDx9()
:	m_opaque(true)
{
}

StateBlockDx9::StateBlockDx9(const StateBlockDx9& stateBlock)
:	m_renderStates(stateBlock.m_renderStates)
,	m_samplerStates(stateBlock.m_samplerStates)
,	m_opaque(stateBlock.m_opaque)
{
}

void StateBlockDx9::setRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
	if (state == D3DRS_ALPHABLENDENABLE)
		m_opaque = bool(value == FALSE);

	for (std::vector< std::pair< uint32_t, uint32_t > >::iterator i = m_renderStates.begin(); i != m_renderStates.end(); ++i)
	{
		if (i->first == state)
		{
			i->second = value;
			return;
		}
	}

	m_renderStates.push_back(std::make_pair(uint32_t(state), uint32_t(value)));
}

void StateBlockDx9::setSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
	std::vector< std::pair< uint32_t, uint32_t > >& states = m_samplerStates[sampler];

	for (std::vector< std::pair< uint32_t, uint32_t > >::iterator i = states.begin(); i != states.end(); ++i)
	{
		if (i->first == type)
		{
			i->second = value;
			return;
		}
	}

	states.push_back(std::make_pair(uint32_t(type), uint32_t(value)));
}

void StateBlockDx9::prepareAnisotropy(int32_t maxAnisotropy)
{
	// If anisotropy enabled then we're already prepared.
	if (maxAnisotropy > 0)
		return;

	// Reduce filtering to plain linear.
	for (std::map< uint32_t, std::vector< std::pair< uint32_t, uint32_t > > >::iterator i = m_samplerStates.begin(); i != m_samplerStates.end(); ++i)
	{
		for (std::vector< std::pair< uint32_t, uint32_t > >::iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			if ((j->first == D3DSAMP_MINFILTER || j->first == D3DSAMP_MAGFILTER) && j->second == D3DTEXF_ANISOTROPIC)
				j->second = D3DTEXF_LINEAR;
		}
	}
}

void StateBlockDx9::apply(ParameterCache* parameterCache)
{
	for (std::vector< std::pair< uint32_t, uint32_t > >::iterator i = m_renderStates.begin(); i != m_renderStates.end(); ++i)
		parameterCache->setRenderState(i->first, i->second);

	for (std::map< uint32_t, std::vector< std::pair< uint32_t, uint32_t > > >::iterator i = m_samplerStates.begin(); i != m_samplerStates.end(); ++i)
	{
		const std::vector< std::pair< uint32_t, uint32_t > >& states = i->second;
		for (std::vector< std::pair< uint32_t, uint32_t > >::const_iterator j = states.begin(); j != states.end(); ++j)
			parameterCache->setSamplerState(i->first, j->first, j->second);
	}
}

StateBlockDx9& StateBlockDx9::operator = (const StateBlockDx9& stateBlock)
{
	m_renderStates = stateBlock.m_renderStates;
	m_samplerStates = stateBlock.m_samplerStates;
	m_opaque = stateBlock.m_opaque;
	return *this;
}

bool StateBlockDx9::serialize(ISerializer& s)
{
	s >> MemberStlVector< std::pair< uint32_t, uint32_t >, MemberStlPair< uint32_t, uint32_t > >(L"renderStates", m_renderStates);
	s >> MemberStlMap< 
		uint32_t,
		std::vector< std::pair< uint32_t, uint32_t > >,
		MemberStlPair<
			uint32_t,
			std::vector< std::pair< uint32_t, uint32_t > >,
			Member< uint32_t >,
			MemberStlVector< std::pair< uint32_t, uint32_t >, MemberStlPair< uint32_t, uint32_t > >
		>
	>(L"samplerStates", m_samplerStates);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"opaque", m_opaque);

	return true;
}

	}
}
