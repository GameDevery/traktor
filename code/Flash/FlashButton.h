#ifndef traktor_flash_FlashButton_H
#define traktor_flash_FlashButton_H

#include "Flash/FlashCharacter.h"
#include "Core/Math/Matrix33.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionScript;

/*! \brief Flash button character.
 * \ingroup Flash
 */
class T_DLLCLASS FlashButton : public FlashCharacter
{
	T_RTTI_CLASS(FlashButton)

public:
	enum ConditionMasks
	{
		CmIdleToOverDown = 1 << 0,
		CmOutDownToIdle = 1 << 1,
		CmOutDownToOverDown = 1 << 2,
		CmOverDownToOutDown = 1 << 3,
		CmOverDownToOverUp = 1 << 4,
		CmOverUpToOverDown = 1 << 5,
		CmOverUpToIdle = 1 << 6,
		CmIdleToOverUp = 1 << 7,
		CmOverDownToIdle = 1 << 8
	};

	enum StateMasks
	{
		SmHitTest = 1 << 0,
		SmDown = 1 << 1,
		SmOver = 1 << 2,
		SmUp = 1 << 3
	};

	struct ButtonLayer
	{
		uint8_t state;
		uint16_t characterId;
		uint16_t placeDepth;
		Matrix33 placeMatrix;
		SwfCxTransform cxform;

		ButtonLayer()
		:	state(0)
		,	characterId(0)
		,	placeDepth(0)
		,	placeMatrix(Matrix33::identity())
		{
			cxform.red[0]   = 1.0f; cxform.red[1]   = 0.0f;
			cxform.green[0] = 1.0f; cxform.green[1] = 0.0f;
			cxform.blue[0]  = 1.0f; cxform.blue[1]  = 0.0f;
			cxform.alpha[0] = 1.0f; cxform.alpha[1] = 0.0f;
		}
	};

	struct ButtonCondition
	{
		uint8_t key;
		uint16_t mask;
		Ref< ActionScript > script;

		ButtonCondition()
		:	key(0)
		,	mask(0)
		{
		}
	};

	typedef AlignedVector< ButtonLayer > button_layers_t;
	typedef std::vector< ButtonCondition > button_conditions_t;

	FlashButton(uint16_t id);

	/*! \brief Add button layer.
	 *
	 * \param layer Button layer description.
	 */
	void addButtonLayer(const ButtonLayer& layer);

	/*! \brief Get button layers.
	 *
	 * \return Button layers.
	 */
	const button_layers_t& getButtonLayers() const;

	/*! \brief Add button condition script.
	 *
	 * \param condition Condition script.
	 */
	void addButtonCondition(const ButtonCondition& condition);

	/*! \brief Get button condition scripts.
	 *
	 * \return Condition scripts.
	 */
	const button_conditions_t& getButtonConditions() const;

	virtual FlashCharacterInstance* createInstance(ActionContext* context, FlashCharacterInstance* parent) const;

private:
	button_layers_t m_layers;
	button_conditions_t m_conditions;
};

	}
}

#endif	// traktor_flash_FlashButton_H
