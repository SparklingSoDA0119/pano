#include "PlateDetector.h"
#include "PanoCvUtil.h"

namespace pano
{
	PlateDetector::PlateDetector()
	{

	}

	PlateDetector::~PlateDetector()
	{

	}

	void PlateDetector::doPlateDetection()
	{
		_fileName = "..\\Pano\\img_192.168.12.3_09-26-10-10.jpg";
		//_fileName = "..\\Pano\\img_192.168.12.5_09-26-10-10.jpg";
		//_fileName = "..\\Pano\\img_192.168.12.6_09-26-10-10.jpg";
		_oriImg = cv::imread(_fileName);

		std::vector<ROI_INFO_S> vRoiInfo = makeRoiFromFile(_fileName);
		doPlateDetection(vRoiInfo);

#if 0
		cv::Mat drawRoiImg = pano_cv::drawRois(_oriImg, vRoiInfo, cv::Scalar(0, 0, 255));
		cv::Mat oriResized = pano_cv::getResizeImage(drawRoiImg, cv::Size(800, 600));

		cv::imshow("Original 800x600", oriResized);
		cv::waitKey(0);
#endif
	}

	void PlateDetector::doPlateDetection(const std::vector<ROI_INFO_S>& vRoiInfo)
	{
		if (!vRoiInfo.size()) {
			return;
		}

		cv::Mat roiImg = pano_cv::getCropImage(_oriImg, vRoiInfo[5].roi);
		cv::Mat rotImg = pano_cv::getRotatedImage(roiImg, 35);
		
		cv::Mat grayImg = pano_cv::getGrayImage(rotImg);
		cv::Mat contImg = pano_cv::getContrastAdjust(grayImg, 90, 1.5, 0.1);
		cv::Mat binImg = pano_cv::getCannyImage(contImg, 80, 150);
		//cv::Mat binImg = pano_cv::getBinaryImage(contImg);
		std::vector<cv::Rect> vRects = getContoursRect(binImg);
		std::vector<cv::Rect> vCharRects = getCharRect(vRects);

		cv::Mat result = pano_cv::drawRect(rotImg, vCharRects, cv::Scalar(0, 0, 255));

		/* Find Plate Rects */
		std::vector<cv::Rect> vPlateRect = getPlateRect(vRects);

		/* Find Real Plate */

		result = pano_cv::drawRect(result, vPlateRect, cv::Scalar(0, 255, 0));

		cv::imshow("Rotation 35", rotImg);
		cv::imshow("Contrast Image", contImg);
		cv::imshow("Canny Image", binImg);
		cv::imshow("Result", result);

		cv::waitKey(0);

#if 0
		for (int32_t i = 0; i < vRoiInfo.size(); i++) {
			cv::Mat roiImg = pano_cv::getCropImage(_oriImg, vRoiInfo[i].roi);
		}
#endif
	}

	void PlateDetector::findRealPlate(const std::vector<cv::Rect>& vRects)
	{
		std::vector<cv::Rect> vCharRects = getCharRect(vRects);
		std::vector<cv::Rect> vPlateRects = getPlateRect(vRects);

		float maxDiagMultiplyer = 5.0f;
		float maxAngleDiff = 12.0f;
		float maxAreaDiff = 0.5f;
		float maxWidthDiff = 0.8f;
		float maxHeightDiff = 0.3f;

		std::vector<std::vector<int32_t>> vIsPlateIdx;
		vIsPlateIdx.clear();

		for (int32_t i = 0; i < vPlateRects.size(); i++) {

			int32_t isInnerCnt = 0;
			std::vector<int32_t> vIsInnerIdxs;
			vIsInnerIdxs.clear();

			std::vector<int32_t> vOnLineIdxs;
			vOnLineIdxs.clear();

			for (int32_t j = 0; j < vCharRects.size(); j++) {
				cv::Rect charRect = vCharRects[j];

				if (!isInnerRect(vPlateRects[i], vCharRects[j])) {
					continue;
				}

				vIsInnerIdxs.push_back(j);
			}

			if (vIsInnerIdxs.size() < 3) {
				vIsPlateIdx.push_back(vOnLineIdxs);
				continue;
			}

			for (int32_t j = 0; j < vIsInnerIdxs.size(); j++) {
				cv::Rect sRect = vCharRects[vIsInnerIdxs[j]];
				double sCenterX = static_cast<double>(getCenterX(sRect));
				double sCenterY = static_cast<double>(getCenterY(sRect));

				cv::Point center = pano_cv::getCenterPoint(sRect);

				int32_t lineCnt = 0;
				vOnLineIdxs.clear();
				vOnLineIdxs.push_back(vIsInnerIdxs[j]);
				for (int32_t k = 0; k < vIsInnerIdxs.size(); k++) {
					if (j == k) {
						lineCnt++;
						continue;
					}

					cv::Rect dRect = vCharRects[vIsInnerIdxs[k]];
					double dCenterX = static_cast<double>(getCenterX(dRect));
					double dCenterY = static_cast<double>(getCenterY(dRect));

					/* Check Distance */
					double diffX = std::abs(dCenterX - sCenterX);
					double diffY = std::abs(dCenterY - sCenterY);
					double distance = std::sqrt((diffX * diffX) + (diffY * diffY));

					if (distance > vPlateRects[i].width) {
						continue;
					}

					/* Check Area */
					double areaDiff = static_cast<double>(std::abs(sRect.area() - dRect.area())) / static_cast<double>(sRect.area());
					if (areaDiff > maxAreaDiff) {
						continue;
					}

					/* Check Angle */
					double angleDiff = 0.0;
					if (diffX == 0.0) {
						angleDiff = 0.0;
					}
					else if (diffY == 0.0) {
						angleDiff = 90.0;
					}
					else {
						double arcTan = std::atan2(diffY, diffX);
						angleDiff = arcTan * 180.0 / 3.14159265358979323846;
					}

					if (angleDiff > maxAngleDiff) {
						continue;
					}

					lineCnt++;
					vOnLineIdxs.push_back(vIsInnerIdxs[k]);
				}

				if (lineCnt > vIsInnerIdxs.size() / 2) {
					/* Find It */
					break;
				}
			}
			vIsPlateIdx.push_back(vOnLineIdxs);
		}

		int32_t detectedIdx = -1;
		std::vector<RESULT_S> vDetIdx;
		vDetIdx.clear();
		for (int32_t i = 0; i < vIsPlateIdx.size(); i++) {
			if (vIsPlateIdx[i].size() >= 3) {
				RESULT_S stResult;
				float sum = 0.0;
				for (int32_t j = 0; j < vIsPlateIdx[i].size(); j++) {
					sum += static_cast<float>(vIsPlateIdx[i][j]);
				}
				float aveIdx = sum / static_cast<float>(vIsPlateIdx[i].size());

				stResult.aveIdx = aveIdx;
				stResult.area = vPlateRects[i].area();
				stResult.plateIdx = i;

				if (!vDetIdx.size()) {
					vDetIdx.push_back(stResult);
				}
				else {
					for (int32_t j = 0; j < vDetIdx.size(); j++) {
						if (std::abs(vDetIdx[j].aveIdx - stResult.aveIdx) < 3) {

							if (vDetIdx[j].area > stResult.area) {
								vDetIdx[j] = stResult;
							}
						}
					}
				}
			}
		}

		for (int32_t i = 0; i < vDetIdx.size(); i++) {
			test.push_back(vPlateRects[vDetIdx[i].plateIdx]);
		}
	}


	cv::Mat drawImg = getDrawRectImage(rotImg, test, cv::Scalar(0, 0, 255));
	//drawImg = getDrawRectImage(drawImg, vCharRects, cv::Scalar(0, 255, 0));

	/* Show Image */
	cv::imshow("Rot Image", drawImg);
}
	}

	std::vector<cv::Rect> PlateDetector::getPlateRect(const std::vector<cv::Rect>& vRects, float minRatio)
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


	std::vector<cv::Rect> PlateDetector::getCharCandidates(const cv::Mat& src)
	{
		cv::Mat contImg = pano_cv::getContrastAdjust(src, 80, 1.5, 0.2);
		cv::Mat blurImg = pano_cv::getDenoiseImage(src, 1);
		cv::Mat binaryImg = pano_cv::getBinaryImage(contImg);
		std::vector<cv::Rect> vRects = getContoursRect(binaryImg);
		std::vector<cv::Rect> vCharRects = getCharRect(vRects);

		return vCharRects;
	}

	std::vector<cv::Rect> PlateDetector::getContoursRect(const cv::Mat& src)
	{
		std::vector<std::vector<cv::Point>> vContours;
		vContours.clear();

		cv::findContours(src, vContours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

		std::vector<cv::Rect> vRects;
		vRects.clear();
		for (int32_t i = 0; i < vContours.size(); i++) {
			cv::Rect rect = cv::boundingRect(vContours[i]);
			vRects.push_back(rect);
		}

		return vRects;
	}



	std::vector<cv::Rect> PlateDetector::getCharRect(const std::vector<cv::Rect>& vRects, float minRatio)
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

	std::vector<ROI_INFO_S> PlateDetector::makeRoiFromFile(const std::string& fileName)
	{
		std::vector<ROI_INFO_S> vRoiInfos;
		vRoiInfos.clear();
		ROI_INFO_S stRoiInfo;
		if (fileName == "..\\Pano\\img_192.168.12.3_09-26-10-10.jpg") {
			stRoiInfo.idx = 0; stRoiInfo.rotAngle = -25; stRoiInfo.roi = cv::Rect(1200, 760, 300, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 1; stRoiInfo.rotAngle = 0; stRoiInfo.roi = cv::Rect(1510, 660, 400, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 2; stRoiInfo.rotAngle = 0; stRoiInfo.roi = cv::Rect(2100, 660, 400, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 3; stRoiInfo.rotAngle = 25; stRoiInfo.roi = cv::Rect(2550, 760, 300, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 4; stRoiInfo.rotAngle = 35; stRoiInfo.roi = cv::Rect(1000, 1880, 200, 200); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 5; stRoiInfo.rotAngle = 35; stRoiInfo.roi = cv::Rect(1200, 1930, 300, 250); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 6; stRoiInfo.rotAngle = 15; stRoiInfo.roi = cv::Rect(1500, 2030, 400, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 7; stRoiInfo.rotAngle = 0; stRoiInfo.roi = cv::Rect(2100, 2030, 400, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 8; stRoiInfo.rotAngle = -5; stRoiInfo.roi = cv::Rect(2530, 1960, 300, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 9; stRoiInfo.rotAngle = -25; stRoiInfo.roi = cv::Rect(2840, 1890, 220, 220); vRoiInfos.push_back(stRoiInfo);
		}
		else if (fileName == "..\\Pano\\img_192.168.12.5_09-26-10-10.jpg") {
			stRoiInfo.idx = 0; stRoiInfo.rotAngle = 0; stRoiInfo.roi = cv::Rect(1660, 2030, 415, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 1; stRoiInfo.rotAngle = 0; stRoiInfo.roi = cv::Rect(2120, 2030, 440, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 2; stRoiInfo.rotAngle = 90; stRoiInfo.roi = cv::Rect(2710, 1560, 300, 400); vRoiInfos.push_back(stRoiInfo);
		}
		else if (fileName == "..\\Pano\\img_192.168.12.6_09-26-10-10.jpg") {
			stRoiInfo.idx = 0; stRoiInfo.rotAngle = -15; stRoiInfo.roi = cv::Rect(1200, 610, 300, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 1; stRoiInfo.rotAngle = -5; stRoiInfo.roi = cv::Rect(1520, 560, 500, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 2; stRoiInfo.rotAngle = 5; stRoiInfo.roi = cv::Rect(1520, 2080, 520, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 3; stRoiInfo.rotAngle = 0; stRoiInfo.roi = cv::Rect(2210, 2060, 460, 300); vRoiInfos.push_back(stRoiInfo);
			stRoiInfo.idx = 4; stRoiInfo.rotAngle = -5; stRoiInfo.roi = cv::Rect(2670, 1920, 300, 330); vRoiInfos.push_back(stRoiInfo);
		}

		
		return vRoiInfos;
	}

	bool PlateDetector::isInnerRect(const cv::Rect& external, const cv::Rect& inner)
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
}