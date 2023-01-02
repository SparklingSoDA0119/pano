#ifndef _PLATE_DETECTOR_H_
#define _PLATE_DETECTOR_H_

#include <stdio.h>
#include <iostream>

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#define ABS_VAL(X) X < 0 ? -X : X;



enum class ROI_TYPE_E
{
	NONE = 0,
	QUADRANT_1 = 1,
	QUADRANT_2,
	QUADRANT_3,
	QUADRANT_4,
	QUADRANT_12,
	QUADRANT_34,
	QUADRANT_13,
	QUDATANT_24,
	USER_DEFINE,
};

cv::Rect getRoi(ROI_TYPE_E eRoiType, int32_t width, int32_t height);

typedef struct roi_info_s
{
	cv::Rect roi;
	int32_t idx;
	int32_t rotAngle;

	roi_info_s() : roi(cv::Rect()), idx(0), rotAngle(0) {}
	roi_info_s(const roi_info_s& stRoiInfo) : roi_info_s() { *this = stRoiInfo; }

	roi_info_s& operator=(const roi_info_s& stRoiInfo)
	{ 
		roi = stRoiInfo.roi;
		idx = stRoiInfo.idx;
		rotAngle = stRoiInfo.rotAngle;
		return *this;
	}
} ROI_INFO_S;

typedef struct result_s
{
	int32_t plateIdx;
	int32_t area;
	float aveIdx;

	result_s() : plateIdx(-1), area(-1), aveIdx(0.0) {}
	result_s(const result_s& stResult) : result_s() { *this = stResult; }

	result_s& operator=(const result_s& stResult)
	{
		plateIdx = stResult.plateIdx;
		area = stResult.area;
		aveIdx = stResult.aveIdx;

		return *this;
	}
	
} RESULT_S;

class PlateDetector
{
public :
	PlateDetector();
	virtual ~PlateDetector();

private :
	std::string _fileName;
	cv::Rect _roi;
	cv::Mat _oriImg;

public :
	void doDetection(const std::string& fileName, ROI_TYPE_E eRoiType);
	void doDetection(const std::string& fileName, const std::vector<ROI_INFO_S>& vRoiInfos);
	void doDetection(const cv::Mat& img, int32_t angle);
	void doDetectionEx(const std::string& fileName, const std::vector<ROI_INFO_S>& vRoiInfos);

private :
	void openImageFile(const std::string& fileName);

public :
	cv::Mat getGrayImage(const cv::Mat& src);
	cv::Mat getCropImage(const cv::Mat& src, const cv::Rect& rect);
	cv::Mat getResizedImage(const cv::Mat& src, const cv::Size& res);
	cv::Mat getDenoiseImage(const cv::Mat& src, int32_t strength);
	cv::Mat getBinaryImage(const cv::Mat& src);
	cv::Mat getDrawRectImage(const cv::Mat& src, const std::vector<cv::Rect>& vRects, const cv::Scalar& color);

	std::vector<cv::Rect> getContoursRect(const cv::Mat& src);
	std::vector<cv::Rect> getPlateRect(const std::vector<cv::Rect>& vRects, float minRatio = 0.25f);
	std::vector<cv::Rect> getCharRect(const std::vector<cv::Rect>& vRects, float minRatio = 0.1f);

	std::vector<int32_t> findLicenseCharRect(const std::vector<cv::Rect>& vRects);
	int32_t findPlateCandidate(const std::vector<cv::Rect>& vPlateRect, const std::vector<cv::Rect>& vCharRect, const std::vector<int32_t>& vCharIdxSet);

	int32_t getCenterX(const cv::Rect& rect) { return rect.x + (rect.width / 2); }
	int32_t getCenterY(const cv::Rect& rect) { return rect.y + (rect.height / 2); }

	std::vector<cv::Rect> getPlateCandidates(const cv::Mat& src);
	std::vector<cv::Rect> getCharCandidates(const cv::Mat& src);
	bool isInnerRect(const cv::Rect& external, const cv::Rect& inner);
};

#endif