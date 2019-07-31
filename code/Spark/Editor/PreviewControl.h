#pragma once

#define T_USE_ACCELERATED_RENDERER 1

#include "Core/Timer/Timer.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace drawing
	{

class Image;

	}

	namespace editor
	{

class IEditor;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace render
	{

class IRenderSystem;
class IRenderView;
class RenderTargetSet;
class Shader;

	}

	namespace sound
	{

class ISoundPlayer;

	}

	namespace graphics
	{

class IGraphicsSystem;

	}

	namespace spark
	{

class AccDisplayRenderer;
class Movie;
class MoviePlayer;
class MovieRenderer;
class SoundRenderer;
class SwDisplayRenderer;

class T_DLLCLASS PreviewControl : public ui::Widget
{
	T_RTTI_CLASS;

public:
	PreviewControl(editor::IEditor* editor);

	bool create(
		ui::Widget* parent,
		int style,
		db::Database* database,
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		sound::ISoundPlayer* soundPlayer
	);

	virtual void destroy() override final;

	void setMovie(Movie* movie);

	void rewind();

	void play();

	void stop();

	void forward();

	bool playing() const;

	virtual ui::Size getPreferedSize() const override final;

	MoviePlayer* getMoviePlayer() const { return m_moviePlayer; }

private:
	editor::IEditor* m_editor;
	Ref< ui::EventSubject::IEventHandler > m_idleEventHandler;
	Ref< db::Database > m_database;
#if T_USE_ACCELERATED_RENDERER
	Ref< render::IRenderView > m_renderView;
	Ref< AccDisplayRenderer > m_displayRenderer;
#else
	Ref< graphics::IGraphicsSystem > m_graphicsSystem;
	Ref< drawing::Image > m_image;
	Ref< SwDisplayRenderer > m_displayRenderer;
#endif
	Ref< SoundRenderer > m_soundRenderer;
	Ref< MovieRenderer > m_movieRenderer;
	Ref< MoviePlayer > m_moviePlayer;
	Ref< Movie > m_movie;
	Timer m_timer;
	bool m_playing;

	ui::Point getTwips(const ui::Point& pt) const;

	void eventSize(ui::SizeEvent* event);

	void eventPaint(ui::PaintEvent* event);

	void eventIdle(ui::IdleEvent* event);

	void eventKey(ui::KeyEvent* event);

	void eventKeyDown(ui::KeyDownEvent* event);

	void eventKeyUp(ui::KeyUpEvent* event);

	void eventButtonDown(ui::MouseButtonDownEvent* event);

	void eventButtonUp(ui::MouseButtonUpEvent* event);

	void eventMouseMove(ui::MouseMoveEvent* event);

	void eventMouseWheel(ui::MouseWheelEvent* event);
};

	}
}

