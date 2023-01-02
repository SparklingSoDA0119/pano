#ifndef _PLATE_DETECTOR_H_
#define _PLATE_DETECTOR_H_

#include <iostream>

#include "Define.h"

#include <vector>
#include <string>

namespace pano
{

	class PlateDetector
	{
	public :
		PlateDetector();
		virtual ~PlateDetector();

	private :
		cv::Mat _oriImg;
		std::string _fileName;

	public :
		void doPlateDetection();
		//void doPlateDetection(const std::string& fileName);
		void doPlateDetection(const std::vector<ROI_INFO_S>& vRoiInfo);
		void doPlateDetection(const cv::Mat& roiImg);

	public :
		std::vector<ROI_INFO_S> makeRoiFromFile(const std::string& fileName);

	private :
		std::vector<cv::Rect> getCharCandidates(const cv::Mat& src);
		std::vector<cv::Rect> getContoursRect(const cv::Mat& src);
		std::vector<cv::Rect> getCharRect(const std::vector<cv::Rect>& vRects, float minRatio = 0.1f);
		std::vector<cv::Rect> getPlateRect(const std::vector<cv::Rect>& vRects, float minRatio = 0.25f);

		void findRealPlate(const std::vector<cv::Rect>& vRects);
		bool isInnerRect(const cv::Rect& external, const cv::Rect& inner);
	};
}	// namespace pano

#endif	// _PLATE_DETECTOR_H_