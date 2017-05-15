// -*-c++-*-

/*
 * OFF (Open File Format ) loader for Open Scene Graph
 *
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

 #ifndef OFF_WRITER_NODE_VISITOR_HEADER__
 #define OFF_WRITER_NODE_VISITOR_HEADER__


#include <string>
#include <stack>
#include <sstream>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <map>
#include <set>

#include "off.h"



 /*-------------------------------------------------------------------------------\
 |                             ValueVisitor				                         |
 \-------------------------------------------------------------------------------*/
 // writes all values of an array out to a stream, applies a matrix beforehand if necessary
class ValueVisitor : public osg::ValueVisitor
{
private:
	osg::Matrix		_m;									//world transform
	bool			_applyMatrix;						//to know if is necessary to use matrix transform.

	bool			_isNormal;							//custom position gesture for normals
	osg::Vec3		_origin;

	size_t			_forceVecX;							//if != -1, force to display as a VecX
	osg::Vec4		_currentValue;


public:
	ValueVisitor(const osg::Matrix& m = osg::Matrix::identity()) :
		osg::ValueVisitor(),
		_m(m)
	{
		_applyMatrix = (_m != osg::Matrix::identity());
		_isNormal = false;
		_origin = osg::Vec3(0, 0, 0);

		_forceVecX = (size_t)-1;
		_currentValue = osg::Vec4(0, 0, 0, 1);
	}

	
	void	setIsNormal(bool isNormal)
	{
		if (_isNormal == isNormal)
			return;
		_isNormal = isNormal;

		_origin = ((_isNormal) ? (osg::Vec3(0, 0, 0) * _m) : osg::Vec3(0, 0, 0));
	}

	void	unforceVecX() { _forceVecX = (size_t)-1; }
	void	forceVecX(size_t dim) { _forceVecX = dim; }

	void	display(size_t dim, float x = 0.0, float y = 0.0, float z = 0.0, float w = 1.0)
	{
		size_t dimToDisplay = (_forceVecX != (size_t)-1) ? _forceVecX : dim;
		_currentValue = osg::Vec4(0, 0, 0, 1);

		_currentValue[0] = x;
		if (dimToDisplay>1)
			_currentValue[1] = y;
		if (dimToDisplay>2)
			_currentValue[2] = z;
		if (dimToDisplay>3)
			_currentValue[3] = w;
	}

	osg::Vec4 getCurrentValue() { return _currentValue; }


	virtual void apply(osg::Vec2 & inv) { display(2, inv[0], inv[1]); }
	virtual void apply(osg::Vec2b & inv) { display(2, inv[0], inv[1]); }
	virtual void apply(osg::Vec2s & inv) { display(2, inv[0], inv[1]); }

	virtual void apply(osg::Vec3 & inv)
	{
		osg::Vec3 v(inv);
		if (_applyMatrix)
			v = (v * _m) - _origin;
		display(3, v[0], v[1], v[2]);
	}

	virtual void apply(osg::Vec3b & inv)
	{
		osg::Vec3 v(inv[0], inv[1], inv[2]);
		if (_applyMatrix)
			v = (v * _m) - _origin;
		display(3, v[0], v[1], v[2]);
	}

	virtual void apply(osg::Vec3s & inv)
	{
		osg::Vec3 v(inv[0], inv[1], inv[2]);
		if (_applyMatrix)
			v = (v * _m) - _origin;
		display(3, v[0], v[1], v[2]);
	}

	virtual void apply(osg::Vec4 & inv) { display(4, inv[0], inv[1], inv[2], inv[3]); }
	virtual void apply(osg::Vec4b & inv) { display(4, inv[0], inv[1], inv[2], inv[3]); }
	virtual void apply(osg::Vec4s & inv) { display(4, inv[0], inv[1], inv[2], inv[3]); }

private:
	ValueVisitor& operator = (const ValueVisitor&) { return *this; }
};


 /*-------------------------------------------------------------------------------\
 |                             OffPrimitiveIndexWriter	                         |
 \-------------------------------------------------------------------------------*/
 // writes all primitives of a primitive-set out to a stream, decomposes quads to triangles, line-strips to lines etc
class OffPrimitiveIndexWriter : public osg::PrimitiveIndexFunctor
{
private:
	off::Model* mOff;
	osg::Geometry* mGeo;
	osg::Array* mVertices;
	osg::Array* mNormals;
	osg::Array* mTexcoords;
	osg::Array* mColors;

	size_t mOsg_startVertexIndex;
	size_t mOsg_startNormalIndex;
	size_t mOsg_startTexCoordIndex;
	size_t mOsg_startColorIndex;

	size_t mFaceNormalIndex;

	GLenum               _modeCache;
	std::vector<GLuint>  _indexCache;

	ValueVisitor mVv;									//custom visitor for arrays 



public:
	OffPrimitiveIndexWriter(osg::Geometry* geo, off::Model* off, osg::Array* vertices, osg::Array* normals, osg::Array* texcoords, osg::Array* colors, osg::Matrix& m) :
		osg::PrimitiveIndexFunctor(),
		_modeCache(0),
		mGeo(geo),
		mOff(off),
		mVertices(vertices),
		mNormals(normals),
		mTexcoords(texcoords),
		mColors(colors),
		mVv(m)
	{
		mFaceNormalIndex = 0;
	}

	void	notifyNextPrimitive(size_t primitiveIndex) { mFaceNormalIndex = primitiveIndex; }

	virtual void drawArrays(GLenum mode, GLint first, GLsizei count);
	virtual void drawElements(GLenum mode, GLsizei count, const GLubyte* indices) { drawElementsImplementation<GLubyte>(mode, count, indices); }
	virtual void drawElements(GLenum mode, GLsizei count, const GLushort* indices) { drawElementsImplementation<GLushort>(mode, count, indices); }
	virtual void drawElements(GLenum mode, GLsizei count, const GLuint* indices) { drawElementsImplementation<GLuint>(mode, count, indices); }
	
	void writeTriangle(unsigned int i1, unsigned int i2, unsigned int i3);	// operator for triangles
	void writeLine(unsigned int i1, unsigned int i2);		// operator for lines
	void writePoint(unsigned int i1);						// operator for points
	size_t mergeVertex(size_t i);							// try to add new Vertex or get old similar vertex.

	virtual void begin(GLenum mode);
	virtual void vertex(unsigned int vert);
	virtual void end();

	virtual void setVertexArray(unsigned int, const osg::Vec2*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec2d*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec3*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec3d*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec4*) {}
	virtual void setVertexArray(unsigned int, const osg::Vec4d*) {}

protected:
	template<typename T>void drawElementsImplementation(GLenum mode, GLsizei count, const T* indices);

private:
	OffPrimitiveIndexWriter& operator = (const OffPrimitiveIndexWriter&) { return *this; }
};




/*-------------------------------------------------------------------------------\
|                             OFFWriterNodeVisitor		                         |
\-------------------------------------------------------------------------------*/
class OFFWriterNodeVisitor: public osg::NodeVisitor
{
private:
	std::ostream& _fout;
	off::Model mOff;								// if there is many geometries, Off don't have Node way, and fout can't be writen by insert lines, so we only could merge geometries into off::Model (merge vertices and faces lists), before writing in fout.

public:
	OFFWriterNodeVisitor(std::ostream& fout) :
		osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
		_fout(fout)
	{

	}

	virtual	~OFFWriterNodeVisitor(void) { }


	virtual	void	apply(osg::Geode &node);
	void	traverse(osg::Node &node);
	virtual	void	apply(osg::Group &node);

	void	writeOFF(bool binary = false);

protected:
	void processGeometry(osg::Geometry* geo, osg::Matrix& m);
	
	OFFWriterNodeVisitor& operator = (const OFFWriterNodeVisitor&) { return *this; }
};

#endif
