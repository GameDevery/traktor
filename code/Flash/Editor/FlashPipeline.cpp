#include <cstring>
#include <list>
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineSettings.h"
#include "Flash/BitmapImage.h"
#include "Flash/BitmapResource.h"
#include "Flash/Font.h"
#include "Flash/Frame.h"
#include "Flash/Movie.h"
#include "Flash/MovieFactory.h"
#include "Flash/Optimizer.h"
#include "Flash/Packer.h"
#include "Flash/Shape.h"
#include "Flash/Sprite.h"
#include "Flash/SwfReader.h"
#include "Flash/Editor/FlashEmptyMovieAsset.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Flash/Editor/FlashPipeline.h"
#include "Render/Shader.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Resource/Id.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const Guid c_idFlashShaderAssets(L"{14D6A2DB-796D-E54D-9D70-73DE4AE7C4E8}");

struct AtlasBitmap
{
	uint16_t id;
	Ref< const BitmapImage > bitmap;
	Packer::Rectangle packedRect;
};

struct AtlasBucket
{
	Ref< Packer > packer;
	std::list< AtlasBitmap > bitmaps;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashPipeline", 54, FlashPipeline, editor::IPipeline)

FlashPipeline::FlashPipeline()
:	m_generateMips(false)
,	m_sharpenStrength(0.0f)
,	m_useTextureCompression(true)
,	m_textureSizeDenom(1)
,	m_textureAtlasSize(1024)
{
}

bool FlashPipeline::create(const editor::IPipelineSettings* settings)
{
	m_assetPath = settings->getProperty< std::wstring >(L"Pipeline.AssetPath", L"");
	m_generateMips = settings->getProperty< bool >(L"FlashPipeline.GenerateMips", false);
	m_sharpenStrength = settings->getProperty< bool >(L"FlashPipeline.SharpenStrength", false);
	m_useTextureCompression = settings->getProperty< bool >(L"FlashPipeline.UseTextureCompression", true);
	m_textureSizeDenom = settings->getProperty< int32_t >(L"FlashPipeline.TextureSizeDenom", 1);
	m_textureAtlasSize = settings->getProperty< int32_t >(L"FlashPipeline.TextureAtlasSize", 1024);
	return true;
}

void FlashPipeline::destroy()
{
}

TypeInfoSet FlashPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FlashEmptyMovieAsset >());
	typeSet.insert(&type_of< FlashMovieAsset >());
	return typeSet;
}

bool FlashPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid
) const
{
	if (const FlashMovieAsset* movieAsset = dynamic_type_cast< const FlashMovieAsset* >(sourceAsset))
		pipelineDepends->addDependency(traktor::Path(m_assetPath), movieAsset->getFileName().getOriginal());
	pipelineDepends->addDependency(c_idFlashShaderAssets, editor::PdfBuild | editor::PdfResource);	// Solid
	return true;
}

bool FlashPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const editor::IPipelineDependencySet* dependencySet,
	const editor::PipelineDependency* dependency,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	const Object* buildParams,
	uint32_t reason
) const
{
	Ref< Movie > movie;
	bool optimize = false;

	if (const FlashMovieAsset* movieAsset = dynamic_type_cast< const FlashMovieAsset* >(sourceAsset))
	{
		Ref< IStream > sourceStream = pipelineBuilder->openFile(traktor::Path(m_assetPath), movieAsset->getFileName().getOriginal());
		if (!sourceStream)
		{
			log::error << L"Failed to import Flash; unable to open file \"" << movieAsset->getFileName().getOriginal() << L"\"" << Endl;
			return false;
		}

		// Try to load image and embedd into a movie first, if extension
		// not supported then this fail quickly.
		Ref< drawing::Image > image = drawing::Image::load(sourceStream, movieAsset->getFileName().getExtension());
		if (image)
		{
			// Create a single frame and place shape.
			Ref< Frame > frame = new Frame();

			Frame::PlaceObject p;
			p.hasFlags = Frame::PfHasCharacterId;
			p.depth = 1;
			p.characterId = 1;
			frame->placeObject(p);

			// Create sprite and add frame.
			Ref< Sprite > sprite = new Sprite();
			sprite->addFrame(frame);

			// Create quad shape and fill with bitmap.
			Ref< Shape > shape = new Shape();
			shape->create(1, image->getWidth() * 20, image->getHeight() * 20);

			// Setup dictionary.
			movie = new Movie(Aabb2(Vector2(0.0f, 0.0f), Vector2(image->getWidth() * 20.0f, image->getHeight() * 20.0f)), sprite);
			movie->defineBitmap(1, new BitmapImage(image));
			movie->defineCharacter(1, shape);
		}
		else
		{
			Ref< SwfReader > swf = new SwfReader(sourceStream);
			movie = MovieFactory(movieAsset->m_includeAS).createMovie(swf);
			if (!movie)
			{
				log::error << L"Failed to import Flash; unable to parse SWF" << Endl;
				return false;
			}
		}

		safeClose(sourceStream);

		optimize = movieAsset->m_staticMovie;
	}
	else if (const FlashEmptyMovieAsset* emptyMovieAsset = dynamic_type_cast< const FlashEmptyMovieAsset* >(sourceAsset))
	{
		const Color4ub& bc = emptyMovieAsset->getBackgroundColor();

		Ref< Sprite > sprite = new Sprite(0, emptyMovieAsset->getFrameRate());

		Ref< Frame > frame = new Frame();
		frame->changeBackgroundColor(Color4f(bc.r, bc.g, bc.b, bc.a) / Scalar(255.0f));
		sprite->addFrame(frame);

		movie = new Movie(
			Aabb2(
				Vector2(0.0f, 0.0f),
				Vector2(emptyMovieAsset->getStageWidth() * 20.0f, emptyMovieAsset->getStageHeight() * 20.0f)
			),
			sprite
		);
	}

	// Show some information about the Flash.
	log::info << L"SWF successfully loaded," << Endl;
	log::info << IncreaseIndent;
	log::info << movie->getFonts().size() << L" font(s)" << Endl;
	log::info << movie->getBitmaps().size() << L" bitmap(s)" << Endl;
	log::info << movie->getSounds().size() << L" sound(s)" << Endl;
	log::info << movie->getCharacters().size() << L" character(s)" << Endl;
	log::info << DecreaseIndent;

	// Merge all characters of first frame into a single sprite.
	if (optimize)
	{
		movie = Optimizer().merge(movie);
		if (!movie)
		{
			log::error << L"Failed to import Flash; failed to optimize static SWF" << Endl;
			return false;
		}
	}

	// Generate triangles of every shape in movie.
	Optimizer().triangulate(movie, false);

	// Replace all bitmaps with resource references to textures.
	SmallMap< uint16_t, Ref< Bitmap > > bitmaps = movie->getBitmaps();

	// Create atlas buckets of small bitmaps.
	std::list< AtlasBucket > buckets;
	std::list< AtlasBitmap > standalone;
	Packer::Rectangle r;

	for (SmallMap< uint16_t, Ref< Bitmap > >::const_iterator i = bitmaps.begin(); i != bitmaps.end(); ++i)
	{
		const BitmapImage* bitmapData = dynamic_type_cast< const BitmapImage* >(i->second);
		if (!bitmapData)
		{
			log::warning << L"Skipped bitmap as it not a static bitmap (" << type_name(i->second) << L")" << Endl;
			continue;
		}

		bool foundBucket = false;

		for (std::list< AtlasBucket >::iterator j = buckets.begin(); j != buckets.end(); ++j)
		{
			if (j->packer->insert(bitmapData->getWidth() + 2, bitmapData->getHeight() + 2, r))
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = r;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				j->bitmaps.push_back(ab);
				foundBucket = true;
				break;
			}
		}

		if (!foundBucket)
		{
			buckets.push_back(AtlasBucket());

			AtlasBucket& b = buckets.back();
			b.packer = new Packer(m_textureAtlasSize, m_textureAtlasSize);

			if (b.packer->insert(bitmapData->getWidth() + 2, bitmapData->getHeight() + 2, r))
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect = r;
				ab.packedRect.x += 1;
				ab.packedRect.y += 1;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				b.bitmaps.push_back(ab);
			}
			else
			{
				AtlasBitmap ab;
				ab.id = i->first;
				ab.bitmap = bitmapData;
				ab.packedRect.x = 0;
				ab.packedRect.y = 0;
				ab.packedRect.width = bitmapData->getWidth();
				ab.packedRect.height = bitmapData->getHeight();
				standalone.push_back(ab);
			}
		}
	}

	log::info << L"Packed bitmaps into " << uint32_t(buckets.size()) << L" atlas(es)." << Endl;

	uint32_t count = 1;

	for (std::list< AtlasBucket >::const_iterator i = buckets.begin(); i != buckets.end(); ++i)
	{
		log::info << L"Atlas " << count << L", containing " << uint32_t(i->bitmaps.size()) << L" bitmaps." << Endl;

		if (i->bitmaps.size() > 1)
		{
			Ref< drawing::Image > atlasImage = new drawing::Image(
				drawing::PixelFormat::getA8B8G8R8(),
				m_textureAtlasSize,
				m_textureAtlasSize
			);

			atlasImage->clear(Color4f(0.0f, 0.0f, 0.0f, 0.0f));

			for (std::list< AtlasBitmap >::const_iterator j = i->bitmaps.begin(); j != i->bitmaps.end(); ++j)
			{
				Ref< drawing::Image > bitmapImage = new drawing::Image(
					drawing::PixelFormat::getA8B8G8R8(),
					j->bitmap->getWidth(),
					j->bitmap->getHeight()
				);

				std::memcpy(
					bitmapImage->getData(),
					j->bitmap->getBits(),
					j->bitmap->getWidth() * j->bitmap->getHeight() * 4
				);

				for (int32_t y = -1; y < j->packedRect.height + 1; ++y)
				{
					for (int32_t x = -1; x < j->packedRect.width + 1; ++x)
					{
						int32_t sx = x;
						int32_t sy = y;

						if (sx < 0)
							sx = j->packedRect.width - 1;
						else if (sx > j->packedRect.width - 1)
							sx = 0;

						if (sy < 0)
							sy = j->packedRect.height - 1;
						else if (sy > j->packedRect.height - 1)
							sy = 0;

						Color4f tmp;
						bitmapImage->getPixel(sx, sy, tmp);

						atlasImage->setPixel(j->packedRect.x + x, j->packedRect.y + y, tmp);
					}
				}
			}

#if defined(_DEBUG)
			atlasImage->save(L"FlashBitmapAtlas" + toString(count) + L".png");
#endif

			Guid bitmapOutputGuid = outputGuid.permutate(count++);

			Ref< render::TextureOutput > output = new render::TextureOutput();
			output->m_textureFormat = render::TfInvalid;
			output->m_generateNormalMap = false;
			output->m_scaleDepth = 0.0f;
			output->m_generateMips = m_generateMips;
			output->m_keepZeroAlpha = false;
			output->m_textureType = render::Tt2D;
			output->m_hasAlpha = false;
			output->m_ignoreAlpha = false;
			output->m_scaleImage = false;
			output->m_scaleWidth = 0;
			output->m_scaleHeight = 0;
			output->m_enableCompression = m_useTextureCompression;
			output->m_enableNormalMapCompression = false;
			output->m_inverseNormalMapY = false;
			output->m_linearGamma = true;
			output->m_generateSphereMap = false;
			output->m_preserveAlphaCoverage = false;
			output->m_alphaCoverageReference = 0.0f;
			output->m_sharpenRadius = m_sharpenStrength > 0.0f ? 5 : 0;
			output->m_sharpenStrength = m_sharpenStrength;
			output->m_systemTexture = true;

			if (m_textureSizeDenom > 1)
			{
				output->m_scaleImage = true;
				output->m_scaleWidth = atlasImage->getWidth() / m_textureSizeDenom;
				output->m_scaleHeight = atlasImage->getHeight() / m_textureSizeDenom;
			}

			std::wstring bitmapOutputPath = traktor::Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
			if (!pipelineBuilder->buildOutput(
				output,
				bitmapOutputPath,
				bitmapOutputGuid,
				atlasImage
			))
				return false;

			for (std::list< AtlasBitmap >::const_iterator j = i->bitmaps.begin(); j != i->bitmaps.end(); ++j)
			{
				movie->defineBitmap(j->id, new BitmapResource(
					j->packedRect.x,
					j->packedRect.y,
					j->packedRect.width,
					j->packedRect.height,
					m_textureAtlasSize,
					m_textureAtlasSize,
					bitmapOutputGuid
				));
			}
		}
		else if (i->bitmaps.size() == 1)
		{
			AtlasBitmap ab = i->bitmaps.front();
			ab.packedRect.x = 0;
			ab.packedRect.y = 0;
			ab.packedRect.width = ab.bitmap->getWidth();
			ab.packedRect.height = ab.bitmap->getHeight();
			standalone.push_back(ab);
		}
	}

	log::info << uint32_t(standalone.size()) << L" bitmap(s) didn't fit in any atlas..." << Endl;

	for (std::list< AtlasBitmap >::const_iterator i = standalone.begin(); i != standalone.end(); ++i)
	{
		Ref< drawing::Image > bitmapImage = new drawing::Image(
			drawing::PixelFormat::getA8B8G8R8(),
			i->bitmap->getWidth(),
			i->bitmap->getHeight()
		);

		std::memcpy(
			bitmapImage->getData(),
			i->bitmap->getBits(),
			i->bitmap->getWidth() * i->bitmap->getHeight() * 4
		);

#if defined(_DEBUG)
		bitmapImage->save(L"FlashBitmap" + toString(count) + L".png");
#endif

		Guid bitmapOutputGuid = outputGuid.permutate(count++);

		Ref< render::TextureOutput > output = new render::TextureOutput();
		output->m_textureFormat = render::TfInvalid;
		output->m_generateNormalMap = false;
		output->m_scaleDepth = 0.0f;
		output->m_generateMips = m_generateMips;
		output->m_keepZeroAlpha = false;
		output->m_textureType = render::Tt2D;
		output->m_hasAlpha = false;
		output->m_ignoreAlpha = false;
		output->m_scaleImage = false;
		output->m_scaleWidth = 0;
		output->m_scaleHeight = 0;
		output->m_enableCompression = m_useTextureCompression;
		output->m_enableNormalMapCompression = false;
		output->m_inverseNormalMapY = false;
		output->m_linearGamma = true;
		output->m_generateSphereMap = false;
		output->m_preserveAlphaCoverage = false;
		output->m_alphaCoverageReference = 0.0f;
		output->m_sharpenRadius = m_sharpenStrength > 0.0f ? 5 : 0;
		output->m_sharpenStrength = m_sharpenStrength;
		output->m_systemTexture = true;

		if (m_textureSizeDenom > 1)
		{
			output->m_scaleImage = true;
			output->m_scaleWidth = bitmapImage->getWidth() / m_textureSizeDenom;
			output->m_scaleHeight = bitmapImage->getHeight() / m_textureSizeDenom;
		}

		std::wstring bitmapOutputPath = traktor::Path(outputPath).getPathOnly() + L"/Textures/" + bitmapOutputGuid.format();
		if (!pipelineBuilder->buildOutput(
			output,
			bitmapOutputPath,
			bitmapOutputGuid,
			bitmapImage
		))
			return false;

		movie->defineBitmap(i->id, new BitmapResource(
			0,
			0,
			bitmapImage->getWidth(),
			bitmapImage->getHeight(),
			bitmapImage->getWidth(),
			bitmapImage->getHeight(),
			bitmapOutputGuid
		));
	}

	Ref< db::Instance > instance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!instance)
	{
		log::error << L"Failed to import Flash; unable to create instance" << Endl;
		return false;
	}

	instance->setObject(movie);

	if (!instance->commit())
	{
		log::info << L"Failed to import Flash; unable to commit instance" << Endl;
		return false;
	}

	return true;
}

Ref< ISerializable > FlashPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
