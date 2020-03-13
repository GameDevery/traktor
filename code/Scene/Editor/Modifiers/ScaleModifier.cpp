#include <numeric>
#include "Core/Math/Line2.h"
#include "Core/Math/Winding2.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Modifiers/ScaleModifier.h"
#include "Ui/Application.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_guideThickness(0.02f);
const Scalar c_guideScale(0.15f);
const Scalar c_guideMinLength(1.0f);
const float c_infinite = 1e4f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.ScaleModifier", ScaleModifier, IModifier)

ScaleModifier::ScaleModifier(SceneEditorContext* context)
:	m_context(context)
,	m_axisEnable(0)
,	m_axisHot(0)
{
}

bool ScaleModifier::activate()
{
	return true;
}

void ScaleModifier::deactivate()
{
}

void ScaleModifier::selectionChanged()
{
	m_entityAdapters.clear();
	m_context->getEntities(m_entityAdapters, SceneEditorContext::GfDefault | SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfNoExternalChild);

	m_baseScale.clear();
	m_center = Vector4::zero();

	for (RefArray< EntityAdapter >::const_iterator i = m_entityAdapters.begin(); i != m_entityAdapters.end(); ++i)
	{
		Transform T = (*i)->getTransform();
		m_baseScale.push_back(Vector4::one()); // T.scale());
		m_center += T.translation();
	}

	if (!m_entityAdapters.empty())
		m_center /= Scalar(float(m_entityAdapters.size()));

	m_center = m_center.xyz1();
	m_deltaScale = Vector4::zero();
}

bool ScaleModifier::cursorMoved(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (m_entityAdapters.empty())
		return false;

	float axisLength = m_context->getGuideSize();
	float arrowLength = axisLength / 8.0f;
	float squareLength = axisLength / 3.0f;

	TransformChain tc = transformChain;
	tc.pushWorld(translate(m_center));

	Vector4 viewDirection = m_center - tc.getView().inverse().translation();
	float sx = viewDirection.x() < 0.0f ? 1.0f : -1.0f;
	float sy = viewDirection.y() < 0.0f ? 1.0f : -1.0f;
	float sz = viewDirection.z() < 0.0f ? 1.0f : -1.0f;

	Vector2 center, axis0[3], axis1[3];
	tc.objectToScreen(Vector4(0.0f, 0.0f, 0.0f, 1.0f), center);
	tc.objectToScreen(Vector4(-axisLength, 0.0f, 0.0f, 1.0f), axis0[0]);
	tc.objectToScreen(Vector4(0.0f, -axisLength, 0.0f, 1.0f), axis0[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, -axisLength, 1.0f), axis0[2]);
	tc.objectToScreen(Vector4(axisLength + arrowLength, 0.0f, 0.0f, 1.0f), axis1[0]);
	tc.objectToScreen(Vector4(0.0f, axisLength + arrowLength, 0.0f, 1.0f), axis1[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, axisLength + arrowLength, 1.0f), axis1[2]);

	tc.popWorld();

	m_axisHot = 0;

	float guideThickness = ui::getSystemDPI() * c_guideThickness / 96.0f;
	if (Line2(axis0[0], axis1[0]).classify(cursorPosition, guideThickness))
		m_axisHot |= 1;
	if (Line2(axis0[1], axis1[1]).classify(cursorPosition, guideThickness))
		m_axisHot |= 2;
	if (Line2(axis0[2], axis1[2]).classify(cursorPosition, guideThickness))
		m_axisHot |= 4;

	return m_axisHot != 0;
}

bool ScaleModifier::handleCommand(const ui::Command& command)
{
	return false;
}

bool ScaleModifier::begin(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	int32_t mouseButton
)
{
	m_axisEnable = m_axisHot;
	return true;
}

void ScaleModifier::apply(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta
)
{
	if (m_axisEnable & 1)
		m_deltaScale += Vector4(screenDelta.x(), 0.0f, 0.0f) * Scalar(0.01f);
	if (m_axisEnable & 2)
		m_deltaScale += Vector4(0.0f, screenDelta.x(), 0.0f) * Scalar(0.01f);
	if (m_axisEnable & 4)
		m_deltaScale += Vector4(0.0f, 0.0f, screenDelta.x()) * Scalar(0.01f);

	//for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
	//{
	//	Transform T = m_entityAdapters[i]->getTransform();
	//	m_entityAdapters[i]->setTransform(Transform(
	//		T.translation(),
	//		T.rotation(),
	//		(m_baseScale[i] + m_deltaScale).xyz1()
	//	));
	//}

	m_axisHot = m_axisEnable;
}

void ScaleModifier::end(const TransformChain& transformChain)
{
	selectionChanged();
	m_axisEnable = 0;
}

void ScaleModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_entityAdapters.empty())
		return;

	float axisLength = m_context->getGuideSize();

	primitiveRenderer->pushWorld(translate(m_center));
	primitiveRenderer->pushDepthState(false, false, false);

	// Guide axis lines.
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(axisLength * (1.0f + m_deltaScale.x()), 0.0f, 0.0f, 1.0f),
		(m_axisHot & 1) ? 3.0f : 1.0f,
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, axisLength * (1.0f + m_deltaScale.y()), 0.0f, 1.0f),
		(m_axisHot & 2) ? 3.0f : 1.0f,
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, 0.0f, axisLength * (1.0f + m_deltaScale.z()), 1.0f),
		(m_axisHot & 4) ? 3.0f : 1.0f,
		Color4ub(0, 0, 255, 255)
	);

	// Guide blocks.
	primitiveRenderer->drawSolidPoint(
		Vector4(axisLength * (1.0f + m_deltaScale.x()), 0.0f, 0.0f, 1.0f),
		6.0f,
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawSolidPoint(
		Vector4(0.0f, axisLength * (1.0f + m_deltaScale.y()), 0.0f, 1.0f),
		6.0f,
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawSolidPoint(
		Vector4(0.0f, 0.0f, axisLength * (1.0f + m_deltaScale.z()), 1.0f),
		6.0f,
		Color4ub(0, 0, 255, 255)
	);

	primitiveRenderer->popDepthState();
	primitiveRenderer->popWorld();
}

	}
}
