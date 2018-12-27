/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_Quantize_H
#define traktor_model_Quantize_H

#include "Model/IModelOperation.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace model
	{

/*! \brief
 * \ingroup Model
 */
class T_DLLCLASS Quantize : public IModelOperation
{
	T_RTTI_CLASS;

public:
	Quantize(float step);

	Quantize(const Vector4& step);

	virtual bool apply(Model& model) const override final;

private:
	Vector4 m_step;
};

	}
}

#endif	// traktor_model_Quantize_H
