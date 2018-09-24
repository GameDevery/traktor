/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_ShaderDependencyPane_H
#define traktor_render_ShaderDependencyPane_H

#include "Render/Editor/Shader/ShaderDependencyTracker.h"
#include "Ui/Container.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class ListBox;

	}

	namespace render
	{

class ShaderDependencyTracker;

class ShaderDependencyPane
:	public ui::Container
,	public ShaderDependencyTracker::IListener
{
	T_RTTI_CLASS;

public:
	ShaderDependencyPane(editor::IEditor* editor, const Guid& shaderId);

	virtual void destroy() T_OVERRIDE T_FINAL;

	bool create(ui::Widget* parent);

private:
	editor::IEditor* m_editor;
	Guid m_shaderId;
	Ref< ShaderDependencyTracker > m_dependencyTracker;
	Ref< ui::ListBox > m_refereeList;

	virtual void dependencyAdded(const Guid& fromShader, const Guid& toShader) T_OVERRIDE T_FINAL;

	virtual void dependencyRemoved(const Guid& fromShader, const Guid& toShader) T_OVERRIDE T_FINAL;

	void eventRefereeListDoubleClick(ui::MouseDoubleClickEvent* event);
};

	}
}

#endif	// traktor_render_ShaderDependencyPane_H
