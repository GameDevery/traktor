#ifndef traktor_render_ContextDx10_H
#define traktor_render_ContextDx10_H

#include <vector>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Misc/ComRef.h"
#include "Render/Dx10/Platform.h"

namespace traktor
{
	namespace render
	{

/*! \brief DX10 context.
 * \ingroup DX10
 */
class ContextDx10 : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Garbage delete callback.
	 *
	 * Context keeps a queue for deletion callbacks
	 * to ensure resources are deleted properly (not used, same thread etc).
	 */
	struct DeleteCallback
	{
		virtual ~DeleteCallback() {}

		virtual void deleteResource() = 0;
	};

	/*! \brief Release COM object. */
	struct ReleaseComObjectCallback : public DeleteCallback
	{
		ComRef< IUnknown > m_unk;

		ReleaseComObjectCallback(IUnknown* unk);

		virtual void deleteResource();
	};

	/*! \brief Release COM objects. */
	struct ReleaseComObjectArrayCallback : public DeleteCallback
	{
		ComRefArray< IUnknown > m_unks;

		ReleaseComObjectArrayCallback(IUnknown** unks, size_t count);

		virtual void deleteResource();
	};

	ContextDx10(
		ID3D10Device* d3dDevice,
		IDXGIFactory* dxgiFactory,
		IDXGIOutput* dxgiOutput
	);

	void deleteResource(DeleteCallback* callback);

	void deleteResources();

	ID3D10Device* getD3DDevice() { return m_d3dDevice; }

	IDXGIFactory* getDXGIFactory() { return m_dxgiFactory; }

	IDXGIOutput* getDXGIOutput() { return m_dxgiOutput; }

	template < typename InterfaceType >
	void releaseComRef(ComRef< InterfaceType >& unk)
	{
		if (unk)
		{
			deleteResource(new ReleaseComObjectCallback(unk));
			unk.release();
		}
	}

	template < typename InterfaceType >
	void releaseComRef(ComRefArray< InterfaceType >& unks)
	{
		if (!unks.empty())
		{
			deleteResource(new ReleaseComObjectArrayCallback((IUnknown**)unks.base(), unks.size()));
			unks.resize(0);
		}
	}

private:
	ComRef< ID3D10Device > m_d3dDevice;
	ComRef< IDXGIFactory > m_dxgiFactory;
	ComRef< IDXGIOutput > m_dxgiOutput;
	Semaphore m_deleteResourcesLock;
	std::vector< DeleteCallback* > m_deleteResources;
};

	}
}

#endif	// traktor_render_ContextDx10_H
