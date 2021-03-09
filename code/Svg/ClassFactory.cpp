#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedAabb2.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedColor4f.h"
#include "Core/Class/Boxes/BoxedMatrix33.h"
#include "Core/Class/Boxes/BoxedRefArray.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Svg/ClassFactory.h"
#include "Svg/Document.h"
#include "Svg/Gradient.h"
#include "Svg/IShapeVisitor.h"
#include "Svg/Parser.h"
#include "Svg/Path.h"
#include "Svg/PathShape.h"
#include "Svg/Shape.h"
#include "Svg/Style.h"
#include "Xml/Document.h"

namespace traktor
{
	namespace svg
	{
		namespace
		{

class BoxedSubPath : public Object
{
	T_RTTI_CLASS;

public:
	explicit BoxedSubPath(const SubPath& subPath)
	:	m_subPath(subPath)
	{
	}

	int32_t getType() const { return m_subPath.type; }

	bool getClosed() const { return m_subPath.closed; }

	const Vector2& getOrigin() const { return m_subPath.origin; }

	const AlignedVector< Vector2 >& getPoints() const { return m_subPath.points; }

private:
	const SubPath& m_subPath;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.svg.SubPath", BoxedSubPath, Object)

class ShapeVisitorDelegate : public IShapeVisitor
{
public:
	ShapeVisitorDelegate(IRuntimeDelegate* delegateEnter, IRuntimeDelegate* delegateLeave)
	:	m_delegateEnter(delegateEnter)
	,	m_delegateLeave(delegateLeave)
	{
	}

	virtual void enter(Shape* shape) override final
	{
		if (m_delegateEnter)
		{
			Any argv[] = { CastAny< Shape* >::set(shape) };
			Any ret = m_delegateEnter->call(sizeof_array(argv), argv);
		}
	}

	virtual void leave(Shape* shape) override final
	{
		if (m_delegateLeave)
		{
			Any argv[] = { CastAny< Shape* >::set(shape) };
			Any ret = m_delegateLeave->call(sizeof_array(argv), argv);
		}
	}

private:
	Ref< IRuntimeDelegate > m_delegateEnter;
	Ref< IRuntimeDelegate > m_delegateLeave;
};

RefArray< BoxedSubPath > Path_getSubPaths(Path* self)
{
	RefArray< BoxedSubPath > bsp;
	for (const auto& sp : self->getSubPaths())
		bsp.push_back(new BoxedSubPath(sp));
	return bsp;
}

void Path_quadricTo_1(Path* self, float x, float y, bool relative = false)
{
	self->quadricTo(x, y, relative);
}

void Path_quadricTo_2(Path* self, float x1, float y1, float x, float y, bool relative = false)
{
	self->quadricTo(x1, y1, x, y, relative);
}

void Path_cubicTo_1(Path* self, float x2, float y2, float x, float y, bool relative = false)
{
	self->cubicTo(x2, y2, x, y, relative);
}

void Path_cubicTo_2(Path* self, float x1, float y1, float x2, float y2, float x, float y, bool relative = false)
{
	self->cubicTo(x1, y1, x2, y2, x, y, relative);
}

void Shape_visit(Shape* self, IRuntimeDelegate* enter, IRuntimeDelegate* leave)
{
	ShapeVisitorDelegate visitor(enter, leave);
	self->visit(&visitor);
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.svg.ClassFactory", 0, ClassFactory, IRuntimeClassFactory)

void ClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classDocument = new AutoRuntimeClass< Document >();
	classDocument->addConstructor();
	classDocument->addProperty("viewBox", &Document::setViewBox, &Document::getViewBox);
	classDocument->addProperty("size", &Document::setSize, &Document::getSize);
	registrar->registerClass(classDocument);

	auto classGradient = new AutoRuntimeClass< Gradient >();
	classGradient->addMethod("addStop", &Gradient::addStop);
	registrar->registerClass(classGradient);

	auto classIShapeVisitor = new AutoRuntimeClass< IShapeVisitor >();
	registrar->registerClass(classIShapeVisitor);

	auto classParser = new AutoRuntimeClass< Parser >();
	classParser->addConstructor();
	classParser->addMethod("parse", &Parser::parse);
	registrar->registerClass(classParser);

	auto classPath = new AutoRuntimeClass< Path >();
	classPath->addConstructor();
	classPath->addProperty("cursor", &Path::getCursor);
	classPath->addProperty("subPaths", &Path_getSubPaths);
	classPath->addMethod< void, float, float >("moveTo", &Path::moveTo);
	classPath->addMethod< void, float, float, bool >("moveTo", &Path::moveTo);
	classPath->addMethod< void, float, float >("moveTo", &Path::lineTo);
	classPath->addMethod< void, float, float, bool >("moveTo", &Path::lineTo);
	classPath->addMethod< void, float, float >("quadricTo", &Path_quadricTo_1);
	classPath->addMethod< void, float, float, bool >("quadricTo", &Path_quadricTo_1);
	classPath->addMethod< void, float, float, float, float >("quadricTo", &Path_quadricTo_2);
	classPath->addMethod< void, float, float, float, float, bool >("quadricTo", &Path_quadricTo_2);
	classPath->addMethod< void, float, float, float, float >("cubicTo", &Path_cubicTo_1);
	classPath->addMethod< void, float, float, float, float, bool >("cubicTo", &Path_cubicTo_1);
	classPath->addMethod< void, float, float, float, float, float, float >("cubicTo", &Path_cubicTo_2);
	classPath->addMethod< void, float, float, float, float, float, float, bool >("cubicTo", &Path_cubicTo_2);
	classPath->addMethod("close", &Path::close);
	classPath->addMethod("transform", &Path::transform);
	registrar->registerClass(classPath);

	auto classBoxedSubPath = new AutoRuntimeClass< BoxedSubPath >();
	classBoxedSubPath->addConstant("Undefined", Any::fromInt32(SptUndefined));
	classBoxedSubPath->addConstant("Linear", Any::fromInt32(SptLinear));
	classBoxedSubPath->addConstant("Quadric", Any::fromInt32(SptQuadric));
	classBoxedSubPath->addConstant("Cubic", Any::fromInt32(SptCubic));
	classBoxedSubPath->addProperty("type", &BoxedSubPath::getType);
	classBoxedSubPath->addProperty("closed", &BoxedSubPath::getClosed);
	classBoxedSubPath->addProperty("origin", &BoxedSubPath::getOrigin);
	classBoxedSubPath->addProperty("points", &BoxedSubPath::getPoints);
	registrar->registerClass(classBoxedSubPath);

	auto classPathShape = new AutoRuntimeClass< PathShape >();
	classPathShape->addConstructor< const Path& >();
	classPathShape->addProperty("path", &PathShape::getPath);
	registrar->registerClass(classPathShape);

	auto classShape = new AutoRuntimeClass< Shape >();
	classShape->addProperty("style", &Shape::setStyle, &Shape::getStyle);
	classShape->addProperty("transform", &Shape::setTransform, &Shape::getTransform);
	classShape->addProperty("globalTransform", &Shape::getGlobalTransform);
	classShape->addMethod("addChild", &Shape::addChild);
	classShape->addMethod("visit", &Shape_visit);
	registrar->registerClass(classShape);

	auto classStyle = new AutoRuntimeClass< Style >();
	classStyle->addConstructor();
	classStyle->addProperty("fillEnable", &Style::setFillEnable, &Style::getFillEnable);
	classStyle->addProperty("fillGradient", &Style::setFillGradient, &Style::getFillGradient);
	classStyle->addProperty("fill", &Style::setFill, &Style::getFill);
	classStyle->addProperty("strokeEnable", &Style::setStrokeEnable, &Style::getStrokeEnable);
	classStyle->addProperty("strokeGradient", &Style::setStrokeGradient, &Style::getStrokeGradient);
	classStyle->addProperty("strokeWidth", &Style::setStrokeWidth, &Style::getStrokeWidth);
	classStyle->addProperty("stroke", &Style::setStroke, &Style::getStroke);
	classStyle->addProperty("opacity", &Style::setOpacity, &Style::getOpacity);
	registrar->registerClass(classStyle);
}

	}
}
