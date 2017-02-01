# Exercise: the hulk compressor

We want to optimize the size of our 3d data to reduce storage costs and more importantly bandwidth which has a direct impact on the user experience. We will extend an [OpenSceneGraph](#openscenegraph) plugin named ["hulk"](#hulk-plugin) to experiment with 3d compression.


## Getting started

1. compile OpenSceneGraph; for Windows see [documentation](http://trac.openscenegraph.org/projects/osg//wiki/Support/PlatformSpecifics/VisualStudio), for Linux/OSX use

    ```
    git clone --branch exercice https://github.com/marchelbling/osg "${HOME}/osg" &&
    mkdir -p "${HOME}/osg/build/release" &&
    cd "${HOME}/osg/build/release" &&
    cmake -DCMAKE_BUILD_TYPE='Release' \
          -DCMAKE_DEBUG_POSTFIX='' \
          -DOSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION=OFF \
          -DCMAKE_INSTALL_PREFIX='/usr/local' \
          ../.. &&
    make -j4 install && echo "Build sucessful" || echo "Build failed"
    ```
1. prepare samples

    ```
    mkdir -p "${HOME}/osg/src/osgPlugins/hulk/data/{raw,compressed}" &&
    unzip "${HOME}/osg/src/osgPlugins/hulk/data/obama.zip"        -d "${HOME}/osg/src/osgPlugins/hulk/data/raw/" &&
    unzip "${HOME}/osg/src/osgPlugins/hulk/data/diorama.zip"      -d "${HOME}/osg/src/osgPlugins/hulk/data/raw/" &&
    unzip "${HOME}/osg/src/osgPlugins/hulk/data/mask.zip"         -d "${HOME}/osg/src/osgPlugins/hulk/data/raw/" &&
    unzip "${HOME}/osg/src/osgPlugins/hulk/data/starry_night.zip" -d "${HOME}/osg/src/osgPlugins/hulk/data/raw/" &&
    unzip "${HOME}/osg/src/osgPlugins/hulk/data/light.zip"        -d "${HOME}/osg/src/osgPlugins/hulk/data/raw/"
    ```
1. run an `osgconv` call on provided samples; by default consider using

    ```
    cd "${HOME}/osg/src/osgPlugins/hulk/data" &&
    LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/ \
    OSG_NOTIFY_LEVEL=WARN \
    OSG_OPTIMIZER=DEFAULT \
    osgconv raw/obama/obama.obj.gles.hulk compressed/obama.osgb -O "noReverseFaces noTriStripPolygons disableTriStrip"
    ```

    which should output

    ```
    Error reading file raw/obama/mayakey.jpg: not implemented
    Error reading file mayakey.jpg: not implemented
    Monitor: animation.no_animation_manager

    Warning: [computeVertexNormals] [[normals]] Geometry '' normals have been recomputed
    Monitor: normal.recompute
    Compressing geometry ...
    Compressing geometry ...
    Compressing geometry ...
    Compressing geometry ...
    Compressing geometry ...
    Compressing geometry ...
    ```

## Objectives

Compression is a broad subject. We want to prototype [*some*](literature--ideas-box) techniques in the context of an OSG [pipeline](#pipeline) and see what gains are easily achievable.

One may approach this in many ways:

* focus on "data pipeline", leveraging the OSG toolbox, and apply "standard" compression
* focus on specific compression of geometric data
* a mix of both.

Overall, as an exploratory phase, we are looking for quick results (along with a way to reproduce the results).

## Constraints

* reasonably fast compression (will be performed server-side)
* fast decompression (will **later** be performed client-side in JavaScript)
* destructive compression accepted as long as not noticible
* keep it simple
* compression should be "model agnostic" and work nicely on 3d scans (see obama sample) as well as high poly sculpts (light) or scenes with multiple objects (diorama, starry night)

## To do

1. implement [`Compressor::compress`](https://github.com/marchelbling/osg/blob/exercice/src/osgPlugins/hulk/Compressor#L34-L37) to reduce a geometry binary size
1. (**optional**) implement [`Compressor::decompress`](https://github.com/marchelbling/osg/blob/exercice/src/osgPlugins/hulk/Compressor#L40-L43) to retrieve a viewable data
1. (**optional**) optimize the [`osgconv` call](#anatomy-of-an-osgconv-call)
1. open a [Pull Request](https://help.github.com/articles/about-pull-requests/) when experimentation is finished!


# OpenSceneGraph

## Overview

We rely on [OpenSceneGraph](https://en.wikipedia.org/wiki/OpenSceneGraph) (OSG) as our "ETL" main tool. It is a C++ (2003 flavor mostly) scene graph manipulation library that supports a bunch of 3d formats IO as well as tools to transform (`osgconv` binary) or visualize (`osgviewer` binary) models.

The project is divided into subsets and we mostly use:

* osg: the core component defining the scene graph nodes
* osgDB: the component defining IO utils and calling osgPlugins to load formats
* osgAnimation: the NodeKit handling animation (not useful for this specific project though).

## Core concepts

To enable extension of core code, OSG heavily relies on the [visitor pattern](https://en.wikipedia.org/wiki/Visitor_pattern) aka double dispatch (see [A polyglot's guide to multiple dispatch](http://eli.thegreenplace.net/2016/a-polyglots-guide-to-multiple-dispatch) for a very nice introduction).

OSG also has garbage collection based on reference counting (`osg::Object` class inherits from `osg::Referenced`). This prevents most memory leaks (especially important for real-time rendering usage of OSG) and, in our case, implies that one should be very careful around memory management. Smart pointers usage should rely on `osg::ref_ptr` and [no other implementation (C++11, boost)](http://forum.openscenegraph.org/viewtopic.php?t=14695&view=next).

## Documentation

For build or other questions, please refer to the (possibly not very up to date —it is sometimes safer to check the code itself—) documentation:

* [Getting started](http://trac.openscenegraph.org/projects/osg/wiki/Support/GettingStarted)
* [OSG CMake](http://trac.openscenegraph.org/projects/osg//wiki/Build/CMake)
* [Visual studio](http://trac.openscenegraph.org/projects/osg//wiki/Support/PlatformSpecifics/VisualStudio)
* [Troubleshooting plugins](http://trac.openscenegraph.org/projects/osg//wiki/Support/PlatformSpecifics/VisualStudio#Importantnoteaboutplugins)
* [OSG environment variables](http://trac.openscenegraph.org/projects/osg//wiki/Support/UserGuides/EnvironmentVariables)
* [OSG forum](http://forum.openscenegraph.org/)



# Processing

## Anatomy of an `osgconv` call

We use `osgconv` binary along with OSG optimizer and plugins to process user data. We typically issue commands like:

```
OSG_NOTIFY_LEVEL=INFO OSG_OPTIMIZER=DEFAULT osgconv my_model.obj.preoptim skfb.osgb.postoptim
```

This call involves 5 stages:

1. `obj`: plugin used to load a model in the OBJ file format
2. `preoptim` (**optional**): pseudo-loader plugin (in the sense that it doesn't parse a file but will process some scene already loaded) used to perform some scene graph processing *before* using OSG optimizer; when using multiple pseudo-loaders, those are called from left-most one to the right-most one
3. [OpenSceneGraph optimizer](http://trac.openscenegraph.org/projects/osg/wiki/Support/UserGuides/OptimizerOptions) (**optional**): controlled through the `OSG_OPTIMIZER` [environment variable](https://github.com/marchelbling/osg/blob/exercice/src/osgUtil/Optimizer.cpp#L65-L136)
4. `postoptim` (**optional**): pseudo-writer plugin used to perform some scene graph processing *after* OSG optimizer; when using multiple pseudo-writers, those are called from right-most one to the left-most one
5. `osgb`: plugin used to save the data in OSG binary format (useful as it supports user-value [de]serialization so this is useful to keep some metadata on graph nodes; e.g. to store compression attributes)

Notes:

* plugins may support some options passed using `-O` flag when calling osgconv; e.g. when loading some OBJ file, we usually add [some options](https://github.com/openscenegraph/OpenSceneGraph/blob/master/src/osgPlugins/obj/ReaderWriterOBJ.cpp#L60-L65): `OSG_NOTIFY_LEVEL=INFO OSG_OPTIMIZER=OFF osgconv data.obj data.osgt -O "noTriStripPolygons noReverseFaces"`.
* we may actually chain more than one pseudo-loader/writer if needed (see [`gles` plugin](#gles-plugin)).


## Gles plugin

Sketchfab uses the `gles` pseudo-loader/writer to make data OpenGL ES compliant. It may be useful for compression as the [`OpenGLESGeometryOptimizer`](https://github.com/openscenegraph/OpenSceneGraph/blob/master/src/osgPlugins/gles/OpenGLESGeometryOptimizer.cpp#L17-L67) has a bunch of visitors providing model optimization for modern rendering:

* transform any overall or per-face attribute to per-vertex attribute
* index all data (most plugins unfortunately generate unindexed `DrawArrays` primitives which is usually both bad in term of data size *and* rendering performance)
* triangulate all faces
* generate vertex normals if not provided
* (**optional**) split large geometries into 65k vertice chunks (note: for our exercise we will not care about this constraint)
* (**optional**) generate triangle strips
* (**optional**) [pre-transform](https://tomforsyth1000.github.io/papers/fast_vert_cache_opt.html) —see Additional Notes— all primitives (as "opposed" to [post-transform](https://www.khronos.org/opengl/wiki/Post_Transform_Cache))


## Hulk plugin

We have just started implementing the `hulk` pseudo-loader/pseudo-writer to handle compression and decompression. The compression/decompression switch depends on the presence of the `isCompressed` [boolean uservalue](https://github.com/marchelbling/osg/blob/exercice/src/osgPlugins/hulk/Compressor#L21) on geometries.

As a first step, we are mostly interested in compressing static 3d data. We are still in an exploratory phase and are interested in prototyping *any* type of compression on geometry (vertex: position, normal, uv) and/or topology (note: we only consider triangles).

We may add any options to ease compression tests (the plugin currently has 2 "supported" —dummy— options: `mode` and `bits`). The compression code will reside in the `Compressor` visitor which ensures that scene geometries are processed only once.


## Literature / ideas box

* [Edgebreaker](http://www.cc.gatech.edu/~jarek/edgebreaker/)
* [TFAN: A Low Complexity 3d Mesh Compression Algorithm](http://www.khaledmammou.com/AllPublications/casa2009.pdf)
* [A Survey of Efficient Representations for Independent Unit Vectors](http://jcgt.org/published/0003/02/01/paper.pdf)
* [OpenCTM](http://openctm.sourceforge.net/) (see the [specification](http://openctm.sourceforge.net/media/FormatSpecification.pdf) for details)
* [Google Body](https://docs.google.com/presentation/d/1XgKaFEgPIzF2psVgY62-KnylV81gsjCWu999h4QtaOE/present#slide=id.i1)
* [Encoding Normal Vectors using Optimized Spherical Coordinates](http://faculty.cs.tamu.edu/schaefer/research/normalCompression.pdf)
* [simple lossless index buffer compression](http://fgiesen.wordpress.com/2013/12/14/simple-lossless-index-buffer-compression/) and [follow-up](http://fgiesen.wordpress.com/2013/12/17/index-compression-follow-up/)
* [quaternion compression](http://www.geomerics.com/blogs/quaternions-rotations-and-compression/)
* [Compressing Polygon Mesh Geometry with Parallelogram Prediction](https://www.cs.unc.edu/~isenburg/papers/ia-cpmgpp-02.pdf)

## Samples:

Here is a set of samples to test the compression code (see [`data`](https://github.com/marchelbling/osg/tree/exercice/src/osgPlugins/hulk/data)):

* obama: [zip](https://github.com/marchelbling/osg/raw/exercice/src/osgPlugins/hulk/data/obama.zip); on [sketchfab](https://sketchfab.com/models/3010d6b0fdd843b397fa9e98437bc22a) (CC Attribution)
* diorama: [zip](https://github.com/marchelbling/osg/raw/exercice/src/osgPlugins/hulk/data/diorama.zip); on [sketchfab](https://sketchfab.com/models/6597e6c9a5184f07a638ac33c08c2ad5) (CC Attribution)
* starry night: [zip](https://github.com/marchelbling/osg/raw/exercice/src/osgPlugins/hulk/data/starry_night.zip); on [sketchfab](https://sketchfab.com/models/3e0b5185d1f8435b993e1bad2f82928e) (CC Attribution)
* light: [zip](https://github.com/marchelbling/osg/raw/exercice/src/osgPlugins/hulk/data/light.zip); on [sketchfab](https://sketchfab.com/models/3e66c241e2cd4e2eb160b2815ad74497) (CC Attribution)
* mask: [zip](https://github.com/marchelbling/osg/raw/exercice/src/osgPlugins/hulk/data/mask.zip); on [sketchfab](https://sketchfab.com/models/9b68ac54e73f4d558c6e1bf55616be63) (CC Attribution)

Note: some samples were modified to not require the FBX plugin compilation.
