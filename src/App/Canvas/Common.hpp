#pragma once

#include <QImage>

namespace cv {
class Mat;
}

namespace PixelForge {

QImage imageFromMat(const cv::Mat &image);

}
