// ***************************************************************************
//
//   Generated automatically by genwrapper.
//   Please DO NOT EDIT this file!
//
// ***************************************************************************

#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/StaticMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/BoundingSphere>
#include <osg/CopyOp>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osg/OperationThread>
#include <osg/TransferFunction>
#include <osgTerrain/Layer>
#include <osgTerrain/Locator>
#include <osgTerrain/TerrainNode>
#include <osgTerrain/TerrainTechnique>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ENUM_REFLECTOR(osgTerrain::TerrainNode::Filter)
	I_DeclaringFile("osgTerrain/TerrainNode");
	I_EnumLabel(osgTerrain::TerrainNode::NEAREST);
	I_EnumLabel(osgTerrain::TerrainNode::LINEAR);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osgTerrain::TerrainNode)
	I_DeclaringFile("osgTerrain/TerrainNode");
	I_BaseType(osg::Group);
	I_Constructor0(____TerrainNode,
	               "",
	               "");
	I_ConstructorWithDefaults2(IN, const osgTerrain::TerrainNode &, x, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY,
	                           ____TerrainNode__C5_TerrainNode_R1__C5_osg_CopyOp_R1,
	                           "Copy constructor using CopyOp to manage deep vs shallow copy. ",
	                           "");
	I_Method0(osg::Object *, cloneType,
	          Properties::VIRTUAL,
	          __osg_Object_P1__cloneType,
	          "clone an object of the same type as the node. ",
	          "");
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop,
	          Properties::VIRTUAL,
	          __osg_Object_P1__clone__C5_osg_CopyOp_R1,
	          "return a clone of a node, with Object* return type. ",
	          "");
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj,
	          Properties::VIRTUAL,
	          __bool__isSameKindAs__C5_osg_Object_P1,
	          "return true if this and obj are of the same kind of object. ",
	          "");
	I_Method0(const char *, className,
	          Properties::VIRTUAL,
	          __C5_char_P1__className,
	          "return the name of the node's class type. ",
	          "");
	I_Method0(const char *, libraryName,
	          Properties::VIRTUAL,
	          __C5_char_P1__libraryName,
	          "return the name of the node's library. ",
	          "");
	I_Method1(void, accept, IN, osg::NodeVisitor &, nv,
	          Properties::VIRTUAL,
	          __void__accept__osg_NodeVisitor_R1,
	          "Visitor Pattern : calls the apply method of a NodeVisitor with this node's type. ",
	          "");
	I_Method1(void, traverse, IN, osg::NodeVisitor &, nv,
	          Properties::VIRTUAL,
	          __void__traverse__osg_NodeVisitor_R1,
	          "Traverse downwards : calls children's accept method with NodeVisitor. ",
	          "");
	I_Method0(void, init,
	          Properties::NON_VIRTUAL,
	          __void__init,
	          "Call init on any attached TerrainTechnique. ",
	          "");
	I_Method1(void, setTerrainTechnique, IN, osgTerrain::TerrainTechnique *, TerrainTechnique,
	          Properties::NON_VIRTUAL,
	          __void__setTerrainTechnique__osgTerrain_TerrainTechnique_P1,
	          "Set the TerrainTechnique. ",
	          "");
	I_Method0(osgTerrain::TerrainTechnique *, getTerrainTechnique,
	          Properties::NON_VIRTUAL,
	          __TerrainTechnique_P1__getTerrainTechnique,
	          "Get the TerrainTechnique. ",
	          "");
	I_Method0(const osgTerrain::TerrainTechnique *, getTerrainTechnique,
	          Properties::NON_VIRTUAL,
	          __C5_TerrainTechnique_P1__getTerrainTechnique,
	          "Get the const TerrainTechnique. ",
	          "");
	I_Method1(void, setLocator, IN, osgTerrain::Locator *, locator,
	          Properties::NON_VIRTUAL,
	          __void__setLocator__Locator_P1,
	          "Set the coordinate frame locator of the terrain node. ",
	          "The locator takes non-dimensional s,t coordinates into the X,Y,Z world coords and back. ");
	I_Method0(osgTerrain::Locator *, getLocator,
	          Properties::NON_VIRTUAL,
	          __Locator_P1__getLocator,
	          "Get the coordinate frame locator of the terrain node. ",
	          "");
	I_Method0(const osgTerrain::Locator *, getLocator,
	          Properties::NON_VIRTUAL,
	          __C5_Locator_P1__getLocator,
	          "Get the coordinate frame locator of the terrain node. ",
	          "");
	I_Method1(void, setElevationLayer, IN, osgTerrain::Layer *, layer,
	          Properties::NON_VIRTUAL,
	          __void__setElevationLayer__Layer_P1,
	          "Set the layer to use to define the elevations of the terrain. ",
	          "");
	I_Method0(osgTerrain::Layer *, getElevationLayer,
	          Properties::NON_VIRTUAL,
	          __Layer_P1__getElevationLayer,
	          "Get the layer to use to define the elevations of the terrain. ",
	          "");
	I_Method0(const osgTerrain::Layer *, getElevationLayer,
	          Properties::NON_VIRTUAL,
	          __C5_Layer_P1__getElevationLayer,
	          "Get the const layer to use to define the elevations of the terrain. ",
	          "");
	I_Method2(void, setColorLayer, IN, unsigned int, i, IN, osgTerrain::Layer *, layer,
	          Properties::NON_VIRTUAL,
	          __void__setColorLayer__unsigned_int__osgTerrain_Layer_P1,
	          "Set a color layer with specified layer number. ",
	          "");
	I_Method1(osgTerrain::Layer *, getColorLayer, IN, unsigned int, i,
	          Properties::NON_VIRTUAL,
	          __Layer_P1__getColorLayer__unsigned_int,
	          "Get color layer with specified layer number. ",
	          "");
	I_Method1(const osgTerrain::Layer *, getColorLayer, IN, unsigned int, i,
	          Properties::NON_VIRTUAL,
	          __C5_Layer_P1__getColorLayer__unsigned_int,
	          "Set const color layer with specified layer number. ",
	          "");
	I_Method2(void, setColorTransferFunction, IN, unsigned int, i, IN, osg::TransferFunction *, tf,
	          Properties::NON_VIRTUAL,
	          __void__setColorTransferFunction__unsigned_int__osg_TransferFunction_P1,
	          "Set a color transfer function with specified layer number. ",
	          "");
	I_Method1(osg::TransferFunction *, getColorTransferFunction, IN, unsigned int, i,
	          Properties::NON_VIRTUAL,
	          __osg_TransferFunction_P1__getColorTransferFunction__unsigned_int,
	          "Get color transfer function with specified layer number. ",
	          "");
	I_Method1(const osg::TransferFunction *, getColorTransferFunction, IN, unsigned int, i,
	          Properties::NON_VIRTUAL,
	          __C5_osg_TransferFunction_P1__getColorTransferFunction__unsigned_int,
	          "Get const color transfer function with specified layer number. ",
	          "");
	I_Method2(void, setColorFilter, IN, unsigned int, i, IN, osgTerrain::TerrainNode::Filter, filter,
	          Properties::NON_VIRTUAL,
	          __void__setColorFilter__unsigned_int__Filter,
	          "Set a color filter with specified layer number. ",
	          "");
	I_Method1(osgTerrain::TerrainNode::Filter, getColorFilter, IN, unsigned int, i,
	          Properties::NON_VIRTUAL,
	          __Filter__getColorFilter__unsigned_int,
	          "Set const color filter with specified layer number. ",
	          "");
	I_Method0(unsigned int, getNumColorLayers,
	          Properties::NON_VIRTUAL,
	          __unsigned_int__getNumColorLayers,
	          "Get the number of colour layers. ",
	          "");
	I_Method1(void, setRequiresNormals, IN, bool, flag,
	          Properties::NON_VIRTUAL,
	          __void__setRequiresNormals__bool,
	          "Set hint to whether the TerrainTechnique should create per vertex normals for lighting purposes. ",
	          "");
	I_Method0(bool, getRequiresNormals,
	          Properties::NON_VIRTUAL,
	          __bool__getRequiresNormals,
	          "Get whether the TerrainTechnique should create per vertex normals for lighting purposes. ",
	          "");
	I_Method1(void, setTreatBoundariesToValidDataAsDefaultValue, IN, bool, flag,
	          Properties::NON_VIRTUAL,
	          __void__setTreatBoundariesToValidDataAsDefaultValue__bool,
	          "Set the hint to whether the TerrainTechnique should treat the invalid Layer entries that at are neigbours to valid entries with the default value. ",
	          "");
	I_Method0(bool, getTreatBoundariesToValidDataAsDefaultValue,
	          Properties::NON_VIRTUAL,
	          __bool__getTreatBoundariesToValidDataAsDefaultValue,
	          "Get whether the TeatBoundariesToValidDataAsDefaultValue hint. ",
	          "");
	I_Method1(void, setOperationQueue, IN, osg::OperationQueue *, operations,
	          Properties::NON_VIRTUAL,
	          __void__setOperationQueue__osg_OperationQueue_P1,
	          "Set an OperationQueue to do an data initialization and update work. ",
	          "");
	I_Method0(osg::OperationQueue *, getOperationQueue,
	          Properties::NON_VIRTUAL,
	          __osg_OperationQueue_P1__getOperationQueue,
	          "Get the OperationsQueue if one is attached, return NULL otherwise. ",
	          "");
	I_Method0(const osg::OperationQueue *, getOperationsQueue,
	          Properties::NON_VIRTUAL,
	          __C5_osg_OperationQueue_P1__getOperationsQueue,
	          "Get the const OperationsQueue if one is attached, return NULL otherwise. ",
	          "");
	I_Method0(osg::BoundingSphere, computeBound,
	          Properties::VIRTUAL,
	          __osg_BoundingSphere__computeBound,
	          "Compute the bounding volume of the terrain by computing the union of the bounding volumes of all layers. ",
	          "");
	I_IndexedProperty(osgTerrain::TerrainNode::Filter, ColorFilter, 
	                  __Filter__getColorFilter__unsigned_int, 
	                  __void__setColorFilter__unsigned_int__Filter, 
	                  0);
	I_ArrayProperty(osgTerrain::Layer *, ColorLayer, 
	                __Layer_P1__getColorLayer__unsigned_int, 
	                __void__setColorLayer__unsigned_int__osgTerrain_Layer_P1, 
	                __unsigned_int__getNumColorLayers, 
	                0, 
	                0, 
	                0);
	I_IndexedProperty(osg::TransferFunction *, ColorTransferFunction, 
	                  __osg_TransferFunction_P1__getColorTransferFunction__unsigned_int, 
	                  __void__setColorTransferFunction__unsigned_int__osg_TransferFunction_P1, 
	                  0);
	I_SimpleProperty(osgTerrain::Layer *, ElevationLayer, 
	                 __Layer_P1__getElevationLayer, 
	                 __void__setElevationLayer__Layer_P1);
	I_SimpleProperty(osgTerrain::Locator *, Locator, 
	                 __Locator_P1__getLocator, 
	                 __void__setLocator__Locator_P1);
	I_SimpleProperty(osg::OperationQueue *, OperationQueue, 
	                 __osg_OperationQueue_P1__getOperationQueue, 
	                 __void__setOperationQueue__osg_OperationQueue_P1);
	I_SimpleProperty(const osg::OperationQueue *, OperationsQueue, 
	                 __C5_osg_OperationQueue_P1__getOperationsQueue, 
	                 0);
	I_SimpleProperty(bool, RequiresNormals, 
	                 __bool__getRequiresNormals, 
	                 __void__setRequiresNormals__bool);
	I_SimpleProperty(osgTerrain::TerrainTechnique *, TerrainTechnique, 
	                 __TerrainTechnique_P1__getTerrainTechnique, 
	                 __void__setTerrainTechnique__osgTerrain_TerrainTechnique_P1);
	I_SimpleProperty(bool, TreatBoundariesToValidDataAsDefaultValue, 
	                 __bool__getTreatBoundariesToValidDataAsDefaultValue, 
	                 __void__setTreatBoundariesToValidDataAsDefaultValue__bool);
END_REFLECTOR

