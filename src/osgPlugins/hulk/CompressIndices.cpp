#include "CompressIndices.hpp"
#include <cassert>
static void add_double_tri(osg::DrawElementsUInt& out_inds, int a, int b, int c, int d)
{
    assert(a < b);
    out_inds.push_back(a);
    out_inds.push_back(b);
    out_inds.push_back(c);
    out_inds.push_back(d);
}

static void add_tri(osg::DrawElementsUInt& out_inds, int a, int b, int c)
{
    assert(a >= b);
    out_inds.push_back(a);
    out_inds.push_back(b);
    out_inds.push_back(c);
}


bool try_merge_with_next(osg::DrawElementsUInt& out_inds, const osg::DrawElementsUInt& inds, size_t base)
{
    // is there even a next tri?
    if (base + 3 >= inds.size())
        return false;

    // is this tri degenerate?
    const unsigned int *tri = &inds[base];
    if (tri[0] == tri[1] || tri[1] == tri[2] || tri[2] == tri[0])
        return false;

    // does the next tri contain the opposite of at least one
    // of our edges?
    const unsigned int *next = &inds[base + 3];

    // go through 3 edges of tri
    for (int i = 0; i < 3; i++)
    {
        // try to find opposite of edge ab, namely ba.
        int a = tri[i];
        int b = tri[(i + 1) % 3];
        int c = tri[(i + 2) % 3];

        for (int j = 0; j < 3; j++)
        {
            if (next[j] == b && next[(j + 1) % 3] == a)
            {
                int d = next[(j + 2) % 3];

                if (a < b)
                    add_double_tri(out_inds, a, b, c, d);
                else // must be c > a, since we checked that a != c above; this ends up swapping two tris.
                    add_double_tri(out_inds, b, a, d, c);

                return true;
            }
        }
    }

    return false;
}

void pack_inds(osg::DrawElementsUInt& out_inds, const osg::DrawElementsUInt& inds)
{
    for (size_t base = 0; base < inds.size(); )
    {
        if (try_merge_with_next(out_inds, inds, base))
            base += 6; // consume 2 tris
        else
        {
            const unsigned int *tri = &inds[base];
            if (tri[0] >= tri[1])
                add_tri(out_inds, tri[0], tri[1], tri[2]);
            else if (tri[1] >= tri[2])
                add_tri(out_inds, tri[1], tri[2], tri[0]);
            else
            {
                // must have tri[2] >= tri[0],
                // otherwise we'd have tri[0] < tri[1] < tri[2] < tri[0] (contradiction)
                add_tri(out_inds, tri[2], tri[0], tri[1]);
            }

            base += 3;
        }
    }
}

void unpack_inds(osg::DrawElementsUInt& out_inds, const osg::DrawElementsUInt & inds)
{
    for (size_t base = 0; base < inds.size(); )
    {
        int a = inds[base++];
        int b = inds[base++];
        int c = inds[base++];
        out_inds.push_back(a); out_inds.push_back(b); out_inds.push_back(c);

        if (a < b) // two tris: (a,b,c), (a,d,b)
        {
            int d = inds[base++];
            out_inds.push_back(a); out_inds.push_back(d); out_inds.push_back(b);
        }
    }
}
