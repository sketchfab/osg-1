# Exercise: parsing OFF format

Sketchfab aims at being a safe place for all 3D assets and as such, we need to support as many 3D formats as possible.
We will therefore write a new [OpenSceneGraph](#openscenegraph) plugin to read the [OFF](http://paulbourke.net/dataformats/off/) format.


## Getting started

1. compile OpenSceneGraph; for Windows see [documentation](http://www.openscenegraph.org/index.php/documentation/platform-specifics/windows/37-visual-studio), for Linux/OSX use

    ```
    git clone --branch exercice-off https://github.com/sketchfab/osg "${HOME}/osg" &&
    mkdir -p "${HOME}/osg/build/release" &&
    cd "${HOME}/osg/build/release" &&
    cmake -DCMAKE_BUILD_TYPE='Release' \
          -DCMAKE_DEBUG_POSTFIX='' \
          -DOSG_USE_REF_PTR_IMPLICIT_OUTPUT_CONVERSION=OFF \
          -DCMAKE_INSTALL_PREFIX='/usr/local' \
          ../.. &&
    make -j4 install && echo "Build sucessful" || echo "Build failed"
    ```
1. get some samples e.g. from http://people.sc.fsu.edu/~jburkardt/data/off/off.html
1. run an `osgconv` call on fetched samples to test the parser

    ```
    LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/lib64/ \
    OSG_NOTIFY_LEVEL=WARN \
    OSG_OPTIMIZER=DEFAULT \
    osgconv sample.off sample.osgt
    ```

## Objectives

* get some knowledge of Sketchfab tools used in the processing pipeline
* understand how Sketchfab scenes are structured
* implement a new plugin that can be reused/extended by the community


## To do

1. add the plugin structure by copying other plugins
1. implement the `osgDB::ReaderWriter::readNode` to parse OFF ascii files
1. (optional) implement the `osgDB::ReaderWriter::writeNode` to dump OFF ascii files
1. (optional) implement OFF binary support
1. open a [Pull Request](https://help.github.com/articles/about-pull-requests/) when done!


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

* [Getting started](http://www.openscenegraph.org/index.php/documentation/getting-started)
* [OSG CMake](https://github.com/openscenegraph/OpenSceneGraph#section-1-how-to-build-openscenegraph)
* [Visual studio](http://www.openscenegraph.org/index.php/documentation/platform-specifics/windows)
* [Troubleshooting plugins](http://www.openscenegraph.org/projects/osg/wiki/Support/PlatformSpecifics/VisualStudio#Importantnoteaboutplugins)
* [OSG environment variables](http://www.openscenegraph.org/projects/osg/wiki/Support/UserGuides/EnvironmentVariables)
* [OSG forum](http://forum.openscenegraph.org/)
