// -*-c++-*-

/*
 * OFF (Open File Format) loader for Open Scene Graph
 *
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#include <osg/io_utils>
#include "OFFWriterNodeVisitor.h"

/*-------------------------------------------------------------------------------\
|                             writeOFF					                         |
\-------------------------------------------------------------------------------*/
void OFFWriterNodeVisitor::writeOFF(bool binary)
{
	mOff.writeOFF(_fout, binary);
}
/*-------------------------------------------------------------------------------\
|                             apply					                             |
\-------------------------------------------------------------------------------*/
void OFFWriterNodeVisitor::apply(osg::Group &node)
{
	traverse(node);
}
/*-------------------------------------------------------------------------------\
|                             traverse				                             |
\-------------------------------------------------------------------------------*/
void OFFWriterNodeVisitor::traverse(osg::Node &node)
{
	osg::NodeVisitor::traverse(node);
}
/*-------------------------------------------------------------------------------\
|                             apply					                             |
\-------------------------------------------------------------------------------*/
void OFFWriterNodeVisitor::apply(osg::Geode &node)
{
	osg::Matrix m = osg::computeLocalToWorld(getNodePath());					//Node transformation
	osg::Geometry* geo;

	size_t count = node.getNumDrawables();
	for (size_t i = 0; i < count; i++)
	{
		geo = node.getDrawable(i)->asGeometry();
		if (geo == NULL)
			continue;

		processGeometry(geo, m);
	}
}
/*-------------------------------------------------------------------------------\
|                             processGeometry		                             |
\-------------------------------------------------------------------------------*/
void OFFWriterNodeVisitor::processGeometry(osg::Geometry* geo, osg::Matrix& m)
{
	if (geo->containsDeprecatedData())
		geo->fixDeprecatedData();


	osg::Array* vertices = geo->getVertexArray();
	osg::Array* normals = geo->getNormalArray();
	osg::Array* texcoords = geo->getTexCoordArray(0);							// OFF doesn't support more than one textureCoordinates.
	osg::Array* colors = geo->getColorArray();

	if ((!vertices)||(geo->getNumPrimitiveSets()==0))
		return;

	if (colors)
		mOff.mhaveVertexColors = true;
	if (normals)
		mOff.mhaveVertexNormals = true;
	if (texcoords)
		mOff.mhaveVertexTextureCoordinates = true;




	OffPrimitiveIndexWriter pif(geo, &mOff, vertices, normals, texcoords, colors, m);
	osg::PrimitiveSet* ps;
	

	size_t nbPrimitives = geo->getNumPrimitiveSets();
	for (size_t i = 0; i < nbPrimitives; ++i)
	{
		ps = geo->getPrimitiveSet(i);

		pif.notifyNextPrimitive(i);

		ps->accept(pif);														//the Visitor of Primitives will complete the off::Model.
	}
}


/*-------------------------------------------------------------------------------\
|                             drawArrays			                             |
\-------------------------------------------------------------------------------*/
void OffPrimitiveIndexWriter::drawArrays(GLenum mode, GLint first, GLsizei count)
{
	switch (mode)
	{

	case(GL_TRIANGLES):
	{
		unsigned int pos = first;
		for (GLsizei i = 2; i<count; i += 3, pos += 3)
			writeTriangle(pos, pos + 1, pos + 2);
	}
	break;


	case(GL_TRIANGLE_STRIP):
	{
		unsigned int pos = first;
		for (GLsizei i = 2; i<count; ++i, ++pos)
		{
			if ((i % 2))
				writeTriangle(pos, pos + 2, pos + 1);
			else
				writeTriangle(pos, pos + 1, pos + 2);
		}
	}
	break;

	case(GL_QUADS):
	{
		unsigned int pos = first;
		for (GLsizei i = 3; i<count; i += 4, pos += 4)
		{
			writeTriangle(pos, pos + 1, pos + 2);
			writeTriangle(pos, pos + 2, pos + 3);
		}
	}
	break;

	case(GL_QUAD_STRIP):
	{
		unsigned int pos = first;
		for (GLsizei i = 3; i<count; i += 2, pos += 2)
		{
			writeTriangle(pos, pos + 1, pos + 2);
			writeTriangle(pos + 1, pos + 3, pos + 2);
		}
	}
	break;

	case(GL_POLYGON):																// treat polygons as GL_TRIANGLE_FAN
	case(GL_TRIANGLE_FAN):
	{
		unsigned int pos = first + 1;
		for (GLsizei i = 2; i<count; ++i, ++pos)
			writeTriangle(first, pos, pos + 1);
		
	}
	break;

	case(GL_LINES):
	{
		for (GLsizei i = 0; i<count; i += 2)
			writeLine(i, i + 1);
	}
	break;


	case(GL_LINE_STRIP):
	{
		for (GLsizei i = 1; i<count; ++i)
			writeLine(i - 1, i);
	}
	break;

	case(GL_LINE_LOOP):
	{
		for (GLsizei i = 1; i<count; ++i)
			writeLine(i - 1, i);
		
		writeLine(count - 1, 0);
		
	}
	break;


	case(GL_POINTS):
	{
		for (GLsizei i = 0; i<count; ++i)
			writePoint(i);
	}
	break;

	default:
		OSG_WARN << "OFFWriterNodeVisitor :: can't handle mode " << mode << std::endl;
		break;
	}
}
/*-------------------------------------------------------------------------------\
|                             drawElementsImplementation                         |
\-------------------------------------------------------------------------------*/
template<typename T>void OffPrimitiveIndexWriter::drawElementsImplementation(GLenum mode, GLsizei count, const T* indices)
{
	if (indices == 0 || count == 0) return;

	typedef const T* IndexPointer;

	switch (mode)
	{

	case(GL_TRIANGLES):
	{
		IndexPointer ilast = &indices[count];
		for (IndexPointer iptr = indices; iptr<ilast; iptr += 3)
			writeTriangle(*iptr, *(iptr + 1), *(iptr + 2));
	}
	break;

	case(GL_TRIANGLE_STRIP):
	{
		IndexPointer iptr = indices;
		for (GLsizei i = 2; i<count; ++i, ++iptr)
		{
			if ((i % 2))
				writeTriangle(*(iptr), *(iptr + 2), *(iptr + 1));
			else
				writeTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
		}	
	}
	break;

	case(GL_QUADS):
	{
		IndexPointer iptr = indices;
		for (GLsizei i = 3; i<count; i += 4, iptr += 4)
		{
			writeTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
			writeTriangle(*(iptr), *(iptr + 2), *(iptr + 3));
		}
	}
	break;


	case(GL_QUAD_STRIP):
	{
		IndexPointer iptr = indices;
		for (GLsizei i = 3; i<count; i += 2, iptr += 2)
		{
			writeTriangle(*(iptr), *(iptr + 1), *(iptr + 2));
			writeTriangle(*(iptr + 1), *(iptr + 3), *(iptr + 2));
		}	
	}
	break;



	case(GL_POLYGON):																// treat polygons as GL_TRIANGLE_FAN
	case(GL_TRIANGLE_FAN):
	{
		IndexPointer iptr = indices;
		unsigned int first = *iptr;
		++iptr;
		for (GLsizei i = 2; i<count; ++i, ++iptr)
			writeTriangle(first, *(iptr), *(iptr + 1));
	}
	break;


	case(GL_LINES):
	{
		IndexPointer ilast = &indices[count];
		for (IndexPointer iptr = indices; iptr<ilast; iptr += 2)
			writeLine(*iptr, *(iptr + 1));
		
	}
	break;


	case(GL_LINE_STRIP):
	{

		IndexPointer ilast = &indices[count];
		for (IndexPointer iptr = indices + 1; iptr<ilast; iptr += 2)
			writeLine(*(iptr - 1), *iptr);
	}
	break;


	case(GL_LINE_LOOP):
	{
		IndexPointer ilast = &indices[count];
		for (IndexPointer iptr = indices + 1; iptr<ilast; iptr += 2)
			writeLine(*(iptr - 1), *iptr);

		writeLine(*ilast, *indices);
	}
	break;


	case(GL_POINTS):
	{
		IndexPointer ilast = &indices[count];
		for (IndexPointer iptr = indices; iptr<ilast; ++iptr)
			writePoint(*iptr);
	}
	break;

	default: break;
	}
}
/*-------------------------------------------------------------------------------\
|                             writeTriangle				                         |
\-------------------------------------------------------------------------------*/
// operator for triangles
void OffPrimitiveIndexWriter::writeTriangle(unsigned int i1, unsigned int i2, unsigned int i3)
{
	off::Model::Face face;
	
	size_t i1_new = mergeVertex(i1);
	size_t i2_new = mergeVertex(i2);
	size_t i3_new = mergeVertex(i3);

	face.vertexIndexList.push_back(i1_new);
	face.vertexIndexList.push_back(i2_new);
	face.vertexIndexList.push_back(i3_new);

	mOff->mFaceArray.push_back(face);
}
/*-------------------------------------------------------------------------------\
|                             writeLine					                         |
\-------------------------------------------------------------------------------*/
// operator for lines
void OffPrimitiveIndexWriter::writeLine(unsigned int i1, unsigned int i2)
{
	off::Model::Face face;

	size_t i1_new = mergeVertex(i1);
	size_t i2_new = mergeVertex(i2);

	face.vertexIndexList.push_back(i1_new);
	face.vertexIndexList.push_back(i2_new);

	mOff->mFaceArray.push_back(face);
}
/*-------------------------------------------------------------------------------\
|                             writePoint				                         |
\-------------------------------------------------------------------------------*/
// operator for points
void OffPrimitiveIndexWriter::writePoint(unsigned int i1)
{
	off::Model::Face face;

	size_t i1_new = mergeVertex(i1);

	face.vertexIndexList.push_back(i1_new);

	mOff->mFaceArray.push_back(face);
}
/*-------------------------------------------------------------------------------\
|                             mergeVertex				                         |
\-------------------------------------------------------------------------------*/
size_t OffPrimitiveIndexWriter::mergeVertex(size_t i)
{	
	//build the result Vertex informations.
	off::Model::Vertex vertex;

	mVv.forceVecX(3);
	mVertices->accept(i, mVv);
	vertex.position = mVv.getCurrentValue();
	
	if(mNormals)
	{
		size_t indexNormal = (osg::getBinding(mNormals) == osg::Array::BIND_PER_VERTEX) ? i : mFaceNormalIndex;		//the opposite is one by face
		
		if (indexNormal < mNormals->getNumElements())
		{
			mVv.forceVecX(3);
			mVv.setIsNormal(true);
			mNormals->accept(indexNormal, mVv);
			vertex.normal = mVv.getCurrentValue();

			mVv.setIsNormal(false);
		}
	}

	if (mColors)
	{
		size_t indexColor = (osg::getBinding(mColors) == osg::Array::BIND_PER_VERTEX) ? i : mFaceNormalIndex;		//the opposite is one by face
		
		if (indexColor < mColors->getNumElements())
		{
			mVv.forceVecX(4);
			mColors->accept(indexColor, mVv);
			vertex.color = mVv.getCurrentValue();
		}
	}

	if (mTexcoords)
	{
		size_t indexTextCoord = (osg::getBinding(mTexcoords) == osg::Array::BIND_PER_VERTEX) ? i : mFaceNormalIndex;		//the opposite is one by face
		
		if (indexTextCoord < mTexcoords->getNumElements())
		{
			mVv.forceVecX(2);
			mTexcoords->accept(indexTextCoord, mVv);
			vertex.texcoord = osg::Vec2(mVv.getCurrentValue().x(), mVv.getCurrentValue().y());
		}
	}

	return mOff->merge(vertex);													//merge by comparing values. this methode will reduce number of copy of vertex, if possible.
}
/*-------------------------------------------------------------------------------\
|                             begin						                         |
\-------------------------------------------------------------------------------*/
void OffPrimitiveIndexWriter::begin(GLenum mode)
{
	_modeCache = mode;
	_indexCache.clear();
}
/*-------------------------------------------------------------------------------\
|                             begin						                         |
\-------------------------------------------------------------------------------*/
void OffPrimitiveIndexWriter::vertex(unsigned int vert)
{
	_indexCache.push_back(vert);
}
/*-------------------------------------------------------------------------------\
|                             begin						                         |
\-------------------------------------------------------------------------------*/
void OffPrimitiveIndexWriter::end()
{
	if (!_indexCache.empty())
		drawElements(_modeCache, _indexCache.size(), &_indexCache.front());
}



