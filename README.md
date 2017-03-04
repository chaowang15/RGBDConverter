# RGBDConverter
This code converts RGB-D data between PNG images and a single KLG log file used in [ElasticFusion] (https://github.com/mp3guy/ElasticFusion) code. KLG file contains the rgb and depth data from all sequences in compressed binary format.

This code is written in C++ and is tested successfully in Visual Studio 2013 in Windows and Ubuntu 14.04.

##Dependencies
- OpenCV 2.4.X -- read and write image files
- Boost (>= 1.50) -- create folders
- Zlib -- compress image

##Build
To compile the code:
* In Windows, use Visual Studio to open .sln file and compile.
* In Linux, use the standard approach
```
  $ mkdir build
  $ cd build
  $ cmake ..
  $ make
```
Note to modify corresponding paths of dependencies.

##Usage
```bash
RGBDConverter -option filepath
```
where 
*-option*:
* -p2k: convert PNG images to KLG file
* -k2p: convert KLG file to PNG images

*filepath*: the path of PNG images or KLG file

##Note
* The PNG images must be saved in separate folders: all depth images should be saved in a *depth* folder, while all color images must be saved in *rgb* folder. And all images are named as X.png where X is the frame index number starting from 0. For instance:
```bash
rgbd
 - depth
   - 0.png 1.png 2.png ...
 - rgb
   - 0.png 1.png 2.png ...
```
* See the source code for details about information in KLG format.
