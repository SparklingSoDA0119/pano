#ifndef _PANO_PLATE_DETECTOR_H_
#define _PANO_PLATE_DETECTOR_H_

#include "Const.h"
#include "PanoPlateDetectorSetup.h"

namespace pano
{
	typedef struct plate_detection_result_s
	{

	} PDR_S;

	class PanoPlateDetector
	{
	public :
		PanoPlateDetector();
		virtual ~PanoPlateDetector();

	public :
		static void doPlateDetectionFromFile(const std::string& fileName, const std::string& roiFile);
		static void doPlateDetection(const cv::Mat& src, const AREA_COORD_S& stCoord);

	private :
		double calcRoiAngle(const cv::Point& center, const AREA_COORD_S& stCoord);

	private:
		static void pickPlate(const std::vector<cv::Rect>& vRects);
		
	private :
		static std::vector<cv::Rect> getAllRects(const cv::Mat& src);
		static std::vector<cv::Rect> getPlateCandidateRects(const std::vector<cv::Rect>& vRects, float minRatio = 0.25f);
		static std::vector<cv::Rect> getCharCandidateRects(const std::vector<cv::Rect>& vRects, float minRatio = 0.1f);

	public :
		static bool isInnerRect(const cv::Rect& external, const cv::Rect& inner);
		static double calcAreaDiffRatio(int32_t area1, int32_t area2);
		static double calcDiagAngle(const cv::Point& diff);
		static std::vector<int32_t> getInnerIdxList(const cv::Rect& bigger,
													const std::vector<cv::Rect>& vSmallers);

	};	// class PanoPlateDetector
}	// namespace pano

#endif	// _PANO_PLATE_DETECTOR_H_