#include <locale>
#include <sstream>
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Svg/Document.h"
#include "Svg/Gradient.h"
#include "Svg/Parser.h"
#include "Svg/Path.h"
#include "Svg/PathShape.h"
#include "Svg/Shape.h"
#include "Svg/Style.h"
#include "Svg/TextShape.h"
#include "Xml/Attribute.h"
#include "Xml/Document.h"
#include "Xml/Element.h"

namespace traktor
{
	namespace svg
	{
		namespace
		{

const struct { const wchar_t* name; Color4f color; } c_colorTable[] =
{
	L"black",	Color4f( 0.0f, 0.0f, 0.0f, 1.0f),
	L"red",		Color4f( 1.0f, 0.0f, 0.0f, 1.0f),
	L"green",	Color4f( 0.0f, 1.0f, 0.0f, 1.0f),
	L"blue",	Color4f( 0.0f, 0.0f, 1.0f, 1.0f),
	L"yellow",	Color4f( 1.0f, 1.0f, 0.0f, 1.0f),
	L"white",	Color4f( 1.0f, 1.0f, 1.0f, 1.0f),
	L"lime",	Color4f(0.25f, 0.5f, 1.0f, 1.0f)
};

bool parseColor(const std::wstring& color, Color4f& outColor)
{
	if (startsWith(color, L"#"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"#%02x%02x%02x", &red, &green, &blue);
		outColor = Color4f(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
		return true;
	}
	else if (startsWith(color, L"rgb"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"rgb(%d,%d,%d)", &red, &green, &blue);
		outColor = Color4f(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
		return true;
	}
	else if (toLower(color) == L"none")
		return false;

	for (int32_t i = 0; i < sizeof(c_colorTable) / sizeof(c_colorTable[0]); ++i)
	{
		if (toLower(color) == c_colorTable[i].name)
		{
			outColor = c_colorTable[i].color;
			return true;
		}
	}

	log::warning << L"Unknown color \"" << color << L"\"" << Endl;
	return false;
}

bool isWhiteSpace(wchar_t ch)
{
	return bool(ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n' || ch == L',');
}

bool isDigit(wchar_t ch)
{
	return bool(std::wstring(L"0123456789").find(ch) != std::wstring::npos);
}

bool isCommand(wchar_t ch)
{
	return bool(std::wstring(L"MLVHQTCSAZ").find(toupper(ch)) != std::wstring::npos);
}

void skipUntil(std::wstring::iterator& i, std::wstring::iterator end, bool (*isProc)(wchar_t))
{
	while (i != end && !isProc(*i))
		++i;
}
void skipUntilNot(std::wstring::iterator& i, std::wstring::iterator end, bool (*isProc)(wchar_t))
{
	while (i != end && isProc(*i))
		++i;
}

float parseDecimalNumber(std::wstring::iterator& i, std::wstring::iterator end)
{
	skipUntilNot(i, end, isWhiteSpace);

	std::wstring::iterator j = i;

	if (*i == L'-' || *i == L'+')
		++i;

	skipUntilNot(i, end, isDigit);

	if (*i == L'.' && isDigit(*(i+1)))
	{
		++i;
		skipUntilNot(i, end, isDigit);
	}
	else if (*i == L'e' && ( *(i+1) == L'-' || *(i+1) == L'+' || isDigit(*(i+1)) ))
	{
		++i;
		if (*i == L'-' || *i == L'+')
			++i;
		skipUntilNot(i, end, isDigit);
	}

	float number = 0.0f;
	std::wstringstream(std::wstring(j, i)) >> number;

	return number;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.Parser", Parser, Object)

Ref< Shape > Parser::parse(xml::Document* doc)
{
	return traverse(doc->getDocumentElement());
}

Ref< Shape > Parser::traverse(xml::Element* elm)
{
	Ref< Shape > shape;

	std::wstring name = elm->getName();
	if (name == L"svg")
		shape = parseDocument(elm);
	else if(name == L"g")
		shape = parseGroup(elm);
	else if (name == L"circle")
		shape = parseCircle(elm);
	else if (name == L"rect")
		shape = parseRect(elm);
	else if (name == L"polygon")
		shape = parsePolygon(elm);
	else if (name == L"polyline")
		shape = parsePolyLine(elm);
	else if (name == L"path")
		shape = parsePath(elm);
	else if (name == L"text")
		shape = parseText(elm);
	else if (name == L"defs")
		parseDefs(elm);
	else
		log::debug << L"Unknown SVG element \"" << name << L"\"" << Endl;

	if (shape)
	{
		if (!shape->getStyle())
		{
			shape->setStyle(
				parseStyle(elm)
			);
		}

		shape->setTransform(
			parseTransform(elm)
		);

		for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
		{
			if (!is_a< xml::Element >(child))
				continue;

			Ref< Shape > childShape = traverse(static_cast< xml::Element* >(child));
			if (childShape)
				shape->addChild(childShape);
		}
	}

	return shape;
}

Ref< Shape > Parser::parseDocument(xml::Element* elm)
{
	Ref< Document > doc = new Document();

	float width = parseString< float >(elm->getAttribute(L"width", L"0")->getValue());
	float height = parseString< float >(elm->getAttribute(L"height", L"0")->getValue());
	doc->setSize(Vector2(width, height));

	if (elm->hasAttribute(L"viewBox"))
	{
		std::wstring viewBox = elm->getAttribute(L"viewBox")->getValue();
		std::wstring::iterator i = viewBox.begin();

		float left = parseDecimalNumber(i, viewBox.end());
		float top = parseDecimalNumber(i, viewBox.end());
		float width = parseDecimalNumber(i, viewBox.end());
		float height = parseDecimalNumber(i, viewBox.end());

		doc->setViewBox(Aabb2(
			Vector2(left, top),
			Vector2(left + width, top + height)
		));
	}

	Ref< Style > defaultStyle = new Style();
	defaultStyle->setFillEnable(true);
	defaultStyle->setFill(Color4f(0.0f, 0.0f, 0.0f, 1.0f));
	doc->setStyle(defaultStyle);

	return doc;
}

Ref< Shape > Parser::parseGroup(xml::Element* elm)
{
	return new Shape();
}

Ref< Shape > Parser::parseCircle(xml::Element* elm)
{
	const float c_circleMagic = 0.5522847498f;

	float cx = parseAttr(elm, L"cx");
	float cy = parseAttr(elm, L"cy");
	float r = parseAttr(elm, L"r");
	float rk = r * c_circleMagic;

	Path path;

	path.moveTo(cx, cy + r);
	path.cubicTo(
		cx + rk, cy + r,
		cx + r, cy + rk,
		cx + r, cy
	);
	path.cubicTo(
		cx + r, cy - rk,
		cx + rk, cy - r,
		cx, cy - r
	);
	path.cubicTo(
		cx - rk, cy - r,
		cx - r, cy - rk,
		cx - r, cy
	);
	path.cubicTo(
		cx - r, cy + rk,
		cx - rk, cy + r,
		cx, cy + r
	);
	path.close();

	return new PathShape(path);
}

Ref< Shape > Parser::parseRect(xml::Element* elm)
{
	float x = parseAttr(elm, L"x");
	float y = parseAttr(elm, L"y");
	float width = parseAttr(elm, L"width");
	float height = parseAttr(elm, L"height");
	float round = parseAttr(elm, L"ry");

	Path path;

	if (round <= 0.0f)
	{
		path.moveTo(x, y);
		path.lineTo(x + width, y);
		path.lineTo(x + width, y + height);
		path.lineTo(x, y + height);
		path.close ();
	}
	else
	{
		path.moveTo (x + round, y);
		path.lineTo (x + width - round, y);
		path.cubicTo(x + width, y, x + width, y, x + width, y + round);
		path.lineTo (x + width, y + height - round);
		path.cubicTo(x + width, y + height, x + width, y + height, x + width - round, y + height);
		path.lineTo (x + round, y + height);
		path.cubicTo(x, y + height, x, y + height, x, y + height - round);
		path.lineTo (x, y + round);
		path.cubicTo(x, y, x, y, x + round, y);
		path.close  ();
	}

	return new PathShape(path);
}

Ref< Shape > Parser::parsePolygon(xml::Element* elm)
{
	Path path;
	bool first = true;

	std::wstring points = elm->getAttribute(L"points")->getValue();
	std::wstring::iterator i = points.begin();
	while (i != points.end())
	{
		float x = parseDecimalNumber(i, points.end());
		float y = parseDecimalNumber(i, points.end());

		if (first)
			path.moveTo(x, y);
		else
			path.lineTo(x, y);

		first = false;
	}

	path.close();

	return new PathShape(path);
}

Ref< Shape > Parser::parsePolyLine(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"points"))
		return nullptr;

	Path path;
	bool first = true;

	std::wstring points = elm->getAttribute(L"points")->getValue();
	std::wstring::iterator i = points.begin();
	while (i != points.end())
	{
		float x = parseDecimalNumber(i, points.end());
		float y = parseDecimalNumber(i, points.end());

		if (first)
			path.moveTo(x, y);
		else
			path.lineTo(x, y);

		first = false;
	}

	return new PathShape(path);
}

Ref< Shape > Parser::parsePath(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"d"))
		return nullptr;

	std::wstring def = elm->getAttribute(L"d")->getValue();
	std::wstring::iterator i = def.begin();
	wchar_t cmdLead = 0;

	Path path;
	while (i != def.end())
	{
		skipUntilNot(i, def.end(), isWhiteSpace);
		if (i == def.end())
			break;

		wchar_t cmd = *i;
		if (!isCommand(cmd))
		{
			// No command; assume shorthand expressions.
			if (toupper(cmdLead) == L'M')
				cmd = isupper(cmdLead) ? L'L' : L'l';
			else if (toupper(cmdLead) == L'C')
				cmd = isupper(cmdLead) ? L'C' : L'c';
			else if (toupper(cmdLead) == L'L')
				cmd = isupper(cmdLead) ? L'L' : L'l';
			else
				return nullptr;
		}
		else
		{
			cmdLead = cmd;
			++i;
		}

		bool relative = (cmd != toupper(cmd));
		switch (toupper(cmd))
		{
		case L'M':	// Move to
			{
				float x = parseDecimalNumber(i, def.end());
				float y = parseDecimalNumber(i, def.end());
				path.moveTo(x, y, relative);
			}
			break;

		case L'L':	// Line to
			{
				float x = parseDecimalNumber(i, def.end());
				float y = parseDecimalNumber(i, def.end());
				path.lineTo(x, y, relative);
			}
			break;

		case L'V':	// Vertical line to
			{
				float x = parseDecimalNumber(i, def.end());
				path.lineTo(x, path.getCursor().y, relative);
			}
			break;

		case L'H':	// Horizontal line to
			{
				float y = parseDecimalNumber(i, def.end());
				path.lineTo(path.getCursor().x, y, relative);
			}
			break;

		case L'Q':	// Quadric to
			{
				float x1 = parseDecimalNumber(i, def.end());
				float y1 = parseDecimalNumber(i, def.end());
				float x2 = parseDecimalNumber(i, def.end());
				float y2 = parseDecimalNumber(i, def.end());
				path.quadricTo(x1, y1, x2, y2, relative);
			}
			break;

		case L'T':	// Quadric to (shorthand/smooth)
			{
				float x = parseDecimalNumber(i, def.end());
				float y = parseDecimalNumber(i, def.end());
				path.quadricTo(x, y, relative);
			}
			break;

		case L'C':	// Cubic to
			{
				float x1 = parseDecimalNumber(i, def.end());
				float y1 = parseDecimalNumber(i, def.end());
				float x2 = parseDecimalNumber(i, def.end());
				float y2 = parseDecimalNumber(i, def.end());
				float x = parseDecimalNumber(i, def.end());
				float y = parseDecimalNumber(i, def.end());
				path.cubicTo(x1, y1, x2, y2, x, y, relative);
			}
			break;

		case L'S':	// Cubic to (shorthand/smooth)
			{
				float x1 = parseDecimalNumber(i, def.end());
				float y1 = parseDecimalNumber(i, def.end());
				float x2 = parseDecimalNumber(i, def.end());
				float y2 = parseDecimalNumber(i, def.end());
				path.cubicTo(x1, y1, x2, y2, relative);
			}
			break;

		case L'A':	// Elliptic arc
			{
				float rx = parseDecimalNumber(i, def.end());
				float ry = parseDecimalNumber(i, def.end());
				float rotation = parseDecimalNumber(i, def.end());
				float la = parseDecimalNumber(i, def.end());
				float sf = parseDecimalNumber(i, def.end());
				float x = parseDecimalNumber(i, def.end());
				float y = parseDecimalNumber(i, def.end());
				path.lineTo(x, y, relative);
			}
			break;

		case L'Z':	// Close sub path
			path.close();
			break;

		default:
			log::error << L"Unknown path command character \"" << *i << L"\"" << Endl;
		}
	}

	return new PathShape(path);
}

Ref< Shape > Parser::parseText(xml::Element* elm)
{
	float x = parseAttr(elm, L"x");
	float y = parseAttr(elm, L"y");
	return new TextShape(Vector2(x, y));
}

void Parser::parseDefs(xml::Element* elm)
{
	for (xml::Node* child = elm->getFirstChild(); child; child = child->getNextSibling())
	{
		if (!is_a< xml::Element >(child))
			continue;

		xml::Element* ch = static_cast< xml::Element* >(child);
		if (!ch->hasAttribute(L"id"))
		{
			log::warning << L"Invalid definition, no \"id\" attribute" << Endl;
			continue;
		}

		std::wstring name = ch->getName();
		std::wstring id = ch->getAttribute(L"id")->getValue();

		if (name == L"linearGradient")
		{
			RefArray< xml::Element > stops;
			elm->get(L"stop", stops);

			if (!stops.empty())
			{
				Ref< Gradient > gradient = new Gradient(Gradient::GtLinear);
				for (RefArray< xml::Element >::iterator i = stops.begin(); i != stops.end(); ++i)
				{
					xml::Element* stop = *i;
					if (!stop->hasAttribute(L"offset") || !stop->hasAttribute(L"stop-color"))
						continue;

					float offset;
					std::wstringstream(stop->getAttribute(L"offset")->getValue()) >> offset;

					Color4f color;
					parseColor(stop->getAttribute(L"stop-color")->getValue(), color);

					gradient->addStop(offset, color);
				}
				m_gradients[id] = gradient;
			}
		}
		else if (name == L"radialGradient")
		{
			RefArray< xml::Element > stops;
			elm->get(L"stop", stops);

			if (!stops.empty())
			{
				Ref< Gradient > gradient = new Gradient(Gradient::GtRadial);
				for (RefArray< xml::Element >::iterator i = stops.begin(); i != stops.end(); ++i)
				{
					xml::Element* stop = *i;
					if (!stop->hasAttribute(L"offset") || !stop->hasAttribute(L"stop-color"))
						continue;

					float offset;
					std::wstringstream(stop->getAttribute(L"offset")->getValue()) >> offset;

					Color4f color;
					parseColor(stop->getAttribute(L"stop-color")->getValue(), color);

					gradient->addStop(offset, color);
				}
				m_gradients[id] = gradient;
			}
		}
		else
			log::error << L"Unknown definition element \"" << name << L"\"" << Endl;
	}
}

Ref< Style > Parser::parseStyle(xml::Element* elm)
{
	if (!elm)
		return nullptr;

	Ref< Style > style;
	Color4f color;

	if (elm->hasAttribute(L"fill"))
	{
		style = new Style();

		std::wstring fillDesc = elm->getAttribute(L"fill")->getValue();
		if (parseColor(fillDesc, color))
		{
			style->setFillEnable(true);
			style->setFill(color);
		}
		else
			style->setFillEnable(false);
	}
	else if (elm->hasAttribute(L"style"))
	{
		style = new Style();

		std::vector< std::wstring > styles;
		Split< std::wstring >::any(elm->getAttribute(L"style")->getValue(), L";", styles);

		for (std::vector< std::wstring >::iterator i = styles.begin(); i != styles.end(); ++i)
		{
			std::wstring::size_type j = i->find(L':');
			if (j == std::string::npos)
				continue;

			std::wstring key = trim(i->substr(0, j));
			std::wstring value = trim(i->substr(j + 1));

			if (key == L"display")
				;
			else if (key == L"opacity")
			{
				float opacity;
				std::wstringstream(value) >> opacity;
				style->setOpacity(opacity);
			}
			else if (key == L"fill")
			{
				if (parseColor(value, color))
				{
					style->setFillEnable(true);
					style->setFill(color);
				}
				else
					style->setFillEnable(false);
			}
			else if (key == L"fill-rule")
				;
			else if (key == L"fill-opacity")
			{
				float fillOpacity;
				std::wstringstream(value) >> fillOpacity;
				style->setOpacity(fillOpacity);
			}
			else if (key == L"stroke")
			{
				if (parseColor(value, color))
				{
					style->setStrokeEnable(true);
					style->setStroke(color);
				}
				else
					style->setStrokeEnable(false);
			}
			else if (key == L"stroke-width")
			{
				float strokeWidth;
				std::wstringstream(value) >> strokeWidth;
				style->setStrokeWidth(strokeWidth);
			}
			else if (key == L"stroke-dasharray")
				;
			else if (key == L"stroke-dashoffset")
				;
			else if (key == L"stroke-opacity")
				;
			else if (key == L"stroke-linecap")
				;
			else if (key == L"stroke-linejoin")
				;
			else if (key == L"stroke-miterlimit")
				;
			else
				log::debug << L"Unknown CSS style \"" << key << L"\"" << Endl;
		}
	}

	return style;
}

Matrix33 Parser::parseTransform(xml::Element* elm)
{
	if (!elm || !elm->hasAttribute(L"transform"))
		return Matrix33::identity();

	Matrix33 transform = Matrix33::identity();

	std::wstring transformDesc = elm->getAttribute(L"transform")->getValue();
	std::wstring::iterator i = transformDesc.begin();

	while (i != transformDesc.end())
	{
		skipUntilNot(i, transformDesc.end(), isWhiteSpace);

		std::wstring::iterator j = i;

		while (i != transformDesc.end() && *i != L'(')
			++i;

		if (i == transformDesc.end())
			break;

		std::wstring fnc(j, i);

		j = ++i;

		while (i != transformDesc.end() && *i != L')')
			++i;

		if (i == transformDesc.end())
			break;

		std::wstring args(j, i);

		if (fnc == L"matrix")
		{
			std::vector< float > argv;
			Split< std::wstring, float >::any(args, L",", argv);

			if (argv.size() >= 6)
				transform *= Matrix33(
					argv[0], argv[1], 0.0f,
					argv[2], argv[3], 0.0f,
					argv[4], argv[5], 1.0f
				).transpose();
		}
		else if (fnc == L"translate")
		{
			std::vector< float > argv;
			Split< std::wstring, float >::any(args, L",", argv);

			if (argv.size() >= 2)
				transform *= translate(argv[0], argv[1]);
		}
		else if (fnc == L"scale")
		{
			std::vector< float > argv;
			Split< std::wstring, float >::any(args, L",", argv);

			if (argv.size() >= 1)
				transform * scale(argv[0], argv[0]);
		}
		else
			log::error << L"Unknown transform function \"" << fnc << L"\"" << Endl;

		++i;
	}

	return transform;
}

float Parser::parseAttr(xml::Element* elm, const std::wstring& attrName, float defValue) const
{
	if (elm && elm->hasAttribute(attrName))
	{
		std::wstring attrValue = elm->getAttribute(attrName)->getValue();
		std::wstringstream ss(attrValue);
		ss >> defValue;
	}
	return defValue;
}

	}
}
