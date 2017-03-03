#include "RGBDConverter.h"

void printUsage(){
	cout << "Usage: " << endl
		<< "   RGBDConverter -option InputFilePath" << endl << endl;
	cout << "-option:" << endl
		<< "   -k2p: convert a single KLG file to PNG images" << endl
		<< "   -p2k: convert PNG images to a singe KLG file" << endl;
	cout << "InputFilePath: " << endl
		<< "   path of KLG file or PNG images" << endl << endl;
	cout << "For instance:" << endl
		<< "    RGBDCapture -k2p rgbd.klg" << endl
		<< "    RGBDCapture -p2k C:/rgbd/" << endl << endl;
}

int main(int argc, char** argv)
{
	if (argc != 3 || (string(argv[1]) != "-k2p" && string(argv[1]) != "-p2k"))
	{
		printUsage();
		return EXIT_FAILURE;
	}
	string option(argv[1]), filepath(argv[2]);
	RGBDConverter rgbdconverter;
	if (option == "-k2p")
	{
		rgbdconverter.klg2png(filepath);
	}
	else if (option == "-p2k")
	{
		rgbdconverter.png2klg(filepath);
	}
	return EXIT_SUCCESS;
}