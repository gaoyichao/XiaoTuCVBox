#ifndef XTCVB_QRCODE_DETECTOR_H
#define XTCVB_QRCODE_DETECTOR_H

#include <string>
#include <vector>

#include <zbar.h>
#include <opencv2/opencv.hpp>

namespace xiaotu {
namespace cv {

    struct ArtMarker {
        std::string type;
        std::string msg;
        std::vector<::cv::Point> location;
    };


    class ArtMarkerDetector
    {
        public:
            ArtMarkerDetector()
            {
                mScanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
            }

            int ScanGrayImg(::cv::Mat const & gray, std::vector<ArtMarker> & markers)
            {
                zbar::Image zbar_image(gray.cols, gray.rows, "Y800", (uint8_t *)gray.data, gray.cols * gray.rows);

                markers.clear();
                int n = mScanner.scan(zbar_image);
                if (n <= 0)
                    return n;

                int idx = 0;
                markers.resize(n);
                for (auto it = zbar_image.symbol_begin(); it != zbar_image.symbol_end(); ++it) {
                    markers[idx].type = it->get_type_name();
                    markers[idx].msg = it->get_data();

                    for (int i = 0; i < it->get_location_size(); i++)
                        markers[idx].location.push_back(::cv::Point(it->get_location_x(i), it->get_location_y(i)));
                    idx++;
                }

                return n;
            }

            void Draw(::cv::Mat & im, std::vector<ArtMarker> & markers)
            {
                for (int i = 0; i < markers.size(); i++) {
                    std::vector<::cv::Point> points = markers[i].location;
                    std::vector<::cv::Point> hull;
                    if (points.size() > 4)
                        ::cv::convexHull(points, hull);
                    else
                        hull = points;

                    int n = hull.size();
                    for (int j = 0; j < n; j++)
                        ::cv::line(im, hull[j], hull[(j+1) % n], ::cv::Scalar(255, 0, 0), 3);
                }
            }

        private:
            zbar::ImageScanner mScanner;
    };

}
}

#endif
