# Exercise: Cleaning Geometries

Sketchfab aims at being a safe place for all 3D assets. Sketchfab users usually upload files with wrong attributes, bad normals or non performant structures.
We will therefore write a new [OpenSceneGraph](#openscenegraph) plugin to clean objects and log errors in a human readable structures.


## Getting started

1. compile OpenSceneGraph: see [documentation](https://github.com/openscenegraph/OpenSceneGraph/blob/master/README.md)
1. get some samples from the [data](https://github.com/sketchfab/osg-exercice/blob/exercice-cleaner/src/osgPlugins/cleaner/data/) folder
1. run an `osgconv` call on fetched samples to test the parser (notice the usage of .cleaner pseudoloader)

    ```
    LD_LIBRARY_PATH=/path/to/OSG/libs/ \
    OSG_NOTIFY_LEVEL=WARN \
    OSG_OPTIMIZER=DEFAULT \
    osgconv sample.obj.cleaner sample.osgt
    ```

## Objectives

* get some knowledge of Sketchfab tools used in the processing pipeline
* implement a new Cleaner plugin that handles wrong user data
* Log the results and put them in a human readable format(json)


## To do

1. Start from (https://github.com/sketchfab/osg-exercice/blob/exercice-cleaner/src/osgPlugins/cleaner/ReaderWriterCleaner.cpp)
1. A lazy programmer started the plugin implementing a CleanerVisitor. It compiles but it has some bugs. So right now is not working at all. First of all fix the errors in the code to get it working.
1. After that, implement the code missing to check vertices, normals and UVs. The code should fix wrong data when possible and generating log errors when needed. If fixing is not possible, dismiss the geometry. (If a node does not have any valid geometry then the node is invalid).
1. Finally create a python script that calls osgconv and writes the logs from the Cleaner plugin to a json file. We also want to know how many errors of each type the plugin ecountered.
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