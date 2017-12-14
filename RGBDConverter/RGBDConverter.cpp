#include "RGBDConverter.h"
#include <fstream>

bool RGBDConverter::readColorImage(string filename, unsigned char* colorPtr)
{
	cv::Mat colorImg = cv::imread(filename, CV_LOAD_IMAGE_COLOR);
	if (colorImg.empty() || colorImg.depth() != CV_8U)
		return false;
	int rows = colorImg.rows, cols = colorImg.cols;
#pragma omp parallel for
	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			int tmp = (i * cols + j) * 3;
			Vec3b bgr = colorImg.at<Vec3b>(i, j);
			for (int k = 0; k < 3; ++k)
			{
				// OpenCV loads color image in BGR order while we need RGB
				colorPtr[tmp + 2 - k] = bgr(k);
			}
		}
	}
	return true;
}

bool RGBDConverter::readDepthImage(string filename, DepthValueType* depthPtr)
{
	cv::Mat depthImg = cv::imread(filename, CV_LOAD_IMAGE_ANYDEPTH);
	if (depthImg.empty() || depthImg.depth() != CV_16U)
		return false;

	cv::Mat scaledDepth;
	// Remember to scale the depth value by scale factor
	depthImg.convertTo(scaledDepth, CV_16UC1, static_cast<double>(1.0 / m_depthScaleFactor));

	int rows = scaledDepth.rows, cols = scaledDepth.cols;
#pragma omp parallel for
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			depthPtr[i*cols + j] = scaledDepth.at<DepthValueType>(i, j);
		}
	}
	return true;
}

void RGBDConverter::png2klg(string filepath, string association_file)
{
	string klg_filename = filepath;
	if (filepath.back() == '/' || filepath.back() == '\\')
	{
		klg_filename = filepath.substr(0, filepath.length() - 1);
	}
	else
	{
		filepath += "/";
	}
	size_t pos = klg_filename.find_last_of("/\\");
	klg_filename = klg_filename.substr(pos + 1, klg_filename.length() - pos - 1) + ".klg";

	// Get depth image resolution
	boost::filesystem::path depthPath(filepath + "depth/"), colorPath(filepath + "rgb/");
	cout << depthPath << endl << colorPath << endl;
	if (!boost::filesystem::is_directory(depthPath) || !boost::filesystem::is_directory(colorPath))
	{
		cout << "WARNING: The input path \"" << filepath << "\" does not contain depth or rgb folder. Quiting..." << endl;
		return;
	}

	int frameNum = 0;
	int64_t timestamp = 0;
	string depth_filename, color_filename;
	unsigned char* colorPtr = new unsigned char[m_colorWidth * m_colorHeight * 3];
	DepthValueType* depthPtr = new DepthValueType[m_depthWidth * m_depthHeight];

	DataCompression dataComp;
	int originalMemory = m_depthWidth * m_depthHeight * sizeof(DepthValueType);
	dataComp.initDepthMemory(originalMemory);
	dataComp.writeHeader(klg_filename);

	if (association_file == "")
	{
		// If there is no association file, then corresponding depth and color images have exactly the SAME name. 
		// Just traverse all images in the depth and color folder.
		vector<boost::filesystem::path> depthFilelists;
		std::copy(boost::filesystem::directory_iterator(depthPath), boost::filesystem::directory_iterator(), back_inserter(depthFilelists));

		// Sort the images according to the timestamp, i.e., name of the image files.
		auto cmp = [](boost::filesystem::path p1, boost::filesystem::path p2){
			string s1 = p1.string(), s2 = p2.string();
			size_t pos1 = p1.string().find_last_of("/\\"), pos2 = p2.string().find_last_of("/\\");
			return stoll(s1.substr(pos1 + 1, s1.length() - pos1 - 5)) < stoll(s2.substr(pos2 + 1, s2.length() - pos2 - 5));
		};
		sort(depthFilelists.begin(), depthFilelists.end(), cmp);
		for (vector<boost::filesystem::path>::const_iterator it(depthFilelists.begin()), it_end(depthFilelists.end()); it != it_end; ++it)
		{
			// Compress depth image
			depth_filename = it->string();
			bool flag = readDepthImage(depth_filename, depthPtr);
			if (!flag)
			{
				cout << "WARNING: can NOT read depth image \"" << depth_filename << "\". Quiting ..." << endl;
				return;
			}
			dataComp.compressDepth((unsigned char*)depthPtr);

			// Compress color image
			size_t pos = depth_filename.find_last_of("/\\");
			string timestamp_str = depth_filename.substr(pos + 1, depth_filename.length() - pos - 5);
			timestamp = stoll(timestamp_str);
			color_filename = colorPath.string() + timestamp_str + ".png";
			flag = readColorImage(color_filename, colorPtr);
			if (!flag)
			{
				cout << "WARNING: can NOT read color image \"" << color_filename << "\". Quiting ..." << endl;
				return;
			}
			dataComp.compressColor((cv::Vec<unsigned char, 3> *)colorPtr, m_colorWidth, m_colorHeight);

			// Write compressed data
			dataComp.writeBody(timestamp);
			frameNum++;

			cout << "Timestamp " << timestamp << " is done." << endl;
		}
	}
	else
	{
		// If there is association file as input, then it contains the correspondence between
		// depth and color image timestamps. 
		string tmp, timestamp_str;
		ifstream readin(association_file.c_str(), ios::in);
		if (!readin.good())
		{
			cout << "WARNING: can NOT read the association file \"" << association_file << "\". Quiting..." << endl;
			return;
		}
		while (readin.good())
		{
			readin >> timestamp_str >> depth_filename >> tmp >> color_filename;
			if (readin.good())
			{
				size_t pos = timestamp_str.find_last_of(".");
				timestamp = stoll(timestamp_str.substr(0, pos) + timestamp_str.substr(pos + 1));
				depth_filename = filepath + depth_filename;
				color_filename = filepath + color_filename;
				bool flag = readDepthImage(depth_filename, depthPtr);
				if (!flag)
				{
					cout << "WARNING: can NOT read depth image \"" << depth_filename << "\". Quiting ..." << endl;
					return;
				}
				dataComp.compressDepth((unsigned char*)depthPtr);
				flag = readColorImage(color_filename, colorPtr);
				if (!flag)
				{
					cout << "WARNING: can NOT read color image \"" << color_filename << "\". Quiting ..." << endl;
					return;
				}
				dataComp.compressColor((cv::Vec<unsigned char, 3> *)colorPtr, m_colorWidth, m_colorHeight);

				// Write compressed data
				dataComp.writeBody(timestamp);
				frameNum++;

				cout << "Timestamp " << timestamp_str << " is done." << endl;
			}
		}
		readin.close();
	}
	dataComp.closeKLGFile(frameNum);

	delete[]colorPtr;
	delete[]depthPtr;
}

void RGBDConverter::klg2png(string filename)
{
	FILE* logFile = fopen(filename.c_str(), "rb");
	if (!logFile)
	{
		cerr << "WARNING: cannot open the file " << filename << endl;
		return;
	}

	std::size_t idx = filename.find_last_of("/\\");
	string folder = "./" + filename.substr(idx + 1, filename.length() - idx - 5);
	string depthFolder = folder + "/depth/";
	string colorFolder = folder + "/rgb/";
	boost::filesystem::path dir(folder);
	boost::filesystem::path dirDepth(depthFolder);
	boost::filesystem::path dirColor(colorFolder);

	if (boost::filesystem::is_directory(dir))
	{
		boost::filesystem::remove_all(dirDepth);
		boost::filesystem::remove_all(dirColor);
	}
	else
		boost::filesystem::create_directory(dir);
	boost::filesystem::create_directory(dirDepth);
	boost::filesystem::create_directory(dirColor);

	int numFrames = 0, depthSize = 0, rgbSize = 0;
	int64_t timestamp = 0; // note for the type of timestamp
	fread(&numFrames, sizeof(int), 1, logFile);

	// The klg file used in ElasticFusion code does NOT contain resolution parameters.
	// However, we leave the following codes here just in case you compressed these
	// parameters into the klg file during compresssion.
	//fread(&depthWidth, sizeof(int), 1, logFile);
	//fread(&depthHeight, sizeof(int), 1, logFile);
	//fread(&colorWidth, sizeof(int), 1, logFile);
	//fread(&colorHeight, sizeof(int), 1, logFile);

	unsigned char *rgbData = new unsigned char[m_colorWidth * m_colorHeight];
	unsigned char *depthData = new unsigned char[m_depthWidth * m_depthHeight * sizeof(DepthValueType)];
	for (int i = 0; i < numFrames; ++i)
	{
		fread(&timestamp, sizeof(int64_t), 1, logFile);
		cout << "Decompressing frame " << timestamp << "..." << endl;

		fread(&depthSize, sizeof(int), 1, logFile);
		fread(&rgbSize, sizeof(int), 1, logFile);

		memset(depthData, 0, m_depthWidth * m_depthHeight * 2);

		// Decompress depth image
		unsigned char *depthDataBinary = new unsigned char[depthSize];
		fread(depthDataBinary, depthSize, 1, logFile);
		unsigned long len = (unsigned long)(m_depthWidth * m_depthHeight * 2);
		int res = uncompress(depthData, &len, depthDataBinary, (unsigned long)depthSize);
		delete[]depthDataBinary;

		// Scale depth image
		depthDataBinary = NULL;
		cv::Mat mImageDepth(m_depthHeight, m_depthWidth, CV_16UC1, (void *)depthData);
		cv::Mat mScaledDepth;
		mImageDepth.convertTo(mScaledDepth, CV_16UC1, m_depthScaleFactor);

		string depthImageName = depthFolder + to_string(timestamp) + ".png";
		cv::imwrite(depthImageName, mScaledDepth);

		// Decompress color image
		fread(rgbData, rgbSize, 1, logFile);
		CvMat mat = cvMat(m_colorHeight, m_colorWidth, CV_8UC3, rgbData);
		CvMat *p = cvDecodeImageM(&mat, 1);
		cv::Mat m = cvarrToMat(p);
		cv::cvtColor(m, m, CV_BGR2RGB);

		// Write color image
		string rgbImageName = colorFolder + to_string(timestamp) + ".png";
		imwrite(rgbImageName, m);
	}
	fclose(logFile);
	delete[]rgbData;
	delete[]depthData;
}