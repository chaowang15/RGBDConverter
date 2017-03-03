#ifndef RGBDCONVERTER_H
#define RGBDCONVERTER_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <zlib.h>
#include "global.h"
#include "DataCompression.h"

using namespace std;

const float g_depthScaleFactor = 5000;

class RGBDConverter
{
public:
	RGBDConverter();

	~RGBDConverter();

	void klg2png(string filename);

	void png2klg(string filepath);

private:
	// depth image reader
	bool readDepthImage(string filename, DepthValueType* depthPtr);

	// color image reader
	bool readColorImage(string filename, unsigned char* colorPtr);


private:
	
};

#endif