#include "RGBDConverter.h"

RGBDConverter::RGBDConverter()
{

}

RGBDConverter::~RGBDConverter()
{

}

bool RGBDConverter::readColorImage(string filename, unsigned char* colorPtr)
{
	cv::Mat colorImg = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
	if (colorImg.data && colorImg.depth() == CV_8U)
	{
		int rows = colorImg.rows, cols = colorImg.cols;
#pragma omp parallel for
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				int tmp = (i * cols + j) * 3;
				Vec3b bgr = colorImg.at<Vec3b>(i, j);
				for (int j = 0; j < 3; ++j)
				{
					// OpenCV loads color image in BGR order while we need RGB
					colorPtr[tmp + 2 - j] = bgr(j);
					//cout << int(bgr(j)) << " ";
				}
				//cout << endl;
			}
		}
		return true;
	}
	else
	{
		//std::cerr << "CHAO WARNING: cannot read color image " << filename << std::endl;
		return false;
	}
}

bool RGBDConverter::readDepthImage(string filename, DepthValueType* depthPtr)
{
	cv::Mat depthImg = cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH);
	if (depthImg.depth() == CV_16U)
	{
		cv::Mat scaledDepth;
		depthImg.convertTo(scaledDepth, CV_16UC1, static_cast<double>(1.0/c_depthScaleFactor));

		int rows = scaledDepth.rows, cols = scaledDepth.cols;
#pragma omp parallel for
		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				// Remember to scale the depth value by scale factor
				depthPtr[i*cols + j] = scaledDepth.at<DepthValueType>(i, j);
				//cout << depthPtr[i*cols + j] << endl;
			}
		}
		return true;
	}
	else
	{
		//std::cerr << "CHAO WARNING: cannot read depth image " << filename << std::endl;
		return false;
	}
}

void RGBDConverter::png2klg(string filepath)
{
	string filename = filepath;
	if (filename.back() == '/' || filename.back() == '\\')
	{
		filename = filename.substr(0, filename.length()-1) + ".klg";
	}
	else
	{
		filename += ".klg";
		filepath += "/";
	}
	// Get depth image resolution
	string depthPath = filepath + "depth/";
	string colorPath = filepath + "rgb/";
	int frameNum = 0, depthWidth = 0, depthHeight = 0, colorWidth = 0, colorHeight = 0;
	cv::Mat depthImg = cv::imread(depthPath + "0.png", CV_LOAD_IMAGE_ANYDEPTH);
	if (!depthImg.data)                              // Check for invalid input
	{
		std::cout << "WARNING: Could not open or find depth image in the path " << depthPath << std::endl;
		return;
	}
	depthWidth = depthImg.cols;
	depthHeight = depthImg.rows;
	// Get color image resolution
	cv::Mat colorImg = cv::imread(colorPath + "0.png", CV_LOAD_IMAGE_COLOR);
	if (!colorImg.data)                              // Check for invalid input
	{
		std::cout << "WARNING: Could not open or find color image in the path " << colorImg << std::endl;
		return;
	}
	colorWidth = colorImg.cols;
	colorHeight = colorImg.rows;

	unsigned char* colorPtr = new unsigned char[colorWidth * colorHeight * 3];
	DepthValueType* depthPtr = new DepthValueType[depthWidth * depthHeight];

	DataCompression dataComp;
	dataComp.writeHeader(filename, frameNum, depthWidth, depthHeight, colorWidth, colorHeight);
	
	while (true)
	{
		cout << "Compressing frame " << frameNum << " ..." << endl;
		string colorImgName = colorPath + to_string(frameNum) + ".png";
		bool flag = readColorImage(colorImgName, colorPtr);
		if (!flag)
		{
			cout << "No further frames. Compression is finished. Quitting..." << endl;
			break;
		}
		dataComp.compressColor((cv::Vec<unsigned char, 3> *)colorPtr, colorWidth, colorHeight);

		string depthImgName = depthPath + to_string(frameNum) + ".png";
		flag = readDepthImage(depthImgName, depthPtr);
		if (!flag)
		{
			break;
		}
		dataComp.compressDepth((unsigned char*)depthPtr);
		
		dataComp.writeBody(frameNum);

		
		frameNum++;
	}
	dataComp.closeKLGFile(frameNum);

}

void RGBDConverter::klg2png(string filename)
{
	FILE* logFile = fopen(filename.c_str(), "rb");
	if (!logFile)
	{
		cerr << "WARNING: cannot open the file " << filename << endl;
		return;
	}

	string folder = "./" + filename.substr(0, filename.length() - 4);
	string depthFolder = folder + "/depth/";
	string colorFolder = folder + "/rgb/";
	boost::filesystem::path dir(folder);
	boost::filesystem::path dirDepth(depthFolder);
	boost::filesystem::path dirColor(colorFolder);

	// Create local folder/subfolders
	if (boost::filesystem::is_directory(dir)){
		boost::filesystem::remove_all(dirDepth);
		boost::filesystem::remove_all(dirColor);
	}
	boost::filesystem::create_directory(dir);
	boost::filesystem::create_directory(dirDepth);
	boost::filesystem::create_directory(dirColor);

	int depthWidth = 0, depthHeight = 0, colorWidth = 0, colorHeight = 0;
	int numFrames = 0, frameIdx = 0, depthSize = 0, rgbSize = 0;
	fread(&numFrames, sizeof(int), 1, logFile);
	fread(&depthWidth, sizeof(int), 1, logFile);
	fread(&depthHeight, sizeof(int), 1, logFile);
	fread(&colorWidth, sizeof(int), 1, logFile);
	fread(&colorHeight, sizeof(int), 1, logFile);

	unsigned char *rgbData = new unsigned char[colorWidth * colorHeight];
	unsigned char *depthData = new unsigned char[depthWidth * depthHeight * sizeof(DepthValueType)];
	int numDigits = getNumberOfDigits(numFrames);
	for (int i = 0; i < numFrames; ++i)
	{
		cout << "Decompressing frame " << i << "..." << endl;

		fread(&frameIdx, sizeof(int), 1, logFile);
		fread(&depthSize, sizeof(int), 1, logFile);
		fread(&rgbSize, sizeof(int), 1, logFile);

		memset(depthData, 0, depthWidth * depthHeight * 2);

		// Decompress depth image
		unsigned char *depthDataBinary = new unsigned char[depthSize];
		fread(depthDataBinary, depthSize, 1, logFile);
		unsigned long len = (unsigned long)(depthWidth * depthHeight * 2);
		int res = uncompress(depthData, &len, depthDataBinary, (unsigned long)depthSize);
		delete[]depthDataBinary;

		// Scale depth image
		depthDataBinary = NULL;
		cv::Mat mImageDepth(depthHeight, depthWidth, CV_16UC1, (void *)depthData);
		cv::Mat mScaledDepth;
		mImageDepth.convertTo(mScaledDepth, CV_16UC1, c_depthScaleFactor);
		//cv::flip(mScaledDepth, mScaledDepth, 1);

		string depthImageName = depthFolder + to_string(frameIdx) + ".png";
		cv::imwrite(depthImageName, mScaledDepth);

		// Decompress color image
		fread(rgbData, rgbSize, 1, logFile);
		CvMat mat = cvMat(colorHeight, colorWidth, CV_8UC3, rgbData);
		//IplImage *p = cvDecodeImage( &mat, 1 );
		CvMat *p = cvDecodeImageM(&mat, 1);
		cv::Mat m(p);
		cv::cvtColor(m, m, CV_BGR2RGB);

		// Write color image
		//cv::flip(m, m, 1);
		string rgbImageName = colorFolder + to_string(frameIdx) + ".png";
		imwrite(rgbImageName, m);
	}
	fclose(logFile);
	delete[]rgbData;
	delete[]depthData;
}