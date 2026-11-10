#include "wajah.h"
#include <opencv2/opencv.hpp>

static cv::Ptr<cv::FaceDetectorYN>  detector;
static cv::Ptr<cv::FaceRecognizerSF> recognizer;
static cv::VideoCapture video_capture;
static cv::Mat frame;

// = face_recognition.face_locations()
static cv::Mat face_locations_of(const cv::Mat &image) {
    cv::Mat faces;
    detector->setInputSize(image.size());
    detector->detect(image, faces);
    return faces;
}

// = face_recognition.face_encodings()
// Dinormalisasi (panjang vektor = 1) supaya jarak Euclidean di main.c
// sama dengan FR_NORM_L2 milik SFace.
static void face_encoding_of(const cv::Mat &image, const cv::Mat &face, float out[WAJAH_DIM]) {
    cv::Mat aligned, encoding;
    recognizer->alignCrop(image, face, aligned);
    recognizer->feature(aligned, encoding);
    cv::normalize(encoding, encoding);
    for (int i = 0; i < WAJAH_DIM; i++) out[i] = encoding.at<float>(0, i);
}

extern "C" {

int model_init(const char *model_deteksi, const char *model_kenali) {
    try {
        detector   = cv::FaceDetectorYN::create(model_deteksi, "", cv::Size(320, 320));
        recognizer = cv::FaceRecognizerSF::create(model_kenali, "");
    } catch (const cv::Exception &) {
        return 0;
    }
    return 1;
}

int load_face_encoding(const char *file, float encoding[WAJAH_DIM]) {
    cv::Mat image = cv::imread(file);
    if (image.empty()) return -1;
    cv::Mat faces = face_locations_of(image);
    if (faces.rows == 0) return 0;
    face_encoding_of(image, faces.row(0), encoding);
    return 1;
}

int video_capture_open(int index) {
    video_capture.open(index);
    return video_capture.isOpened() ? 1 : 0;
}

int video_capture_read(void) {
    return (video_capture.read(frame) && !frame.empty()) ? 1 : 0;
}

void video_capture_release(void) {
    video_capture.release();
}

int face_locations_and_encodings(double scale, FaceRect locations[],
                                 float encodings[][WAJAH_DIM], int max_faces) {
    if (frame.empty()) return 0;

    cv::Mat small_frame;
    cv::resize(frame, small_frame, cv::Size(), scale, scale);
    cv::Mat faces = face_locations_of(small_frame);

    int n = faces.rows < max_faces ? faces.rows : max_faces;
    for (int i = 0; i < n; i++) {
        // Kembalikan koordinat ke ukuran asli (14 angka pertama: kotak + 5 titik)
        cv::Mat face = faces.row(i).clone();
        for (int k = 0; k < 14; k++) face.at<float>(0, k) /= (float)scale;

        locations[i].x      = (int)face.at<float>(0, 0);
        locations[i].y      = (int)face.at<float>(0, 1);
        locations[i].width  = (int)face.at<float>(0, 2);
        locations[i].height = (int)face.at<float>(0, 3);

        face_encoding_of(frame, face, encodings[i]);
    }
    return n;
}

void draw_rectangle(int left, int top, int right, int bottom,
                    int b, int g, int r, int thickness) {
    cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom),
                  cv::Scalar(b, g, r), thickness);
}

void put_text(const char *text, int x, int y, double scale,
              int b, int g, int r, int thickness) {
    cv::putText(frame, text, cv::Point(x, y), cv::FONT_HERSHEY_DUPLEX,
                scale, cv::Scalar(b, g, r), thickness);
}

void imshow_frame(const char *window_name) {
    if (!frame.empty()) cv::imshow(window_name, frame);
}

int wait_key(int delay_ms) {
    return cv::waitKey(delay_ms);
}

void destroy_all_windows(void) {
    cv::destroyAllWindows();
}

} /* extern "C" */