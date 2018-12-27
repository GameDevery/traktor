/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputMappingStateData_H
#define traktor_input_InputMappingStateData_H

#include <map>
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{
	
class InputStateData;

/*! \brief Input mapping state data
 * \ingroup Input
 *
 * Serializable description of input states.
 */
class T_DLLCLASS InputMappingStateData : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setStateData(const std::wstring& id, InputStateData* data);
	
	const std::map< std::wstring, Ref< InputStateData > >& getStateData() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::map< std::wstring, Ref< InputStateData > > m_stateData;
};

	}
}

#endif	// traktor_input_InputMappingStateData_H
