#include "PanoCvUtil.h"

namespace pano
{
	namespace pano_cv
	{
		cv::Mat getResizeImage(const cv::Mat& src, const cv::Size& res)
		{
			cv::Mat resizeImg;
			cv::resize(src, resizeImg, res);

			return resizeImg;
		}

		cv::Mat getCropImage(const cv::Mat& src, const cv::Rect& rect)
		{
			return src(rect);
		}

		cv::Mat getRotatedImage(const cv::Mat& src, const int32_t angle)
		{
			cv::Mat rotFactor = cv::getRotationMatrix2D(cv::Point(0, 0), angle, 1.5);
			cv::Mat img;
			cv::warpAffine(src, img, rotFactor, cv::Size(src.cols * 1.5, src.rows * 1.5));

			return img;
		}

		cv::Mat getGrayImage(const cv::Mat& src)
		{
			cv::Mat img;
			cv::cvtColor(src, img, cv::COLOR_BGR2GRAY);

			return img;
		}

		cv::Mat getContrastAdjust(const cv::Mat& src, int32_t centerVal, double upperFactor, double underFactor)
		{
			cv::Mat img = cv::Mat::zeros(src.size(), src.type());

			for (int32_t i = 0; i < src.rows; i++) {
				for (int32_t j = 0; j < src.cols; j++) {
					int32_t pixVal = src.at<uchar>(i, j);
					if (pixVal <= centerVal) {
						pixVal = static_cast<int32_t>(pixVal * underFactor);
					}
					else {
						pixVal = static_cast<int32_t>(pixVal * upperFactor);
					}

					img.at<uchar>(i, j) = cv::saturate_cast<uchar>(pixVal);
				}
			}

			return img;
		}

		cv::Mat getCannyImage(const cv::Mat& src, int32_t under, int32_t upper)
		{
			cv::Mat cannyImg;
			cv::Canny(src, cannyImg, 50, 150);

			return cannyImg;
		}

		cv::Mat getHistEquationImage(const cv::Mat& src)
		{
			cv::Mat histImg;
			cv::equalizeHist(src, histImg);

			return histImg;
		}

		cv::Mat getBinaryImage(const cv::Mat& src)
		{
			cv::Mat img;
			cv::adaptiveThreshold(src, img, 255.0, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 19, 5.0);
			return img;
		}

		cv::Mat getDenoiseImage(const cv::Mat& src, int32_t strength)
		{
			cv::Mat img;
			cv::GaussianBlur(src, img, cv::Size(strength, strength), 5);

			return img;
		}

#if 0
		cv::Mat drawRois(const cv::Mat& src, const std::vector<ROI_INFO_S>& vRoiInfos, const cv::Scalar& color)
		{
			cv::Mat img;
			src.copyTo(img);

			if (!vRoiInfos.size()) {
				return img;
			}

			for (int32_t i = 0; i < vRoiInfos.size(); i++) {
				cv::rectangle(img, vRoiInfos[i].roi, color, 3);
			}

			return img;
		}
#endif

		cv::Mat drawRect(const cv::Mat& src, const std::vector<cv::Rect>& vRects, const cv::Scalar& color)
		{
			cv::Mat img;
			src.copyTo(img);

			if (!vRects.size()) {
				return img;
			}

			for (int32_t i = 0; i < vRects.size(); i++) {
				cv::rectangle(img, vRects[i], color, 1);
			}

			return img;
		}

		cv::Point getCenterPoint(const cv::Rect& rect)
		{
			cv::Point cp;
			cp.x = rect.x + rect.width / 2;
			cp.y = rect.y + rect.height / 2;

			return cp;
		}
	}
}