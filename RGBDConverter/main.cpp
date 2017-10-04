#include "RGBDConverter.h"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;

int main(int argc, char** argv)
{
	boost::program_options::options_description all_options;
	all_options.add_options()
		("help", "display the help text")
		("p2k,p", "compress PNG images to a singe KLG file")
		("k2p,k", "decompress KLG file into PNG images")
		("file_path,f", boost::program_options::value<vector<string> >(), "path of KLG file or PNG images")
		("depth_width,w", boost::program_options::value<int>(), "width of depth image")
		("depth_height,h", boost::program_options::value<int>(), "height of depth image")
		("color_width,c", boost::program_options::value<int>(), "width of color image")
		("color_height,t", boost::program_options::value<int>(), "height of color image")
		("association_file,a", boost::program_options::value<vector<string> >(), "association/correspondence between "
		"depth and color timestamps (only used in -p2k). Do NOT need it if corresponding "
		"depth and color timestamps are exactly the same")
		;
	
	po::positional_options_description pos_options;
	pos_options.add("file_path", 1);
	pos_options.add("depth_width", 1);
	pos_options.add("depth_height", 1);
	pos_options.add("color_width", 1);
	pos_options.add("color_height", 1);
	pos_options.add("association_file", 1);

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(all_options).positional(pos_options).run(), vm);
	po::notify(vm);
	if (vm.count("help") || (!vm.count("p2k") && !vm.count("k2p")) || !vm.count("file_path"))
	{
		cout << "Usage: \tRGBDConverter -p2k/-k2p InputFilePath <depthWidth depthHeight colorWidth colorHeight> <associationFile>" << endl;
		cout << all_options << endl;
		cout << "For instance:" << endl
			<< "\tRGBDCapture -k2p rgbd.klg" << endl
			<< "\tRGBDCapture -p2k C:/rgbd_data/ -a association.txt" << endl << endl;
		return EXIT_FAILURE;
	}

	// Load the manifest.
	fs::path file_path = fs::path(vm["file_path"].as<vector<string>>()[0]);
	fs::path association_file;
	int depth_width = 640, depth_height = 480, color_width = 640, color_height = 480;
	if (vm.count("depth_width"))
		depth_width = vm["depth_width"].as<int>();
	if (vm.count("depth_height"))
		depth_height = vm["depth_height"].as<int>();
	if (vm.count("color_width"))
		color_width = vm["color_width"].as<int>();
	if (vm.count("color_height"))
		color_height = vm["color_height"].as<int>();
	if (vm.count("association_file"))
		association_file = fs::path(vm["association_file"].as<vector<string> >()[0]);

	RGBDConverter rgbdconverter(depth_width, depth_height, color_width, color_height);
	if (vm.count("k2p"))
	{
		rgbdconverter.klg2png(file_path.string());
	}
	else if (vm.count("p2k"))
	{
		rgbdconverter.png2klg(file_path.string());
	}
	return EXIT_SUCCESS;
}