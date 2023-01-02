#ifndef _PANO_CV_UTIL_H_
#define _PANO_CV_UTIL_H_

#include "Const.h"

namespace pano
{
	namespace pano_cv
	{
		cv::Mat getResizeImage(const cv::Mat& src, const cv::Size& res);
		cv::Mat getCropImage(const cv::Mat& src, const cv::Rect& rect);
		cv::Mat getRotatedImage(const cv::Mat& src, const int32_t angle);
		cv::Mat getGrayImage(const cv::Mat& src);
		cv::Mat getContrastAdjust(const cv::Mat& src, int32_t centerVal, double upperFactor, double underFactor);
		cv::Mat getCannyImage(const cv::Mat& src, int32_t under, int32_t upper);
		cv::Mat getHistEquationImage(const cv::Mat& src);
		cv::Mat getBinaryImage(const cv::Mat& src);
		cv::Mat getDenoiseImage(const cv::Mat& src, int32_t strength);

		//cv::Mat drawRois(const cv::Mat& src, const std::vector<ROI_INFO_S>& vRoiInfos, const cv::Scalar& color);
		cv::Mat drawRect(const cv::Mat& src, const std::vector<cv::Rect>& vRects, const cv::Scalar& color);

		cv::Point getCenterPoint(const cv::Rect& rect);
	}
}

#endif	// _PANO_CV_UTIL_H_