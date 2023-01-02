#include "PlateDetector_Temp.h"

cv::Rect getRoi(ROI_TYPE_E eRoiType, int32_t width, int32_t height)
{
	int32_t x = 0;
	int32_t y = 0;
	int32_t w = 0;
	int32_t h = 0;

	switch (eRoiType)
	{
		case ROI_TYPE_E::QUADRANT_1 :
			w = width >> 1;
			h = height >> 1;
			break;

		case ROI_TYPE_E::QUADRANT_2 :
			x = width >> 1;
			w = x;
			h = height >> 1;
			break;

		case ROI_TYPE_E::QUADRANT_3 :
			w = width >> 1;
			y = height >> 1;
			h = y;
			break;

		case ROI_TYPE_E::QUADRANT_4 :
			x = width >> 1;
			w = x;
			y = height >> 1;
			h = y;
			break;

		case ROI_TYPE_E::QUADRANT_12 :
			w = width;
			h = height >> 1;
			break;

		case ROI_TYPE_E::QUADRANT_34 :
			w = width;
			y = height >> 1;
			h = y;
			break;

		case ROI_TYPE_E::QUADRANT_13 :
			w = width >> 1;
			h = height;
			break;

		case ROI_TYPE_E::QUDATANT_24 :
			x = width >> 1;
			w = x;
			h = height;
			break;

		case ROI_TYPE_E::NONE :
			w = width;
			h = height;
			break;

		default :
			break;
	}

	return cv::Rect(x, y, w, h);
}

PlateDetector::PlateDetector()
{

}

PlateDetector::~PlateDetector()
{

}

void PlateDetector::doDetection(const std::string& fileName, ROI_TYPE_E eRoiType)
{
	_fileName = fileName;

	/* 1. Image Open */
	openImageFile(_fileName);

	/* 3. Get Gray Image */
	cv::Mat grayImg = getGrayImage(_oriImg);
	
	/* 4. Get ROI Image */
	_roi = getRoi(eRoiType, _oriImg.cols, _oriImg.rows);
	if (fileName == "..\\Sample\\Sample1.png") {
		_roi.height = _roi.height - 50;
	}

	cv::Mat colorOriRoiImg;
	_oriImg.copyTo(colorOriRoiImg);
	cv::rectangle(colorOriRoiImg, _roi, cv::Scalar(0, 0, 255), 2);
	colorOriRoiImg = getResizedImage(colorOriRoiImg, cv::Size(352, 288));
	
	cv::Mat roiImg = getCropImage(grayImg, _roi);
	cv::Mat colorRoiImg = getCropImage(_oriImg, _roi);

	/* 5. Resize to 320x160 */ 
	cv::Mat resizeImg = getResizedImage(roiImg, cv::Size(352, 288));
	cv::Mat outImage = getResizedImage(colorRoiImg, cv::Size(352, 288));

	/* 5. De-noise */
	cv::Mat denoiseImg3 = getDenoiseImage(resizeImg, 3);
	cv::Mat denoiseImg5 = getDenoiseImage(resizeImg, 5);
	cv::Mat denoiseImg7 = getDenoiseImage(resizeImg, 5);
	cv::Mat denoiseImg9 = getDenoiseImage(resizeImg, 5);

	/* 6. Get Binary Image */
	cv::Mat thresh3 = getBinaryImage(denoiseImg3);
	cv::Mat thresh5 = getBinaryImage(denoiseImg5);
	cv::Mat thresh7 = getBinaryImage(denoiseImg7);
	cv::Mat thresh9 = getBinaryImage(denoiseImg9);

	std::vector<cv::Rect> vRects3 = getContoursRect(thresh3);
	std::vector<cv::Rect> vRects5 = getContoursRect(thresh5);
	std::vector<cv::Rect> vRects7 = getContoursRect(thresh7);
	std::vector<cv::Rect> vRects9 = getContoursRect(thresh9);
	
	std::vector<cv::Rect> vPlateRect3 = getPlateRect(vRects3);
	std::vector<cv::Rect> vPlateRect5 = getPlateRect(vRects5);
	std::vector<cv::Rect> vPlateRect7 = getPlateRect(vRects7);
	std::vector<cv::Rect> vPlateRect9 = getPlateRect(vRects9);

	std::vector<cv::Rect> vCharRect3 = getCharRect(vRects3);
	std::vector<cv::Rect> vCharRect5 = getCharRect(vRects5);
	std::vector<cv::Rect> vCharRect7 = getCharRect(vRects7);
	std::vector<cv::Rect> vCharRect9 = getCharRect(vRects9);

	std::vector<int32_t> vCharIdxSet3 = findLicenseCharRect(vCharRect3);
	std::vector<int32_t> vCharIdxSet5 = findLicenseCharRect(vCharRect5);
	std::vector<int32_t> vCharIdxSet7 = findLicenseCharRect(vCharRect7);
	std::vector<int32_t> vCharIdxSet9 = findLicenseCharRect(vCharRect9);

	int32_t idx3 = findPlateCandidate(vPlateRect3, vCharRect3, vCharIdxSet3);
	int32_t idx5 = findPlateCandidate(vPlateRect5, vCharRect5, vCharIdxSet5);
	int32_t idx7 = findPlateCandidate(vPlateRect7, vCharRect7, vCharIdxSet7);
	int32_t idx9 = findPlateCandidate(vPlateRect9, vCharRect9, vCharIdxSet9);

	cv::Scalar red(0, 0, 255);
	cv::Scalar green(0, 255, 0);
	cv::Scalar blue(255, 0, 0);
	
	cv::Mat out3 = getDrawRectImage(outImage, vPlateRect3, red);
	cv::Mat out5 = getDrawRectImage(outImage, vPlateRect5, red);
	cv::Mat out7 = getDrawRectImage(outImage, vPlateRect7, red);
	cv::Mat out9 = getDrawRectImage(outImage, vPlateRect9, red);

	out3 = getDrawRectImage(out3, vCharRect3, green);
	out5 = getDrawRectImage(out5, vCharRect5, green);
	out7 = getDrawRectImage(out7, vCharRect7, green);
	out9 = getDrawRectImage(out9, vCharRect9, green);

	if (idx3 != -1) { cv::rectangle(out3, vPlateRect3[idx3], blue, 2); }
	if (idx5 != -1) { cv::rectangle(out5, vPlateRect5[idx5], blue, 2); }
	if (idx7 != -1) { cv::rectangle(out7, vPlateRect7[idx7], blue, 2); }
	if (idx9 != -1) { cv::rectangle(out9, vPlateRect9[idx9], blue, 2); }

	cv::imshow("ROI Image", colorOriRoiImg);
	cv::moveWindow("ROI Image", 100, 100);
	cv::imshow("denoise 3", out3);
	cv::moveWindow("denoise 3", 455, 100);
	cv::imshow("denoise 5", out5);
	cv::moveWindow("denoise 5", 811, 100);
	cv::imshow("denoise 7", out7);
	cv::moveWindow("denoise 7", 100, 420);
	cv::imshow("denoise 9", out9);
	cv::moveWindow("denoise 9", 455, 420);
	
	cv::waitKey(0);
}

void PlateDetector::doDetection(const std::string& fileName, const std::vector<ROI_INFO_S>& vRoiInfos)
{
	_fileName = fileName;

	/* 1. Image Open */
	openImageFile(_fileName);

	if (!vRoiInfos.size()) {
		return;
	}

	std::vector<cv::Mat> vRoiImgs;
	vRoiImgs.clear();
	for (int32_t i = 0; i < vRoiInfos.size(); i++) {
		cv::Mat roiImg = getCropImage(_oriImg, vRoiInfos[i].roi);
		vRoiImgs.push_back(roiImg);
	}

	for (int32_t i = 0; i < vRoiImgs.size(); i++) {
		doDetection(vRoiImgs[i], vRoiInfos[i].rotAngle);
	}

	for (int32_t i = 0; i < vRoiImgs.size(); i++) {
		std::string windowsName("RoiImg");
		windowsName.append(std::to_string(i));

		//cv::imshow(windowsName, vRoiImgs[i]);
	}
	cv::waitKey(0);
}

void PlateDetector::doDetection(const cv::Mat& img, int32_t angle)
{
	/* Rotate */
	cv::Mat rotFactor = cv::getRotationMatrix2D(cv::Point(img.cols / 2, img.rows / 2), angle, 1.5);
	cv::Mat rotImg;
	cv::warpAffine(img, rotImg, rotFactor, cv::Size(img.cols, img.rows));

	/* Get Gray Image & Blur*/
	cv::Mat grayImg = getGrayImage(rotImg);

	cv::Mat newImg = cv::Mat::zeros(grayImg.size(), grayImg.type());

	for (int32_t i = 0; i < grayImg.rows; i++) {
		for (int32_t j = 0; j < grayImg.cols; j++) {
			int32_t pixVal = grayImg.at<uchar>(i, j);
			if (pixVal <= 128) {
				pixVal = static_cast<int32_t>(pixVal * 0.8);
			}
			else {
				pixVal = static_cast<int32_t>(pixVal * 1.2);
			}

			newImg.at<uchar>(i, j) = cv::saturate_cast<uchar>(pixVal);
		}
	}

	/* Get Plate Candidate */
	std::vector<cv::Rect> vPlateRects = getPlateCandidates(newImg);
	
	/* Get Char Candidate */
	std::vector<cv::Rect> vCharRects = getCharCandidates(newImg);

	/* Check charactor in Plate */
	std::vector<cv::Rect> test;
	{
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

		/* Check Score */
#if 0
		std::vector<int32_t> vScore;
		for (int32_t i = 0; i < vIsPlateIdx.size(); i++) {
			if (vIsPlateIdx[i].size() > 3) {
				int32_t area = vPlateRects[i].area();
				int32_t sum = 0;
				for (int32_t j = 0; j < vIsPlateIdx[i].size(); j++) {
					sum += vIsPlateIdx[i][j];
				}
				int32_t aveIdx = sum / vIsPlateIdx[i].size();
				test.push_back(vPlateRects[i]);
			}
			else {
				vScore.push_back(0);
			}
		}
#endif
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

void PlateDetector::doDetectionEx(const std::string& fileName, const std::vector<ROI_INFO_S>& vRoiInfos)
{
	_fileName = fileName;

	/* 1. Image Open */
	openImageFile(_fileName);

	/* Crop ROI */
	cv::Mat cropImg = getCropImage(_oriImg, vRoiInfos[0].roi);

	cv::Mat grayImg = getGrayImage(cropImg);
	cv::Mat blurImg = getDenoiseImage(grayImg, 3);
	cv::Mat newImg = cv::Mat::zeros(blurImg.size(), blurImg.type());
	
	for (int32_t i = 0; i < blurImg.rows; i++) {
		for (int32_t j = 0; j < blurImg.cols; j++) {
			int32_t pixVal = blurImg.at<uchar>(i, j);
			if (pixVal <= 128) {
				pixVal = static_cast<int32_t>(pixVal * 0.8);
			}
			else {
				pixVal = static_cast<int32_t>(pixVal * 1.2);
			}

			newImg.at<uchar>(i, j) = cv::saturate_cast<uchar>(pixVal);
		}
	}
	cv::Mat cannyImg;
	cv::Canny(newImg, cannyImg, 50, 150);

	cv::Mat binaryImg = getBinaryImage(blurImg);
	
#if 0
	/* Get Plate Candidate */
	std::vector<cv::Rect> vPlateRects = getPlateCandidates(newImg);
	
	/* Get Char Candidate */
	std::vector<cv::Rect> vCharRects = getCharCandidates(newImg);
#endif

	cv::imshow("Gray Original Image", grayImg);
	cv::imshow("Denoise 3 Image", blurImg);
	cv::imshow("New Image", newImg);
	cv::imshow("Canny Image", cannyImg);
	cv::imshow("binary Image", binaryImg);
	
	cv::waitKey(0);
	
}

std::vector<cv::Rect> PlateDetector::getPlateCandidates(const cv::Mat& src)
{
	cv::Mat blurImg = getDenoiseImage(src, 5);
	cv::Mat binaryImg;
	cv::Canny(blurImg, binaryImg, 50, 150);
	std::vector<cv::Rect> vContourRects = getContoursRect(binaryImg);
	std::vector<cv::Rect> vPlateRect = getPlateRect(vContourRects);

	return vPlateRect;
}

std::vector<cv::Rect> PlateDetector::getCharCandidates(const cv::Mat& src)
{
	cv::Mat blurImg = getDenoiseImage(src, 1);
	cv::Mat binaryImg = getBinaryImage(blurImg);
	std::vector<cv::Rect> vRects = getContoursRect(binaryImg);
	std::vector<cv::Rect> vCharRects = getCharRect(vRects);
	
	return vCharRects;
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

void PlateDetector::openImageFile(const std::string& fileName)
{
	_oriImg = cv::imread(fileName);
}

cv::Mat PlateDetector::getGrayImage(const cv::Mat& src)
{
	cv::Mat img;
	cv::cvtColor(src, img, cv::COLOR_BGR2GRAY);

	return img;
}

cv::Mat PlateDetector::getCropImage(const cv::Mat& src, const cv::Rect& rect)
{
	return src(rect);
}

cv::Mat PlateDetector::getResizedImage(const cv::Mat& src, const cv::Size& res)
{
	cv::Mat img;
	cv::resize(src, img, res);

	return img;
}

cv::Mat PlateDetector::getDenoiseImage(const cv::Mat& src, int32_t strength)
{
	cv::Mat img;
	cv::GaussianBlur(src, img, cv::Size(strength, strength), 5);

	return img;
}

cv::Mat PlateDetector::getBinaryImage(const cv::Mat& src)
{
	cv::Mat img;
	cv::adaptiveThreshold(src, img, 255.0, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 19, 5.0);
	return img;
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

cv::Mat PlateDetector::getDrawRectImage(const cv::Mat& src, const std::vector<cv::Rect>& vRects, const cv::Scalar& color)
{
	cv::Mat img;
	src.copyTo(img);

	for (int32_t i = 0; i < vRects.size(); i++) {
		cv::rectangle(img, vRects[i], color, 1);
	}

	return img;
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

std::vector<cv::Rect> PlateDetector::getCharRect(const std::vector<cv::Rect>& vRects, float minRatio)
{
	std::vector<cv::Rect> vCharRects;
	vCharRects.clear();

	for (int32_t i = 0; i < vRects.size(); i++) {
		float ratio = static_cast<float>(vRects[i].width) / static_cast<float>(vRects[i].height);
		if (vRects[i].area() > 30 && ratio > 0.25 && ratio <= 0.9f) {
			vCharRects.push_back(vRects[i]);
		}
	}

	return vCharRects;
}

std::vector<int32_t> PlateDetector::findLicenseCharRect(const std::vector<cv::Rect>& vRects)
{
	float maxDiagMultiplyer = 5.0f;
	float maxAngleDiff = 20.0f;
	float maxAreaDiff = 0.5f;
	float maxWidthDiff = 0.8f;
	float maxHeightDiff = 0.3f;

	std::vector<std::vector<int32_t>> vMatchIdxResult;
	vMatchIdxResult.clear();

	for (int32_t i = 0; i < vRects.size(); i++) {
		std::vector<int32_t> vMatchIdx;
		vMatchIdx.clear();
		for (int32_t j = 0; j < vRects.size(); j++) {
			if (i == j) {
				vMatchIdx.push_back(j);
				continue;
			}

			double diagLength = std::sqrt(vRects[i].width ^ 2 + vRects[i].height ^ 2) * 5.0;

			int32_t dX = std::abs(getCenterX(vRects[i]) - getCenterX(vRects[j]));
			int32_t dY = std::abs(getCenterY(vRects[i]) - getCenterY(vRects[j]));
			double length = std::sqrt(dX ^ 2 + dY ^ 2);

			double areaDiff = static_cast<double>(::abs(vRects[i].area() - vRects[j].area())) / static_cast<double>(vRects[i].area());
			double widthDiff = static_cast<double>(::abs(vRects[i].width - vRects[j].width)) / static_cast<double>(vRects[i].width);
			double heightDiff = static_cast<double>(::abs(vRects[i].height - vRects[j].height)) / static_cast<double>(vRects[i].height);
			double angleDiff = 0;

			if (dX == 0) {
				angleDiff = 0;
			}
			else if (dY == 0) {
				angleDiff = 90.0;
			}
			else {
				double arcTan = std::atan2(dY, dX);
				angleDiff = arcTan * 180.0 / 3.141592;
			}

			if (areaDiff <= maxAreaDiff && widthDiff <= maxWidthDiff && heightDiff <= maxHeightDiff &&
				length <= diagLength && angleDiff <= maxAngleDiff)
			{
				vMatchIdx.push_back(j);
			}
		}

		vMatchIdxResult.push_back(vMatchIdx);
	}

	std::vector<int32_t> counter;
	counter.clear();
	counter.resize(vRects.size());
	int32_t maxCount = 0;
	for (int32_t i = 0; i < vMatchIdxResult.size(); i++) {
		for (int32_t j = 0; j < vMatchIdxResult[i].size(); j++) {
			counter[vMatchIdxResult[i][j]]++;
			if (maxCount < counter[vMatchIdxResult[i][j]]) {
				maxCount = counter[vMatchIdxResult[i][j]];
			}
		}
	}

	std::vector<int32_t> vResultIdx;
	vResultIdx.clear();
	maxCount = (maxCount + 1) >> 1;
	for (int32_t i = 0; i < counter.size(); i++) {
		if (counter[i] > maxCount) {
			vResultIdx.push_back(i);
		}
	}

	return vResultIdx;
}

int32_t PlateDetector::findPlateCandidate(const std::vector<cv::Rect>& vPlateRect,
									   const std::vector<cv::Rect>& vCharRect,
									   const std::vector<int32_t>& vCharIdxSet)
{
	int32_t idx = -1;
	
	for (int32_t i = 0; i < vPlateRect.size(); i++) {
		int32_t matchCnt = 0;
		
		for (int32_t j = 0; j < vCharIdxSet.size(); j++) {
			cv::Rect charRect = vCharRect[vCharIdxSet[j]];

			int32_t chCenterX = getCenterX(charRect);
			int32_t chCenterY = getCenterY(charRect);

			int32_t plateRight = vPlateRect[i].x + vPlateRect[i].width;
			int32_t plateBottom = vPlateRect[i].y + vPlateRect[i].height;
			
			if (chCenterX > vPlateRect[i].x && chCenterX <= plateRight &&
				chCenterY > vPlateRect[i].y && chCenterY <= plateBottom)
			{
				matchCnt++;
			}

		}

		if (matchCnt >= 2) {
			if (idx == -1) {
				idx = i;
			}
			else {
				if (vPlateRect[idx].area() > vPlateRect[i].area()) {
					idx = i;
				}
			}
		}
	}

	return idx;

}