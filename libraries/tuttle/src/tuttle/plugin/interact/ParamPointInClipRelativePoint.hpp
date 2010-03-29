#ifndef _TUTTLE_PLUGIN_PARAMPOINTINCLIPRELATIVEPOINT_HPP_
#define	_TUTTLE_PLUGIN_PARAMPOINTINCLIPRELATIVEPOINT_HPP_

#include "InteractInfos.hpp"
#include "InteractObject.hpp"
#include "ParamPointInClip.hpp"
#include <ofxsParam.h>

namespace tuttle {
namespace plugin {
namespace interact {

template<ECoordonateSystem coord>
class ParamPointInClipRelativePoint : public ParamPointInClip<coord>
{
public:
	ParamPointInClipRelativePoint( const InteractInfos& infos, OFX::Double2DParam* param, OFX::Clip* relativeClip, PointInteract* relativePoint );
	~ParamPointInClipRelativePoint();
	
private:
	PointInteract* _relativePoint;

public:
	Point2 getPoint() const;
	void setPoint( const Scalar& x, const Scalar& y );
};

template<ECoordonateSystem coord>
ParamPointInClipRelativePoint<coord>::ParamPointInClipRelativePoint( const InteractInfos& infos, OFX::Double2DParam* param, OFX::Clip* relativeClip, PointInteract* relativePoint )
: ParamPointInClip<coord>( infos, param, relativeClip )
, _relativePoint( relativePoint )
{
}

template<ECoordonateSystem coord>
ParamPointInClipRelativePoint<coord>::~ParamPointInClipRelativePoint( ) { }


template<>
Point2 ParamPointInClipRelativePoint<eCoordonateSystemXXcn>::getPoint() const
{
	OfxPointD rod = this->_relativeClip->getCanonicalRodSize( this->getTime() );
	Point2 relativePoint = _relativePoint->getPoint();
	OfxPointD pointData = this->_param->getValue();
	Point2 point = ofxToGil( interact::pointNormalizedXXToCanonicalXX( this->_param->getValue(), rod ) );
	Point2 res = relativePoint + point;
	return res;
}

template<>
void ParamPointInClipRelativePoint<eCoordonateSystemXXcn>::setPoint( const Scalar& x, const Scalar& y )
{
	if( _relativeClip->isConnected() )
	{
		Point2 rod = ofxToGil( this->_relativeClip->getCanonicalRodSize( this->getTime() ) );
		Point2 point = interact::pointCanonicalXYToNormalizedXX( Point2(x,y), rod );
		Point2 relativePoint = interact::pointCanonicalXYToNormalizedXX( _relativePoint->getPoint(), rod );
		_param->setValue( point.x - relativePoint.x, point.y - relativePoint.y );
		return;
	}
	_param->setValue( 0, 0 );
}

}
}
}

#endif
