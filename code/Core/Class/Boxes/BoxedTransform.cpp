#include "Core/Class/BoxedAllocator.h"
#include "Core/Class/Boxes/BoxedQuaternion.h"
#include "Core/Class/Boxes/BoxedMatrix44.h"
#include "Core/Class/Boxes/BoxedTransform.h"
#include "Core/Class/Boxes/BoxedVector4.h"

namespace traktor
{
	namespace
	{
	
BoxedAllocator< BoxedTransform, 8192 > s_allocBoxedTransform;
	
	}
T_IMPLEMENT_RTTI_CLASS(L"traktor.Transform", BoxedTransform, Boxed)

BoxedTransform::BoxedTransform()
{
}

BoxedTransform::BoxedTransform(const Transform& value)
:	m_value(value)
{
}

BoxedTransform::BoxedTransform(const BoxedVector4* translation, const BoxedQuaternion* rotation)
:	m_value(translation->unbox(), rotation->unbox())
{
}

BoxedTransform::BoxedTransform(const BoxedMatrix44* m)
:	m_value(m->unbox())
{
}

Plane BoxedTransform::get_planeX() const
{
	return Plane(m_value.axisX(), m_value.translation());
}

Plane BoxedTransform::get_planeY() const
{
	return Plane(m_value.axisY(), m_value.translation());
}

Plane BoxedTransform::get_planeZ() const
{
	return Plane(m_value.axisZ(), m_value.translation());
}

Transform BoxedTransform::inverse() const
{
	return m_value.inverse();
}

Matrix44 BoxedTransform::toMatrix44() const
{
	return m_value.toMatrix44();
}

Transform BoxedTransform::concat(const BoxedTransform* t) const
{
	return m_value * t->m_value;
}

Vector4 BoxedTransform::transform(const BoxedVector4* v) const
{
	return m_value * v->unbox();
}

Transform BoxedTransform::lookAt(const BoxedVector4* position, const BoxedVector4* target, const BoxedVector4* up)
{
	return Transform(traktor::lookAt(position->unbox(), target->unbox(), up->unbox()).inverse());
}

std::wstring BoxedTransform::toString() const
{
	return L"(transform)";
}

void* BoxedTransform::operator new (size_t size)
{
	return s_allocBoxedTransform.alloc();
}

void BoxedTransform::operator delete (void* ptr)
{
	s_allocBoxedTransform.free(ptr);
}

}
