// -*-c++-*-

/*
 * Object File Format (OFF) loader for Open Scene Graph
 *
 * Copyright (C) 2017 Guillaume Bittoun <guillaume.bittoun@gmail.com>
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

#include <fstream>
#include <sstream>
#include <string>

#include <osg/Geometry>
#include <osg/PrimitiveSet>
#include <osgDB/ReaderWriter>
#include <osgDB/Registry>
#include <osg/Vec3>
#include <osg/Vec4>

#define _unused(x) x=x;

class OFFReaderStateMachine
{
    enum OFFReaderState
    {
        ReadingStartSequence,
        ReadingVFECount,  // VFE => Vertices, Faces, Edges
        ReadingVertices,
        ReadingPolygons,
        Success
    };

    OFFReaderState state;

    bool colorSupport;
    unsigned int nVertices;
    unsigned int nFaces;
    unsigned int nEdges;

    std::vector< osg::Vec3 > vertices;
    std::vector< osg::Vec4 > faceColors;
    std::vector< std::vector<unsigned int> > polygons;

    bool isComment(std::string line) const
    {
        if (line.size() > 0)
            return line.c_str()[0] == '#';

        return false;
    }

    void readStartSequence(std::string line)
    {
        if (line != "OFF")
            OSG_NOTICE << "No \"OFF\" begin sequence in OFF file, possibly corrupted file." << std::endl;

        state = ReadingVFECount;
    }

    void readVFECount(std::string line)
    {
        std::istringstream iss(line);

        iss >> nVertices;
        iss >> nFaces;
        iss >> nEdges;

        if (iss.rdstate() == std::ios::goodbit || iss.rdstate() == std::ios::eofbit)
        {
            state = ReadingVertices;
            OSG_NOTICE << "nVertices = " << nVertices << ", nFaces = " << nFaces << ", nEdges = " << nEdges << std::endl;
        }
        else if (line.size() > 0 && !isComment(line))
        {
            OSG_NOTICE << "*** line not handled *** :" << line << std::endl;
        }
    }

    void readVertex(std::string line)
    {
        std::istringstream iss(line);

        float x, y, z;

        iss >> x;
        iss >> y;
        iss >> z;

        if (iss.rdstate() == std::ios::goodbit || iss.rdstate() == std::ios::eofbit)
        {
            OSG_NOTICE << "Vertex read : " << x << ", " << y << ", " << z << std::endl;
            vertices.push_back(osg::Vec3(x, y, z));
        }
        else if (line.size() > 0 && !isComment(line))
        {
            OSG_NOTICE << "*** line not handled *** :" << line << std::endl;
        }

        if (vertices.size() == nVertices)
        {
            OSG_NOTICE << "Vertices have been read correctly, parsing polygons" << std::endl;
            state = ReadingPolygons;
        }
    }

    void readPolygon(std::string line)
    {
        std::istringstream iss(line);

        int polygonEdges;
        std::vector<unsigned int> indices;

        iss >> polygonEdges;

        if (iss.rdstate() == std::ios::goodbit)
        {
            OSG_NOTICE << "Polygon edges count correctly read :" << polygonEdges << std::endl;
        }
        else
        {
            OSG_NOTICE << "*** line not handled *** :" << line << std::endl;
            return;
        }

        for (int i = 0 ; i < polygonEdges ; ++i)
        {
            int vertexIndex;
            iss >> vertexIndex;

            if (vertexIndex < 0 || ((unsigned int)vertexIndex) >= vertices.size())
            {
                OSG_NOTICE << "*** line not handled *** :" << line << std::endl;
                return;
            }

            indices.push_back(vertexIndex);
        }

        if (iss.rdstate() == std::ios::goodbit || iss.rdstate() == std::ios::eofbit)
        {
            OSG_NOTICE << "Polygon indices correctly read" << std::endl;
        }
        else
        {
            OSG_NOTICE << "*** line not handled *** :" << line << std::endl;
            return;
        }

        polygons.push_back(indices);
        if (polygons.size() == nFaces)
            state = Success;

        // Color parsing
        faceColors.push_back(osg::Vec4());  // Default color is black

        float r, g, b;

        iss >> r;
        iss >> g;
        iss >> b;

        if (iss.rdstate() == std::ios::goodbit || iss.rdstate() == std::ios::eofbit)
        {
            OSG_NOTICE << "Face color correctly read" << std::endl;
        }
        else
        {
            OSG_NOTICE << "No color information for face #" << polygons.size() << std::endl;
            return;
        }

        *faceColors.rbegin() = osg::Vec4(r, g, b, 1);
        colorSupport = true;

        float alpha = 0;

        iss >> alpha;

        if (iss.rdstate() == std::ios::goodbit || iss.rdstate() == std::ios::eofbit)
            faceColors.rbegin()->a() = alpha;
    }

    void reset()
    {
        *this = OFFReaderStateMachine();  // Resetting the entire object
    }

public:

    OFFReaderStateMachine() :
        state(),
        colorSupport(false),
        nVertices(0),
        nFaces(0),
        nEdges(0)
    {
    }

    void read(std::istream& fin, const osgDB::ReaderWriter::Options* options)
    {
        _unused(options);  // Avoids warning

        reset();

        while (fin.rdstate() == std::ios::goodbit && state != Success)
        {
            std::string line;
            std::getline(fin, line);

            switch (state)
            {
                case ReadingStartSequence:
                    readStartSequence(line);
                    break;
                case ReadingVFECount:
                    readVFECount(line);
                    break;
                case ReadingVertices:
                    readVertex(line);
                    break;
                case ReadingPolygons:
                    readPolygon(line);
                    break;
                default:
                    OSG_NOTICE << "There seems to be a problem in the OFF parsing" << std::endl;
                    break;
            }
        }
    }

    osgDB::ReaderWriter::ReadResult translateToOSG()
    {
        if (state != Success)
            return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

        osg::Group* root = new osg::Group();
        osg::Geode* meshGeode = new osg::Geode();
        osg::Geometry* meshGeometry = new osg::Geometry();

        meshGeode->addDrawable(meshGeometry);
        root->addChild(meshGeode);

        osg::Vec3Array* geometryVertices = new osg::Vec3Array;
        for (std::vector<osg::Vec3>::iterator itVertex = vertices.begin() ; itVertex != vertices.end() ; ++itVertex)
            geometryVertices->push_back(*itVertex);
        meshGeometry->setVertexArray(geometryVertices);

        // C++11 ? :'-(
        for (std::vector<std::vector<unsigned int> >::iterator itPolygon = polygons.begin() ; itPolygon != polygons.end() ; ++itPolygon)
        {
            std::vector<unsigned int> & polygon = *itPolygon;
            osg::DrawElementsUInt* geometryPrimitive = 0;

            if (polygon.size() == 1)
                geometryPrimitive = new osg::DrawElementsUInt(osg::PrimitiveSet::POINTS);
            else if (polygon.size() == 2)
                geometryPrimitive = new osg::DrawElementsUInt(osg::PrimitiveSet::LINES);
            else if (polygon.size() == 3)
                geometryPrimitive = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
            else if (polygon.size() == 4)
                geometryPrimitive = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS);
            else if (polygon.size() >= 5)
                geometryPrimitive = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON);
            else // No vertex in polygon or really nasty error
            {
                OSG_NOTICE << "Polygon found with invalid number of vertex. This must not happen, aborting mesh load." << std::endl;
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }

            for (std::vector<unsigned int>::iterator itIndex = polygon.begin() ; itIndex != polygon.end() ; ++itIndex)
            {
                geometryPrimitive->push_back(*itIndex);
            }

            meshGeometry->addPrimitiveSet(geometryPrimitive);
        }

        // If color is supported, write faceColor vector
        if (colorSupport)
        {
            osg::Vec4Array* colorArray = new osg::Vec4Array;
            for (std::vector<osg::Vec4>::iterator itColor = faceColors.begin() ; itColor != faceColors.end() ; ++itColor)
                colorArray->push_back(*itColor);
            meshGeometry->setColorArray(colorArray);
            meshGeometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
        }

        return root;
    }
};

class ReaderWriterOFF : public osgDB::ReaderWriter
{
public:

    ReaderWriterOFF()
    {
        supportsExtension("off","Alias Object File Format");
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::ifstream is(fileName.c_str());

        return readNode(is, options);
    }

    virtual ReadResult readNode(std::istream& fin, const Options* options) const
    {
        OFFReaderStateMachine reader;
        reader.read(fin, options);

        return reader.translateToOSG();
    }
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(off, ReaderWriterOFF)
