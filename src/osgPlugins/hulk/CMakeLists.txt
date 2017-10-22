SET(TARGET_SRC
    ReaderWriterHulk.cpp
    CompressorNormals.cpp
    CompressIndices.cpp
    )

SET(TARGET_H
    Compressor
    GeometryUniqueVisitor
    StatLogger
    CompressorNormals.hpp
    CompressIndices.hpp
)

#### end var setup  ###
SETUP_PLUGIN(hulk)
