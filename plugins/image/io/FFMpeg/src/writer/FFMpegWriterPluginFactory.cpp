#include "FFMpegWriterPluginFactory.hpp"
#include "FFMpegWriterPlugin.hpp"
#include "FFMpegWriterDefinitions.hpp"

#include <ffmpeg/VideoFFmpegWriter.hpp>

#include <tuttle/plugin/context/WriterPluginFactory.hpp>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <vector>

namespace tuttle {
namespace plugin {
namespace ffmpeg {
namespace writer {

void addOptionsFromAVOption( OFX::ImageEffectDescriptor& desc, OFX::GroupParamDescriptor* group, void* av_class, int req_flags, int rej_flags )
{
	std::vector<OFX::ChoiceParamDescriptor*> choices;
	std::vector<std::string> unit;
	std::vector<int> defaultEnumChoices;
	std::vector<int> defaultChoices;
	std::vector<OFX::GroupParamDescriptor*> groups;
	const AVOption *opt = NULL;

	while( (opt = av_opt_next( av_class, opt)  ) != NULL )
	{
		if( !opt ||
			!opt->name ||
			(opt->flags & req_flags) != req_flags ||
			(opt->flags & rej_flags) )
		{
			continue;
		}
		
		if( opt->unit && opt->type == AV_OPT_TYPE_FLAGS )
		{
			std::string name = "g_";
			name += opt->unit;
			OFX::GroupParamDescriptor* param = desc.defineGroupParam( name );
			if( opt->help )
				param->setHint( opt->help );
			param->setParent( group );
			param->setAsTab( );
			groups.push_back( param );
			continue;
		}
		if( opt->unit && opt->type == AV_OPT_TYPE_INT )
		{
			OFX::ChoiceParamDescriptor* param = desc.defineChoiceParam( opt->name );
			if( opt->help )
				param->setHint( opt->help );
			param->setParent( group );
			choices.push_back( param );
			unit.push_back( opt->unit );
			defaultEnumChoices.push_back( opt->default_val.dbl );
			//TUTTLE_COUT( opt->unit << " = " << opt->default_val.dbl );
			continue;
		}
		
		switch( opt->type )
		{
			case AV_OPT_TYPE_FLAGS:
			{
				OFX::BooleanParamDescriptor* param = desc.defineBooleanParam( opt->name );
				param->setDefault( opt->default_val.i64 );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_INT:
			case AV_OPT_TYPE_INT64:
			{
				OFX::IntParamDescriptor* param = desc.defineIntParam( opt->name );
				param->setDefault( opt->default_val.dbl );
				param->setRange( opt->min, opt->max );
				param->setDisplayRange( opt->min, opt->max );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_DOUBLE:
			case AV_OPT_TYPE_FLOAT:
			{
				OFX::DoubleParamDescriptor* param = desc.defineDoubleParam( opt->name );
				param->setDefault( opt->default_val.dbl );
				param->setRange( opt->min, opt->max );
				param->setDisplayRange( opt->min, opt->max );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_STRING:
			{
				OFX::StringParamDescriptor* param = desc.defineStringParam( opt->name );
				if( opt->default_val.str )
					param->setDefault( opt->default_val.str );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_RATIONAL:
			{
				OFX::Int2DParamDescriptor* param = desc.defineInt2DParam( opt->name );
				param->setDefault( opt->default_val.q.num, opt->default_val.q.den );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_IMAGE_SIZE:
			{
				OFX::Int2DParamDescriptor* param = desc.defineInt2DParam( opt->name );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_BINARY:
			{
				OFX::StringParamDescriptor* param = desc.defineStringParam( opt->name );
				if( opt->help )
					param->setHint( opt->help );
				param->setParent( group );
				break;
			}
			case AV_OPT_TYPE_CONST:
			{
				break;
			}
		}
	}

	defaultChoices.resize( defaultEnumChoices.size() );
	
	// adding enum values and flags in groups
	opt = NULL;
	while( (opt = av_opt_next( av_class, opt)  ) != NULL )
	{
		if( !opt ||
			!opt->name ||
			( opt->flags & req_flags ) != req_flags ||
			( opt->flags & rej_flags ) ||
			( opt->unit && opt->type == AV_OPT_TYPE_FLAGS ) ||
			( opt->unit && opt->type == AV_OPT_TYPE_INT ) )
		{
			continue;
		}
		
		switch( opt->type )
		{
			case AV_OPT_TYPE_CONST:
			{
				for( size_t i = 0; i < unit.size(); i++ )
				{
					if( opt->unit == unit.at( i ) )
					{
						//TUTTLE_COUT( "add " << opt->name << " to " << choices.at(i)->getName() );
						if( opt->help )
							choices.at(i)->appendOption( opt->name, opt->help );
						else
							choices.at(i)->appendOption( opt->name );
						if( opt->default_val.dbl == defaultEnumChoices.at(i) )
						{
							defaultChoices.at(i) = choices.at(i)->getNOptions() - 1;
						}
						
					}
				}
				
				BOOST_FOREACH( OFX::GroupParamDescriptor* g, groups )
				{
					std::string name = "g_";
					name += opt->unit;
					if( name == g->getName() )
					{
						OFX::BooleanParamDescriptor* param = desc.defineBooleanParam( opt->name );
						param->setDefault( opt->default_val.i64 );
						if( opt->help )
							param->setHint( opt->help );
						param->setParent( g );
						break;
					}
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	
	// set default value for enums
	opt = NULL;
	while( (opt = av_opt_next( av_class, opt)  ) != NULL )
	{
		if( !opt ||
			!opt->name ||
			( opt->flags & req_flags ) != req_flags ||
			( opt->flags & rej_flags ) ||
			( opt->unit && opt->type == AV_OPT_TYPE_FLAGS ) )
		{
			continue;
		}
		
		if( opt->unit && opt->type == AV_OPT_TYPE_INT )
		{
			for( size_t i = 0; i < defaultChoices.size(); i++ )
			{
				choices.at(i)->setDefault( defaultChoices.at(i) );
			}
			
		}
	}
}

/**
 * @brief Function called to describe the plugin main features.
 * @param[in, out]   desc     Effect descriptor
 */
void FFMpegWriterPluginFactory::describe( OFX::ImageEffectDescriptor& desc )
{
	desc.setLabels(
		"TuttleFfmpegWriter",
		"FfmpegWriter",
		"Ffmpeg video writer" );
	desc.setPluginGrouping( "tuttle/image/io" );

	std::vector<std::string> supportedExtensions;
	{
		AVOutputFormat* oFormat = av_oformat_next( NULL );
		while( oFormat != NULL )
		{
			if( oFormat->extensions != NULL )
			{
				using namespace boost::algorithm;
				const std::string extStr( oFormat->extensions );
				std::vector<std::string> exts;
				split( exts, extStr, is_any_of(",") );
				
				// remove empty extensions...
				for( std::vector<std::string>::iterator it = exts.begin(); it != exts.end(); )
				{
					if( it->size() == 0 )
						it = exts.erase( it );
					else
						++it;
				}
				supportedExtensions.insert( supportedExtensions.end(), exts.begin(), exts.end() );
			}
			oFormat = av_oformat_next( oFormat );
		}
	}
	
	desc.setDescription( "Video writer based on FFMpeg library\n\n"
			"Supported extensions: \n" +
			boost::algorithm::join( supportedExtensions, ", " )
		);
	
	// add the supported contexts
	desc.addSupportedContext( OFX::eContextWriter );
	desc.addSupportedContext( OFX::eContextGeneral );

	// add supported pixel depths
	desc.addSupportedBitDepth( OFX::eBitDepthUByte );
	desc.addSupportedBitDepth( OFX::eBitDepthUShort );
	desc.addSupportedBitDepth( OFX::eBitDepthFloat );

    // add supported extensions
	desc.addSupportedExtensions( supportedExtensions );
	
	// plugin flags
	desc.setRenderThreadSafety( OFX::eRenderInstanceSafe );
	desc.setHostFrameThreading( false );
	desc.setSupportsMultiResolution( false );
	desc.setSupportsMultipleClipDepths( true );
	desc.setSupportsTiles( kSupportTiles );
}

/**
 * @brief Function called to describe the plugin controls and features.
 * @param[in, out]   desc       Effect descriptor
 * @param[in]        context    Application context
 */
void FFMpegWriterPluginFactory::describeInContext( OFX::ImageEffectDescriptor& desc,
						   OFX::EContext               context )
{
	VideoFFmpegWriter writer;

	OFX::ClipDescriptor* srcClip = desc.defineClip( kOfxImageEffectSimpleSourceClipName );

	srcClip->addSupportedComponent( OFX::ePixelComponentRGBA );
	srcClip->addSupportedComponent( OFX::ePixelComponentRGB );
	srcClip->addSupportedComponent( OFX::ePixelComponentAlpha );
	srcClip->setSupportsTiles( kSupportTiles );

	// Create the mandated output clip
	OFX::ClipDescriptor* dstClip = desc.defineClip( kOfxImageEffectOutputClipName );
	dstClip->addSupportedComponent( OFX::ePixelComponentRGBA );
	dstClip->addSupportedComponent( OFX::ePixelComponentRGB );
	dstClip->addSupportedComponent( OFX::ePixelComponentAlpha );
	dstClip->setSupportsTiles( kSupportTiles );
	
	// Controls
	describeWriterParamsInContext( desc, context );
	OFX::ChoiceParamDescriptor* bitDepth = static_cast<OFX::ChoiceParamDescriptor*>( desc.getParamDescriptor( kTuttlePluginBitDepth ) );
	bitDepth->appendOption( kTuttlePluginBitDepth8 );
	/// @todo: 16 bits
	bitDepth->setDefault( eTuttlePluginBitDepth8 );
	bitDepth->setEnabled( false );

	OFX::BooleanParamDescriptor* premult = static_cast<OFX::BooleanParamDescriptor*>( desc.getParamDescriptor( kParamPremultiplied ) );
	premult->setDefault( true );

	// Groups
	OFX::GroupParamDescriptor* formatGroup = desc.defineGroupParam( kParamFormatGroup );
	OFX::GroupParamDescriptor* videoGroup  = desc.defineGroupParam( kParamVideoGroup );
	OFX::GroupParamDescriptor* audioGroup  = desc.defineGroupParam( kParamAudioGroup );
	OFX::GroupParamDescriptor* metaGroup   = desc.defineGroupParam( kParamMetaGroup );
	
	formatGroup->setLabel( "Format" );
	videoGroup->setLabel( "Video" );
	audioGroup->setLabel( "Audio" );
	metaGroup->setLabel( "Metadata" );
	
	formatGroup->setAsTab( );
	videoGroup->setAsTab( );
	audioGroup->setAsTab( );
	metaGroup->setAsTab( );
	

	/// FORMAT PARAMETERS
	int default_format = 0;
	OFX::ChoiceParamDescriptor* format = desc.defineChoiceParam( kParamFormat );
	for( std::vector<std::string>::const_iterator itShort = writer.getFormatsShort().begin(),
		itLong = writer.getFormatsLong().begin(),
		itEnd = writer.getFormatsShort().end();
		itShort != itEnd;
		++itShort,
		++itLong )
	{
		format->appendOption( *itShort, *itLong );
		if( (*itShort) == "mp4" )
			default_format = format->getNOptions() - 1;
	}
	format->setCacheInvalidation( OFX::eCacheInvalidateValueAll );
	format->setDefault( default_format );
	format->setParent( formatGroup );

	AVFormatContext* avFormatContext;
	avFormatContext = avformat_alloc_context();
	addOptionsFromAVOption( desc, formatGroup, (void*)avFormatContext, AV_OPT_FLAG_ENCODING_PARAM, 0 );
	avformat_free_context( avFormatContext );
	
	OFX::GroupParamDescriptor* formatDetailledGroup = desc.defineGroupParam( kParamFormatDetailledGroup );
	formatDetailledGroup->setLabel( "Detailled" );
	formatDetailledGroup->setAsTab( );
	formatDetailledGroup->setParent( formatGroup );
	
	std::vector<AVPrivOption> privOptions = writer.getFormatPrivOpts();
	
	BOOST_FOREACH( AVPrivOption& opt, privOptions )
	{
		if(opt.o.name)
		{
			TUTTLE_COUT( opt.class_name << "\t" << opt.o.name );
		}
	}
	
	/// VIDEO PARAMETERS
	int default_codec = 0;
	OFX::ChoiceParamDescriptor* videoCodec = desc.defineChoiceParam( kParamVideoCodec );
	for( std::vector<std::string>::const_iterator itShort = writer.getVideoCodecsShort().begin(),
		itLong  = writer.getVideoCodecsLong().begin(),
		itEnd = writer.getVideoCodecsShort().end();
		itShort != itEnd;
		++itShort,
		++itLong )
	{
		videoCodec->appendOption( *itShort, *itLong );
		if( (*itShort) == "mpeg4" )
			default_codec = videoCodec->getNOptions() - 1;
	}
	videoCodec->setCacheInvalidation( OFX::eCacheInvalidateValueAll );
	videoCodec->setDefault( default_codec );
	videoCodec->setParent( videoGroup );
	
	const std::vector<std::string> codecListWithPreset = writer.getPresets().getCodecListWithConfig();
	BOOST_FOREACH( const std::string& codecName, codecListWithPreset )
	{
		const std::vector<std::string> codecList = writer.getPresets().getConfigList( codecName );
		if( codecList.empty() )
			continue;
		
		OFX::ChoiceParamDescriptor* choiceConfig = desc.defineChoiceParam( codecName + kParamVideoPreset );
		choiceConfig->setLabel( codecName + " Preset" );
		choiceConfig->setDefault( 0 );
		
		BOOST_FOREACH( const std::string& codec, codecList )
		{
			choiceConfig->appendOption( codec );
		}
		choiceConfig->setParent( videoGroup );
	}
	
	AVCodecContext* avCodecContext;
#if LIBAVCODEC_VERSION_INT > AV_VERSION_INT( 53, 8, 0 )
	avCodecContext = avcodec_alloc_context();
	// deprecated in the same version
	//avCodecContext = avcodec_alloc_context2( AVMEDIA_TYPE_UNKNOWN );
#else
	AVCodec* avCodec = NULL;
	avCodecContext = avcodec_alloc_context3( avCodec );
#endif
	
	addOptionsFromAVOption( desc, videoGroup, (void*)avCodecContext, AV_OPT_FLAG_ENCODING_PARAM | AV_OPT_FLAG_VIDEO_PARAM, 0 );
	
	OFX::GroupParamDescriptor* videoDetailledGroup  = desc.defineGroupParam( kParamVideoDetailledGroup );
	videoDetailledGroup->setLabel( "Detailled" );
	videoDetailledGroup->setAsTab( );
	videoDetailledGroup->setParent( videoGroup );
	
	
	/// AUDIO PARAMETERS
	int default_audio_codec = 0;
	OFX::ChoiceParamDescriptor* audioCodec = desc.defineChoiceParam( kParamAudioCodec );
	for( std::vector<std::string>::const_iterator itShort = writer.getAudioCodecsShort().begin(),
		itLong  = writer.getAudioCodecsLong().begin(),
		itEnd = writer.getAudioCodecsShort().end();
		itShort != itEnd;
		++itShort,
		++itLong )
	{
		audioCodec->appendOption( *itShort, *itLong );
		if( (*itShort) == "pcm_s24be" )
			default_audio_codec = audioCodec->getNOptions() - 1;
	}
	audioCodec->setCacheInvalidation( OFX::eCacheInvalidateValueAll );
	audioCodec->setDefault( default_audio_codec );
	audioCodec->setParent( audioGroup );
	
	addOptionsFromAVOption( desc, audioGroup, (void*)avCodecContext, AV_OPT_FLAG_ENCODING_PARAM | AV_OPT_FLAG_AUDIO_PARAM, AV_OPT_FLAG_VIDEO_PARAM );
	
	OFX::GroupParamDescriptor* audioDetailledGroup  = desc.defineGroupParam( kParamAudioDetailledGroup );
	audioDetailledGroup->setLabel( "Detailled" );
	audioDetailledGroup->setAsTab( );
	audioDetailledGroup->setParent( audioGroup );
	
	av_free( avCodecContext );
	
	/// METADATA PARAMETERS
	OFX::GroupParamDescriptor* metaDetailledGroup   = desc.defineGroupParam( kParamMetaDetailledGroup );
	metaDetailledGroup->setLabel( "Detailled" );
	metaDetailledGroup->setAsTab( );
	metaDetailledGroup->setParent( metaGroup );
}

/**
 * @brief Function called to create a plugin effect instance
 * @param[in] handle  effect handle
 * @param[in] context    Application context
 * @return  plugin instance
 */
OFX::ImageEffect* FFMpegWriterPluginFactory::createInstance( OfxImageEffectHandle handle,
							     OFX::EContext        context )
{
	return new FFMpegWriterPlugin( handle );
}

}
}
}
}
