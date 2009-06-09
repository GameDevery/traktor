#include "Render/Dx10/Platform.h"
#include "Render/Dx10/Utilities.h"

namespace traktor
{
	namespace render
	{

bool setupSampleDesc(ID3D10Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_SAMPLE_DESC& outSampleDesc)
{
	HRESULT hr;

	if (sampleCount > 0)
	{
		outSampleDesc.Count = 0;
		for (uint32_t i = 1; i <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i)
		{
			UINT msQuality = 0;

			hr = d3dDevice->CheckMultisampleQualityLevels(
				colorFormat,
				i,
				&msQuality
			);

			if (SUCCEEDED(hr) && msQuality > 0)
			{
				outSampleDesc.Count = i;
				outSampleDesc.Quality = msQuality;
			}
		}
		if (!outSampleDesc.Count)
			return false;
	}
	else
	{
		outSampleDesc.Count = 1;
		outSampleDesc.Quality = 0;
	}

	return true;
}

bool setupSampleDesc(ID3D10Device* d3dDevice, uint32_t sampleCount, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat, DXGI_SAMPLE_DESC& outSampleDesc)
{
	HRESULT hr;

	if (sampleCount > 0)
	{
		outSampleDesc.Count = 0;
		for (uint32_t i = 1; i <= D3D10_MAX_MULTISAMPLE_SAMPLE_COUNT; ++i)
		{
			UINT msQuality1 = 0;
			UINT msQuality2 = 0;

			hr = d3dDevice->CheckMultisampleQualityLevels(
				colorFormat,
				i,
				&msQuality1
			);

			if (SUCCEEDED(hr) && msQuality1 > 0)
			{
				hr = d3dDevice->CheckMultisampleQualityLevels(
					depthFormat,
					i,
					&msQuality2
				);

				if (SUCCEEDED(hr) && msQuality2 > 0)
				{
					outSampleDesc.Count = i;
					outSampleDesc.Quality = min(msQuality1 - 1, msQuality2 - 1);
				}
			}
		}
		if (!outSampleDesc.Count)
			return false;
	}
	else
	{
		outSampleDesc.Count = 1;
		outSampleDesc.Quality = 0;
	}

	return true;
}

	}
}
