#include "Core/Misc/String.h"
#include "Render/Vulkan/Editor/Glsl/GlslVariable.h"

namespace traktor
{
	namespace render
	{

GlslVariable::GlslVariable(const Node* node, const std::wstring& name, GlslType type)
:	m_node(node)
,	m_name(name)
,	m_type(type)
{
}

std::wstring GlslVariable::cast(GlslType to) const
{
	if (m_type == GtVoid || m_type == GtBoolean || m_type >= GtFloat4x4 || to >= GtFloat4x4)
		return m_name;

	const wchar_t* c[10][10] =
	{
		//       |      I      |         I2        |       I3         |          I4         |    F   |       F1      |         F2         |           F3              |
		//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		/*  I */ {         L"%",     L"ivec2(%, %)", L"ivec3(%, %, %)", L"ivec4(%, %, %, %)",    L"%",  L"vec2(%, %)",    L"vec3(%, %, %)",      L"vec4(%, %, %, %)" },
		/* I2 */ {       L"%.x",               L"%", L"ivec3(%.xy, 0)", L"ivec4(%.xy, 0, 0)",  L"%.x",     L"vec2(%)",  L"vec3(%.xy, 0.0)",  L"vec4(%.xy, 0.0, 0.0)" },
		/* I3 */ {       L"%.x",            L"%.xy",              L"%",   L"ivec4(%.xyz, 0)",  L"%.x",  L"vec2(%.xy)",          L"vec3(%)",      L"vec4(%.xyz, 0.0)" },
		/* I4 */ {       L"%.x",            L"%.xy",          L"%.xyz",                 L"%",  L"%.x",  L"vec2(%.xy)",      L"vec3(%.xyz)",               L"vec4(%)" },
		/*  F */ {    L"int(%)",     L"ivec2(%, %)", L"ivec3(%, %, %)", L"ivec4(%, %, %, %)",    L"%",  L"vec2(%, %)",    L"vec3(%, %, %)",      L"vec4(%, %, %, %)" },
		/* F2 */ {  L"int(%.x)",        L"ivec2(%)", L"ivec3(%.xy, 0)", L"ivec4(%.xy, 0, 0)",  L"%.x",           L"%",  L"vec3(%.xy, 0.0)",  L"vec4(%.xy, 0.0, 0.0)" },
		/* F3 */ {  L"int(%.x)",     L"ivec2(%.xy)",       L"ivec3(%)",   L"ivec4(%.xyz, 0)",  L"%.x",        L"%.xy",                L"%",      L"vec4(%.xyz, 0.0)" },
		/* F4 */ {  L"int(%.x)",     L"ivec2(%.xy)",   L"ivec3(%.xyz)",          L"ivec4(%)",  L"%.x",        L"%.xy",            L"%.xyz",                     L"%" },
	};

	const wchar_t* f = c[m_type - GtInteger][to - GtInteger];
	return f ? replaceAll(f, L"%", m_name) : m_name;
}

GlslVariable& GlslVariable::operator = (const GlslVariable& other)
{
	m_node = other.m_node;
	m_name = other.m_name;
	m_type = other.m_type;
	return *this;
}

	}
}
