#include <cmath>
#include "Core/Math/Const.h"
#include "Drawing/Image.h"
#include "Drawing/Filters/PerlinNoiseFilter.h"

namespace traktor::drawing
{
	namespace
	{

float noise(int x, int y)
{
	x = x + y * 57;
	x = (x << 13) ^ x;
	return (1.0f - ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float smoothNoise(int x, int y)
{
	const float c = (noise(x - 1, y - 1) + noise(x + 1, y - 1) + noise(x - 1, y + 1) + noise(x + 1, y + 1)) / 16;
	const float s = (noise(x - 1, y) + noise(x + 1, y) + noise(x, y - 1) + noise(x, y + 1)) / 8;
	const float e = noise(x, y) / 4;
	return c + s + e;
}

float interpolate(float a, float b, float t)
{
	const float ft = float(t * PI);
	const float f = float(1.0f - std::cos(ft)) * 0.5f;
	return a * (1 - f) + b * f;
}

float interpolateNoise(float x, float y)
{
	const int ix = int(x);
	const float fx = x - ix;

	const int iy = int(y);
	const float fy = y - iy;

	const float v[] =
	{
		smoothNoise(ix, iy),
		smoothNoise(ix + 1, iy),
		smoothNoise(ix, iy + 1),
		smoothNoise(ix + 1, iy + 1)
	};

	const float i[] =
	{
		interpolate(v[0], v[1], fx),
		interpolate(v[2], v[3], fx)
	};

	return interpolate(i[0], i[1], fy);
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.drawing.PerlinNoiseFilter", PerlinNoiseFilter, IImageFilter)

PerlinNoiseFilter::PerlinNoiseFilter(int octaves, float persistence, float magnify)
:	m_octaves(octaves)
,	m_persistence(persistence)
,	m_magnify(magnify)
{
}

void PerlinNoiseFilter::apply(Image* image) const
{
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		float fy = float(y) / image->getHeight();

		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			float fx = float(x) / image->getWidth();

			float perlin = 0.0f;
			for (int32_t i = 0; i < m_octaves; ++i)
			{
				float frequency = std::pow(2.0f, i);
				float amplitude = std::pow(m_persistence, i);
				perlin += interpolateNoise(fx * frequency / m_magnify, fy * frequency / m_magnify) * amplitude;
			}

			if (perlin < 0.0f)
				perlin = 0.0f;
			else if (perlin > 1.0f)
				perlin = 1.0f;

			Color4f out(perlin, perlin, perlin, perlin);
			image->setPixelUnsafe(x, y, out);
		}
	}
}

}
