#ifndef traktor_drawing_Image_H
#define traktor_drawing_Image_H

#include "Core/Heap/Ref.h"
#include "Core/Io/Path.h"
#include "Drawing/Color.h"
#include "Drawing/ImageInfo.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{
	
class PixelFormat;
class Palette;
class ImageFilter;

/*! \brief Image class.
 * \ingroup Drawing
 *
 * Support for basic drawing operations as well as loading and saving
 * to some standard formats such as PNG and JPEG.
 * As the main purpose of this class is to hold image data of various
 * internal formats. The drawing operations are very slow as each pixel
 * gets converted into the current pixel format before it's actually
 * painted onto the image.
 */
class T_DLLCLASS Image : public Object
{
	T_RTTI_CLASS(Image)

public:
	Image();

	Image(const Image& src);

	Image(const PixelFormat* pixelFormat, uint32_t width, uint32_t height, Palette* palette = 0);
	
	virtual ~Image();
	
	/*! \brief Create a clone of the image.
	 *
	 * \param includeData Clone image pixels.
	 */
	Ref< Image > clone(bool includeData = true) const;

	/*! \brief Copy sub-rectangle of a source image into this image.
	 *
	 * \param src Source image.
	 * \param x Destination left coordinate.
	 * \param y Destination top coordinate.
	 * \param width Sub rectangle width.
	 * \param height Sub rectangle height.
	 */
	void copy(const Image* src, int32_t x, int32_t y, int32_t width, int32_t height);
	
	/*! \brief Clear entire image. */
	void clear(const Color& color);

	/*! \brief Get single pixel. */
	bool getPixel(int32_t x, int32_t y, Color& color) const;

	/*! \brief Set single pixel. */
	bool setPixel(int32_t x, int32_t y, const Color& color);
	
	/*! \brief Apply filter on entire image. */
	Ref< Image > applyFilter(ImageFilter* imageFilter) const;

	/*! \brief Convert format of image. */
	void convert(const PixelFormat* intoPixelFormat, Palette* intoPalette = 0);

	/*! \brief Load image from file. */
	static Ref< Image > load(const Path& fileName);

	/*! \brief Load image from resource. */
	static Ref< Image > load(const void* resource, uint32_t size, const std::wstring& extension);

	/*! \brief Save image. */
	bool save(const Path& fileName);
	
	/*! \brief Get current image format. */
	Ref< const PixelFormat > getPixelFormat() const;
	
	/*! \brief Get width of image. */
	const int32_t getWidth() const;

	/*! \brief Get height of image. */
	const int32_t getHeight() const;

	/*! \brief Get image palette. */
	Ref< Palette > getPalette() const;

	/*! \brief Get read-only pointer to image data. */
	const void* getData() const;

	/*! \brief Get pointer to image data. */
	void* getData();

	/*! \brief Get size of data in bytes. */
	uint32_t getDataSize() const;

	/*! \brief Attach image meta data. */
	void setImageInfo(ImageInfo* imageInfo);

	/*! \brief Get image meta data. */
	Ref< ImageInfo > getImageInfo() const;

	/*! \brief Copy image. */
	Image& operator = (const Image& src);
	
private:
	Ref< const PixelFormat > m_pixelFormat;
	int32_t m_width;
	int32_t m_height;
	Ref< Palette > m_palette;
	size_t m_size;
	uint8_t* m_data;
	Ref< ImageInfo > m_imageInfo;
};
	
	}
}

#endif	// traktor_drawing_Image_H
