#include "DataCompression.h"

DataCompression::DataCompression()
{
	m_encodedImage = nullptr;
	m_file = nullptr;
}


DataCompression::~DataCompression()
{
	delete[]m_depthCompressBuf;
	if (m_encodedImage != nullptr)
		cvReleaseMat(&m_encodedImage);

}

void DataCompression::compressColor(cv::Vec<unsigned char, 3> * rgb_data, int width, int height)
{
	cv::Mat3b rgb(height, width, rgb_data, width * 3);
	IplImage * img = new IplImage(rgb);
	int jpeg_params[] = { CV_IMWRITE_JPEG_QUALITY, 90, 0 };
	if (m_encodedImage != 0)
		cvReleaseMat(&m_encodedImage);
	m_encodedImage = cvEncodeImage(".jpg", img, jpeg_params);
	m_colorCompressSize = m_encodedImage->width;
	delete img;
}

void DataCompression::compressDepth(unsigned char* depthDataPtr){
	m_depthCompressSize = m_depthOriginalSize; // initialize the depth size that is large enough to hold all compressed data.
	int res = compress2(m_depthCompressBuf, &m_depthCompressSize, depthDataPtr, m_depthOriginalSize, Z_BEST_SPEED);
	if (res != 0)
		cerr << "WARNING: Compression Error !" << endl;
}


void DataCompression::writeHeader(string klgFilename, int frameNum)
{
	m_file = fopen(klgFilename.c_str(), "wb+");
	// We usually write the number of frames as 0 here temporarily,
	// and update it later after scanning.
	fwrite(&frameNum, sizeof(int32_t), 1, m_file);

	// The klg file used in ElasticFusion code can NOT contain resolution
	// parameters, which are taken as the code input, Here we still
	// leave the following codes just in case you need to compress these 
	// parameters for some other applications.
	//fwrite(&depthWidth, sizeof(int32_t), 1, m_file);
	//fwrite(&depthHeight, sizeof(int32_t), 1, m_file);
	//fwrite(&colorWidth, sizeof(int32_t), 1, m_file);
	//fwrite(&colorHeight, sizeof(int32_t), 1, m_file);
}

void DataCompression::writeBody(int64_t timestamp)
{
	fwrite(&timestamp, sizeof(int64_t), 1, m_file);
	fwrite(&m_depthCompressSize, sizeof(int32_t), 1, m_file);
	fwrite(&m_colorCompressSize, sizeof(int32_t), 1, m_file);
	fwrite((unsigned char *)m_depthCompressBuf, m_depthCompressSize, 1, m_file);
	fwrite((unsigned char *)m_encodedImage->data.ptr, m_colorCompressSize, 1, m_file);
}

void DataCompression::closeKLGFile(int frameNum)
{
	fseek(m_file, 0, SEEK_SET); // frame number is stored in the header at the 0th position
	fwrite(&frameNum, sizeof(int32_t), 1, m_file);
	fflush(m_file);
	fclose(m_file);
}
