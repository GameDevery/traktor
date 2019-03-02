#pragma once

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace render
	{

class ImageProcessStep;

class IImageProcessStepFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(editor::IEditor* editor, const ImageProcessStep* step) const = 0;

	virtual std::wstring getText(editor::IEditor* editor, const ImageProcessStep* step) const = 0;

	virtual void getSources(const ImageProcessStep* step, std::vector< std::wstring >& outSources) const = 0;

	virtual bool canHaveChildren() const = 0;

	virtual bool addChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const = 0;

	virtual bool removeChild(ImageProcessStep* parentStep, ImageProcessStep* childStep) const = 0;

	virtual bool getChildren(const ImageProcessStep* step, RefArray< ImageProcessStep >& outChildren) const = 0;
};

	}
}
