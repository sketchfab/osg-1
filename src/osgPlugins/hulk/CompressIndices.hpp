//Helpers to pack indices
#ifndef _COMPRESS_INDICES__
#define _COMPRESS_INDICES__
#include "stddef.h"
#include <vector>
#include "GeometryUniqueVisitor"
//https://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/
bool try_merge_with_next(osg::DrawElementsUInt &out_inds, const osg::DrawElementsUInt& inds, size_t base);
void pack_inds(osg::DrawElementsUInt& out_inds, const osg::DrawElementsUInt& inds);
void unpack_inds(osg::DrawElementsUInt & out_inds, const osg::DrawElementsUInt& inds);

#endif
