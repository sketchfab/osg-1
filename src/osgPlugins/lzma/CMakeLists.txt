SET(TARGET_SRC 
		CompressorLZMA.cpp

	lzma/common_internal.c
	lzma/compress.c
	lzma/decompress.c
	lzma/lzip_header.c
	lzma/lzma_header.c
	
	lzma/pavlov/7zBuf.c
	lzma/pavlov/7zBuf2.c
	lzma/pavlov/7zCrc.c
	lzma/pavlov/7zFile.c
	lzma/pavlov/7zStream.c
	lzma/pavlov/Alloc.c
	lzma/pavlov/Bcj2.c
	lzma/pavlov/Bra.c
	lzma/pavlov/Bra86.c
	lzma/pavlov/BraIA64.c
	lzma/pavlov/LzFind.c
	lzma/pavlov/LzmaDec.c
	lzma/pavlov/LzmaEnc.c
	lzma/pavlov/LzmaLib.c
 )

SET(TARGET_H
	lzma/common_internal.h
	lzma/lzip_header.h
	lzma/lzma_header.h

	lzma/easylzma/common.h
	lzma/easylzma/compress.h
	lzma/easylzma/decompress.h

	lzma/pavlov/7zBuf.h
	lzma/pavlov/7zCrc.h
	lzma/pavlov/7zFile.h
	lzma/pavlov/7zVersion.h
	lzma/pavlov/Alloc.h
	lzma/pavlov/Bcj2.h
	lzma/pavlov/Bra.h
	lzma/pavlov/CpuArch.h
	lzma/pavlov/LzFind.h
	lzma/pavlov/LzHash.h
	lzma/pavlov/LzmaDec.h
	lzma/pavlov/LzmaEnc.h
	lzma/pavlov/LzmaLib.h
	lzma/pavlov/Types.h
)

INCLUDE_DIRECTORIES(lzma)

#### end var setup  ###
SETUP_PLUGIN(lzma)
