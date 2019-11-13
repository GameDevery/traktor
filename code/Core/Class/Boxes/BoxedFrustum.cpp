#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedAabb3.h"
#include "Core/Class/Boxes/BoxedFrustum.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedFrustum, 16 > s_allocBoxedFrustum;
	
	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.Frustum", BoxedFrustum, Boxed)

BoxedFrustum::BoxedFrustum()
{
}

BoxedFrustum::BoxedFrustum(const Frustum& value)
:	m_value(value)
{
}

void BoxedFrustum::buildPerspective(float vfov, float aspect, float zn, float zf)
{
	m_value.buildPerspective(vfov, aspect, zn, zf);
}

void BoxedFrustum::buildOrtho(float width, float height, float zn, float zf)
{
	m_value.buildOrtho(width, height, zn, zf);
}

void BoxedFrustum::setNearZ(float zn)
{
	m_value.setNearZ(Scalar(zn));
}

float BoxedFrustum::getNearZ() const
{
	return m_value.getNearZ();
}

void BoxedFrustum::setFarZ(float zf)
{
	m_value.setFarZ(Scalar(zf));
}

float BoxedFrustum::getFarZ() const
{
	return m_value.getFarZ();
}

bool BoxedFrustum::insidePoint(const BoxedVector4* point) const
{
	return m_value.inside(point->unbox()) != Frustum::IrOutside;
}

int32_t BoxedFrustum::insideSphere(const BoxedVector4* center, float radius) const
{
	return m_value.inside(center->unbox(), Scalar(radius));
}

int32_t BoxedFrustum::insideAabb(const BoxedAabb3* aabb) const
{
	return m_value.inside(aabb->unbox());
}

const Plane& BoxedFrustum::getPlane(int32_t index) const
{
	return m_value.planes[index];
}

const Vector4& BoxedFrustum::getCorner(int32_t index) const
{
	return m_value.corners[index];
}

const Vector4& BoxedFrustum::getCenter() const
{
	return m_value.center;
}

std::wstring BoxedFrustum::toString() const
{
	return L"(frustum)";
}

void* BoxedFrustum::operator new (size_t size)
{
	return s_allocBoxedFrustum.alloc();
}

void BoxedFrustum::operator delete (void* ptr)
{
	s_allocBoxedFrustum.free(ptr);
}

}
