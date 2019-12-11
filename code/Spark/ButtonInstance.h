#pragma once

#include <map>
#include "Spark/CharacterInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spark
	{

class Button;
class Movie;

/*! Flash button instance.
 * \ingroup Spark
 */
class T_DLLCLASS ButtonInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	ButtonInstance(ActionContext* context, Dictionary* dictionary, CharacterInstance* parent, const Button* button);

	virtual ~ButtonInstance();

	virtual void destroy() override;

	/*! Get button.
	 *
	 * \return Pointer to button.
	 */
	const Button* getButton() const;

	/*! Get button state.
	 *
	 * \return Button state, combination of Button::StateMasks.
	 */
	uint8_t getState() const;

	/*! Get button local bounds.
	 *
	 * \return Button bounds in local space.
	 */
	Aabb2 getLocalBounds() const;

	/*! Get button character.
	 *
	 * \param referenceId Character identity.
	 * \return Character instance.
	 */
	CharacterInstance* getCharacterInstance(uint16_t referenceId) const;

	virtual void eventMouseDown(int x, int y, int button) override final;

	virtual void eventMouseUp(int x, int y, int button) override final;

	virtual void eventMouseMove(int x, int y, int button) override final;

	virtual Aabb2 getBounds() const override final;

protected:
	virtual void trace(visitor_t visitor) const override final;

	virtual void dereference() override final;

private:
	Ref< const Button > m_button;
	std::map< uint16_t, Ref< CharacterInstance > > m_characterInstances;
	uint8_t m_state;
	bool m_inside;
	bool m_pushed;

	void executeCondition(uint32_t conditionMask);

	void executeScriptEvent(const std::string& eventName);
};

	}
}

