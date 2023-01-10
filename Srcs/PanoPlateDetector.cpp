#include "PanoPlateDetector.h"
#include "PanoCvUtil.h"

namespace pano
{
	PanoPlateDetector::PanoPlateDetector()
	{

	}

	PanoPlateDetector::~PanoPlateDetector()
	{

	}

	void PanoPlateDetector::doPlateDetectionFromFile(const std::string& fileName, const std::string& roiFile)
	{
		if (fileName.empty() || roiFile.empty()) {
			return;
		}

		/* Image open */
		cv::Mat srcImg = cv::imread(fileName);
		if (srcImg.empty()) {
			return;
		}

		PpdSetup roiSetup;
		roiSetup.readFile(roiFile);

		std::vector<PdAreaInfo> vAreaInfos = roiSetup.areaInfos();

		if (!vAreaInfos.size()) {
			return;
		}

		for (int32_t i = 0; i < vAreaInfos.size(); i++) {
			if (!vAreaInfos[i].isUse()) {
				continue;
			}

			doPlateDetection(srcImg, vAreaInfos[i].coord());
		}
	}

	void PanoPlateDetector::doPlateDetection(const cv::Mat& src, const AREA_COORD_S& stCoord)
	{
		cv::Rect roi(stCoord.x, stCoord.y, stCoord.w, stCoord.h);

		cv::Mat roiImg = pano_cv::getCropImage(src, roi);
		
		cv::Mat modifiedImg(roiImg);
		//cv::Mat modifiedImg = pano_cv::getMoveAndRotateImage(roiImg, 0.0);

		cv::Mat grayImg = pano_cv::getGrayImage(modifiedImg);
		cv::Mat cannyImg = pano_cv::getCannyImage(grayImg, 80, 150);
		cv::Mat binImg = pano_cv::getBinaryImage(grayImg);

		std::vector<cv::Rect> vRects = getAllRects(binImg);
		
		pickPlate(vRects);
	}

	double PanoPlateDetector::calcRoiAngle(const cv::Point& center, const AREA_COORD_S& stCoord)
	{
		return 0.0;
	}

	std::vector<cv::Rect> PanoPlateDetector::getAllRects(const cv::Mat& src)
	{
		std::vector<std::vector<cv::Point>> vContours;
		vContours.clear();

		cv::findContours(src, vContours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

		std::vector<cv::Rect> vRects;
		vRects.clear();

		for (int32_t i = 0; i < vContours.size(); i++) {
			vRects.push_back(cv::boundingRect(vContours[i]));
		}

		return vRects;
	}

	void PanoPlateDetector::pickPlate(const std::vector<cv::Rect>& vRects)
	{
		std::vector<cv::Rect> vPlateRects = getPlateCandidateRects(vRects);
		std::vector<cv::Rect> vCharRects = getCharCandidateRects(vRects);

		std::vector<std::vector<int32_t>> vIsPlateIdx;
		vIsPlateIdx.clear();

		float maxDiagMultiplyer = 5.0f;
		float maxAngleDiff = 12.0f;
		float maxAreaDiff = 0.5f;
		float maxWidthDiff = 0.8f;
		float maxHeightDiff = 0.3f;

		for (int32_t i = 0; i < vPlateRects.size(); i++) {
			int32_t isInnerCnt = 0;

			std::vector<int32_t> vOnLineIdx;
			vOnLineIdx.clear();

			std::vector<int32_t> vIsInnerIdx = getInnerIdxList(vPlateRects[i], vCharRects);

			if (vIsInnerIdx.size() < 3) {
				vIsPlateIdx.push_back(vOnLineIdx);
				continue;
			}

			for (int32_t j = 0; j < vIsInnerIdx.size(); j++) {
				cv::Rect charRect = vCharRects[vIsInnerIdx[j]];
				cv::Point center = pano_cv::getCenterPoint(charRect);

				int32_t lineCnt = 0;
				vOnLineIdx.clear();
				vOnLineIdx.push_back(vIsInnerIdx[j]);
				for (int32_t k = 0; k < vIsInnerIdx.size(); k++) {
					if (j == k) {
						lineCnt++;
						continue;
					}

					cv::Rect dRect = vCharRects[vIsInnerIdx[k]];
					cv::Point dCenter = pano_cv::getCenterPoint(dRect);
					cv::Point diff = dCenter - center;

					double distance = std::sqrt((diff.x * diff.x) + (diff.y * diff.y));
					if (distance > vPlateRects[i].width) {
						continue;
					}
					
					double areaDiffRatio = calcAreaDiffRatio(charRect.area(), dRect.area());
					if (areaDiffRatio > maxAreaDiff) {
						continue;
					}

					double angleDiff = calcDiagAngle(diff);
					if (angleDiff > maxAngleDiff) {
						continue;
					}

					lineCnt++;
					vOnLineIdx.push_back(vIsInnerIdx[k]);
				}

				if (lineCnt > vIsInnerIdx.size()) {
					/* Find */
					break;
				}
			}
			vIsPlateIdx.push_back(vOnLineIdx);
		}


	}

	std::vector<cv::Rect> PanoPlateDetector::getPlateCandidateRects(const std::vector<cv::Rect>& vRects, float minRatio)
	{
		std::vector<cv::Rect> vPlateRects;
		vPlateRects.clear();

		for (int32_t i = 0; i < vRects.size(); i++) {
			float ratio = static_cast<float>(vRects[i].height) / static_cast<float>(vRects[i].width);

			if (ratio > minRatio && ratio < 1.0f && vRects[i].area() > 300) {
				vPlateRects.push_back(vRects[i]);
			}
		}

		return vPlateRects;
	}

	std::vector<cv::Rect> PanoPlateDetector::getCharCandidateRects(const std::vector<cv::Rect>& vRects, float miniRatio)
	{
		std::vector<cv::Rect> vCharRects;
		vCharRects.clear();

		for (int32_t i = 0; i < vRects.size(); i++) {
			float ratio = static_cast<float>(vRects[i].width) / static_cast<float>(vRects[i].height);
			if (vRects[i].area() > 40 && ratio > 0.25 && ratio < 1.0f) {
				vCharRects.push_back(vRects[i]);
			}
		}

		return vCharRects;
	}

	bool PanoPlateDetector::isInnerRect(const cv::Rect& external, const cv::Rect& inner)
	{
		int32_t exRight = external.x + external.width;
		int32_t exBottom = external.y + external.height;

		int32_t centerX = inner.x + inner.width / 2;
		int32_t centerY = inner.y + inner.height / 2;

		if (external.x < centerX && external.y < centerY &&
			centerX <= exRight && centerY <= exBottom)
		{
			return true;
		}
		else {
			return false;
		}
	}

	double PanoPlateDetector::calcAreaDiffRatio(int32_t area1, int32_t area2)
	{
		double absAreaDiff = static_cast<double>(std::abs(area1 - area2));

		return (absAreaDiff / static_cast<double>(area1));
	}

	double PanoPlateDetector::calcDiagAngle(const cv::Point& diff)
	{
		double angle = 0.0;

		if (diff.x == 0) {
			angle = 0.0;
		}
		else if (diff.y == 0.0) {
			angle = 90.0;
		}
		else {
			double arcTan = std::atan2(diff.y, diff.x);
			angle = arcTan * 180.0 / 3.14159265358979323846;
		}

		return angle;
	}

	std::vector<int32_t> PanoPlateDetector::getInnerIdxList(const cv::Rect& bigger, const std::vector<cv::Rect>& vSmallers)
	{
		std::vector<int32_t> vIsInnerIdx;
		vIsInnerIdx.clear();

		for (int32_t i = 0; i < vSmallers.size(); i++) {
			if (!isInnerRect(bigger, vSmallers[i])) {
				continue;
			}
			else {
				vIsInnerIdx.push_back(i);
			}
		}

		return vIsInnerIdx;
	}
}	// namespace pano