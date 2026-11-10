#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

// Pengganti library face_recognition
cv::Ptr<cv::FaceDetectorYN> detector;
cv::Ptr<cv::FaceRecognizerSF> recognizer;

// = face_recognition.face_locations()
cv::Mat face_locations_of(const cv::Mat& image) {
    cv::Mat faces;
    detector->setInputSize(image.size());
    detector->detect(image, faces);
    return faces;
}

// = face_recognition.face_encodings()
cv::Mat face_encoding_of(const cv::Mat& image, const cv::Mat& face) {
    cv::Mat aligned, encoding;
    recognizer->alignCrop(image, face, aligned);
    recognizer->feature(aligned, encoding);
    return encoding.clone();
}

int main() {
    detector   = cv::FaceDetectorYN::create("face_detection_yunet.onnx", "", cv::Size(320, 320));
    recognizer = cv::FaceRecognizerSF::create("face_recognition_sface.onnx", "");

    cv::VideoCapture video_capture(0);

    // Load a sample picture and learn how to recognize it.
    cv::Mat biden_image = cv::imread("nara.png");
    if (biden_image.empty()) { std::cerr << "yana tidak ditemukan\n"; return 1; }
    cv::Mat biden_faces = face_locations_of(biden_image);
    if (biden_faces.rows == 0) { std::cerr << "Tidak ada wajah di biden.jpg\n"; return 1; }
    cv::Mat biden_face_encoding = face_encoding_of(biden_image, biden_faces.row(0));

    std::vector<cv::Mat> known_face_encodings = {
        biden_face_encoding
    };
    std::vector<std::string> known_face_names = {
        "Yana"
    };

    std::vector<cv::Rect> face_locations;
    std::vector<std::string> face_names;
    bool process_this_frame = true;
    const double TOLERANCE = 1.128;   // batas jarak SFace (mirip tolerance 0.6 di face_recognition)

    cv::Mat frame, small_frame;

    while (true) {
        if (!video_capture.read(frame)) break;

        if (process_this_frame) {
            cv::resize(frame, small_frame, cv::Size(), 0.25, 0.25);
            cv::Mat faces = face_locations_of(small_frame);

            face_locations.clear();
            face_names.clear();

            for (int i = 0; i < faces.rows; i++) {
                // Kembalikan koordinat ke ukuran asli (x4), seperti top *= 4 di Python
                cv::Mat face = faces.row(i).clone();
                for (int k = 0; k < 14; k++) face.at<float>(0, k) *= 4;

                face_locations.push_back(cv::Rect(face.at<float>(0, 0), face.at<float>(0, 1),
                                                  face.at<float>(0, 2), face.at<float>(0, 3)));

                cv::Mat face_encoding = face_encoding_of(frame, face);

                std::string name = "Unknown";

                // face_distance + argmin
                int best_match_index = -1;
                double best_distance = 1e9;
                for (size_t j = 0; j < known_face_encodings.size(); j++) {
                    double distance = recognizer->match(face_encoding, known_face_encodings[j],
                                                        cv::FaceRecognizerSF::FR_NORM_L2);
                    if (distance < best_distance) {
                        best_distance = distance;
                        best_match_index = (int)j;
                    }
                }
                // compare_faces
                if (best_match_index >= 0 && best_distance <= TOLERANCE) {
                    name = known_face_names[best_match_index];
                }
                face_names.push_back(name);
            }
        }

        process_this_frame = !process_this_frame;

        for (size_t i = 0; i < face_locations.size(); i++) {
            int left   = face_locations[i].x;
            int top    = face_locations[i].y;
            int right  = face_locations[i].x + face_locations[i].width;
            int bottom = face_locations[i].y + face_locations[i].height;

            cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 0, 255), 2);
            cv::rectangle(frame, cv::Point(left, bottom - 35), cv::Point(right, bottom), cv::Scalar(0, 0, 255), cv::FILLED);
            int font = cv::FONT_HERSHEY_DUPLEX;
            cv::putText(frame, face_names[i], cv::Point(left + 6, bottom - 6), font, 1.0, cv::Scalar(255, 255, 255), 1);
        }

        cv::imshow("Video", frame);

        if ((cv::waitKey(1) & 0xFF) == 'q') break;
    }

    video_capture.release();
    cv::destroyAllWindows();
    return 0;
}