/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield
 *
 *
 *	OFF (Open File Format) implementation.
 *	Limitation:
 *	only focus on 3 dimensions (+ optionally W/4 component).
 *	
 *
 *	Notice: 
 *	I didn't find some examples for full informations on vertex like specs talk about, or also for binary version.
 *	So binary version is not really tested by real data.
 *	Full description ASCII version have been accepted by meshLab, so I think we are good on that.
 *	
 *	Another problem is about reexport by MeshLab :
 *	-it's not the right order in file description and vertex datas order, as specs description.
 *	-there is "nan" or "-nan"
 *	So some exported file from meshLab could not be load correctly.
 *
 *
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef OFF_H
#define OFF_H

#include <string>
#include <vector>
#include <map>
#include <istream>

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>

#include <osgDB/ReaderWriter>






namespace off
{

/*-------------------------------------------------------------------------------\
|                             Model					                             |
\-------------------------------------------------------------------------------*/
class Model
{
public:

	struct Vertex
	{
		// Notice: the "Ndim components" for position if only implemented for [1-4]  (fill by 0.0f on YZ, and 1.0f for w (uniform))
		osg::Vec4 position;
		osg::Vec4 normal;
		osg::Vec4 color;
		osg::Vec2 texcoord;

		Vertex(void)
		{
			position = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			normal = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			color = osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f);
			texcoord = osg::Vec2(0.0f, 0.0f);
		}

		bool operator==(Vertex &vertex) { return ((position==vertex.position)&& (normal == vertex.normal)&& (color == vertex.color)&& (texcoord == vertex.texcoord));  }

	};

	struct Face
	{
		std::vector<size_t> vertexIndexList;
		osg::Vec4 color;
		size_t colormapIndex;

		Face(void)
		{
			color = osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f);
			colormapIndex = (size_t)-1;
		}
	};


	std::vector< Vertex > mVertexArray;
	std::vector< Face >	mFaceArray;

	std::string databasePath;
	bool mhaveVertexColors;
	bool mhaveVertexNormals;
	bool mhaveVertexTextureCoordinates;
	bool mhaveFaceColors;
	size_t mNumDim;

	





public:
	Model(void);
	virtual ~Model(void);

	void	init(void);
	void	setDatabasePath(const std::string& path) { databasePath = path; }
	const std::string&	getDatabasePath() const { return databasePath; }

	bool	readOFF(std::istream& fin, const osgDB::ReaderWriter::Options* options, bool &isBinary);
	bool	readOFF_binary(std::istream& fin, const osgDB::ReaderWriter::Options* options);
	bool	writeOFF(std::ostream& fout, bool binary = false);
	bool	writeOFF_binary(std::ostream& fout);
	
	size_t	merge(Vertex &vertex);							//merge vertex and return index: copy if don't exist (on all values). or give the index.
private:
	bool	readline(std::istream& fin, char* line, const int LINE_SIZE, bool isBinary = false);
};

}

#endif
