#ifndef RGBDCONVERTER_H
#define RGBDCONVERTER_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <zlib.h>
#include "DataCompression.h"

using namespace std;

class RGBDConverter
{
public:
	RGBDConverter(int depthWidth, int depthHeight, int colorWidth, int colorHeight) :
		m_depthWidth(depthWidth), m_depthHeight(depthHeight), m_colorWidth(colorWidth), m_colorHeight(colorHeight) {}

	~RGBDConverter(){}

	void klg2png(string filename);

	void png2klg(string filepath, string association_file = "");

	using DepthValueType = unsigned short;


private:
	// depth image reader
	bool readDepthImage(string filename, DepthValueType* depthPtr);

	// color image reader
	bool readColorImage(string filename, unsigned char* colorPtr);

	int m_depthWidth;
	int m_depthHeight;
	int m_colorWidth;
	int m_colorHeight;
	float m_depthScaleFactor = 5.0;
};

#endif