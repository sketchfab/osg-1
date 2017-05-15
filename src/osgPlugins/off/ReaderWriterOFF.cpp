// -*-c++-*-

/*
 * OFF (Open File Format) loader for Open Scene Graph
 *
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

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

#include "off.h"
#include "OFFWriterNodeVisitor.h"

#include <map>
#include <set>





/*-------------------------------------------------------------------------------\
|                             ReaderWriterOFF		                             |
\-------------------------------------------------------------------------------*/
class ReaderWriterOFF : public osgDB::ReaderWriter
{

protected:
	struct OffOptionsStruct
	{
		bool outputBinary;
		int precision;


		OffOptionsStruct(void)
		{
			outputBinary = false;

			precision = std::numeric_limits<double>::digits10 + 2;
		}
	};


public:
	ReaderWriterOFF(void);
	virtual ~ReaderWriterOFF(void) {};

    virtual const char* className() const { return "OFF Reader"; }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;
    virtual ReadResult readNode(std::istream& fin, const Options* options) const;
	virtual WriteResult writeObject(const osg::Object& obj, const std::string& fileName, const Options* options = NULL) const;
	virtual WriteResult writeObject(const osg::Object& obj, std::ostream& fout, const Options* options = NULL) const;
	virtual WriteResult writeNode(const osg::Node& node,	const std::string& fileName, const Options* options = NULL) const;
	virtual WriteResult writeNode(const osg::Node& node,	std::ostream& fout, const Options* options = NULL) const;



protected:
	osg::Node* convertModelToSceneGraph(off::Model& model, OffOptionsStruct& localOptions, const Options* options) const;
	osg::Geometry* convertElementListToGeometry(off::Model& model, OffOptionsStruct& localOptions) const;
	
	OffOptionsStruct parseOptions(const Options* options) const;
	inline osg::Vec3 transformVec4(const osg::Vec4& vec) const;
};


// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(off, ReaderWriterOFF)















/*-------------------------------------------------------------------------------\
|                             ReaderWriterOFF		                             |
\-------------------------------------------------------------------------------*/
ReaderWriterOFF::ReaderWriterOFF(void)
{
	supportsExtension("off", "Alias OFF (Open File Format) format");

	supportsOption("binarySave", "if ouput is OFF file, could be in binary format.");
	supportsOption("precision=<digits>", "Set the floating point precision when writing out files");
}
/*-------------------------------------------------------------------------------\
|                             parseOptions				                         |
\-------------------------------------------------------------------------------*/
ReaderWriterOFF::OffOptionsStruct ReaderWriterOFF::parseOptions(const osgDB::ReaderWriter::Options* options) const
{
	OffOptionsStruct localOptions;
	

	if (options != NULL)
	{
		std::istringstream iss(options->getOptionString());
		std::string opt;
		while (iss >> opt)
		{
			// split opt into pre= and post=
			std::string pre_equals;
			std::string post_equals;

			size_t found = opt.find("=");
			if (found != std::string::npos)
			{
				pre_equals = opt.substr(0, found);
				post_equals = opt.substr(found + 1);
			} else {
				pre_equals = opt;
			}


			if (pre_equals == "binarySave")
			{
				localOptions.outputBinary = true;

			}else if (pre_equals == "precision"){

				int val = std::atoi(post_equals.c_str());
				if (val <= 0) {
					OSG_NOTICE << "Warning: invalid precision value: " << post_equals << std::endl;
				}
				else {
					localOptions.precision = val;
				}
			}

		}
	}	

	return localOptions;
}



/*-------------------------------------------------------------------------------\
|                             readNode				                             |
\-------------------------------------------------------------------------------*/
// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult ReaderWriterOFF::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
	std::string ext = osgDB::getLowerCaseFileExtension(file);
	if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

	std::string fileName = osgDB::findDataFile(file, options);
	if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;


	osgDB::ifstream fin(fileName.c_str());
	if (fin)
	{
		// code for setting up the database path so that internally referenced file are searched for on relative paths.
		osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
		local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

		off::Model model;
		model.setDatabasePath(osgDB::getFilePath(fileName.c_str()));
		bool isBinary = false;

		if((!model.readOFF(fin, local_opt.get(), isBinary)) && (isBinary))		//it's a binary file, need a fin open with binary option.
		{
			fin.close();

			osgDB::ifstream fin_bin(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
			model.readOFF_binary(fin_bin, local_opt.get());
		}

		OffOptionsStruct localOptions = parseOptions(options);

		osg::Node* node = convertModelToSceneGraph(model, localOptions, local_opt.get());
		return node;
	}

	return ReadResult::FILE_NOT_HANDLED;
}
/*-------------------------------------------------------------------------------\
|                             readNode				                             |
\-------------------------------------------------------------------------------*/
osgDB::ReaderWriter::ReadResult ReaderWriterOFF::readNode(std::istream& fin, const Options* options) const
{
	if (fin)
	{
		fin.imbue(std::locale::classic());

		off::Model model;
		bool isBinary = false;

		if ((!model.readOFF(fin, options, isBinary)) && (isBinary))					//it's a binary file, need a fin open with binary option.
		{
			//todo : correct this.
			//osgDB::ifstream fin_bin(fin..c_str(), std::ios_base::in | std::ios_base::binary);
			//model.readOFF_binary(fin_bin, local_opt.get());
		}

		OffOptionsStruct localOptions = parseOptions(options);

		osg::Node* node = convertModelToSceneGraph(model, localOptions, options);
		return node;
	}

	return ReadResult::FILE_NOT_HANDLED;
}
/*-------------------------------------------------------------------------------\
|                             writeObject			                             |
\-------------------------------------------------------------------------------*/
osgDB::ReaderWriter::WriteResult ReaderWriterOFF::writeObject(const osg::Object& obj, const std::string& fileName, const Options* options) const
{
	const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
	if (node)
		return writeNode(*node, fileName, options);
	else
		return WriteResult(WriteResult::FILE_NOT_HANDLED);
}
/*-------------------------------------------------------------------------------\
|                             writeObject			                             |
\-------------------------------------------------------------------------------*/
osgDB::ReaderWriter::WriteResult ReaderWriterOFF::writeObject(const osg::Object& obj, std::ostream& fout, const Options* options) const
{
	const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
	if (node)
		return writeNode(*node, fout, options);
	else
		return WriteResult(WriteResult::FILE_NOT_HANDLED);
}
/*-------------------------------------------------------------------------------\
|                             writeNode				                             |
\-------------------------------------------------------------------------------*/
osgDB::ReaderWriter::WriteResult ReaderWriterOFF::writeNode(const osg::Node& node, const std::string& fileName, const Options* options) const
{
	if (!acceptsExtension(osgDB::getFileExtension(fileName)))
		return WriteResult(WriteResult::FILE_NOT_HANDLED);

	OffOptionsStruct localOptions = parseOptions(options);

	osgDB::ofstream f(fileName.c_str(), ((localOptions.outputBinary) ? (std::ios_base::out | std::ios_base::binary) : std::ios_base::out) );
	f.precision(localOptions.precision);

	OFFWriterNodeVisitor nv(f);

	// we must cast away constness
	(const_cast<osg::Node*>(&node))->accept(nv);

	nv.writeOFF(localOptions.outputBinary);										//write file.

	return WriteResult(WriteResult::FILE_SAVED);
}
/*-------------------------------------------------------------------------------\
|                             writeNode				                             |
\-------------------------------------------------------------------------------*/
osgDB::ReaderWriter::WriteResult ReaderWriterOFF::writeNode(const osg::Node& node, std::ostream& fout, const Options* options) const
{
	OffOptionsStruct localOptions = parseOptions(options);
	fout.precision(localOptions.precision);

	OFFWriterNodeVisitor nv(fout);

	// we must cast away constness
	(const_cast<osg::Node*>(&node))->accept(nv);

	nv.writeOFF();																//write file.

	return WriteResult(WriteResult::FILE_SAVED);
}







/*-------------------------------------------------------------------------------\
|                             convertModelToSceneGraph                           |
\-------------------------------------------------------------------------------*/
osg::Node* ReaderWriterOFF::convertModelToSceneGraph(off::Model& model, OffOptionsStruct& localOptions, const Options* options) const
{
	osg::Group* group = new osg::Group;

	osg::Geometry* geometry = convertElementListToGeometry(model, localOptions);

	if (geometry)
	{
		osg::Geode* geode = new osg::Geode;
		geode->addDrawable(geometry);

		group->addChild(geode);

	}
	return group;
}
/*-------------------------------------------------------------------------------\
|                             parseOptions				                         |
\-------------------------------------------------------------------------------*/

osg::Geometry* ReaderWriterOFF::convertElementListToGeometry(off::Model& model, OffOptionsStruct& localOptions) const
{

	size_t numVertexIndices = model.mVertexArray.size();
	if (numVertexIndices == 0)
		return 0;
	
	std::vector<osg::Vec4> colormap;
	colormap.push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
	colormap.push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
	colormap.push_back(osg::Vec4(1.0f, 0.0f, 1.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.0f, 1.0f, 1.0f, 1.0f));
	colormap.push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.5f, 0.0f, 0.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.0f, 0.5f, 0.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.0f, 0.0f, 0.5f, 1.0f));
	colormap.push_back(osg::Vec4(0.5f, 0.5f, 0.0f, 1.0f));
	colormap.push_back(osg::Vec4(0.5f, 0.0f, 0.5f, 1.0f));
	colormap.push_back(osg::Vec4(0.0f, 0.5f, 0.5f, 1.0f));
	colormap.push_back(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));



	


	osg::Vec3Array* vertices = new osg::Vec3Array();
	osg::Vec3Array* normals = model.mhaveVertexNormals ? new osg::Vec3Array : 0;
	osg::Vec2Array* texcoords = model.mhaveVertexTextureCoordinates ? new osg::Vec2Array : 0;
	osg::Vec4Array* colors = ((model.mhaveVertexColors)||(model.mhaveFaceColors)) ? new osg::Vec4Array : 0;

	if (vertices)	vertices->reserve(numVertexIndices);			//just for allocation, to be on block in memory. it's not a resize.
	if (normals)	normals->reserve(numVertexIndices);
	if (texcoords)	texcoords->reserve(numVertexIndices);
	if (colors)		colors->reserve(numVertexIndices);


	osg::Geometry* geometry = new osg::Geometry;
	if (vertices)	geometry->setVertexArray(vertices);
	if (normals)	geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
	if (texcoords)	geometry->setTexCoordArray(0,texcoords);
	if (colors)
	{
		geometry->setColorArray(colors);
		geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
	}
	
	size_t startOffset = 0;
	size_t nbIndices = 0;
	size_t index = 0;
	size_t currentType = 0;
	size_t previousType = 0;							//GL_POINTS, GL_LINES, GL_TRIANGLE_FAN or GL_POLYGON
	size_t nbElementForSameType = 0;
	osg::Vec4 color_tmp;

	size_t nbFaces = model.mFaceArray.size();
	for (size_t i = 0; i < nbFaces; ++i)
	{
		off::Model::Face &face = model.mFaceArray.at(i);

		nbIndices = face.vertexIndexList.size();
		for (size_t j = 0; j < nbIndices; ++j)
		{
			index = face.vertexIndexList.at(j);

			vertices->push_back(transformVec4(model.mVertexArray.at(index).position));
			if (normals)
				normals->push_back(transformVec4(model.mVertexArray.at(index).normal));
			if (colors)
			{
				color_tmp = model.mVertexArray.at(index).color;
				if (model.mhaveFaceColors)					//face's color
					color_tmp = (face.colormapIndex != (size_t)-1) ? colormap.at(face.colormapIndex % colormap.size()) : face.color;

				colors->push_back(color_tmp);
			}
			if (texcoords)
				texcoords->push_back(model.mVertexArray.at(index).texcoord);
		}


		//Notice: we will keep faces orders because order to display. (the second solution would be to separate Points, lines and polygones, for optimize)
		if (nbIndices == 1)
			currentType = GL_POINTS;
		else if (nbIndices == 2)
			currentType = GL_LINES;
		else if (nbIndices <= 4)
			currentType = GL_TRIANGLE_FAN;
		else
			currentType = GL_POLYGON;
		

		if((currentType == previousType) && (currentType == GL_POINTS))
		{
			nbElementForSameType += nbIndices;
			continue;
		}

		if (nbElementForSameType != 0)
		{
			osg::DrawArrays* drawArrays = new osg::DrawArrays(previousType, startOffset, nbElementForSameType);
			geometry->addPrimitiveSet(drawArrays);
		}

		previousType = currentType;
		startOffset += nbElementForSameType;
		nbElementForSameType = nbIndices;
	}

	if (nbElementForSameType != 0)						//last
	{
		osg::DrawArrays* drawArrays = new osg::DrawArrays(previousType, startOffset, nbElementForSameType);
		geometry->addPrimitiveSet(drawArrays);
	}

	return geometry;
}
/*-------------------------------------------------------------------------------\
|                             transformVec4				                         |
\-------------------------------------------------------------------------------*/
inline osg::Vec3 ReaderWriterOFF::transformVec4(const osg::Vec4& vec) const
{
	return osg::Vec3(vec.x() / vec.w(), vec.y() / vec.w(), vec.z() / vec.w()) ;
}