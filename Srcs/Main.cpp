#include <iostream>
#include <stdio.h>

#if defined(_MSC_VER)
	#if defined(_DEBUG)
		#pragma comment(lib, "opencv_world460d.lib")
	#else
		#pragma comment(lib, "opencv_world460.lib")
	#endif
#endif

#include "PanoPlateDetectorSetup.h"

int32_t main(int32_t argc, char* argv[])
{
	pano::PpdSetup setup;
	setup.readFile("pano_plate_detection_setup.json");

	std::vector<pano::PdAreaInfo> vAreaInfos = setup.areaInfos();


	return 0;
}