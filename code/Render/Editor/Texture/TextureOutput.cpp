#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Render/Editor/Texture/TextureOutput.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureOutput", 6, TextureOutput, ISerializable)

TextureOutput::TextureOutput()
:	m_textureFormat(TfInvalid)
,	m_generateNormalMap(false)
,	m_scaleDepth(0.0f)
,	m_generateMips(true)
,	m_keepZeroAlpha(true)
,	m_isCubeMap(false)
,	m_hasAlpha(false)
,	m_ignoreAlpha(false)
,	m_scaleImage(false)
,	m_scaleWidth(0)
,	m_scaleHeight(0)
,	m_enableCompression(true)
,	m_enableNormalMapCompression(false)
,	m_inverseNormalMapY(false)
,	m_linearGamma(true)
,	m_generateSphereMap(false)
,	m_preserveAlphaCoverage(false)
,	m_alphaCoverageReference(0.5f)
{
}

bool TextureOutput::serialize(ISerializer& s)
{
	if (s.getVersion() >= 6)
	{
		const MemberEnum< TextureFormat >::Key c_TextureFormat_Keys[] =
		{
			{ L"TfInvalid", TfInvalid },
			{ L"TfR8", TfR8 },
			{ L"TfR8G8B8A8", TfR8G8B8A8 },
			{ L"TfR5G6B5", TfR5G6B5 },
			{ L"TfR5G5B5A1", TfR5G5B5A1 },
			{ L"TfR4G4B4A4", TfR4G4B4A4 },
			{ L"TfR16G16B16A16F", TfR16G16B16A16F },
			{ L"TfR32G32B32A32F", TfR32G32B32A32F },
			{ L"TfR16G16F", TfR16G16F },
			{ L"TfR32G32F", TfR32G32F },
			{ L"TfR16F", TfR16F },
			{ L"TfR32F", TfR32F },
			{ L"TfDXT1", TfDXT1 },
			{ L"TfDXT2", TfDXT2 },
			{ L"TfDXT3", TfDXT3 },
			{ L"TfDXT4", TfDXT4 },
			{ L"TfDXT5", TfDXT5 },
			{ L"TfPVRTC1", TfPVRTC1 },
			{ L"TfPVRTC2", TfPVRTC2 },
			{ L"TfPVRTC3", TfPVRTC3 },
			{ L"TfPVRTC4", TfPVRTC4 },
			{ 0 }
		};
		s >> MemberEnum< TextureFormat >(L"textureFormat", m_textureFormat, c_TextureFormat_Keys);
	}

	s >> Member< bool >(L"generateNormalMap", m_generateNormalMap);
	s >> Member< float >(L"scaleDepth", m_scaleDepth);
	s >> Member< bool >(L"generateMips", m_generateMips);
	s >> Member< bool >(L"keepZeroAlpha", m_keepZeroAlpha);
	s >> Member< bool >(L"isCubeMap", m_isCubeMap);
	s >> Member< bool >(L"hasAlpha", m_hasAlpha);
	s >> Member< bool >(L"ignoreAlpha", m_ignoreAlpha);
	s >> Member< bool >(L"scaleImage", m_scaleImage);
	s >> Member< int32_t >(L"scaleWidth", m_scaleWidth);
	s >> Member< int32_t >(L"scaleHeight", m_scaleHeight);
	s >> Member< bool >(L"enableCompression", m_enableCompression);

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"enableNormalMapCompression", m_enableNormalMapCompression);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"inverseNormalMapY", m_inverseNormalMapY);

	s >> Member< bool >(L"linearGamma", m_linearGamma);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"generateSphereMap", m_generateSphereMap);

	if (s.getVersion() >= 5)
	{
		s >> Member< bool >(L"preserveAlphaCoverage", m_preserveAlphaCoverage);
		s >> Member< float >(L"alphaCoverageReference", m_alphaCoverageReference);
	}
	return true;
}

	}
}
