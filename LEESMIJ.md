# README van het 4d-tree builder project

## Binary Path
Path: \binaries\

## BINARY TYPES:
There are 2 types of binaries of the builder project:
1. svo_builder_binary.exe
	It is used to build a 4d-tree for a source file containing only geometry. No normal data is going to be used.
2. svo_builder.exe		
	NOT YET FULLY TESTED/SUPPORTED

**Syntax**: svo_builder(\_binary) -options

* -f (path to .tri file) : The path to the .tri file you want to build an SVO from. (Required)
* -ss : spacial gridsize
* -st : temporal gridsize
* -l : (memory limit) : The memory limit for the SVO builder, in Mb. This is where the out-of-core part kicks in, of course. The tool will automatically select the most optimal partition size depending on the given memory limit. (Default: 2048)
* -v : Be very verbose, for debugging purposes. Switch this on if you're running into problems.
* -levels: Generate intermediare SVO levels' voxel payloads by averaging data from lower levels (which is a quick and dirty way to do low-cost Level-Of-Detail hierarchies). If this option is not specified, only the leaf nodes have an actual payload. (Default: off)
* -c (color_mode) :  NOT IN BINARY VOXELIZATION  
Generate colors for the voxels. Keep in mind that when you're using the geometry-only version of the tool (svo_builder_binary), all the color options will be ignored and the voxels will just get a fixed white color. Options for color mode: (Default: model)  
  * model : Give all voxels the color which is embedded in the .tri file. (Which will be white if the original model contained no vertex color information).
  * linear : Give voxels a linear RGB color related to their position in the grid.
  * normal : Get colors for voxels from sample normals of original triangles.
  * fixed : Give voxels a fixed color, configurable in the source code.
* -h : pints help information
* -binvox : generates binvox files for the model on each time point. Useful during debugging to check whether there is an error in the voxelization process.
