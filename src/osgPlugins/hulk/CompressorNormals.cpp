#include "CompressorNormals.hpp"


osg::Vec2 octWrap(const osg::Vec2 &v )
{
    return osg::Vec2((1 - std::abs(v.y()))*(v.x() >= 0.0 ? 1.0 : -1.0), 
              ((1 - std::abs(v.x()))*(v.y() >= 0.0 ? 1.0 : -1.0)));
}

osg::Vec2 encodeOctahedron(const osg::Vec3 &n)
{
    float sum = ( std::abs( n.x() ) + std::abs( n.y() ) + std::abs( n.z() ) );
    osg::Vec3 temp(n);
    temp /= sum;

    osg::Vec2 oct = octWrap(osg::Vec2(temp.x(), temp.y()));
    osg::Vec2 result(temp.z() >= 0.0 ? temp.x() : oct.x(), temp.z() >= 0.0 ? temp.y() : oct.y());
    result.x() = result.x()*0.5f + 0.5f;
    result.y() = result.y()*0.5f + 0.5f;
    return result;
}

osg::Vec3 decodeOcthahedron(const osg::Vec2 &encN )
{
    osg::Vec2 temp(encN);
    temp.x() = temp.x() * 2.0f - 1.0f;
    temp.y() = temp.y() * 2.0f - 1.0f;

    osg::Vec3 n;
    n.z() = 1.0 - std::abs( temp.x() ) - std::abs( temp.y() );
    n.x() = n.z() >= 0.0 ? temp.x() : octWrap( temp ).x();
    n.y() = n.z() >= 0.0 ? temp.y() : octWrap( temp ).y();
    n.normalize();
    return n;
}

osg::Vec2 encodeSphericalCoordinate(const osg::Vec3 &v) {
    return (osg::Vec2(atan2(v.y(), v.x()) / osg::PI, v.z()) + osg::Vec2(1,1))*0.5f;
}


osg::Vec3 decodeSphericalCoordinate(const osg::Vec2 &v) {

    osg::Vec2 ang = v*2 - osg::Vec2(1,1);
    osg::Vec2 scth(sin(ang.x()*osg::PI), cos(ang.x()*osg::PI));
    osg::Vec2 scphi(sqrt(1.0 - ang.y()*ang.y()), ang.y());
    osg::Vec3 normal(scth.y()*scphi.x(), scth.x()*scphi.x(), scphi.y());
    return normal;
}
