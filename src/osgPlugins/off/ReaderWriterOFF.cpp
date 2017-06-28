#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdlib.h>
#include <limits>
#include <string>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Vec3f>

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

#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/Tessellator>

#include "Utils.h"

class ReaderWriterOFF : public osgDB::ReaderWriter
{
public:
    ReaderWriterOFF()
    {
        supportsExtension("off", "Geomview Object File Format");
    }

    virtual const char* className() const { return "OFF Reader"; }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;

    virtual ReadResult readNode(std::istream& fin, const Options* options) const;

protected:
	enum class ParsingState
	{
		MagicNumber,
		Counters,
		Vertices,
		Polygons,
		Finished
	};

	struct Polygon
	{
		std::vector<size_t>	indices;
		osg::Vec4f			color;
	};
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(off, ReaderWriterOFF)

// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterOFF::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext))
		return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile(file, options);
    if (fileName.empty())
		return ReadResult::FILE_NOT_FOUND;

    osgDB::ifstream fin(fileName.c_str());

    return readNode(fin, options);
}

osgDB::ReaderWriter::ReadResult ReaderWriterOFF::readNode(std::istream& input, const Options* options) const
{
    if (!input)
		return ReadResult::FILE_NOT_HANDLED;

	static std::vector<char>	wordsSeparators = {' ', '\t'};

	std::vector<std::string>	lines;
	std::vector<std::string>	words;

	utils::readLines(input, lines);

	ParsingState	state = ParsingState::MagicNumber;

	int		scanfResult;
	int		verticesCount = 0;
	int		facesCount = 0;
	int		edgesCount = 0;
	int		indicesCount = 0;
	bool	colorByFaces = false;

	osg::Vec3f		vector3;
	osg::Vec4f		vector4;

	osg::Vec3Array*			sharedVertices = new osg::Vec3Array;
	osg::Vec4Array*			sharedColors = new osg::Vec4Array;
	std::vector<Polygon>	polygons;

	for (std::string& line : lines)
	{
		switch (state)
		{
		case ParsingState::MagicNumber:
			if (line == "OFF")
				state = ParsingState::Counters;
			else if (sscanf(line.c_str(), "OFF %d %d %d", &verticesCount, &facesCount, &edgesCount))	// Special case for socket.off file
			{
				sharedVertices->reserve(verticesCount);
				sharedColors->reserve(verticesCount);
				polygons.reserve(facesCount);
				state = ParsingState::Vertices;
			}
			break;
		case ParsingState::Counters:
			if (utils::isACommentOrEmptyLine(line))
				break;
			scanfResult = sscanf(line.c_str(), "%d %d %d", &verticesCount, &facesCount, &edgesCount);
			if (scanfResult == 3)
			{
				sharedVertices->reserve(verticesCount);
				sharedColors->reserve(verticesCount);
				polygons.reserve(facesCount);
				state = ParsingState::Vertices;
			}
			break;
		case ParsingState::Vertices:
			if (utils::isACommentOrEmptyLine(line))
				break;
			scanfResult = sscanf(line.c_str(), "%f %f %f %f %f %f %f", &vector3.x(), &vector3.y(), &vector3.z(), &vector4.r(), &vector4.g(), &vector4.b(), &vector4.a());
			if (scanfResult == 3)	// Vertex
				sharedVertices->push_back(vector3);
			else if (scanfResult == 3 + 3)	// Vertex + RGB color
			{
				sharedVertices->push_back(vector3);
				vector4.a() = 1.0f;
				sharedColors->push_back(vector4);
			}
			else if (scanfResult == 3 + 4)	// Vertex + RGBA color
			{
				sharedVertices->push_back(vector3);
				sharedColors->push_back(vector4);
			}

			if ((int)sharedVertices->getNumElements() == verticesCount
				&& (sharedColors->getNumElements() == 0
					|| (int)sharedColors->getNumElements() == verticesCount))
				state = ParsingState::Polygons;
			break;
		case ParsingState::Polygons:
		{
			if (utils::isACommentOrEmptyLine(line))
				break;
			words.resize(0);	// resize instead of clear because Memory stay allocated, it will reduce the number of allocations made by consecutives calls of utils::explodeString
			utils::explodeString(line, wordsSeparators, words);
			if (words.empty())
				break;

			scanfResult = sscanf(words[0].c_str(), "%d", &indicesCount);
			if (scanfResult != 1
				|| !((int)words.size() == 1 + indicesCount
					|| (int)words.size() == 1 + indicesCount + 3
					|| (int)words.size() == 1 + indicesCount + 4))
				break;

			int	nbColorChanels = words.size() - (indicesCount + 1);

			polygons.push_back(Polygon());
			std::vector<size_t>&	indices = polygons[polygons.size() - 1].indices;
			bool					outOfRangeIndice = false;
			int						index;
			float					colorChanelValue;

			indices.reserve(indicesCount);

			// Extracting vertices indices
			for (int i = 1; i < 1 + indicesCount; i++)
				if (sscanf(words[i].c_str(), "%d", &index) == 1)
				{
					if (index >= 0 && index < verticesCount)
						indices.push_back(index);
					else
						outOfRangeIndice = true;
				}

			// Extracting color of the face
			for (int i = 1 + indicesCount; i < 1 + indicesCount + nbColorChanels; i++)
				if (sscanf(words[i].c_str(), "%f", &colorChanelValue) == 1)
					vector4[i - (1 + indicesCount)] = colorChanelValue;

			// Applying color to the polygon
			if ((int)words.size() > 1 + indicesCount
				&& outOfRangeIndice == false)
			{
				colorByFaces = true;
				if (nbColorChanels == 3)
					vector4.a() = 1.0f;
				polygons[polygons.size() - 1].color = vector4;
			}

			if ((int)indices.size() != indicesCount	// parsing failed
				&& outOfRangeIndice == false)
				polygons.resize(polygons.size() - 1);	// Discarding this line result

			if ((int)polygons.size() == facesCount)
				state = ParsingState::Finished;
			break;
		}
		}
	}

	if (state != ParsingState::Finished)
		return ReadResult::ERROR_IN_READING_FILE;

	bool			hasColor = sharedColors->size() || colorByFaces;
	osg::Group*		group = new osg::Group;
	osg::Geode*		geode = new osg::Geode;
	osg::Geometry*	geometry = new osg::Geometry;
	osg::Vec3Array*	vertices = new osg::Vec3Array;
	osg::Vec4Array*	colors = new osg::Vec4Array;

	// Build the objects hierachy
	geode->addDrawable(geometry);
	group->addChild(geode);
	// --

	// Fill the geometry
	geometry->setVertexArray(vertices);
	if (hasColor)
	{
		geometry->setColorArray(colors);
		geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}

	int	startPos = 0;
	for (Polygon polygon : polygons)
	{
		vertices->reserve(vertices->getNumElements() + polygon.indices.size());
		if (hasColor)
			colors->reserve(colors->getNumElements() + polygon.indices.size());
		for (size_t index : polygon.indices)
		{
			vertices->push_back((*sharedVertices)[index]);
			if (sharedColors->size())
				colors->push_back((*sharedColors)[index]);
			else if (colorByFaces)
				colors->push_back(polygon.color);
		}

		osg::DrawArrays*	drawArrays = new osg::DrawArrays(GL_POLYGON, startPos, polygon.indices.size());

		geometry->addPrimitiveSet(drawArrays);

		startPos += polygon.indices.size();
	}

	// Geometry post processing
	osgUtil::Tessellator tessellator;
	tessellator.setTessellationType(osgUtil::Tessellator::TessellationType::TESS_TYPE_DRAWABLE);
	tessellator.retessellatePolygons(*geometry);

	osgUtil::TriStripVisitor tsv;
	tsv.stripify(*geometry);
	// --

	return group;
}
