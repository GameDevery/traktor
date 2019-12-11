#pragma once

#include "Input/InputTypes.h"
#include "Input/Binding/IInputSourceData.h"

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

/*! Control input source.
 *
 * A control input source allows the graph
 * to query attributes of an input device such
 * as if it's connected etc.
 *
 * \ingroup Input
 */
class T_DLLCLASS ControlInputSourceData : public IInputSourceData
{
	T_RTTI_CLASS;

public:
	enum ControlQuery
	{
		CqMatchingDevice,
		CqConnectedDevice
	};

	ControlInputSourceData();

	ControlInputSourceData(
		InputCategory category,
		ControlQuery controlQuery
	);

	ControlInputSourceData(
		InputCategory category,
		ControlQuery controlQuery,
		int32_t index
	);

	void setCategory(InputCategory category);

	InputCategory getCategory() const;

	void setControlQuery(ControlQuery controlQuery);

	ControlQuery getControlQuery() const;

	void setIndex(int32_t index);

	int32_t getIndex() const;

	virtual Ref< IInputSource > createInstance(DeviceControlManager* deviceControlManager) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	InputCategory m_category;
	ControlQuery m_controlQuery;
	int32_t m_index;
};

	}
}

