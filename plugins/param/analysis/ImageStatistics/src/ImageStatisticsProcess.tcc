#include "ImageStatisticsPlugin.hpp"
#include "ImageStatisticsProcess.hpp"
#include <tuttle/plugin/image/gil/globals.hpp>
#include <boost/gil/extension/numeric/pixel_numeric_operations.hpp>
#include <boost/gil/extension/numeric/pixel_numeric_operations2.hpp>
#include <boost/gil/extension/toolbox/hsl.hpp>
#include <boost/units/pow.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/erase.hpp>
#include <boost/mpl/find.hpp>

#include <tuttle/common/utils/global.hpp>

namespace boost {
namespace gil {

////////////////////////////////////////////////////////////////////////////////

/// \ingroup ChannelNumericOperations
/// \brief ch2 = min( ch1, ch2 )
/// structure for adding one channel to another
/// this is a generic implementation; user should specialize it for better performance
template <typename ChannelSrc,typename ChannelDst>
struct channel_assign_min_t : public std::binary_function<ChannelSrc,ChannelDst,ChannelDst>
{
    typename channel_traits<ChannelDst>::reference
	operator()( typename channel_traits<ChannelSrc>::const_reference ch1,
                typename channel_traits<ChannelDst>::reference ch2 ) const
	{
        return ch2 = std::min( ChannelDst( ch1 ), ch2 );
    }
};

/// \ingroup PixelNumericOperations
/// \brief p2 = min( p1, p2 )
template <typename PixelSrc, // models pixel concept
          typename PixelDst> // models pixel value concept
struct pixel_assign_min_t
{
    PixelDst& operator()( const PixelSrc& p1,
                          PixelDst& p2 ) const
	{
        static_for_each( p1, p2,
                         channel_assign_min_t<typename channel_type<PixelSrc>::type,
                                              typename channel_type<PixelDst>::type>() );
        return p2;
    }
};

////////////////////////////////////////////////////////////////////////////////

/// \ingroup ChannelNumericOperations
/// \brief ch2 = max( ch1, ch2 )
/// this is a generic implementation; user should specialize it for better performance
template <typename ChannelSrc,typename ChannelDst>
struct channel_assign_max_t : public std::binary_function<ChannelSrc,ChannelDst,ChannelDst>
{
    typename channel_traits<ChannelDst>::reference
	operator()( typename channel_traits<ChannelSrc>::const_reference ch1,
                typename channel_traits<ChannelDst>::reference ch2 ) const
	{
        return ch2 = std::max( ChannelDst( ch1 ), ch2 );
    }
};

/// \ingroup PixelNumericOperations
/// \brief p2 = max( p1, p2 )
template <typename PixelSrc, // models pixel concept
          typename PixelDst> // models pixel value concept
struct pixel_assign_max_t
{
    PixelDst& operator()( const PixelSrc& p1,
                          PixelDst& p2 ) const
	{
        static_for_each( p1, p2,
                         channel_assign_max_t<typename channel_type<PixelSrc>::type,
                                              typename channel_type<PixelDst>::type>() );
        return p2;
    }
};

////////////////////////////////////////////////////////////////////////////////

/*
namespace detail {

// compile-time recursion for per-element operations on color bases
template <int N>
struct element_recursion {
//	//static_for_each with two sources
//    template <typename P1,typename P2,typename Op>
//    static Op static_for_each(P1& p1, P2& p2, Op op) {
//        Op op2(element_recursion<N-1>::static_for_each(p1,p2,op));
//        op2(semantic_at_c<N-1>(p1), semantic_at_c<N-1>(p2));
//        return op2;
//    }
//    template <typename P1,typename P2,typename Op>
//    static Op static_for_each(P1& p1, const P2& p2, Op op) {
//        Op op2(element_recursion<N-1>::static_for_each(p1,p2,op));
//        op2(semantic_at_c<N-1>(p1), semantic_at_c<N-1>(p2));
//        return op2;
//    }
    template <typename P1,typename P2,typename Op, typename ChannelFilter = mpl::vector>
    static Op static_for_each(const P1& p1, P2& p2, Op op) {
        Op op2(element_recursion<N-1>::static_for_each(p1,p2,op));
        op2(semantic_at_c<N-1>(p1), semantic_at_c<N-1>(p2));
        return op2;
    }
//    template <typename P1,typename P2,typename Op>
//    static Op static_for_each(const P1& p1, const P2& p2, Op op) {
//        Op op2(element_recursion<N-1>::static_for_each(p1,p2,op));
//        op2(semantic_at_c<N-1>(p1), semantic_at_c<N-1>(p2));
//        return op2;
//    }
};

// Termination condition of the compile-time recursion for element operations on a color base
template<> struct element_recursion<0> {
    //static_for_each with two sources
    template <typename P1,typename P2,typename Op>
    static Op static_for_each(const P1&,const P2&,Op op){return op;}
};

}

//static_for_each with two sources
//template <typename P1,typename P2,typename Op>
//GIL_FORCEINLINE
//Op static_for_each(P1& p1,      P2& p2, Op op)             { return detail::element_recursion<size<P1>::value>::static_for_each(p1,p2,op); }
//template <typename P1,typename P2,typename Op>
//GIL_FORCEINLINE
//Op static_for_each(P1& p1,const P2& p2, Op op)             { return detail::element_recursion<size<P1>::value>::static_for_each(p1,p2,op); }
template <typename P1,typename P2,typename Op>
GIL_FORCEINLINE
Op static_for_each(const P1& p1,      P2& p2, Op op)             { return detail::element_recursion<size<P1>::value>::static_for_each(p1,p2,op); }
//template <typename P1,typename P2,typename Op>
//GIL_FORCEINLINE
//Op static_for_each(const P1& p1,const P2& p2, Op op)             { return detail::element_recursion<size<P1>::value>::static_for_each(p1,p2,op); }

*/
/*
template <typename PixelRef,  // models pixel concept
          typename PixelRefR> // models pixel concept
struct without_alpha_channel_t
{
	typedef typename color_space_type<PixelRef>::type Colorspace;
//	typedef typename mpl::find<Colorspace, alpha_t>::type AlphaPos;
//	typedef typename mpl::erase<Colorspace, AlphaPos>::type ColorspaceWithoutAlpha;
//	typedef ColorspaceWithoutAlpha ColorspaceR;
//	typedef typename mpl::if_<contains_color<PixelRef, alpha_t>,
//           ColorspaceWithoutAlpha,
//           PixelRef>::type ColorspaceR;
//    typedef typename rgb_t ColorspaceR;
	typedef pixel<typename channel_type<PixelRef>::type, layout<ColorspaceR> > PixelRefR;
    PixelRefR operator () (const PixelRef& src,
                           PixelRefR& dst) const
	{
		get_color( dst, red_t()   ) = get_color( src, red_t()   );
		get_color( dst, green_t() ) = get_color( src, green_t() );
		get_color( dst, blue_t()  ) = get_color( src, blue_t()  );
        return dst;
    }
};
*/

/// \ingroup PixelNumericOperations
///definition and a generic implementation for casting and assigning a pixel to another
///user should specialize it for better performance
template <typename PixelRef,  // models pixel concept
          typename PixelRefR> // models pixel concept
struct pixel_assigns_without_alpha_t {
    PixelRefR& operator () (const PixelRef& src,
                           PixelRefR& dst) const {
        static_for_each(src,dst,channel_assigns_t<typename channel_type<PixelRef>::type,
                                                  typename channel_type<PixelRefR>::type>());
        return dst;
    }
};

}
}



namespace tuttle {
namespace plugin {
namespace imageStatistics {

template<class View>
ImageStatisticsProcess<View>::ImageStatisticsProcess( ImageStatisticsPlugin &instance )
: ImageGilFilterProcessor<View>( instance )
, _plugin( instance )
{
	this->setNoMultiThreading( );
}


template<typename T>
T standard_deviation( const T v_sum, const T v_sum_x2, const std::size_t nb )
{
	using namespace boost::units;
	return std::sqrt( (v_sum_x2 - pow<2>(v_sum)) / nb );
}

/**
 * @brief In probability theory and statistics, kurtosis is a measure of the
 * "peakedness" of the probability distribution of a real-valued random variable.
 *
 * Higher kurtosis means more of the variance is the result of infrequent extreme deviations,
 * as opposed to frequent modestly sized deviations.
 */
template<typename T>
T kurtosis( const T v_mean, const T v_standard_deviation, const T v_sum, const T v_sum_x2, const T v_sum_x3, const T v_sum_x4, const std::size_t nb )
{
	using namespace boost::units;
	return ( (v_sum_x4 - 4.0 * v_mean * v_sum_x3 + 6.0*pow<2>(v_mean)* v_sum_x2) / nb - 3.0*pow<4>(v_mean) ) /
		   ( pow<4>(v_standard_deviation) ) -3.0;
}

/**
 * @brief In probability theory and statistics, skewness is a measure of the
 * asymmetry of the probability distribution of a real-valued random variable.
 *
 * A zero value indicates that the values are relatively evenly distributed
 * on both sides of the mean, typically but not necessarily implying
 * a symmetric distribution.
 */
template<typename T>
T skewness( const T v_mean, const T v_standard_deviation, const T v_sum, const T v_sum_x2, const T v_sum_x3, const std::size_t nb )
{
	using namespace boost::units;
	return ( (v_sum_x3 - 3.0*v_mean*v_sum_x2) / nb + 2.0*pow<3>(v_mean) ) / pow<3>(v_standard_deviation);
}

template<class View>
void ImageStatisticsProcess<View>::setup( const OFX::RenderArguments &args )
{
	using namespace boost::gil;

	ImageGilFilterProcessor<View>::setup( args );

	// recovery parameters values
	OfxRectI srcRod = this->_clipSrc->getPixelRod( args.time );
	_processParams = _plugin.getProcessParams( srcRod );

	Point2 rectSize( std::abs( _processParams._rect.x2 - _processParams._rect.x1 ),
					 std::abs( _processParams._rect.y2 - _processParams._rect.y1 ) );

	COUT_VAR( _processParams._rect );
	COUT_VAR( rectSize );

	typedef pixel<bits32f, layout<typename color_space_type<View>::type> > Pixel32f;
	typedef pixel<typename channel_type<View>::type, layout<gray_t> > PixelGray;

	// declare values and init
	Pixel firstPixel = this->_srcView(_processParams._rect.x1, _processParams._rect.y1); // for initialization only
	PixelGray firstPixelGray;
	color_convert( firstPixel, firstPixelGray );

	Pixel32f average;
	pixel_zeros_t<Pixel32f>()( average );
	Pixel minChannel = firstPixel;
	Pixel maxChannel = firstPixel;
	Pixel minLuminosity = firstPixel;
	PixelGray minLuminosityGray = firstPixelGray;
	Pixel maxLuminosity = firstPixel;
	PixelGray maxLuminosityGray = firstPixelGray;

	hsl32f_pixel_t hslSum;
	hsl32f_pixel_t hslSum_p2;
	hsl32f_pixel_t hslSum_p3;
	hsl32f_pixel_t hslSum_p4;
	pixel_zeros_t<hsl32f_pixel_t>()( hslSum );
	pixel_zeros_t<hsl32f_pixel_t>()( hslSum_p2 );
	pixel_zeros_t<hsl32f_pixel_t>()( hslSum_p3 );
	pixel_zeros_t<hsl32f_pixel_t>()( hslSum_p4 );

	for( int y = _processParams._rect.y1;
		 y < _processParams._rect.y2;
		 ++y )
	{
		typename View::x_iterator src_it = this->_srcView.x_at( _processParams._rect.x1, y );
		Pixel32f lineAverage;
		pixel_zeros_t<Pixel32f>()( lineAverage );
		
		for( int x = _processParams._rect.x1;
			 x < _processParams._rect.x2;
			 ++x, ++src_it )
		{
			Pixel32f pix;
			pixel_assigns_t<Pixel, Pixel32f>()( *src_it, pix ); // pix = src_it;
			rgb32f_pixel_t pixRgb;
			rgba_to_rgb_t<Pixel32f>()( pix, pixRgb ); // pixRgb = pix;

			hsl32f_pixel_t pixHsl;
			color_convert( pixRgb, pixHsl );

			hsl32f_pixel_t pixHsl_p2;
			pixel_assigns_t<hsl32f_pixel_t, hsl32f_pixel_t>()( pixel_pow_t<hsl32f_pixel_t, 2>()( pixHsl ), pixHsl_p2 ); // pixHsl_p2 = pow<2>( pixHsl );
//			pixel_assigns_t<hsl32f_pixel_t, hsl32f_pixel_t>()( pixel_multiplies_t<hsl32f_pixel_t, hsl32f_pixel_t, hsl32f_pixel_t>()( pix, pix ), pixPow2 ); // pixHsl_p2 = pixHsl * pixHsl;
			hsl32f_pixel_t pixHsl_p3;
			pixel_assigns_t<hsl32f_pixel_t, hsl32f_pixel_t>()( pixel_multiplies_t<hsl32f_pixel_t, hsl32f_pixel_t, hsl32f_pixel_t>()( pixHsl, pixHsl_p2 ), pixHsl_p3 ); // pixHsl_p3 = pixHsl * pixHsl_p2;
			hsl32f_pixel_t pixHsl_p4;
			pixel_assigns_t<hsl32f_pixel_t, hsl32f_pixel_t>()( pixel_multiplies_t<hsl32f_pixel_t, hsl32f_pixel_t, hsl32f_pixel_t>()( pixHsl_p2, pixHsl_p2 ), pixHsl_p4 ); // pixHsl_p4 = pixHsl_p2 * pixHsl_p2;

			// for average : accumulate
			pixel_plus_assign_t<Pixel32f, Pixel32f>()( pix, lineAverage ); // lineAverage += pix;

			// search min for each channel
			pixel_assign_min_t<Pixel, Pixel>()( *src_it, minChannel );
			// search max for each channel
			pixel_assign_max_t<Pixel, Pixel>()( *src_it, maxChannel );
			
			PixelGray grayCurrentPixel; // current pixel in gray colorspace
			color_convert( *src_it, grayCurrentPixel );
			// search min luminosity
			if( get_color( grayCurrentPixel, gray_color_t() ) < get_color( minLuminosityGray, gray_color_t() ) )
			{
				minLuminosity = *src_it;
				minLuminosityGray = grayCurrentPixel;
			}
			// search max luminosity
			if( get_color( grayCurrentPixel, gray_color_t() ) > get_color( maxLuminosityGray, gray_color_t() ) )
			{
				maxLuminosity = *src_it;
				maxLuminosityGray = grayCurrentPixel;
			}
		}
		// for average : divide by number of accumulated pixels
		pixel_divides_scalar_assign_t<double, Pixel32f>()( rectSize.x, lineAverage ); // lineAverage /= rectSize.x;
		// for average : accumulate each line
		pixel_plus_assign_t<Pixel32f, Pixel32f>()( lineAverage, average ); // _average += lineAverage;
	}
	// for average : divide by number of accumulated lines
	pixel_divides_scalar_assign_t<double, Pixel32f>()( rectSize.y, average ); // _average /= rectSize.y;


	rgba32f_pixel_t outputRgbaValue;
	rgb32f_pixel_t outputRgbValue;
	hsl32f_pixel_t outputHslValue;
	
	color_convert( average, outputRgbaValue );
	_plugin._paramOutputAverage->setValueAtTime( args.time,
	                                        get_color( outputRgbaValue, red_t() ),
	                                        get_color( outputRgbaValue, green_t() ),
	                                        get_color( outputRgbaValue, blue_t() ),
	                                        get_color( outputRgbaValue, alpha_t() )
	                                      );

	rgba_to_rgb_t<Pixel32f>()( average, outputRgbValue );
	color_convert( outputRgbValue, outputHslValue );
	_plugin._paramOutputAverageHsl->setValueAtTime( args.time,
	                                        get_color( outputHslValue, hsl_color_space::hue_t() ),
	                                        get_color( outputHslValue, hsl_color_space::saturation_t() ),
	                                        get_color( outputHslValue, hsl_color_space::lightness_t() )
	                                      );
	color_convert( minChannel, outputRgbaValue );
	_plugin._paramOutputChannelMin->setValueAtTime( args.time,
	                                        get_color( outputRgbaValue, red_t() ),
	                                        get_color( outputRgbaValue, green_t() ),
	                                        get_color( outputRgbaValue, blue_t() ),
	                                        get_color( outputRgbaValue, alpha_t() )
	                                      );
	color_convert( maxChannel, outputRgbaValue );
	_plugin._paramOutputChannelMax->setValueAtTime( args.time,
	                                        get_color( outputRgbaValue, red_t() ),
	                                        get_color( outputRgbaValue, green_t() ),
	                                        get_color( outputRgbaValue, blue_t() ),
	                                        get_color( outputRgbaValue, alpha_t() )
	                                      );
	color_convert( minLuminosity, outputRgbaValue );
	_plugin._paramOutputLuminosityMin->setValueAtTime( args.time,
	                                        get_color( outputRgbaValue, red_t() ),
	                                        get_color( outputRgbaValue, green_t() ),
	                                        get_color( outputRgbaValue, blue_t() ),
	                                        get_color( outputRgbaValue, alpha_t() )
	                                      );
	color_convert( maxLuminosity, outputRgbaValue );
	_plugin._paramOutputLuminosityMax->setValueAtTime( args.time,
	                                        get_color( outputRgbaValue, red_t() ),
	                                        get_color( outputRgbaValue, green_t() ),
	                                        get_color( outputRgbaValue, blue_t() ),
	                                        get_color( outputRgbaValue, alpha_t() )
	                                      );
	
	switch( _processParams._chooseOutput )
	{
		case eParamChooseOutputSource:
			break;
		case eParamChooseOutputAverage:
			color_convert( average, _outputPixel );
			break;
		case eParamChooseOutputChannelMin:
			_outputPixel = minChannel;
			break;
		case eParamChooseOutputChannelMax:
			_outputPixel = maxChannel;
			break;
		case eParamChooseOutputLuminosityMin:
			_outputPixel = minLuminosity;
			break;
		case eParamChooseOutputLuminosityMax:
			_outputPixel = maxLuminosity;
			break;
	}
}

/**
 * @param[in] procWindowRoW  Processing window in RoW
 */
template<class View>
void ImageStatisticsProcess<View>::multiThreadProcessImages( const OfxRectI& procWindowRoW )
{
	using namespace boost::gil;
	OfxRectI procWindowOutput = this->translateRoWToOutputClipCoordinates( procWindowRoW );
	if( _processParams._chooseOutput == eParamChooseOutputSource )
	{
		for( int y = procWindowOutput.y1;
			 y < procWindowOutput.y2;
			 ++y )
		{
			typename View::x_iterator src_it = this->_srcView.x_at( procWindowOutput.x1, y );
			typename View::x_iterator dst_it = this->_dstView.x_at( procWindowOutput.x1, y );
			for( int x = procWindowOutput.x1;
				 x < procWindowOutput.x2;
				 ++x, ++src_it, ++dst_it )
			{
				(*dst_it) = (*src_it);
			}
			if( this->progressForward() )
				return;
		}
	}
	else
	{
		for( int y = procWindowOutput.y1;
			 y < procWindowOutput.y2;
			 ++y )
		{
			typename View::x_iterator dst_it = this->_dstView.x_at( procWindowOutput.x1, y );
			for( int x = procWindowOutput.x1;
				 x < procWindowOutput.x2;
				 ++x, ++dst_it )
			{
				( *dst_it ) = _outputPixel;
			}
			if( this->progressForward( ) )
				return;
		}
	}
}

}
}
}
