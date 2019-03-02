#pragma once

#include <list>
#include <map>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionObject;
class ActionFunction;
class ActionValue;
class AsKey;
class AsMouse;
class AsStage;
class As_flash_external_ExternalInterface;
class IActionVM;
class ICharacterFactory;
class IMovieLoader;
class ISoundRenderer;
class Movie;
class MovieDebugger;
class MovieRenderer;
class SpriteInstance;

struct CallArgs;
struct IExternalCall;

/*! \brief Flash movie player.
 * \ingroup Flash
 *
 * Flash movie player is the main class used by applications
 * in order to properly play SWF movies.
 */
class T_DLLCLASS MoviePlayer : public Object
{
	T_RTTI_CLASS;

public:
	MoviePlayer(
		const ICharacterFactory* characterFactory,
		const IMovieLoader* movieLoader,
		const MovieDebugger* movieDebugger
	);

	virtual ~MoviePlayer();

	/*! \brief Create movie player.
	 *
	 * \param movie Root movie.
	 * \param width Output render width (in pixels).
	 * \param height Output render height (in pixels).
	 * \param soundRenderer Optional implementation of sound renderer provided to AS Sound.
	 * \return True if created successfully.
	 */
	bool create(Movie* movie, int32_t width, int32_t height, ISoundRenderer* soundRenderer);

	/*! \brief Destroy resources used by movie player. */
	void destroy();

	/*! \brief Goto frame and continue playing.
	 *
	 * \param frame Frame number.
	 */
	void gotoAndPlay(uint32_t frame);

	/*! \brief Goto frame and stop playing.
	 *
	 * \param frame Frame number.
	 */
	void gotoAndStop(uint32_t frame);

	/*! \brief Goto frame and continue playing.
	 *
	 * \param frameLabel Frame label.
	 * \return False if label not found.
	 */
	bool gotoAndPlay(const std::string& frameLabel);

	/*! \brief Goto frame and stop playing.
	 *
	 * \param frameLabel Frame label.
	 * \return False if label not found.
	 */
	bool gotoAndStop(const std::string& frameLabel);

	/*! \brief Get number of frames in root movie.
	 *
	 * \return Number of frames.
	 */
	uint32_t getFrameCount() const;

	/*! \brief Render frame. */
	void render(MovieRenderer* movieRenderer) const;

	/*! \brief Execute events in current frame. */
	void execute(ISoundRenderer* soundRenderer);

	/*! \brief Progress until next frame.
	 *
	 * \param deltaTime Time in seconds to progress movie.
	 * \return True if new frame.
	 */
	bool progress(float deltaTime, ISoundRenderer* soundRenderer);

	/*! \brief Post key event.
	 *
	 * \param unicode Unicode character.
	 */
	void postKey(wchar_t unicode);

	/*! \brief Post key down event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyDown(int32_t keyCode);

	/*! \brief Post key up event.
	 *
	 * \param keyCode Key code.
	 */
	void postKeyUp(int32_t keyCode);

	/*! \brief Post mouse button down event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseDown(int32_t x, int32_t y, int32_t button);

	/*! \brief Post mouse button up event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseUp(int32_t x, int32_t y, int32_t button);

	/*! \brief Post mouse move event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param button Mouse buttons being held down.
	 */
	void postMouseMove(int32_t x, int32_t y, int32_t button);

	/*! \brief Post mouse wheel event.
	 *
	 * \param x Mouse cursor X coordinate.
	 * \param y Mouse cursor Y coordinate.
	 * \param delta Wheel delta motion
	 */
	void postMouseWheel(int32_t x, int32_t y, int32_t delta);

	/*! \brief Post view resize event.
	 *
	 * \param width New view width (in pixels).
	 * \param height New view height (in pixels).
	 */
	void postViewResize(int32_t width, int32_t height);

	/*! \brief Pop FS command from queue.
	 *
	 * \param outCommand Output FS command.
	 * \param outArgs Output FS arguments.
	 * \return True any command was popped.
	 */
	bool getFsCommand(std::string& outCommand, std::string& outArgs);

	/*! \brief Set external call interface.
	 */
	void setExternalCall(IExternalCall* externalCall);

	/*! \brief Dispatch callback through ExternalInterface.
	 */
	ActionValue dispatchCallback(const std::string& methodName, int32_t argc, const ActionValue* argv);

	/*! \brief Get root movie instance.
	 *
	 * \return Root movie instance.
	 */
	SpriteInstance* getMovieInstance() const;

	/*! \brief Get action virtual machine.
	 *
	 * \return action virtual machine.
	 */
	const IActionVM* getVM() const;

	/*! \brief Set global ActionScript value.
	 *
	 * \param name Name of global member.
	 * \param value Value of global member.
	 */
	void setGlobal(const std::string& name, const ActionValue& value);

	/*! \brief Get global ActionScript value.
	 *
	 * \param name Name of global member.
	 * \return Value of global member; undefined if no such member.
	 */
	ActionValue getGlobal(const std::string& name) const;

	/*! \brief Set GC enable/disable.
	 */
	void setGCEnable(bool gcEnable);

private:
	struct Event
	{
		uint32_t eventType;
		union
		{
			wchar_t unicode;
			int32_t keyCode;
			struct
			{
				int32_t x;
				int32_t y;
				union
				{
					int32_t button;
					int32_t delta;
				};
			}
			mouse;
			struct
			{
				int32_t width;
				int32_t height;
			}
			view;
		};
	};

	struct Interval
	{
		uint32_t count;
		uint32_t interval;
		Ref< ActionObject > target;
		Ref< ActionFunction > function;
	};

	Ref< const ICharacterFactory > m_characterFactory;
	Ref< const IMovieLoader > m_movieLoader;
	Ref< const MovieDebugger > m_movieDebugger;
	Ref< IActionVM > m_actionVM;
	Ref< AsKey > m_key;
	Ref< AsMouse > m_mouse;
	Ref< AsStage > m_stage;
	Ref< As_flash_external_ExternalInterface > m_externalInterface;
	Ref< Movie > m_movie;
	Ref< SpriteInstance > m_movieInstance;
	AlignedVector< Event > m_events;
	std::list< std::pair< std::string, std::string > > m_fsCommands;
	std::map< uint32_t, Interval > m_interval;
	uint32_t m_intervalNextId;
	float m_timeCurrent;
	float m_timeNext;
	float m_timeNextFrame;
	bool m_gcEnable;
	int32_t m_framesUntilCollection;

	void Global_getURL(CallArgs& ca);

	void Global_setInterval(CallArgs& ca);

	void Global_clearInterval(CallArgs& ca);
};

	}
}

