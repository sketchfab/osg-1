/* -*-c++-*- OpenSceneGraph - Copyright (C) Cedric Pinson
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#include <cmath>
#include <string>
#include <vector>

#include <osg/ValueObject>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/ReadFile>
#include <osgDB/ReaderWriter>
#include <osg/NodeVisitor>
#include <osg/Geometry>

class CleanerVisitor: public osg::NodeVisitor
{
    public:
        // Default constructor
        CleanerVisitor():_processed(),_isValidScene(true){}
        // Default destructor
        virtual ~CleanerVisitor(){}

        void apply(osg::Geometry& geometry)
        {
            if(_processed.find(&geometry) == _processed.end())
            {
                return;
            }
            clean(&geometry);
            _processed.insert(&geometry);
        }

        void clean(osg::Geometry *geometry)
        {
            cleanPrimitiveSets(geometry);

            if(geometry->getNumPrimitiveSets()>0 && !hasInvalidVertices(geometry))
            {
                cleanNormals(geometry);
                cleanUVs(geometry);
            }
        }

        // Detect, log, and dismiss/fix the geometry in case it has invalid vertices (NaN)
        bool hasInvalidVertices(const osg::Geometry *geometry)
        {
            return true;
        }

        // Detect, log, and normalize/create normals in case they are wrong or not existing
        void cleanNormals(const osg::Geometry *geometry)
        {
        }

        // Detect, log and dismiss/fix UVs if not valid (NaNs)
        void cleanUVs(const osg::Geometry *geometry)
        {
        }


        void cleanPrimitiveSets(osg::Geometry *geometry)
        {
            osg::Geometry::PrimitiveSetList validPrimitives;
            for(unsigned int i = 0; i <= geometry->getNumPrimitiveSets(); ++i)
            {
                osg::PrimitiveSet* primitive = geometry->getPrimitiveSet(i);
                if(primitive && primitive->getNumIndices())
                    validPrimitives.push_back(primitive);
                else
                    OSG_WARN << "PrimitiveSet Error: geometry.empty_primitiveset" << std::endl;
            }

            geometry->setPrimitiveSetList(validPrimitives);
        }

        bool isValidScene() { return _isValidScene; }
    private:
        std::set<osg::Geometry*> _processed;
        bool _isValidScene;
};



class ReaderWriterCleaner : public osgDB::ReaderWriter
{

  public:

    ReaderWriterCleaner()
    {
        supportsExtension("cleaner", "Pseudo-loader to clean geometries.");
    }

    virtual const char* className() const { return "ReaderWriterCleaner"; }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;
        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) 
            return ReadResult::FILE_NOT_FOUND;

        // recursively load the subfile.
        osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(fileName, options);
        if (!node.valid())
        {
            // propagate the read failure upwards
            OSG_WARN << "file: " << fileName << "not found" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }
        // clean attributes
        CleanerVisitor cleaner;
        node->accept(cleaner);

        if (!cleaner.isValidScene())
        {
            OSG_WARN << "Asset \"" << fileName << "\" is not a valid scene" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        return node.release();
    }

};

// Add the plugin to the Registry
REGISTER_OSGPLUGIN(cleaner, ReaderWriterCleaner)
