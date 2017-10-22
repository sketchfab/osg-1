//Helpers to encoe normals
#ifndef _COMPRESSOR_NORMALS
#define _COMPRESSOR_NORMALS

#include <osg/ValueObject> // {set,get}UserValue
#include <osg/Notify>

#include "GeometryUniqueVisitor"

//Encode normals using the octahedron method
osg::Vec2 octWrap(const osg::Vec2 &v );
osg::Vec2 encodeOctahedron(const osg::Vec3 &n);
osg::Vec3 decodeOcthahedron(const osg::Vec2 &encN );

//Encode normals using the spherical coordinate method
osg::Vec2 encodeSphericalCoordinate(const osg::Vec3 &v);
osg::Vec3 decodeSphericalCoordinate(const osg::Vec2 &v);
#endif
