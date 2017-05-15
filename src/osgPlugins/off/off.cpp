/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield
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

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <functional>

#include "off.h"

#include <osg/Notify>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <string.h>

using namespace off;


#ifdef _MSC_VER
#define strncasecmp strnicmp
#endif




/*-------------------------------------------------------------------------------\
|                             trim					                             |
\-------------------------------------------------------------------------------*/
std::string trim(const std::string& s)
{
  if(s.length() == 0)
    return s;
  int b = s.find_first_not_of(" \t");
  int e = s.find_last_not_of(" \t");
  if(b == -1)																// No non-spaces
    return "";
  return std::string(s, b, e - b + 1);
}
/*-------------------------------------------------------------------------------\
|                             toUpperCase			                             |
\-------------------------------------------------------------------------------*/
void toUpperCase(std::string& str)
{
	std::transform( str.begin(), str.end(), str.begin(), toupper);
}
/*-------------------------------------------------------------------------------\
|                             toString				                             |
\-------------------------------------------------------------------------------*/
std::string toString(float val, unsigned short precision, unsigned short width, char fill, std::ios::fmtflags flags)
{
	std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > stream;
	stream.precision(precision);
	stream.width(width);
	stream.fill(fill);
	if (flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}
/*-------------------------------------------------------------------------------\
|                             toString				                             |
\-------------------------------------------------------------------------------*/
std::string toString(int val, unsigned short width, char fill, std::ios::fmtflags flags)
{
	std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > stream;
	stream.width(width);
	stream.fill(fill);
	if (flags)
		stream.setf(flags);
	stream << val;
	return stream.str();
}
/*-------------------------------------------------------------------------------\
|                             parseFloat			                             |
\-------------------------------------------------------------------------------*/
float parseFloat(const std::string& val)
{
	// Use istringstream for direct correspondence with toString
	std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > str(val);
	float ret = 0;
	str >> ret;

	return ret;
}
/*-------------------------------------------------------------------------------\
|                             parseInt				                             |
\-------------------------------------------------------------------------------*/
int parseInt(const std::string& val)
{
	// Use istringstream for direct correspondence with toString
	std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > str(val);
	int ret = 0;
	str >> ret;

	return ret;
}
/*-------------------------------------------------------------------------------\
|                             parseUnsignedInt		                             |
\-------------------------------------------------------------------------------*/
unsigned int parseUnsignedInt(const std::string& val)
{
	// Use istringstream for direct correspondence with toString
	std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > str(val);
	unsigned int ret = 0;
	str >> ret;

	return ret;
}
/*-------------------------------------------------------------------------------\
|                             isNumber				                             |
\-------------------------------------------------------------------------------*/
bool isNumber(const std::string& val)
{
	std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > str(val);
	float tst;
	str >> tst;
	return !str.fail() && str.eof();
}





/*-------------------------------------------------------------------------------\
|                             endianSwap			                             |
\-------------------------------------------------------------------------------*/
void endianSwap(unsigned int& x)
{
	x = (x >> 24) |
		((x << 8) & 0x00FF0000) |
		((x >> 8) & 0x0000FF00) |
		(x << 24);
}
/*-------------------------------------------------------------------------------\
|                             writeInt				                             |
\-------------------------------------------------------------------------------*/
void writeInt(std::ostream& fout, int src, bool big_endian)
{
	unsigned int target = src;
	if (big_endian)
		endianSwap(target);

	fout.write(reinterpret_cast<const char *>(&target), sizeof(int));
}
/*-------------------------------------------------------------------------------\
|                             writeFloat			                             |
\-------------------------------------------------------------------------------*/
void writeFloat(std::ostream& fout, float src, bool big_endian)
{
	float target = src;
	if (big_endian)
		endianSwap(*((unsigned int *)&target));

	fout.write(reinterpret_cast<const char *>(&target), sizeof(float));
}
/*-------------------------------------------------------------------------------\
|                             readInt				                             |
\-------------------------------------------------------------------------------*/
int readInt(std::istream& fin, long fileSize, long &startOffset, bool big_endian)
{
	size_t nbBytes = sizeof(int);
	char* buffer = new char[nbBytes];
	
	if (startOffset + (long)nbBytes > fileSize)
		return 0;
	
	fin.read(buffer, nbBytes);
	startOffset += nbBytes;
	
	unsigned int target = *(reinterpret_cast<unsigned int*>(buffer));
	if (big_endian)
		endianSwap(target);

	delete buffer;

	return target;
}
/*-------------------------------------------------------------------------------\
|                             readFloat				                             |
\-------------------------------------------------------------------------------*/
float readFloat(std::istream& fin, long fileSize, long &startOffset, bool big_endian)
{
	size_t nbBytes = sizeof(float);
	if (startOffset + (long)nbBytes > fileSize)
		return 0.0f;

	char* buffer = new char[nbBytes];

	fin.read(buffer, nbBytes);
	startOffset += nbBytes;

	unsigned int target = *reinterpret_cast<unsigned int*>(buffer);
	if (big_endian)
		endianSwap(target);

	delete buffer;

	return *((float*)&target);
}





/*-------------------------------------------------------------------------------\
|                             Model					                             |
\-------------------------------------------------------------------------------*/
Model::Model(void)
{
	init();
}
/*-------------------------------------------------------------------------------\
|                             ~Model				                             |
\-------------------------------------------------------------------------------*/
Model::~Model(void)
{
	
}
/*-------------------------------------------------------------------------------\
|                             init					                             |
\-------------------------------------------------------------------------------*/
void Model::init(void)
{
	mVertexArray.clear();
	mFaceArray.clear();

	databasePath = "";
	mhaveVertexColors = mhaveVertexNormals = mhaveVertexTextureCoordinates = mhaveFaceColors = false;
	mNumDim = 3;
}
/*-------------------------------------------------------------------------------\
|                             readOFF				                             |
\-------------------------------------------------------------------------------*/
bool Model::readOFF(std::istream& fin, const osgDB::ReaderWriter::Options* options, bool &isBinary)
{
    OSG_INFO<<"Reading OFF file"<<std::endl;

	init();


	//specs : http://www.geomview.org/docs/html/OFF.html

	enum FileState
	{
		FS_FileDescription = 0,
		FS_NDimension,
		FS_VertexsAndFacesNumber,
		FS_VertexsList,
		FS_FacesList,
		FS_Exit,
	};

	enum VertexState
	{
		VS_Position = 0,
		VS_Position_w,						//4 component
		VS_Normal,
		VS_Normal_w,
		VS_Color,
		VS_Texcoord,
		VS_Exit,
	};
	//Notice: the specs (at http://www.geomview.org/docs/html/OFF.html) talk about a order on fileDescription, but MeshLab use CSTOFF and there is some "nan" or "-nan" values.
	// So, may be I'm wrong, and C N ST could be in free order.
	// but in this case the order of specs (ST C N) give datas order (normal, color, textureCoords ), and in meshlab file, (C ST) give datas order (C ST). it's seam strange.
	// Or, may be meshLab have a wrong implementation on exporter (import work well with the spec).



	const int LINE_SIZE = 4096;
	char line[LINE_SIZE];
	std::string str = "";
	size_t index;
	std::vector<std::string> sl;
	FileState fileState = FS_FileDescription;

	isBinary = false;
	bool is4Component = false;
	bool isNDimensions = false;
	size_t nbDim = 3;
	size_t nbVertices = 0;
	size_t nbFaces = 0;

	size_t vertice_index = 0;
	size_t face_index = 0;
	



	while (fin)
	{
		readline(fin, line, LINE_SIZE);

		str = line;


		//first, we have to remove Comments. 
		index = str.find('#');
		if (index != std::string::npos)
		{
			// comment line
			OSG_NOTICE << "Comment: " << str.substr(index) << std::endl;

			str = str.substr(0, index);
		}

		//remove spaces on border
		str = trim(str);
		if (str.length() == 0)														//if is a blank line,
			continue;																//do nothing.


		switch (fileState)
		{


		//------------------------------------------------------------------------
		case FS_FileDescription:
		{
			if ((str.length() >= 6) && (str.substr(str.length() - 6) == "BINARY"))
			{
				isBinary = true;
				str = trim(str.substr(0, str.length() - 6));

				OSG_NOTICE << "Binary format found " << std::endl;
				return false;														//it's another function for that.
			}


			if ((str.length() < 3) || (str.substr(str.length() - 3) != "OFF"))
			{
				//if there isn't the good file descriptor, we have two solutions:
				//-wait the line with the good descriptor
				//-report the error and exit.
				//The second solution is more safe.

				OSG_FATAL << " Error: This OFF file don't have the file descriptor in the first line." << std::endl;
				return false;
			}

			if (str.substr(0, 2) == "ST")
			{
				mhaveVertexTextureCoordinates = true;
				str = str.substr(2);
			}

			if (str.substr(0, 1) == "C")
			{
				mhaveVertexColors = true;
				str = str.substr(1);
			}

			if (str.substr(0, 1) == "N")
			{
				mhaveVertexNormals = true;
				str = str.substr(1);
			}

			if (str.substr(0, 1) == "4")
			{
				is4Component = true;
				nbDim = 4;
				str = str.substr(1);
			}

			if (str.substr(0, 1) == "n")
			{
				isNDimensions = true;
				str = str.substr(1);

				fileState = FS_NDimension;														//next step is to read line with dimension specified
				break;
			}

			fileState = FS_VertexsAndFacesNumber;												//next step is to read line with nb vertex and faces.
		}
		break;






		//------------------------------------------------------------------------
		case FS_NDimension:
		{
			bool isOk = isNumber(str);
			if (isOk)
			{
				sl.clear();
				osgDB::split(str, sl, '.');														//is float ?
				isOk = (sl.size() == 1);
			}

			if (!isOk)
			{
				OSG_FATAL << " Error: nOFF is specified but the second line have more than one int argument." << std::endl;
				return false;
			}

			nbDim = parseUnsignedInt(str);
			//Notice: in specs's document, is4Component give (nDim + 1). So I suppose for nDim == 2, is4Component don't force to use 4 components (as a minimum).

			if (nbDim + ((is4Component) ? 1 : 0) > 4)
			{
				OSG_FATAL << " Error: With nDim (= " << nbDim << ") and 4components (=" << (is4Component ? "true" : "false") << ") specified, the number of components are " << nbDim << ". But this implementation use 4 components maximum (most of cases)." << std::endl;
				return false;
			}else if (nbDim <= 0) {
				OSG_WARN << " Error: With nDim (= " << nbDim << ") . Continue with hypothesis : dim=3." << std::endl;
				nbDim = 3;
			}

			mNumDim = nbDim;
			fileState = FS_VertexsAndFacesNumber;												//next step is to read line with nb vertex and faces.
		}
		break;




		//------------------------------------------------------------------------
		case FS_VertexsAndFacesNumber:
		{
			
			sl.clear();
			osgDB::split(str, sl, ' ');

			if (sl.size() < 2)
			{
				OSG_FATAL << " Error: The line with NumVertices and NumFaces don't have enougth arguments." << std::endl;
				return false;
			}

			nbVertices = parseUnsignedInt(sl.at(0));
			nbFaces = parseUnsignedInt(sl.at(1));
			//Notice: all recent specs documents say the third number, for edges number, we don't have to care about.

			fileState = FS_VertexsList;
		}
		break;


		//------------------------------------------------------------------------
		case FS_VertexsList:
		{
			if (nbVertices == 0)
			{
				fileState = FS_Exit;
				break;
			}
			
			
			sl.clear();
			osgDB::split(str, sl, ' ');

			//first things: remove all non numbers arguments
			for (osgDB::StringList::iterator i = sl.begin(); i != sl.end(); ++i)
				if (!isNumber(*i))
					i = sl.erase(i);

			mVertexArray.push_back(Vertex());
			Vertex &vertex = mVertexArray.back();


			VertexState vertexState = VS_Position;
			size_t indexArg = 0;

			for (osgDB::StringList::iterator i = sl.begin(); i != sl.end(); ++i)
			{
				switch (vertexState)
				{

				case VS_Position:
				{
					vertex.position[indexArg] = parseFloat(*i);

					++indexArg;
					if (indexArg == nbDim)
					{
						if (is4Component)
							vertexState = VS_Position_w;
						else if (mhaveVertexNormals)
							vertexState = VS_Normal;
						else if (mhaveVertexColors)
							vertexState = VS_Color;
						else if (mhaveVertexTextureCoordinates)
							vertexState = VS_Texcoord;
						else
							vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;

				case VS_Position_w:
				{
					vertex.position[3] = parseFloat(*i);		//w

					if (mhaveVertexNormals)
						vertexState = VS_Normal;
					else if (mhaveVertexColors)
						vertexState = VS_Color;
					else if (mhaveVertexTextureCoordinates)
						vertexState = VS_Texcoord;
					else
						vertexState = VS_Exit;
					indexArg = 0;
				}
				break;




				case VS_Normal:
				{
					vertex.normal[indexArg] = parseFloat(*i);

					++indexArg;
					if (indexArg == nbDim)
					{
						if (is4Component)
							vertexState = VS_Normal_w;
						else if (mhaveVertexColors)
							vertexState = VS_Color;
						else if (mhaveVertexTextureCoordinates)
							vertexState = VS_Texcoord;
						else
							vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;

				case VS_Normal_w:
				{
					vertex.normal[3] = parseFloat(*i);		//w

					if (mhaveVertexColors)
						vertexState = VS_Color;
					else if (mhaveVertexTextureCoordinates)
						vertexState = VS_Texcoord;
					else
						vertexState = VS_Exit;
					indexArg = 0;
				}
				break;




				case VS_Color:
				{
					vertex.color[indexArg] = parseFloat(*i);
					if (vertex.color[indexArg] > 1.0f)
						vertex.color[indexArg] /= 255.0f;

					++indexArg;
					if (indexArg == 4)													//on http://www.geomview.org/docs/oogltour.html, color have 4 components.
					{
						if (mhaveVertexTextureCoordinates)
							vertexState = VS_Texcoord;
						else
							vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;


				case VS_Texcoord:
				{
					vertex.texcoord[indexArg] = parseFloat(*i);

					++indexArg;
					if (indexArg == 2)
					{
						vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;

				case VS_Exit:
				default:
					break;
				}
			}



			++vertice_index;
			if (vertice_index >= nbVertices)
				fileState = FS_FacesList;
		}
		break;








		//------------------------------------------------------------------------
		case FS_FacesList:
		{
			if ((nbFaces == 0)||(mVertexArray.size()==0))
			{
				fileState = FS_Exit;
				break;
			}
			
			
			sl.clear();
			osgDB::split(str, sl, ' ');

			//first things: remove all non numbers arguments
			for (osgDB::StringList::iterator i = sl.begin(); i != sl.end(); ++i)
				if (!isNumber(*i))
					i = sl.erase(i);

			size_t nbArgs = sl.size();
			if (nbArgs != 0)
			{
				size_t nbVertexIndex = parseUnsignedInt(sl.at(0));

				if (nbVertexIndex > nbArgs - 1)											//secu
					nbVertexIndex = nbArgs - 1;


				mFaceArray.push_back(Face());
				Face &face = mFaceArray.back();

				std::vector<size_t> &vertexIndexList = face.vertexIndexList;
				for (size_t i = 1; i <= nbVertexIndex; i++)
				{
					if (sl.at(i).find('.') != std::string::npos)						//if is float, don't care.
						continue;

					size_t index_tmp = parseInt(sl.at(i));
					if (index_tmp >= mVertexArray.size())								//Check if index is not too high.	//"Note that earlier versions of the model files had faces with -1 indices into the vertex list. That was due to an error in the conversion program and should be corrected now."
						continue;

					vertexIndexList.push_back(index_tmp);
				}

				if (vertexIndexList.size() < 3)
				{
					mFaceArray.erase(mFaceArray.end());

				} else {																//face colors

					if (nbArgs > nbVertexIndex + 1)
					{
						size_t nbArgForColors = nbArgs - (nbVertexIndex + 1);			//on number of arguments for colors, we could have differente solution for colors definitions.

						if (nbArgForColors == 1)										//colormap index
						{
							face.colormapIndex = parseInt(sl.at(nbVertexIndex + 1));
							mhaveFaceColors = true;

						} else if (nbArgForColors >= 3) {

							face.color[0] = parseFloat(sl.at(nbVertexIndex + 1));		//r
							face.color[1] = parseFloat(sl.at(nbVertexIndex + 2));		//g
							face.color[2] = parseFloat(sl.at(nbVertexIndex + 3));		//b
							face.color[3] = ((nbArgForColors >= 4) ? parseFloat(sl.at(nbVertexIndex + 4)) : 1.0);	//a

							if ((face.color.r() > 1.0) || (face.color.g() > 1.0) || (face.color.b() > 1.0) || (face.color.a() > 1.0))	//case color intervale [0,255]
							{
								face.color[0] /= 255.0;
								face.color[1] /= 255.0;
								face.color[2] /= 255.0;
								face.color[3] /= 255.0;
							}
							mhaveFaceColors = true;
						}
					}
				}
			}


			++face_index;
			if (face_index >= nbFaces)
				fileState = FS_Exit;
		}
		break;




		}

	}

	if (mVertexArray.size() == 0)
	{
		OSG_FATAL << " Error: No Vertex found." << std::endl;
		return false;
	}
	if (mFaceArray.size() == 0)
	{
		OSG_FATAL << " Error: No Face found." << std::endl;
		return false;
	}



#if 0
    OSG_NOTICE <<"vertices :"<< mVertexArray.size()<<std::endl;
    OSG_NOTICE <<"faces :"<< mFaceArray.size()<<std::endl;
    OSG_NOTICE <<"haveVertexColors :"<< mhaveVertexColors <<std::endl;
    OSG_NOTICE <<"haveVertexNormals :"<< mhaveVertexNormals  <<std::endl;
    OSG_NOTICE <<"haveVertexTextureCoordinates :"<< mhaveVertexTextureCoordinates  <<std::endl;
	OSG_NOTICE <<"haveFaceColors :" << mhaveFaceColors << std::endl;
#endif
    return true;
}




/*-------------------------------------------------------------------------------\
|                             readOFF_binary		                             |
\-------------------------------------------------------------------------------*/
bool Model::readOFF_binary(std::istream& fin, const osgDB::ReaderWriter::Options* options)
{
	OSG_INFO << "Reading OFF file" << std::endl;

	init();


	//specs : http://www.geomview.org/docs/html/OFF.html

	enum FileState
	{
		FS_FileDescription = 0,
		FS_NDimension,
		FS_VertexsAndFacesNumber,
		FS_VertexsList,
		FS_FacesList,
		FS_Exit,
	};

	enum VertexState
	{
		VS_Position = 0,
		VS_Position_w,															//4 component
		VS_Normal,
		VS_Normal_w,
		VS_Color,
		VS_Texcoord,
		VS_Exit,
	};


	const int LINE_SIZE = 4096;
	char line[LINE_SIZE];
	std::string str = "";
	size_t index;
	std::vector<std::string> sl;
	FileState fileState = FS_FileDescription;

	bool is4Component = false;
	bool isNDimensions = false;
	size_t nbDim = 3;
	size_t nbVertices = 0;
	size_t nbFaces = 0;

	size_t vertice_index = 0;
	size_t face_index = 0;



	// get size of file
	fin.seekg(0, fin.end);
	long fileSize = fin.tellg();
	fin.seekg(0);

	long startBinaryPart = 0;
	long startOffset = 0;



	while ((fin)&&(fileState!= FS_Exit))
	{
		


		//Before FileDescription, it could have comment and string lines like the notBinary version
		if (fileState == FS_FileDescription)
		{
			readline(fin, line, LINE_SIZE);										//read as a usual line
			str = line;

			//first, we have to remove Comments. 
			index = str.find('#');
			if (index != std::string::npos)
			{
				// comment line
				OSG_NOTICE << "Comment: " << str.substr(index) << std::endl;

				str = str.substr(0, index);
			}

			//remove spaces on border
			str = trim(str);
			if (str.length() == 0)												//if is a blank line,
				continue;														//do nothing.
		}




		switch (fileState)
		{

		//------------------------------------------------------------------------
		case FS_FileDescription:
		{
			if ((str.length() >= 6) && (str.substr(str.length() - 6) == "BINARY"))
				str = trim(str.substr(0, str.length() - 6));
			else
				return false;													//if there isn't "BINARY", it's the wrong function


			if ((str.length() < 3) || (str.substr(str.length() - 3) != "OFF"))
			{
				//if there isn't the good file descriptor, we have two solutions:
				//-wait the line with the good descriptor
				//-report the error and exit.
				//The second solution is more safe.

				OSG_FATAL << " Error: This OFF file don't have the file descriptor in the first line." << std::endl;
				return false;
			}

			if (str.substr(0, 2) == "ST")
			{
				mhaveVertexTextureCoordinates = true;
				str = str.substr(2);
			}

			if (str.substr(0, 1) == "C")
			{
				mhaveVertexColors = true;
				str = str.substr(1);
			}

			if (str.substr(0, 1) == "N")
			{
				mhaveVertexNormals = true;
				str = str.substr(1);
			}

			if (str.substr(0, 1) == "4")
			{
				is4Component = true;
				nbDim = 4;
				str = str.substr(1);
			}

			if (str.substr(0, 1) == "n")
			{
				isNDimensions = true;
				str = str.substr(1);

				startBinaryPart = fin.tellg();
				startOffset = startBinaryPart;
				fileState = FS_NDimension;												//next step is to read line with dimension specified
				break;
			}

			startBinaryPart = fin.tellg();
			startOffset = startBinaryPart;
			fileState = FS_VertexsAndFacesNumber;										//next step is to read line with nb vertex and faces.
		}
		break;






		//------------------------------------------------------------------------
		case FS_NDimension:
		{
			
			nbDim = readInt(fin, fileSize, startOffset, true);
			
				
			if (nbDim + ((is4Component) ? 1 : 0) > 4)
			{
				OSG_FATAL << " Error: With nDim (= " << nbDim << ") and 4components (=" << (is4Component ? "true" : "false") << ") specified, the number of components are " << nbDim << ". But this implementation use 4 components maximum (most of cases)." << std::endl;
				return false;
			}else if (nbDim <= 0) {
				OSG_WARN << " Error: With nDim (= " << nbDim << ") . Continue with hypothesis : dim=3." << std::endl;
				nbDim = 3;
			}

			mNumDim = nbDim;
			fileState = FS_VertexsAndFacesNumber;										//next step is to read line with nb vertex and faces.
		}
		break;




		//------------------------------------------------------------------------
		case FS_VertexsAndFacesNumber:
		{
			nbVertices = readInt(fin, fileSize, startOffset, true);
			nbFaces = readInt(fin, fileSize, startOffset, true);
			size_t nbEdges = readInt(fin, fileSize, startOffset, true);					//have to read it, but it's not used.
			nbEdges = 0;

			fileState = FS_VertexsList;
		}
		break;


		//------------------------------------------------------------------------
		case FS_VertexsList:
		{
			if (nbVertices == 0)
			{
				fileState = FS_Exit;
				break;
			}



			
			mVertexArray.push_back(Vertex());
			Vertex &vertex = mVertexArray.back();

			VertexState vertexState = VS_Position;
			size_t indexArg = 0;

			while (vertexState != VS_Exit)
			{
				switch (vertexState)
				{

				case VS_Position:
				{
					vertex.position[indexArg] = readFloat(fin, fileSize, startOffset, true);

					++indexArg;
					if (indexArg == nbDim)
					{
						if (is4Component)
							vertexState = VS_Position_w;
						else if (mhaveVertexNormals)
							vertexState = VS_Normal;
						else if (mhaveVertexColors)
							vertexState = VS_Color;
						else if (mhaveVertexTextureCoordinates)
							vertexState = VS_Texcoord;
						else
							vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;

				case VS_Position_w:
				{
					vertex.position[3] = readFloat(fin, fileSize, startOffset, true);
					
					if (mhaveVertexNormals)
						vertexState = VS_Normal;
					else if (mhaveVertexColors)
						vertexState = VS_Color;
					else if (mhaveVertexTextureCoordinates)
						vertexState = VS_Texcoord;
					else
						vertexState = VS_Exit;
					indexArg = 0;
				}
				break;




				case VS_Normal:
				{
					vertex.normal[indexArg] = readFloat(fin, fileSize, startOffset, true);
					
					++indexArg;
					if (indexArg == nbDim)
					{
						if (is4Component)
							vertexState = VS_Normal_w;
						else if (mhaveVertexColors)
							vertexState = VS_Color;
						else if (mhaveVertexTextureCoordinates)
							vertexState = VS_Texcoord;
						else
							vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;

				case VS_Normal_w:
				{
					vertex.normal[3] = readFloat(fin, fileSize, startOffset, true);

					if (mhaveVertexColors)
						vertexState = VS_Color;
					else if (mhaveVertexTextureCoordinates)
						vertexState = VS_Texcoord;
					else
						vertexState = VS_Exit;
					indexArg = 0;
				}
				break;




				case VS_Color:
				{
					vertex.color[indexArg] = readFloat(fin, fileSize, startOffset, true);
					
					++indexArg;
					if (indexArg == 4)														//on http://www.geomview.org/docs/oogltour.html, color have 4 components.
					{
						if (mhaveVertexTextureCoordinates)
							vertexState = VS_Texcoord;
						else
							vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;


				case VS_Texcoord:
				{
					vertex.texcoord[indexArg] = readFloat(fin, fileSize, startOffset, true);
					
					++indexArg;
					if (indexArg == 2)
					{
						vertexState = VS_Exit;
						indexArg = 0;
					}
				}
				break;

				case VS_Exit:
				default:
					break;
				}
			}

			++vertice_index;
			if (vertice_index >= nbVertices)
				fileState = FS_FacesList;
		}
		break;








		//------------------------------------------------------------------------
		case FS_FacesList:
		{
			if ((nbFaces == 0) || (mVertexArray.size() == 0))
			{
				fileState = FS_Exit;
				break;
			}


			size_t nbVertexIndex = readInt(fin, fileSize, startOffset, true);

			mFaceArray.push_back(Face());
			Face &face = mFaceArray.back();

			std::vector<size_t> &vertexIndexList = face.vertexIndexList;
			size_t index_tmp = (size_t)-1;
			for (size_t i = 1; i <= nbVertexIndex; i++)
			{
				index_tmp = readInt(fin, fileSize, startOffset, true);

				if (index_tmp >= mVertexArray.size())								//Check if index is not too high. "Note that earlier versions of the model files had faces with -1 indices into the vertex list. That was due to an error in the conversion program and should be corrected now."
					continue;

				vertexIndexList.push_back(index_tmp);
			}

			if (vertexIndexList.size() < 3)
			{
				mFaceArray.erase(mFaceArray.end());

			}
			//else {																//face colors
				// we don't have the notion of line, and with the number of indices on a face, 
				//we can't know about presence of face color or this format
			//}

			++face_index;
			if (face_index >= nbFaces)
				fileState = FS_Exit;
		}
		break;


		}

	}

	if (mVertexArray.size() == 0)
	{
		OSG_FATAL << " Error: No Vertex found." << std::endl;
		return false;
	}
	if (mFaceArray.size() == 0)
	{
		OSG_FATAL << " Error: No Face found." << std::endl;
		return false;
	}



#if 0
	OSG_NOTICE << "vertices :" << mVertexArray.size() << std::endl;
	OSG_NOTICE << "faces :" << mFaceArray.size() << std::endl;
	OSG_NOTICE << "haveVertexColors :" << mhaveVertexColors << std::endl;
	OSG_NOTICE << "haveVertexNormals :" << mhaveVertexNormals << std::endl;
	OSG_NOTICE << "haveVertexTextureCoordinates :" << mhaveVertexTextureCoordinates << std::endl;
	OSG_NOTICE << "haveFaceColors :" << mhaveFaceColors << std::endl;
#endif
	return true;
}




/*-------------------------------------------------------------------------------\
|                             readline				                             |
\-------------------------------------------------------------------------------*/
//fonction from plugins_obj, witch take care of file format on windows, unix and MacOs.
bool Model::readline(std::istream& fin, char* line, const int LINE_SIZE, bool isBinary)
{
	if (LINE_SIZE<1) return false;

	bool eatWhiteSpaceAtStart = !isBinary;
	bool changeTabsToSpaces = !isBinary;
	bool stripTrailingSpaces = !isBinary;

	char* ptr = line;
	char* end = line + LINE_SIZE - 1;
	bool skipNewline = false;
	while (fin && ptr<end)
	{

		int c = fin.get();
		int p = fin.peek();
		if (c == '\r')
		{
			if (p == '\n')
			{
				// we have a windows line endings.
				fin.get();
				// OSG_NOTICE<<"We have dos line ending"<<std::endl;
				if (skipNewline)
				{
					skipNewline = false;
					*ptr++ = ' ';
					continue;
				}else
					break;
			}
			// we have Mac line ending
			// OSG_NOTICE<<"We have mac line ending"<<std::endl;
			if (skipNewline)
			{
				skipNewline = false;
				*ptr++ = ' ';
				continue;
			}else
				break;

		}else if (c == '\n'){
			// we have unix line ending.
			// OSG_NOTICE<<"We have unix line ending"<<std::endl;
			if (skipNewline)
			{
				*ptr++ = ' ';
				continue;
			}else
				break;

		}else if (c == '\\' && (p == '\r' || p == '\n')){
			// need to keep return;
			skipNewline = true;

		}else if (c != std::ifstream::traits_type::eof()){								 // don't copy eof.
			skipNewline = false;

			if (!eatWhiteSpaceAtStart || (c != ' ' && c != '\t'))
			{
				eatWhiteSpaceAtStart = false;
				*ptr++ = c;
			}
		}


	}

	// strip trailing spaces
	if (stripTrailingSpaces)
	{
		while (ptr > line && *(ptr - 1) == ' ')
			--ptr;
	}

	*ptr = 0;																			// \0 at the end.

	if (changeTabsToSpaces)
	{

		for (ptr = line; *ptr != 0; ++ptr)
			if (*ptr == '\t') *ptr = ' ';
	}

	return true;
}




/*-------------------------------------------------------------------------------\
|                             writeOFF				                             |
\-------------------------------------------------------------------------------*/
bool Model::writeOFF(std::ostream& fout, bool binary)
{	
	if (binary)
		return writeOFF_binary(fout);


	
	if ((!mVertexArray.size()) || (mFaceArray.size() == 0))
		return false;

	fout << "# file written by OpenSceneGraph" << std::endl;


	//---------- Descriptor
	if (mhaveVertexTextureCoordinates)
		fout << "ST";
	if (mhaveVertexColors)
		fout << "C";
	if (mhaveVertexNormals)
		fout << "N";
	fout << "OFF";

	fout << std::endl;





	//---------- Number Vertices and Faces
	size_t nbVertex = mVertexArray.size();
	size_t nbFaces = mFaceArray.size();
	fout << nbVertex << " " << nbFaces << " 0" << std::endl;								// " 0" is for depreciate NumberOfEdges





	//---------- Vertices
	for (size_t i = 0; i < nbVertex; ++i)
	{
		Vertex &vertex = mVertexArray.at(i);
		
		fout << vertex.position.x() << ' ' << vertex.position.y() << ' ' << vertex.position.z();
		
		if (mhaveVertexNormals)
			fout << ' ' << vertex.normal.x() << ' ' << vertex.normal.y() << ' ' << vertex.normal.z();

		if (mhaveVertexColors)
			fout << ' ' << vertex.color.r() << ' ' << vertex.color.g() << ' ' << vertex.color.b() << ' ' << vertex.color.a();

		if (mhaveVertexTextureCoordinates)
			fout << ' ' << vertex.texcoord.x() << ' ' << vertex.texcoord.y();

		fout << std::endl;
	}


	//---------- Faces
	size_t nbIndex = 0;
	for (size_t i = 0; i < nbFaces; ++i)
	{
		Face &face = mFaceArray.at(i);
		nbIndex = face.vertexIndexList.size();

		fout << nbIndex;

		for (size_t j = 0; j < nbIndex; ++j)
			fout << ' ' << face.vertexIndexList.at(j);

		if (mhaveFaceColors)
		{
			if (face.colormapIndex == (size_t)-1)
				fout << ' ' << face.color.r() << ' ' << face.color.g() << ' ' << face.color.b() << ' ' << face.color.a();
			else
				fout << ' ' << face.colormapIndex;
		}

		fout << std::endl;
	}

	return true;
}
/*-------------------------------------------------------------------------------\
|                             writeOFF_binary		                             |
\-------------------------------------------------------------------------------*/
bool Model::writeOFF_binary(std::ostream& fout)
{
	if ((!mVertexArray.size()) || (mFaceArray.size() == 0))
		return false;


	std::string str = "# file written by OpenSceneGraph\n";
	fout.write(str.c_str(), str.size());


	//---------- Descriptor
	str = "";
	if (mhaveVertexTextureCoordinates)
		str += "ST";
	if (mhaveVertexColors)
		str += "C";
	if (mhaveVertexNormals)
		str += "N";
	str += "OFF BINARY";
	
	str += "\n";
	fout.write(str.c_str(), str.size());



	//---------- Number Vertices and Faces
	size_t nbVertex = mVertexArray.size();
	size_t nbFaces = mFaceArray.size();

	writeInt(fout, nbVertex, true);
	writeInt(fout, nbFaces, true);
	writeInt(fout, 0, true);


	//---------- Vertices
	for (size_t i = 0; i < nbVertex; ++i)
	{
		Vertex &vertex = mVertexArray.at(i);

		writeFloat(fout, vertex.position.x(), true);
		writeFloat(fout, vertex.position.y(), true);
		writeFloat(fout, vertex.position.z(), true);

		if (mhaveVertexNormals)
		{
			writeFloat(fout, vertex.normal.x(), true);
			writeFloat(fout, vertex.normal.y(), true);
			writeFloat(fout, vertex.normal.z(), true);
		}

		if (mhaveVertexColors)
		{
			writeFloat(fout, vertex.color.r(), true);
			writeFloat(fout, vertex.color.g(), true);
			writeFloat(fout, vertex.color.b(), true);
			writeFloat(fout, vertex.color.a(), true);
		}

		if (mhaveVertexTextureCoordinates)
		{
			writeFloat(fout, vertex.texcoord.x(), true);
			writeFloat(fout, vertex.texcoord.y(), true);
		}
	}

	//---------- Faces
	size_t nbIndex = 0;
	for (size_t i = 0; i < nbFaces; ++i)
	{
		Face &face = mFaceArray.at(i);
		nbIndex = face.vertexIndexList.size();

		writeInt(fout, nbIndex, true);

		for (size_t j = 0; j < nbIndex; ++j)
			writeInt(fout, face.vertexIndexList.at(j), true);

		if (mhaveFaceColors)
		{
			if (face.colormapIndex == (size_t)-1)
			{
				writeFloat(fout, face.color.r(), true);
				writeFloat(fout, face.color.g(), true);
				writeFloat(fout, face.color.b(), true);
				writeFloat(fout, face.color.a(), true);

			} else {
				
				writeInt(fout, face.colormapIndex, true);
			}
		}
	}

	return true;
}
/*-------------------------------------------------------------------------------\
|                             merge					                             |
\-------------------------------------------------------------------------------*/
size_t Model::merge(Vertex &vertex)
{
	size_t nbVertex = mVertexArray.size();
	for (size_t i = 0; i < nbVertex; ++i)
		if (vertex == mVertexArray.at(i))
			return i;

	mVertexArray.push_back(vertex);
	return nbVertex;
}