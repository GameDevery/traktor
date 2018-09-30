#include "Core/Io/StringOutputStream.h"
#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

std::wstring VertexAttribute::getName(DataUsage usage, int32_t index)
{
	const wchar_t* s[] =
	{
		L"Position",
		L"Normal",
		L"Tangent",
		L"Binormal",
		L"Color",
		L"Custom",
		L""
	};
	StringOutputStream ss;
	ss << L"in_" << s[usage] << index;
	return ss.str();
}

int32_t VertexAttribute::getLocation(DataUsage usage, int32_t index)
{
	int32_t base[] =
	{
		0,
		1,
		2,
		3,
		4,
		5,
		16
	};
	int32_t location = base[int(usage)] + index;
	return (location < base[int(usage) + 1]) ? location : -1;
}

	}
}
