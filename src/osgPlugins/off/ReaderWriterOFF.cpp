/*
 * Author Toke Jensen (tokeloke@gmail.com)
 */

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Material>
#include <osg/TriangleFunctor>
#include <osgUtil/Tessellator>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ImageOptions>
#include <vector>
#include <string>
#include <osg/io_utils>

// Functor for parsing vertices and writing the to OFF format
class ValueVisitor : public osg::ValueVisitor {
    public:
        ValueVisitor(std::ostringstream& fout, const osg::Matrix& m = osg::Matrix::identity()) :
            osg::ValueVisitor(),
            _fout(fout),
            _m(m)
        {
        }

        virtual void apply (osg::Vec2 & inv)
        {
            _fout << inv[0] << ' ' << inv[1];
        }

        virtual void apply (osg::Vec3 & v)
        {
            _fout << v[0] << ' ' << v[1] << ' ' << v[2] << std::endl;
        }

    private:

        ValueVisitor& operator = (const ValueVisitor&) { return *this; }

        std::ostringstream&    _fout;
        osg::Matrix        _m;
};

// Functor for parsing faces and writing them to OFF format
class OFFPrimitiveIndexWriter : public osg::PrimitiveIndexFunctor {
    public:
        OFFPrimitiveIndexWriter(std::ostringstream& fout,osg::Geometry* geo):
            osg::PrimitiveIndexFunctor(),
            _fout(fout),
            _geo(geo),
            _faceCount(0)
        {
        }
        virtual void setVertexArray(unsigned int,const osg::Vec2*) {}

        virtual void setVertexArray(unsigned int ,const osg::Vec3* ) {}

        virtual void setVertexArray(unsigned int,const osg::Vec4* ) {}

        virtual void setVertexArray(unsigned int,const osg::Vec2d*) {}

        virtual void setVertexArray(unsigned int ,const osg::Vec3d* ) {}

        virtual void setVertexArray(unsigned int,const osg::Vec4d* ) {}

        void write(unsigned int i)
        {
            _fout << (i) << " ";
        }

        // operator for triangles
        void writeTriangle(unsigned int i1, unsigned int i2, unsigned int i3)
         {
            _faceCount++;
            _fout << "3 ";
            write(i1);
            write(i2);
            write(i3);
            _fout << std::endl;
        }

        // operator for triangles
        void writeQuad(unsigned int i1, unsigned int i2, unsigned int i3, unsigned int i4)
         {
            _faceCount++;
            _fout << "4 ";
            write(i1);
            write(i2);
            write(i3);
            write(i4);
            _fout << std::endl;
        }

        virtual void drawArrays(GLenum mode,GLint first,GLsizei count);

        virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
        {
            drawElementsImplementation<GLubyte>(mode, count, indices);
        }
        virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
        {
            drawElementsImplementation<GLushort>(mode, count, indices);
        }

        virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
        {
            drawElementsImplementation<GLuint>(mode, count, indices);
        }

        int getFaceCount() {
            return _faceCount;
        }

    protected:
        template<typename T>void drawElementsImplementation(GLenum mode, GLsizei count, const T* indices)
        {
            if (indices==0 || count==0) return;

            typedef const T* IndexPointer;

            switch(mode)
            {
                case(GL_TRIANGLES):
                {
                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                        writeTriangle(*iptr,*(iptr+1),*(iptr+2));

                    break;
                }
                case(GL_QUADS):
                {
                    IndexPointer ilast = &indices[count];
                    for(IndexPointer  iptr=indices;iptr<ilast;iptr+=4) {
                    	writeQuad(*(iptr),*(iptr+1),*(iptr+2),*(iptr+3));
		    }

                    break;
                }
                default:
                    break;
            }
        }

        virtual void begin(GLenum mode)
        {
        }

        virtual void vertex(unsigned int vert)
        {
        }

        virtual void end()
        {
        }

    private:
        OFFPrimitiveIndexWriter& operator = (const OFFPrimitiveIndexWriter&) { return *this; }

        std::ostringstream&         _fout;
        osg::Geometry*         _geo;
        unsigned int         _normalIndex;
        int _faceCount;

};

void OFFPrimitiveIndexWriter::drawArrays(GLenum mode,GLint first,GLsizei count)
{
    switch(mode)
    {
        case(GL_TRIANGLES):
        {
            unsigned int pos=first;
            for(GLsizei i=2;i<count;i+=3,pos+=3)
            {
                writeTriangle(pos,pos+1,pos+2);
            }
            break;
        }
        case(GL_QUADS):
        {
            unsigned int pos=first;
            for(GLsizei i=4;i<count;i+=4,pos+=4)
            {
                writeQuad(pos,pos+1,pos+2,pos+4);
            }
            break;
        }
        default:
            break;
    }
}

// Nodevisitor for parsing OSG and printing to OFF
class CreateOFFVisitor : public osg::NodeVisitor
{
public:
    CreateOFFVisitor(std::ostream& fout, const osgDB::ReaderWriter::Options* options = 0):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _fout(fout)
    {
    }

    void processGeometry(osg::Geometry* geo, osg::Matrix& m) {
        std::ostringstream vert_out;
        ValueVisitor vv(vert_out, m);
        osg::Array* vertices = geo->getVertexArray();
        for(unsigned int i = 0; i < vertices->getNumElements(); ++i) {
            vertices->accept(i, vv);
        }

        std::ostringstream faces_out;
        OFFPrimitiveIndexWriter pif(faces_out, geo);
        for(unsigned int i = 0; i < geo->getNumPrimitiveSets(); ++i) {
            osg::PrimitiveSet* ps = geo->getPrimitiveSet(i);
            ps->accept(pif);
        }

	std::string vertOutput = vert_out.str();
	std::string facesOutput =  faces_out.str();

	// Removing last linebreaks
	vertOutput.erase(vertOutput.length() - 1);
	facesOutput.erase(facesOutput.length() - 1);

        _fout << "OFF" << std::endl;
        _fout << vertices->getNumElements() << " ";
        _fout << pif.getFaceCount() << std::endl;		
        _fout << vertOutput << std::endl;		
        _fout << facesOutput << std::endl;		
    }

    virtual void apply(osg::Geode& node)
    {
        osg::Matrix mat = osg::computeLocalToWorld(getNodePath());
        unsigned int count = node.getNumDrawables();

        for ( unsigned int i = 0; i < count; i++ )
        {
            osg::Geometry *g = node.getDrawable( i )->asGeometry();
            if ( g != NULL )
            {
                processGeometry(g,mat);
            }
        }

        ++counter;
    }

    ~CreateOFFVisitor()
    {
        //_fout.close();
    }

    const std::string& getErrorString() const { return m_ErrorString; }

private:
    int counter;
    std::ostream& _fout;
    std::string m_fout;
    std::string m_fout_ext;
    std::string m_ErrorString;
};

// OSG Reader writer for the OFF format
class ReaderWriterOFF : public osgDB::ReaderWriter
{

public:
    ReaderWriterOFF()
    {
        supportsExtension("off","OFF file reader");
    }

    virtual ~ReaderWriterOFF()
    {
    }

    virtual const char* className() const { return "OGR file reader"; }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
	std::string ext = osgDB::getLowerCaseFileExtension(file);

        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) {
             return ReadResult::FILE_NOT_FOUND;
        }

        osgDB::ifstream fin(fileName.c_str());
        if (fin) {
            std::string format, header, line;
            osg::Group* root = new osg::Group();
            osg::Geode* pyramidGeode = new osg::Geode();
            osg::Geometry* pyramidGeometry = new osg::Geometry(); 
            int vertices = 0, 
                faces = 0;

            pyramidGeode->addDrawable(pyramidGeometry); 
            root->addChild(pyramidGeode);

            parseHeader(fin, vertices, faces, format);
            parseVertices(fin, vertices, pyramidGeometry);
            parseFaces(fin, faces, pyramidGeometry);

            root->addChild(pyramidGeometry);

            return root;
        }

	return ReadResult::FILE_NOT_HANDLED;
    }

    virtual WriteResult writeObject(const osg::Object& obj,const std::string& fileName,const Options* options=NULL) const
    {
        const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
        if (node)
            return writeNode(*node, fileName, options);
        else
            return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    virtual WriteResult writeObject(const osg::Object& obj, std::ostream& fout,const Options* options=NULL) const
    {
        const osg::Node* node = dynamic_cast<const osg::Node*>(&obj);
        if (node)
            return writeNode(*node, fout, options);
        else
            return WriteResult(WriteResult::FILE_NOT_HANDLED);
    }

    virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName,const Options* options=NULL) const
    {
        if (!acceptsExtension(osgDB::getFileExtension(fileName))) {
            return WriteResult(WriteResult::FILE_NOT_HANDLED);
        }

        osgDB::ofstream fp (fileName.c_str());
        CreateOFFVisitor createOFFVisitor(fp, options);
        const_cast<osg::Node&>(node).accept(createOFFVisitor);

        return WriteResult(WriteResult::FILE_SAVED);
    }


    virtual WriteResult writeNode(const osg::Node& node,std::ostream& fout,const Options* options=NULL) const
    {
        CreateOFFVisitor createOFFVisitor(fout, options);
        const_cast<osg::Node&>(node).accept(createOFFVisitor);

        return WriteResult(WriteResult::FILE_SAVED);
    }

private:
    void getNextLine(osgDB::ifstream & fin, std::string& line) const {
        while (std::getline(fin, line) && line.at(0) == '#');
    }

    void parseVertices(osgDB::ifstream & fin, int numOfVertices, osg::Geometry* pyramidGeometry ) const {
        osg::Vec3Array* pyramidVertices = new osg::Vec3Array;
        std::string line;

        for (int i = 0 ; i < numOfVertices ; i++) {
            getNextLine(fin, line);

            std::istringstream vertice_iss(line);
            float v1, v2, v3;
            
            if (vertice_iss >> v1 >> v2 >> v3) {
                pyramidVertices->push_back( osg::Vec3(v1, v2, v3) ); // front left
            }
        }
	pyramidGeometry->setVertexArray(pyramidVertices);
    }

    void parseFaces(osgDB::ifstream & fin, int numOfFaces, osg::Geometry* pyramidGeometry ) const {
        osg::Vec4Array* colors = new osg::Vec4Array;
        std::string line;

        for (int j = 0 ; j < numOfFaces ; j++) {
            osg::DrawElementsUInt* pyramidBase = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
            getNextLine(fin, line);
            std::istringstream face_iss(line);
            int faceLength = 0;

            // pop first
            face_iss >> faceLength;
            int currentFace;
            for (int idxFace = 0 ; idxFace < faceLength ; idxFace++) {
                face_iss >> currentFace;
                pyramidBase->push_back(currentFace);    
            }

            // Colors?
            int r, g, b, a;
            if (face_iss >> r >> g >> b >> a) {
                colors->push_back(osg::Vec4(r,g,b,a));
            }

            pyramidGeometry->addPrimitiveSet(pyramidBase);
        }

        if (colors->size() > 0) {
            pyramidGeometry->setColorArray(colors);
            pyramidGeometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        }
    }

    void parseHeader(osgDB::ifstream & fin, int& vertices, int& faces, std::string& format) const {
        std::string header;
        getNextLine(fin, format);
        getNextLine(fin, header);
        
        // Parsing header
        std::istringstream iss(header);

        if (iss >> vertices >> faces) {
            
        }
    }


};

REGISTER_OSGPLUGIN(off, ReaderWriterOFF)
