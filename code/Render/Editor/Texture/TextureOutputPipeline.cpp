#include <cstring>
#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Core/Functor/Functor.h"
#include "Core/Io/BufferedStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Log2.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Thread/Job.h"
#include "Core/Thread/JobManager.h"
#include "Database/Instance.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Drawing/Filters/ChainFilter.h"
#include "Drawing/Filters/GammaFilter.h"
#include "Drawing/Filters/MirrorFilter.h"
#include "Drawing/Filters/NormalizeFilter.h"
#include "Drawing/Filters/NormalMapFilter.h"
#include "Drawing/Filters/ScaleFilter.h"
#include "Drawing/Filters/SwizzleFilter.h"
#include "Drawing/Filters/TransformFilter.h"
#include "Editor/IPipelineBuilder.h"
#include "Editor/IPipelineDepends.h"
#include "Editor/IPipelineReport.h"
#include "Editor/IPipelineSettings.h"
#include "Render/Types.h"
#include "Render/Editor/Texture/DxtnCompressor.h"
#include "Render/Editor/Texture/PvrtcCompressor.h"
#include "Render/Editor/Texture/SphereMapFilter.h"
#include "Render/Editor/Texture/TextureOutput.h"
#include "Render/Editor/Texture/TextureOutputPipeline.h"
#include "Render/Editor/Texture/UnCompressor.h"
#include "Render/Resource/TextureResource.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

bool isBinaryAlpha(const drawing::Image* image)
{
	std::set< uint8_t > alphas;
	for (int32_t y = 0; y < image->getHeight(); ++y)
	{
		for (int32_t x = 0; x < image->getWidth(); ++x)
		{
			Color4f color;
			image->getPixel(x, y, color);

			uint8_t alpha = uint8_t(color.getAlpha() * 255.0f);
			alphas.insert(alpha);
			if (alphas.size() > 2)
				return false;
		}
	}
	return true;
}

struct ScaleTextureTask : public Object
{
	Ref< const drawing::Image > image;
	Ref< drawing::IImageFilter > filter;
	Ref< drawing::Image > output;
	float alphaCoverageDesired;
	float alphaCoverageRef;

	void execute()
	{
		output = image->applyFilter(filter);
		T_ASSERT (output);

		if (alphaCoverageDesired > 0.0f)
		{
			float alphaRefMin = 0.0f;
			float alphaRefMax = 1.0f;
			float alphaRefMid = 0.5f;

			for (int32_t i = 0; i < 10; ++i)
			{
				float alphaCoverageMip = 0.0f;

				for (int32_t y = 0; y < output->getHeight(); ++y)
				{
					for (int32_t x = 0; x < output->getWidth(); ++x)
					{
						Color4f color;
						output->getPixelUnsafe(x, y, color);
						alphaCoverageMip += (color.getAlpha() > alphaRefMid) ? 1.0f : 0.0f;
					}
				}

				alphaCoverageMip /= float(output->getWidth() * output->getHeight());

				if (alphaCoverageMip > alphaCoverageDesired + FUZZY_EPSILON)
					alphaRefMin = alphaRefMid;
				else if (alphaCoverageMip < alphaCoverageDesired - FUZZY_EPSILON)
					alphaRefMax = alphaRefMid;
				else
					break;

				alphaRefMid = (alphaRefMin + alphaRefMax) / 2.0f;
			}

			float alphaScale = alphaCoverageRef / alphaRefMid;

			for (int32_t y = 0; y < output->getHeight(); ++y)
			{
				for (int32_t x = 0; x < output->getWidth(); ++x)
				{
					Color4f color;
					output->getPixelUnsafe(x, y, color);
					color.setAlpha(clamp(color.getAlpha() * Scalar(alphaScale), Scalar(0.0f), Scalar(1.0f)));
					output->setPixelUnsafe(x, y, color);
				}
			}
		}
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.TextureOutputPipeline", 23, TextureOutputPipeline, editor::IPipeline)

TextureOutputPipeline::TextureOutputPipeline()
:	m_skipMips(0)
,	m_clampSize(0)
,	m_compressionMethod(CmDXTn)
,	m_compressionQuality(1)
{
}

bool TextureOutputPipeline::create(const editor::IPipelineSettings* settings)
{
	m_skipMips = settings->getProperty< PropertyInteger >(L"TexturePipeline.SkipMips", 0);
	m_clampSize = settings->getProperty< PropertyInteger >(L"TexturePipeline.ClampSize", 0);
	m_compressionQuality = settings->getProperty< PropertyInteger >(L"TexturePipeline.CompressionQuality", 1);

	std::wstring compressionMethod = settings->getProperty< PropertyString >(L"TexturePipeline.CompressionMethod", L"DXTn");
	if (compareIgnoreCase< std::wstring >(compressionMethod, L"None") == 0)
		m_compressionMethod = CmNone;
	else if (compareIgnoreCase< std::wstring >(compressionMethod, L"DXTn") == 0)
		m_compressionMethod = CmDXTn;
	else if (compareIgnoreCase< std::wstring >(compressionMethod, L"PVRTC") == 0)
		m_compressionMethod = CmPVRTC;
	else
	{
		log::error << L"Unknown compression method \"" << compressionMethod << L"\"" << Endl;
		return false;
	}

	return true;
}

void TextureOutputPipeline::destroy()
{
}

TypeInfoSet TextureOutputPipeline::getAssetTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< TextureOutput >());
	return typeSet;
}

bool TextureOutputPipeline::buildDependencies(
	editor::IPipelineDepends* pipelineDepends,
	const db::Instance* sourceInstance,
	const ISerializable* sourceAsset,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	Ref< const Object >& outBuildParams
) const
{
	return true;
}

bool TextureOutputPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset,
	uint32_t sourceAssetHash,
	const Object* buildParams,
	const std::wstring& outputPath,
	const Guid& outputGuid,
	uint32_t reason
) const
{
	const TextureOutput* textureOutput = checked_type_cast< const TextureOutput* >(sourceAsset);
	Ref< drawing::Image > image = checked_type_cast< const drawing::Image*, false >(buildParams)->clone();

	int32_t width = image->getWidth();
	int32_t height = image->getHeight();
	int32_t mipCount = 1;
	bool isNormalMap = false;

	drawing::PixelFormat pixelFormat;
	TextureFormat textureFormat;
	bool needAlpha;

	// Use explicit texture format if specified.
	if (textureOutput->m_textureFormat != TfInvalid)
	{
		log::info << L"Using explicit texture format" << Endl;

		textureFormat = textureOutput->m_textureFormat;
		switch (textureFormat)
		{
		case TfR8:
			pixelFormat = drawing::PixelFormat::getR8();
			break;
		case TfR8G8B8A8:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfR5G6B5:
			pixelFormat = drawing::PixelFormat::getR5G6B5();
			break;
		case TfR5G5B5A1:
			pixelFormat = drawing::PixelFormat::getR5G5B5A1();
			break;
		case TfR4G4B4A4:
			pixelFormat = drawing::PixelFormat::getR4G4B4A4();
			break;
		case TfR16G16B16A16F:
			pixelFormat = drawing::PixelFormat::getRGBAF16();
			break;
		case TfR32G32B32A32F:
			pixelFormat = drawing::PixelFormat::getRGBAF32();
			break;
		//case TfR16G16F:
		//	pixelFormat = drawing::PixelFormat::getR16G16F();
		//	break;
		//case TfR32G32F:
		//	pixelFormat = drawing::PixelFormat::getR32G32F();
		//	break;
		case TfR16F:
			pixelFormat = drawing::PixelFormat::getR16F();
			break;
		case TfR32F:
			pixelFormat = drawing::PixelFormat::getR32F();
			break;
		//case TfR11G11B10F:
		//	break;
		case TfDXT1:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT2:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT3:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT4:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfDXT5:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC1:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC2:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC3:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		case TfPVRTC4:
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			break;
		default:
			log::error << L"TextureOutputPipeline failed; unsupported explicit texture format" << Endl;
			return false;
		}

		needAlpha = 
			(pixelFormat.getAlphaBits() > 0 && !textureOutput->m_ignoreAlpha) ||
			textureOutput->m_enableNormalMapCompression;
	}
	else
	{
		log::info << L"Using automatic texture format" << Endl;

		// Determine pixel and texture format from source image (and hints).
		needAlpha = 
			(image->getPixelFormat().getAlphaBits() > 0 && !textureOutput->m_ignoreAlpha) ||
			textureOutput->m_enableNormalMapCompression;

		if (needAlpha)
		{
			pixelFormat = drawing::PixelFormat::getR8G8B8A8();
			textureFormat = TfR8G8B8A8;
		}
		else
		{
			pixelFormat = drawing::PixelFormat::getR8G8B8X8();
			textureFormat = TfR8G8B8A8;
		}

		// Determine texture compression format.
		if (!textureOutput->m_isCubeMap && (textureOutput->m_enableCompression || textureOutput->m_enableNormalMapCompression))
		{
			if (m_compressionMethod == CmDXTn)
			{
				if (textureOutput->m_enableNormalMapCompression)
				{
					log::info << L"Using DXT5nm compression" << Endl;
					textureFormat = TfDXT5;
				}
				else
				{
					bool binaryAlpha = false;
					//if (textureOutput->m_hasAlpha && !textureOutput->m_ignoreAlpha)
					//	binaryAlpha = isBinaryAlpha(image);
					if (needAlpha && !binaryAlpha)
					{
						log::info << L"Using DXT3 compression" << Endl;
						textureFormat = TfDXT3;
					}
					else
					{
						log::info << L"Using DXT1 compression" << Endl;
						textureFormat = TfDXT1;
					}
				}
			}
			else if (m_compressionMethod == CmPVRTC)
			{
				if (
					width == height &&
					width >= 8 && width <= 2048
				)
				{
					if (needAlpha)
					{
						log::info << L"Using PVRTC3 compression" << Endl;
						textureFormat = TfPVRTC3;
					}
					else
					{
						log::info << L"Using PVRTC1 compression" << Endl;
						textureFormat = TfPVRTC1;
					}
				}
				else
					log::info << L"Using no compression" << Endl;
			}
			else
				log::info << L"Using no compression" << Endl;
		}
		else
			log::info << L"Using no compression" << Endl;
	}

	// Data is stored in big endian as GPUs are big endian machines.
	pixelFormat = pixelFormat.endianSwapped();

	// Generate sphere map from cube map.
	if (textureOutput->m_isCubeMap && textureOutput->m_generateSphereMap)
	{
		log::info << L"Generating sphere map..." << Endl;
		SphereMapFilter sphereMapFilter;
		image = image->applyFilter(&sphereMapFilter);
	}

	// Convert into linear gamma, do it before we're converting image
	// format as it's possible source image has float format thus
	// resulting in greater accuracy.
	if (!textureOutput->m_linearGamma)
	{
		log::info << L"Converting into linear gamma..." << Endl;
		drawing::GammaFilter gammaFilter(1.0f / 2.2f);
		image = image->applyFilter(&gammaFilter);
	}

	// Convert image into proper format.
	image->convert(pixelFormat);

	// Generate normal map from image.
	if (textureOutput->m_generateNormalMap)
	{
		log::info << L"Generating normal map..." << Endl;
		drawing::NormalMapFilter filter(textureOutput->m_scaleDepth);
		image = image->applyFilter(&filter);
		isNormalMap = true;
	}

	// Inverse normal map Y; assume it's a normal map to begin with.
	if (textureOutput->m_inverseNormalMapY)
	{
		log::info << L"Converting normal map..." << Endl;
		drawing::TransformFilter transformFilter(Color4f(1.0f, -1.0f, 1.0f, 1.0f), Color4f(0.0f, 1.0f, 0.0f, 0.0f));
		image = image->applyFilter(&transformFilter);
		isNormalMap = true;
	}

	Ref< drawing::ChainFilter > mipFilters;

	// Swizzle channels to prepare for DXT5nm compression.
	if (
		m_compressionMethod == CmDXTn &&
		textureOutput->m_enableNormalMapCompression
	)
	{
		log::info << L"Preparing for DXT5nm compression..." << Endl;

		mipFilters = new drawing::ChainFilter();

		// Inverse X axis; do it here instead of in shader.
		mipFilters->add(new drawing::TransformFilter(Color4f(-1.0f, 1.0f, 1.0f, 1.0f), Color4f(1.0f, 0.0f, 0.0f, 0.0f)));

		// [rgba] -> [0,g,0,r] (or [a,g,0,r] if we cannot ignore alpha)
		mipFilters->add(new drawing::SwizzleFilter(textureOutput->m_ignoreAlpha ? L"0g0r" : L"ag0r"));

		if (!textureOutput->m_ignoreAlpha)
			log::warning << L"Kept source alpha in red channel; compressed normals might have severe artifacts" << Endl;

		isNormalMap = true;
	}

	// Rescale image.
	if (textureOutput->m_scaleImage)
	{
		width = textureOutput->m_scaleWidth;
		height = textureOutput->m_scaleHeight;
	}

	if (!textureOutput->m_isCubeMap)
	{
		// Skip mips.
		width = std::max(1, width >> m_skipMips);
		height = std::max(1, height >> m_skipMips);

		// Ensure image size doesn't exceed clamp size.
		if (m_clampSize > 0)
		{
			if (width > m_clampSize)
			{
				height = (height * m_clampSize) / width;
				width = m_clampSize;
			}
			if (height > m_clampSize)
			{
				width = (width * m_clampSize) / height;
				height = m_clampSize;
			}
		}

		// Ensure power-of-2 textures.
		if (!isLog2(width) || !isLog2(height))
		{
			log::warning << L"Texture dimension not power-of-2; resized to nearest valid dimension" << Endl;
			
			if (nearestLog2(width) - width < width - previousLog2(width))
				width = nearestLog2(width);
			else
				width = previousLog2(width);

			if (nearestLog2(height) - height < height - previousLog2(height))
				height = nearestLog2(height);
			else
				height = previousLog2(height);
		}
	}

	// Create output instance.
	Ref< TextureResource > outputResource = new TextureResource();
	Ref< db::Instance > outputInstance = pipelineBuilder->createOutputInstance(
		outputPath,
		outputGuid
	);
	if (!outputInstance)
	{
		log::error << L"Unable to create output instance" << Endl;
		return false;
	}

	outputInstance->setObject(outputResource);

	// Create output data stream.
	Ref< IStream > stream = outputInstance->writeData(L"Data");
	if (!stream)
	{
		log::error << L"Unable to create texture data stream" << Endl;
		outputInstance->revert();
		return false;
	}

	int32_t dataOffsetBegin = 0, dataOffsetEnd = 0;

	if (!textureOutput->m_isCubeMap || textureOutput->m_generateSphereMap)
	{
		mipCount = textureOutput->m_generateMips ? log2(std::max(width, height)) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		Writer writer(stream);

		writer << uint32_t(9);
		writer << int32_t(width);
		writer << int32_t(height);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(false);
		writer << bool(true);

		dataOffsetBegin = stream->tell();

		Ref< IStream > streamData = new BufferedStream(new compress::DeflateStreamLzf(stream), 64 * 1024);
		Writer writerData(streamData);

		RefArray< drawing::Image > mipImages(mipCount);

		// Generate each mip level.
		{
			RefArray< ScaleTextureTask > tasks;
			RefArray< Job > jobs;

			log::info << L"Executing mip generation task(s)..." << Endl;

			// Estimate alpha coverage if desired.
			float alphaCoverage = -1.0f;
			if (textureOutput->m_preserveAlphaCoverage)
			{
				alphaCoverage = 0.0f;
				for (int32_t y = 0; y < image->getHeight(); ++y)
				{
					for (int32_t x = 0; x < image->getWidth(); ++x)
					{
						Color4f color;
						image->getPixelUnsafe(x, y, color);
						alphaCoverage += (color.getAlpha() > textureOutput->m_alphaCoverageReference) ? 1.0f : 0.0f;
					}
				}
				alphaCoverage /= float(image->getWidth() * image->getHeight());
				log::info << L"Estimated alpha coverage " << toString(alphaCoverage * 100.0f, 2) << L"%" << Endl;
			}

			// Create task for each mip level.
			for (int32_t i = 0; i < mipCount; ++i)
			{
				int32_t mipWidth = std::max(width >> i, 1);
				int32_t mipHeight = std::max(height >> i, 1);

				log::info << L"Executing mip generation task " << i << L" (" << mipWidth << L"*" << mipHeight << L")..." << Endl;

				// Create chain of image filters.
				Ref< drawing::ChainFilter > taskFilters = new drawing::ChainFilter();

				// First add scaling filter to desired mip size.
				taskFilters->add(new drawing::ScaleFilter(
					mipWidth,
					mipHeight,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgLinear,
					textureOutput->m_keepZeroAlpha
				));

				// Ensure each pixel is renormalized after scaling.
				if (isNormalMap)
					taskFilters->add(new drawing::NormalizeFilter());

				// Append mip filters for compression etc.
				if (mipFilters)
					taskFilters->add(mipFilters);

				Ref< ScaleTextureTask > task = new ScaleTextureTask();
				task->image = image;
				task->filter = taskFilters;
				task->alphaCoverageDesired = alphaCoverage;
				task->alphaCoverageRef = textureOutput->m_alphaCoverageReference;

				Ref< Job > job = JobManager::getInstance().add(makeFunctor(task.ptr(), &ScaleTextureTask::execute));
				T_ASSERT (job);

				tasks.push_back(task);
				jobs.push_back(job);
			}

			log::info << L"Collecting task(s)..." << Endl;

			for (size_t i = 0; i < jobs.size(); ++i)
			{
				jobs[i]->wait();
				jobs[i] = 0;

				mipImages[i] = tasks[i]->output;
				T_ASSERT (mipImages[i]);

				tasks[i] = 0;
			}

			log::info << L"All task(s) collected" << Endl;
		}

		Ref< ICompressor > compressor;
		if (textureFormat >= TfDXT1 && textureFormat <= TfDXT5)
			compressor = new DxtnCompressor();
		else if (textureFormat >= TfPVRTC1 && textureFormat <= TfPVRTC4)
			compressor = new PvrtcCompressor();
		else
			compressor = new UnCompressor();

		compressor->compress(writerData, mipImages, textureFormat, needAlpha, m_compressionQuality);

		streamData->close();

		dataOffsetEnd = stream->tell();
	}
	else
	{
		uint32_t layout = 0;
		uint32_t sideSize = height;

		if (height == width / 6)
		{
			// [+x][-x][+y][-y][+z][-z]
			layout = 0;
			sideSize = height;
		}
		else if (height / 3 == width / 4)
		{
			// [  ][+y][  ][  ]
			// [-x][+z][+x][-z]
			// [  ][-y][  ][  ]
			layout = 1;
			sideSize = height / 3;
		}
		else if (height / 4 == width / 3)
		{
			// [  ][+y][  ]
			// [-x][+z][+x]
			// [  ][-y][  ]
			// [  ][-z][  ]
			layout = 2;
			sideSize = height / 4;
		}
		else
		{
			log::error << L"Cube map must have either a 6:1, 4:3 or 3:4 width/height ratio" << Endl;
			return false;
		}

		mipCount = textureOutput->m_generateMips ? log2(sideSize) + 1 : 1;
		T_ASSERT (mipCount >= 1);

		Writer writer(stream);

		writer << uint32_t(9);
		writer << int32_t(sideSize);
		writer << int32_t(sideSize);
		writer << int32_t(mipCount);
		writer << int32_t(textureFormat);
		writer << bool(true);
		writer << bool(true);

		dataOffsetBegin = stream->tell();

		// Create data writer, use deflate compression if enabled.
		Ref< IStream > streamData = new compress::DeflateStreamLzf(stream);
		Writer writerData(streamData);

		for (int side = 0; side < 6; ++side)
		{
			Ref< drawing::Image > sideImage = new drawing::Image(image->getPixelFormat(), sideSize, sideSize);

			if (layout == 0)
				sideImage->copy(image, side * sideSize, 0, sideSize, sideSize);
			else if (layout == 1)
			{
				const int32_t c_sideOffsets[][2] =
				{
					{ 2, 1 },
					{ 0, 1 },
					{ 1, 0 },
					{ 1, 2 },
					{ 1, 1 },
					{ 3, 1 }
				};
				sideImage->copy(
					image,
					c_sideOffsets[side][0] * sideSize,
					c_sideOffsets[side][1] * sideSize,
					sideSize,
					sideSize
				);
			}
			else if (layout == 2)
			{
				const int32_t c_sideOffsets[][2] =
				{
					{ 2, 1 },
					{ 0, 1 },
					{ 1, 0 },
					{ 1, 2 },
					{ 1, 1 },
					{ 1, 3 }
				};
				sideImage->copy(
					image,
					c_sideOffsets[side][0] * sideSize,
					c_sideOffsets[side][1] * sideSize,
					sideSize,
					sideSize
				);
				if (side == 5)
				{
					// Flip -Z as it's defined up-side down in this layout.
					drawing::MirrorFilter filter(true, true);
					sideImage = sideImage->applyFilter(&filter);
				}
			}

			for (int i = 0; i < mipCount; ++i)
			{
				int mipSize = sideSize >> i;

				drawing::ScaleFilter mipScaleFilter(
					mipSize,
					mipSize,
					drawing::ScaleFilter::MnAverage,
					drawing::ScaleFilter::MgLinear,
					textureOutput->m_keepZeroAlpha
				);
				Ref< drawing::Image > mipImage = sideImage->applyFilter(&mipScaleFilter);
				T_ASSERT (mipImage);

				 writerData.write(
					mipImage->getData(),
					mipSize * mipSize,
					sizeof(uint32_t)
				);
			}
		}

		streamData->close();

		dataOffsetEnd = stream->tell();
	}

	stream->close();

	if (!outputInstance->commit())
	{
		log::error << L"Unable to commit output instance" << Endl;
		return false;
	}

	// Create report.
	Ref< editor::IPipelineReport > report = pipelineBuilder->createReport(L"Texture", outputGuid);
	if (report)
	{
		report->set(L"path", outputPath);
		report->set(L"width", width);
		report->set(L"height", height);
		report->set(L"mipCount", mipCount);
		report->set(L"format", int32_t(textureFormat));
		report->set(L"dataSize", dataOffsetEnd - dataOffsetBegin);
	}

	return true;
}

Ref< ISerializable > TextureOutputPipeline::buildOutput(
	editor::IPipelineBuilder* pipelineBuilder,
	const ISerializable* sourceAsset
) const
{
	T_FATAL_ERROR;
	return 0;
}

	}
}
