#include <cmath>
#include "Animation/Editor/AnimationEditorPage.h"
#include "Animation/Editor/VolumePicker.h"
#include "Animation/Animation/Animation.h"
#include "Animation/Animation/AnimationFactory.h"
#include "Animation/Skeleton.h"
#include "Animation/Bone.h"
#include "Animation/SkeletonUtils.h"
#include "Animation/IK/IKPoseController.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPageSite.h"
#include "Editor/Settings.h"
#include "Editor/TypeBrowseFilter.h"
#include "Editor/UndoStack.h"
#include "Database/Instance.h"
#include "Resource/ResourceManager.h"
#include "Ui/Application.h"
#include "Ui/Clipboard.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/Bitmap.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/Events/PaintEvent.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/QuadSplitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Custom/Sequencer/SequencerControl.h"
#include "Ui/Custom/Sequencer/Sequence.h"
#include "Ui/Custom/Sequencer/Tick.h"
#include "Ui/Itf/IWidget.h"
#include "I18N/Text.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Resource/TextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Math/Const.h"
#include "Core/Math/Vector2.h"
#include "Core/Log/Log.h"

// Resources
#include "Resources/Playback.h"
#include "Resources/Skeleton.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

const float c_fov = 80.0f;
const float c_orthoSize = 100.0f;
const float c_orthoSizeMin = 10.0f;
const float c_nearZ = 0.1f;
const float c_farZ = 1000.0f;
const float c_wheelRotationDelta = -10.0f;

class RenderWidgetData : public Object
{
	T_RTTI_CLASS;

public:
	RenderWidgetData()
	:	orthogonal(false)
	,	size(c_orthoSize)
	,	cameraAngleX(0.0f)
	,	cameraAngleY(0.0f)
	,	cameraOffset(0.0f, 0.0f, 100.0f)
	{
	}

	Ref< render::IRenderView > renderView;
	bool orthogonal;
	float size;
	float cameraAngleX;
	float cameraAngleY;
	Vector4 cameraOffset;
	Ref< VolumePicker > picker;
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RenderWidgetData", RenderWidgetData, Object)

class KeyPoseClipboardData : public ISerializable
{
	T_RTTI_CLASS;

public:
	KeyPoseClipboardData()
	{
	}

	KeyPoseClipboardData(const Animation::KeyPose& pose)
	:	m_pose(pose)
	{
	}

	const Animation::KeyPose& getPose() const
	{
		return m_pose;
	}

	virtual bool serialize(ISerializer& s)
	{
		return s >> MemberComposite< Animation::KeyPose >(L"pose", m_pose);
	}

private:
	Animation::KeyPose m_pose;
};

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.KeyPoseClipboardData", 0, KeyPoseClipboardData, ISerializable)

class PoseIdData : public Object
{
	T_RTTI_CLASS;

public:
	int m_id;

	PoseIdData(int id)
	:	m_id(id)
	{
	}
};

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PoseIdData", PoseIdData, Object)

		}

const int c_animationLength = 10000;

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.AnimationEditorPage", AnimationEditorPage, editor::IEditorPage)

AnimationEditorPage::AnimationEditorPage(editor::IEditor* editor)
:	m_editor(editor)
,	m_selectedBone(0)
,	m_showGhostTrail(false)
,	m_twistLock(false)
,	m_ikEnabled(false)
,	m_relativeTwist(0.0f)
,	m_haveRelativeTwist(0.0f)
,	m_editMode(false)
,	m_previewAnimation(false)
,	m_cameraOffsetScale(1.0f, 1.0f, 1.0f, 1.0f)
,	m_cameraSizeScale(1.0f)
{
}

bool AnimationEditorPage::create(ui::Container* parent, editor::IEditorPageSite* site)
{
	m_site = site;
	T_ASSERT (m_site);

	Ref< render::IRenderSystem > renderSystem = m_editor->getRenderSystem();

	Ref< ui::custom::QuadSplitter > splitter = new ui::custom::QuadSplitter();
	splitter->create(parent, ui::Point(50, 50), true);

	for (int i = 0; i < 4; ++i)
	{
		m_renderWidgets[i] = new ui::Widget();
		m_renderWidgets[i]->create(splitter, ui::WsClientBorder);
		m_renderWidgets[i]->addButtonDownEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventRenderButtonDown));
		m_renderWidgets[i]->addButtonUpEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventRenderButtonUp));
		m_renderWidgets[i]->addMouseMoveEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventRenderMouseMove));
		m_renderWidgets[i]->addMouseWheelEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventRenderMouseWheel));
		m_renderWidgets[i]->addSizeEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventRenderSize));
		m_renderWidgets[i]->addPaintEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventRenderPaint));
		
		Ref< RenderWidgetData > data = new RenderWidgetData();
		m_renderWidgets[i]->setData(L"DATA", data);
	}

	// Create sequencer panel.
	m_sequencerPanel = new ui::Container();
	m_sequencerPanel->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	m_sequencerPanel->setText(i18n::Text(L"ANIMATION_EDITOR_SEQUENCER"));

	m_toolBarPlay = new ui::custom::ToolBar();
	m_toolBarPlay->create(m_sequencerPanel);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourcePlayback, sizeof(c_ResourcePlayback), L"png"), 6);
	m_toolBarPlay->addImage(ui::Bitmap::load(c_ResourceSkeleton, sizeof(c_ResourceSkeleton), L"png"), 5);
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_REWIND"), ui::Command(L"Animation.Editor.Rewind"), 0));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_PLAY"), ui::Command(L"Animation.Editor.Play"), 1));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_STOP"), ui::Command(L"Animation.Editor.Stop"), 2));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_FORWARD"), ui::Command(L"Animation.Editor.Forward"), 3));
	m_toolBarPlay->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_BROWSE_SKELETON"), ui::Command(L"Animation.Editor.BrowseSkeleton"), 6));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_BROWSE_SKIN"), ui::Command(L"Animation.Editor.BrowseSkin"), 9));
	m_toolBarPlay->addItem(new ui::custom::ToolBarSeparator());
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_TOGGLE_TRAIL"), ui::Command(L"Animation.Editor.ToggleTrail"), 7, m_showGhostTrail ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_TOGGLE_TWIST_LOCK"), ui::Command(L"Animation.Editor.ToggleTwistLock"), 8, m_twistLock ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle));
	m_toolBarPlay->addItem(new ui::custom::ToolBarButton(i18n::Text(L"ANIMATION_EDITOR_TOGGLE_IK"), ui::Command(L"Animation.Editor.ToggleIK"), 10, m_ikEnabled ? ui::custom::ToolBarButton::BsDefaultToggled : ui::custom::ToolBarButton::BsDefaultToggle));
	m_toolBarPlay->addClickEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventToolClick));

	m_sequencer = new ui::custom::SequencerControl();
	m_sequencer->create(m_sequencerPanel);
	m_sequencer->setLength(c_animationLength);
	m_sequencer->addButtonDownEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventSequencerButtonDown));
	m_sequencer->addCursorMoveEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventSequencerCursorMove));
	m_sequencer->addTimerEventHandler(ui::createMethodHandler(this, &AnimationEditorPage::eventSequencerTimer));
	m_sequencer->startTimer(30);

	m_site->createAdditionalPanel(m_sequencerPanel, 100, true);

	// Build popup menu.
	m_menuPopup = new ui::PopupMenu();
	m_menuPopup->create();
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Animation.Editor.Create"), i18n::Text(L"ANIMATION_EDITOR_CREATE")));
	m_menuPopup->add(new ui::MenuItem(ui::Command(L"Editor.Delete"), i18n::Text(L"ANIMATION_EDITOR_DELETE")));

	// Create render views.
	const float c_cameraAngles[][2] =
	{
		{ 0.0f, 0.0f },
		{ 0.0f, 0.0f },
		{ PI / 2.0f, 0.0f },
		{ 0.0f, -PI / 2.0f }
	};

	for (int i = 0; i < sizeof_array(m_renderWidgets); ++i)
	{
		Ref< RenderWidgetData > data = m_renderWidgets[i]->getData< RenderWidgetData >(L"DATA");

		render::RenderViewCreateDesc desc;
		desc.depthBits = 16;
		desc.stencilBits = 0;
		desc.multiSample = 4;
		desc.waitVBlank = false;

		data->orthogonal = i > 0;
		data->cameraAngleX = c_cameraAngles[i][0];
		data->cameraAngleY = c_cameraAngles[i][1];

		data->renderView = renderSystem->createRenderView(m_renderWidgets[i]->getIWidget()->getSystemHandle(), desc);
		if (!data->renderView)
			return false;

		data->picker = new VolumePicker();
	}

	Ref< db::Database > database = m_editor->getOutputDatabase();

	m_resourceManager = new resource::ResourceManager();
	m_resourceManager->addFactory(
		new render::TextureFactory(database, renderSystem, 0)
	);
	m_resourceManager->addFactory(
		new render::ShaderFactory(database, renderSystem)
	);
	m_resourceManager->addFactory(
		new AnimationFactory(database)
	);

	m_primitiveRenderer = new render::PrimitiveRenderer();
	if (!m_primitiveRenderer->create(m_resourceManager, renderSystem))
		return false;

	m_undoStack = new editor::UndoStack();

	updateSettings();
	return true;
}

void AnimationEditorPage::destroy()
{
	m_site->destroyAdditionalPanel(m_sequencerPanel);

	// Destroy render widgets.
	for (int i = 0; i < sizeof_array(m_renderWidgets); ++i)
	{
		Ref< RenderWidgetData > data = m_renderWidgets[i]->getData< RenderWidgetData >(L"DATA");
		data->renderView->close();
		m_renderWidgets[i]->destroy();
	}

	// Destroy widgets.
	m_menuPopup->destroy();
	m_sequencerPanel->destroy();
}

void AnimationEditorPage::activate()
{
}

void AnimationEditorPage::deactivate()
{
}

bool AnimationEditorPage::setDataObject(db::Instance* instance, Object* data)
{
	m_animationInstance = instance;
	m_animation = checked_type_cast< Animation* >(data);
	updateSequencer();
	return true;
}

Ref< db::Instance > AnimationEditorPage::getDataInstance()
{
	return m_animationInstance;
}

Ref< Object > AnimationEditorPage::getDataObject()
{
	return m_animation;
}

bool AnimationEditorPage::dropInstance(db::Instance* instance, const ui::Point& position)
{
	Ref< Skeleton > skeleton = instance->getObject< Skeleton >();
	if (!skeleton)
		return false;

	setSkeleton(skeleton);
	return true;
}

bool AnimationEditorPage::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
	{
		updateSettings();
	}
	if (command == L"Editor.Cut" || command == L"Editor.Copy")
	{
		int poseIndex;
		if (!getSelectedPoseId(poseIndex))
			return false;

		const Animation::KeyPose& keyPose = m_animation->getKeyPose(poseIndex);
		ui::Application::getInstance()->getClipboard()->setObject(new KeyPoseClipboardData(keyPose));

		if (command == L"Editor.Cut")
		{
			m_undoStack->push(m_animation);

			m_animation->removeKeyPose(poseIndex);
			updateRenderWidgets();
			updateSequencer();
		}
	}
	else if (command == L"Editor.Paste")
	{
		Ref< KeyPoseClipboardData > data = dynamic_type_cast< KeyPoseClipboardData* >(
			ui::Application::getInstance()->getClipboard()->getObject()
		);
		if (!data)
			return false;

		m_undoStack->push(m_animation);
		
		float time = float(m_sequencer->getCursor() / 1000.0f);

		Animation::KeyPose keyPose;
		keyPose.at = time;
		keyPose.pose = data->getPose().pose;
		m_animation->addKeyPose(keyPose);

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Editor.Undo")
	{
		if (!m_undoStack->canUndo())
			return false;

		m_animation = checked_type_cast< Animation* >(m_undoStack->undo(m_animation));

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Editor.Redo")
	{
		if (!m_undoStack->canRedo())
			return false;

		m_animation = checked_type_cast< Animation* >(m_undoStack->redo(m_animation));

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Animation.Editor.Rewind")
	{
		m_sequencer->setCursor(0);
		m_sequencer->update();
		updateRenderWidgets();
	}
	else if (command == L"Animation.Editor.Play")
	{
		m_previewAnimation = true;
	}
	else if (command == L"Animation.Editor.Stop")
	{
		m_previewAnimation = false;
	}
	else if (command == L"Animation.Editor.Forward")
	{
		if (!m_animation->empty())
		{
			m_sequencer->setCursor(int(m_animation->getLastKeyPose().at * 1000.0f));
			m_sequencer->update();
			updateRenderWidgets();
		}
	}
	else if (command == L"Animation.Editor.BrowseSkeleton")
	{
		editor::TypeBrowseFilter filter(type_of< Skeleton >());
		Ref< db::Instance > skeletonInstance = m_editor->browseInstance(&filter);
		if (skeletonInstance)
		{
			Ref< Skeleton > skeleton = skeletonInstance->getObject< Skeleton >();
			if (skeleton)
				setSkeleton(skeleton);
		}
	}
	else if (command == L"Animation.Editor.BrowseSkin")
	{
	}
	else if (command == L"Animation.Editor.ToggleTrail")
	{
		m_showGhostTrail = !m_showGhostTrail;
		updateRenderWidgets();
	}
	else if (command == L"Animation.Editor.ToggleTwistLock")
	{
		m_twistLock = !m_twistLock;
	}
	else if (command == L"Animation.Editor.ToggleIK")
	{
		m_ikEnabled = !m_ikEnabled;
		updateRenderWidgets();
	}
	else if (command == L"Animation.Editor.SelectPreviousBone")
	{
		if (m_skeleton)
		{
			if (m_selectedBone > 0)
				m_selectedBone--;
			else
				m_selectedBone = m_skeleton->getBoneCount() - 1;
			updateRenderWidgets();
		}
	}
	else if (command == L"Animation.Editor.SelectNextBone")
	{
		if (m_skeleton)
		{
			if (m_selectedBone < int(m_skeleton->getBoneCount()) - 1)
				m_selectedBone++;
			else
				m_selectedBone = 0;
			updateRenderWidgets();
		}
	}
	else if (command == L"Animation.Editor.Create")
	{
		float time = float(m_sequencer->getCursor() / 1000.0f);

		m_undoStack->push(m_animation);

		Animation::KeyPose keyPose;
		keyPose.at = time;
		m_animation->getPose(time, keyPose.pose);
		m_animation->addKeyPose(keyPose);

		updateRenderWidgets();
		updateSequencer();
	}
	else if (command == L"Editor.Delete")
	{
		int poseIndex;
		if (!getSelectedPoseId(poseIndex))
			return false;

		m_undoStack->push(m_animation);

		m_animation->removeKeyPose(poseIndex);
		updateRenderWidgets();
		updateSequencer();
	}
	else
		return false;

	return true;
}

void AnimationEditorPage::handleDatabaseEvent(const Guid& eventId)
{
	if (m_resourceManager)
		m_resourceManager->update(eventId, true);
}

void AnimationEditorPage::setSkeleton(Skeleton* skeleton)
{
	m_skeleton = skeleton;
	T_ASSERT (m_skeleton);

	Aabb boundingBox = calculateBoundingBox(m_skeleton);
	
	int majorAxis = majorAxis3(boundingBox.getExtent());
	float majorExtent = boundingBox.getExtent()[majorAxis];

	m_cameraOffsetScale.set(
		majorExtent / 20.0f,
		majorExtent / 20.0f,
		majorExtent / 20.0f,
		1.0f
	);

	m_cameraSizeScale = majorExtent / 20.0f;

	updateRenderWidgets();
}

bool AnimationEditorPage::getSelectedPoseId(int& outPoseId) const
{
	RefArray< ui::custom::SequenceItem > selectedItems;
	m_sequencer->getSequenceItems(selectedItems, ui::custom::SequencerControl::GfSelectedOnly);
	if (selectedItems.size() != 1)
		return false;

	Ref< ui::custom::Sequence > selectedSequence = checked_type_cast< ui::custom::Sequence* >(selectedItems.front());
	Ref< ui::custom::Key > selectedKey = selectedSequence->getSelectedKey();
	if (!selectedKey)
		return false;

	Ref< PoseIdData > id = selectedKey->getData< PoseIdData >(L"ID");
	T_ASSERT (id);

	outPoseId = id->m_id;
	return true;
}

void AnimationEditorPage::updateRenderWidgets()
{
	for (int i = 0; i < sizeof_array(m_renderWidgets); ++i)
		m_renderWidgets[i]->update();
}

void AnimationEditorPage::updateSequencer()
{
	uint32_t poseCount = m_animation->getKeyPoseCount();
	if (!poseCount)
		return;

	Ref< ui::custom::Sequence > sequence = new ui::custom::Sequence(i18n::Text(L"ANIMATION_EDITOR_SEQUENCE"));
	for (uint32_t i = 0; i < poseCount; ++i)
	{
		int ms = int(m_animation->getKeyPose(i).at * 1000.0f);

		Ref< ui::custom::Tick > tick = new ui::custom::Tick(ms);
		tick->setData(L"ID", new PoseIdData(i));

		sequence->addKey(tick);
	}

	m_sequencer->removeAllSequenceItems();
	m_sequencer->addSequenceItem(sequence);
	m_sequencer->update();
}

bool AnimationEditorPage::calculateRelativeTwist(int poseIndex, int boneIndex, float& outRelativeTwist) const
{
	if (!m_skeleton)
		return false;

	int parentId = m_skeleton->getBone(m_selectedBone)->getParent();
	if (parentId < 0)
		return false;

	Pose& pose = m_animation->getKeyPose(poseIndex).pose;

	AlignedVector< Transform > poseTransforms;
	calculatePoseTransforms(m_skeleton, &pose, poseTransforms);

	Vector4 parentAxisX = poseTransforms[parentId] * Vector4(1.0f, 0.0f, 0.0f);
	Vector4 parentAxisY = poseTransforms[parentId] * Vector4(0.0f, 1.0f, 0.0f);
	Vector4 parentAxisZ = poseTransforms[parentId] * Vector4(0.0f, 0.0f, 1.0f);

	Vector4 currentAxisX = poseTransforms[m_selectedBone] * Vector4(1.0f, 0.0f, 0.0f);
	Vector4 currentAxisZ = poseTransforms[m_selectedBone] * Vector4(0.0f, 0.0f, 1.0f);
	Vector4 currentAxisXinParent = (currentAxisX - parentAxisZ * dot3(parentAxisZ, currentAxisX)).normalized();

	float relativeDirection = dot3(currentAxisZ, parentAxisZ);
	if (abs(relativeDirection) < FUZZY_EPSILON)
		return false;

	float ax = dot3(currentAxisXinParent, parentAxisY);
	float ay = dot3(currentAxisXinParent, parentAxisX);
	outRelativeTwist = atan2f(ax, ay);

	if (relativeDirection < 0.0f)
		outRelativeTwist = -outRelativeTwist;

	return true;
}

void AnimationEditorPage::drawSkeleton(float time, const Color& defaultColor, const Color& selectedColor, bool drawAxis) const
{
	Pose pose;
	m_animation->getPose(time, pose);

	AlignedVector< Transform > boneTransforms;
	calculatePoseTransforms(
		m_skeleton,
		&pose,
		boneTransforms
	);

	if (m_ikEnabled)
	{
		bool continueUpdate = true;
		IKPoseController(0, 0, 10).evaluate(
			0.0f,
			Transform::identity(),
			m_skeleton,
			boneTransforms,
			boneTransforms,
			continueUpdate
		);
	}

	for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
	{
		const Bone* bone = m_skeleton->getBone(i);

		Vector4 start = boneTransforms[i].translation();
		Vector4 end = boneTransforms[i].translation() + boneTransforms[i] * Vector4(0.0f, 0.0f, bone->getLength(), 0.0f);

		const Color& color = (m_selectedBone == i) ? selectedColor : defaultColor;

		Vector4 d = boneTransforms[i].axisZ();
		Vector4 a = boneTransforms[i].axisX();
		Vector4 b = boneTransforms[i].axisY();

		Scalar radius = bone->getRadius();
		d *= radius;
		a *= radius;
		b *= radius;

		m_primitiveRenderer->drawLine(start, start + d + a + b, color);
		m_primitiveRenderer->drawLine(start, start + d - a + b, color);
		m_primitiveRenderer->drawLine(start, start + d + a - b, color);
		m_primitiveRenderer->drawLine(start, start + d - a - b, color);

		m_primitiveRenderer->drawLine(start + d + a + b, end, color);
		m_primitiveRenderer->drawLine(start + d - a + b, end, color);
		m_primitiveRenderer->drawLine(start + d + a - b, end, color);
		m_primitiveRenderer->drawLine(start + d - a - b, end, color);

		m_primitiveRenderer->drawLine(start + d + a + b, start + d - a + b, color);
		m_primitiveRenderer->drawLine(start + d - a + b, start + d - a - b, color);
		m_primitiveRenderer->drawLine(start + d - a - b, start + d + a - b, color);
		m_primitiveRenderer->drawLine(start + d + a - b, start + d + a + b, color);

		m_primitiveRenderer->drawLine(start, end, color);

		if (drawAxis)
		{
			m_primitiveRenderer->drawLine(start, start + a * Scalar(2.0f), Color(255, 0, 0, color.a));
			m_primitiveRenderer->drawLine(start, start + b * Scalar(2.0f), Color(0, 255, 0, color.a));
		}
	}
}

void AnimationEditorPage::updateSettings()
{
	Ref< editor::PropertyGroup > colors = m_editor->getSettings()->getProperty< editor::PropertyGroup >(L"Editor.Colors");
	m_colorClear = colors->getProperty< editor::PropertyColor >(L"Background");
	m_colorGrid = colors->getProperty< editor::PropertyColor >(L"Grid");
	m_colorBone = colors->getProperty< editor::PropertyColor >(L"BoneWire");
	m_colorBoneSel = colors->getProperty< editor::PropertyColor >(L"BoneWireSelected");
	updateRenderWidgets();
}

void AnimationEditorPage::eventRenderButtonDown(ui::Event* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	if (!m_skeleton)
		return;

	m_editMode = (event->getKeyState() & ui::KsControl) != ui::KsControl;
	if (m_editMode)
		m_undoStack->push(m_animation);

	m_lastMousePosition = checked_type_cast< ui::MouseEvent* >(event)->getPosition();

	// Trace volume
	// @fixme Only orthogonal views work with picking.
	if (m_editMode && data->orthogonal)
	{
		render::Viewport viewport = data->renderView->getViewport();

		Vector4 position(
			2.0f * float(m_lastMousePosition.x) / viewport.width - 1.0f,
			1.0f - 2.0f * float(m_lastMousePosition.y) / viewport.height,
			0.0f/*c_nearZ*/,
			1.0f
		);
		int hit = data->picker->traceVolume(position);
		if (hit >= 0)
			m_selectedBone = hit;
	}

	int poseIndex;
	if (getSelectedPoseId(poseIndex))
		m_haveRelativeTwist = calculateRelativeTwist(poseIndex, m_selectedBone, m_relativeTwist);
	else
		m_haveRelativeTwist = false;

	updateRenderWidgets();

	renderWidget->setCapture();
	renderWidget->setFocus();
}

void AnimationEditorPage::eventRenderButtonUp(ui::Event* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	if (renderWidget->hasCapture())
		renderWidget->releaseCapture();

	m_haveRelativeTwist = false;
}

void AnimationEditorPage::eventRenderMouseMove(ui::Event* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (!m_skeleton || !renderWidget->hasCapture())
		return;

	ui::Point mousePosition = mouseEvent->getPosition();

	Vector2 mouseDelta(
		float(m_lastMousePosition.x - mousePosition.x),
		float(m_lastMousePosition.y - mousePosition.y)
	);

	mouseDelta /= 60.0f;

	if (m_editMode)
	{
		int poseIndex;
		if (getSelectedPoseId(poseIndex))
		{
			Pose& pose = m_animation->getKeyPose(poseIndex).pose;

			if ((mouseEvent->getKeyState() & ui::KsMenu) != ui::KsMenu)
			{
				if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
				{
					Quaternion orientation = pose.getBoneOrientation(m_selectedBone);
					orientation *= Quaternion(Vector4(0.0f, mouseDelta.x, 0.0f, 0.0f)) * Quaternion(Vector4(mouseDelta.y, 0.0f, 0.0f, 0.0f));
					pose.setBoneOrientation(m_selectedBone, orientation.normalized());

					// Compensate for applied twist.
					if (m_twistLock && m_haveRelativeTwist)
					{
						float relativeTwist;
						if (calculateRelativeTwist(poseIndex, m_selectedBone, relativeTwist))
						{
							Quaternion orientation = pose.getBoneOrientation(m_selectedBone);
							orientation *= Quaternion(Vector4(0.0f, 0.0f, m_relativeTwist - relativeTwist));
							pose.setBoneOrientation(m_selectedBone, orientation.normalized());
						}
					}
				}
				else
				{
					Quaternion orientation = pose.getBoneOrientation(m_selectedBone);
					orientation *= Quaternion(Vector4(0.0f, 0.0f, mouseDelta.x, 0.0f));
					pose.setBoneOrientation(m_selectedBone, orientation.normalized());
				}
			}
			else
			{
				Vector4 delta;
				if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
					delta = Vector4(mouseDelta.x, mouseDelta.y, 0.0f, 0.0f);
				else
					delta = Vector4(0.0f, 0.0f, mouseDelta.y, 0.0f);

				Vector4 offset = pose.getBoneOffset(m_selectedBone);
				offset += delta;
				pose.setBoneOffset(m_selectedBone, offset);
			}

			updateRenderWidgets();
		}
	}
	else
	{
		if (mouseEvent->getButton() == ui::MouseEvent::BtLeft)
		{
			data->cameraAngleX -= mouseDelta.x / 2.0f;
			data->cameraAngleY += mouseDelta.y / 2.0f;
		}
		else
		{
			if (data->orthogonal)
				data->cameraOffset += Vector4(0.0f, mouseDelta.y * 10.0f, 0.0f, 0.0f);
			else
				data->cameraOffset += Vector4(0.0f, 0.0f, -mouseDelta.y * 10.0f, 0.0f);

			data->cameraOffset += Vector4(-mouseDelta.x * 10.0f, 0.0f, 0.0f, 0.0f);
		}

		renderWidget->update();
	}

	m_lastMousePosition = mousePosition;
}

void AnimationEditorPage::eventRenderMouseWheel(ui::Event* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);

	if (data->orthogonal)
	{
		data->size += mouseEvent->getWheelRotation() * c_wheelRotationDelta;
		data->size = std::max(data->size, c_orthoSizeMin);
		renderWidget->update();
	}
}

void AnimationEditorPage::eventRenderSize(ui::Event* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	ui::SizeEvent* s = static_cast< ui::SizeEvent* >(event);
	ui::Size sz = s->getSize();

	data->renderView->resize(sz.cx, sz.cy);
	data->renderView->setViewport(render::Viewport(0, 0, sz.cx, sz.cy, 0, 1));
}

void AnimationEditorPage::eventRenderPaint(ui::Event* event)
{
	Ref< ui::Widget > renderWidget = checked_type_cast< ui::Widget* >(event->getSender());

	Ref< RenderWidgetData > data = renderWidget->getData< RenderWidgetData >(L"DATA");
	T_ASSERT (data);

	ui::PaintEvent* paintEvent = checked_type_cast< ui::PaintEvent* >(event);
	ui::Rect rc = renderWidget->getInnerRect();

	T_ASSERT (m_primitiveRenderer);

	if (!m_animation)
		return;

	if (!data->renderView->begin())
		return;

	float tmp[4];
	m_colorClear.getRGBA32F(tmp);

	data->renderView->clear(
		render::CfColor | render::CfDepth,
		tmp,
		1.0f,
		128
	);

	// Calculate transformation matrices.
	render::Viewport viewport = data->renderView->getViewport();

	Matrix44 projection;
	if (!data->orthogonal)
	{
		projection = perspectiveLh(
			deg2rad(c_fov),
			float(viewport.width) / viewport.height,
			c_nearZ,
			c_farZ
		);
	}
	else
	{
		float ratio = float(viewport.width) / viewport.height;
		projection = orthoLh(
			data->size * m_cameraSizeScale * ratio,
			data->size * m_cameraSizeScale,
			c_nearZ,
			c_farZ
		);
	}

	Matrix44 view = translate(data->cameraOffset * m_cameraOffsetScale) * rotateX(data->cameraAngleY) * rotateY(data->cameraAngleX);

	// Set transformation in picker.
	data->picker->setPerspectiveTransform(projection);
	data->picker->setViewTransform(view);

	// Begin rendering primitives.
	if (m_primitiveRenderer->begin(data->renderView))
	{
		m_primitiveRenderer->pushProjection(projection);
		m_primitiveRenderer->pushView(view);

		for (int x = -10; x <= 10; ++x)
		{
			m_primitiveRenderer->drawLine(
				Vector4(float(x), 0.0f, -10.0f, 1.0f),
				Vector4(float(x), 0.0f, 10.0f, 1.0f),
				m_colorGrid
			);
			m_primitiveRenderer->drawLine(
				Vector4(-10.0f, 0.0f, float(x), 1.0f),
				Vector4(10.0f, 0.0f, float(x), 1.0f),
				m_colorGrid
			);
		}

		m_primitiveRenderer->drawArrowHead(
			Vector4(11.0f, 0.0f, 0.0f, 1.0f),
			Vector4(13.0f, 0.0f, 0.0f, 1.0f),
			0.8f,
			Color(255, 0, 0)
		);
		m_primitiveRenderer->drawArrowHead(
			Vector4(0.0f, 0.0f, 11.0f, 1.0f),
			Vector4(0.0f, 0.0f, 13.0f, 1.0f),
			0.8f,
			Color(0, 0, 255)
		);

		if (m_skeleton)
		{
			float time = float(m_sequencer->getCursor() / 1000.0f);

			// Render ghost trail.
			if (m_showGhostTrail)
			{
				for (int i = 0; i < 10; ++i)
				{
					float ghostTime = time - (i + 1) * 0.1f;
					if (ghostTime < 0.0f)
						break;

					uint8_t alpha = 100 - i * 100 / 10;
					drawSkeleton(ghostTime, Color(0, 0, 0, alpha), Color(0, 0, 0, alpha), false);
				}
			}

			// Render current skeleton.
			drawSkeleton(
				time,
				m_colorBone,
				m_colorBoneSel,
				true
			);

			// Update picker volumes.
			Pose pose;
			m_animation->getPose(time, pose);

			AlignedVector< Transform > boneTransforms;
			calculatePoseTransforms(
				m_skeleton,
				&pose,
				boneTransforms
			);

			data->picker->removeAllVolumes();
			for (int i = 0; i < int(m_skeleton->getBoneCount()); ++i)
			{
				const Bone* bone = m_skeleton->getBone(i);

				Vector4 center(0.0f, 0.0f, bone->getLength() / 2.0f, 1.0f);
				Vector4 extent(bone->getRadius(), bone->getRadius(), bone->getLength() / 2.0f, 0.0f);

				Aabb boneVolume(
					center - extent,
					center + extent
				);

				data->picker->addVolume(boneTransforms[i].toMatrix44(), boneVolume, i);
			}
		}

		m_primitiveRenderer->end(data->renderView);
	}

	data->renderView->end();
	data->renderView->present();

	paintEvent->consume();
}

void AnimationEditorPage::eventSequencerButtonDown(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() != ui::MouseEvent::BtRight)
		return;

	Ref< ui::MenuItem > selectedItem = m_menuPopup->show(m_sequencer, mouseEvent->getPosition());
	if (selectedItem)
		handleCommand(selectedItem->getCommand());

	mouseEvent->consume();
}

void AnimationEditorPage::eventSequencerCursorMove(ui::Event* event)
{
	updateRenderWidgets();
}

void AnimationEditorPage::eventSequencerTimer(ui::Event* event)
{
	if (!m_previewAnimation || !m_animation)
		return;

	int end = int(m_animation->getLastKeyPose().at * 1000.0f);

	int cursor = 0;
	if (end > 0)
		cursor = (m_sequencer->getCursor() + 1000 / 30) % end;

	m_sequencer->setCursor(cursor);
	m_sequencer->update();

	updateRenderWidgets();
}

void AnimationEditorPage::eventToolClick(ui::Event* event)
{
	const ui::Command& command = checked_type_cast< ui::CommandEvent* >(event)->getCommand();
	handleCommand(command);
}

	}
}
