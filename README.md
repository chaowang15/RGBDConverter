# RGBDConverter
This code converts RGB-D data between PNG/JPG images and a compressed file format KLG used in [ElasticFusion](https://github.com/mp3guy/ElasticFusion) code. Another code [RGBDCapture](https://github.com/chaowang15/RGBDCapture) can be used to scan and save RGB-D data in PNG images.

This code is written in C++ and has already been tested successfully in Visual Studio 2013 in Windows and Ubuntu 16.04.

## Dependencies
- OpenCV (>= 2.4.7) -- read and write image files and compress color image
- Boost (>= 1.50) -- file system
- Zlib -- compress depth image

## Build
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

## Usage
```bash
RGBDConverter -p2k/-k2p InputFilePath <depthWidth depthHeight colorWidth colorHeight> <associationFile>
```
where
* *--help*: display the help text
* *-p [ --p2k ]*: compress PNG images to a singe KLG file
* *-k [ --k2p ]*: decompress KLG file into PNG images
* *-f [ --file_path ]*        path of KLG file or PNG images
* *-w [ --depth_width ]*:      width of depth image
* *-h [ --depth_height ]*:     height of depth image
* *-c [ --color_width ]*:      width of color image
* *-t [ --color_height ]*:     height of color image
* *-a [ --association_file ]*: association/correspondence between depth and color timestamps (only used in -p2k). Do NOT add it if corresponding depth and color timestamps are exactly the same.

## Data Format
* The PNG images are exactly the same as [TUM dataset](https://vision.in.tum.de/data/datasets/rgbd-dataset).
* See the source code or [Logger2 code](https://github.com/mp3guy/Logger2/blob/master/src/Logger2.cpp#L237) for details about information in KLG format.
