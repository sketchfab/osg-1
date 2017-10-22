/* -*-c++-*- OpenSceneGraph -
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

#include <osg/Notify>
#include <osg/Geode>
#include <osg/Version>
#include <osg/Endian>

#include <osgDB/ReaderWriter>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <string>

#include "Compressor"



class ReaderWriterHulk : public osgDB::ReaderWriter
{
public:
     struct OptionsStruct
     {
         unsigned int mode;
         int bits;

         OptionsStruct() {
             mode = 0;
             bits = -1;
         }
    };


    ReaderWriterHulk()
    { supportsExtension("hulk", "The 3d compressor"); }

    virtual const char* className() const
    { return "Hulk compressor"; }


    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if(!acceptsExtension(ext))
        {
            return ReadResult::FILE_NOT_HANDLED;
        }

        OSG_INFO << "ReaderWriterHulk( \"" << fileName << "\" )" << std::endl;
        // strip the pseudo-loader extension
        std::string realName = osgDB::getNameLessExtension(fileName);

        if (realName.empty())
        {
            return ReadResult::FILE_NOT_HANDLED;
        }

        // recursively load the subfile.
        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile(realName, options);
        if(!node)
        {
            // propagate the read failure upwards
            OSG_WARN << "Subfile \"" << realName << "\" could not be loaded" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        return handle(*node, parseOptions(options));
    }

    virtual osgDB::ReaderWriter::WriteResult writeNode(const osg::Node& node, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
      std::string ext = osgDB::getLowerCaseFileExtension(fileName);
      if (!acceptsExtension(ext))
      {
          return WriteResult::FILE_NOT_HANDLED;
      }

      std::string realFileName = osgDB::getNameLessExtension(fileName);
      if(realFileName.empty())
      {
          return WriteResult::FILE_NOT_HANDLED;
      }

      // run compression
      osg::ref_ptr<osg::Node> compressed = handle(node, parseOptions(options));

      // forward writing to next plugin
      osg::ref_ptr<osgDB::ReaderWriter> rw = getReaderWriter(realFileName);
      if(rw)
      {
          return rw->writeNode(*compressed, realFileName, options);
      }
      else
      {
          return WriteResult::ERROR_IN_WRITING_FILE;
      }
    }

    ReaderWriterHulk::OptionsStruct parseOptions(const osgDB::ReaderWriter::Options* options) const
    {
        OptionsStruct localOptions;

        if (options)
        {
            OSG_NOTICE << "options: " << options->getOptionString() << std::endl;
            std::istringstream iss(options->getOptionString());
            std::string opt;
            while (iss >> opt)
            {
                // split opt into pre= and post=
                std::string pre_equals;
                std::string post_equals;

                size_t found = opt.find("=");
                if(found!=std::string::npos)
                {
                    pre_equals = opt.substr(0,found);
                    post_equals = opt.substr(found+1);
                }
                else
                {
                    pre_equals = opt;
                }

                if (post_equals.size())
                {
                    if (pre_equals == "mode")
                    {
                        localOptions.mode = atoi(post_equals.c_str());
                    }
                    else if (pre_equals == "bits")
                    {
                        localOptions.bits = atoi(post_equals.c_str());
                    }
                }
            }
        }
        return localOptions;
    }


    osg::Node* handle(const osg::Node& node, const ReaderWriterHulk::OptionsStruct& options) const
    {
        osg::ref_ptr<osg::Node> model = osg::clone(&node);
        Compressor hulk(options.mode, options.bits);
        model->accept(hulk);

        return model.release();
    }


protected:
    osgDB::ReaderWriter* getReaderWriter(const std::string& fileName) const
    {
        osg::ref_ptr<osgDB::Registry> registry = osgDB::Registry::instance();
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        return registry->getReaderWriterForExtension(ext);
    }
};

// now register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(hulk, ReaderWriterHulk)
