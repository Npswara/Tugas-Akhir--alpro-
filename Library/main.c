#include <stdio.h>
#include <math.h>
#include "wajah.h"

#define MAX_FACES 20

/* = recognizer->match(a, b, FR_NORM_L2): jarak Euclidean dua encoding */
static double face_distance(const float *a, const float *b) {
    double total = 0.0;
    for (int i = 0; i < WAJAH_DIM; i++) {
        double d = a[i] - b[i];
        total += d * d;
    }
    return sqrt(total);
}

int main(void) {
    if (!model_init("face_detection_yunet.onnx", "face_recognition_sface.onnx")) {
        fprintf(stderr, "Model .onnx gagal dimuat\n");
        return 1;
    }

    video_capture_open(0);

    /* Load a sample picture and learn how to recognize it. */
    float biden_face_encoding[WAJAH_DIM];
    int status = load_face_encoding("nara.png", biden_face_encoding);
    if (status == -1) { fprintf(stderr, "yana tidak ditemukan\n"); return 1; }
    if (status == 0)  { fprintf(stderr, "Tidak ada wajah di nara.png\n"); return 1; }

    float *known_face_encodings[] = {
        biden_face_encoding
    };
    const char *known_face_names[] = {
        "Yana"
    };
    int known_count = sizeof(known_face_names) / sizeof(known_face_names[0]);

    FaceRect    face_locations[MAX_FACES];
    float       face_encodings[MAX_FACES][WAJAH_DIM];
    const char *face_names[MAX_FACES];
    int         face_count = 0;
    int         process_this_frame = 1;
    const double TOLERANCE = 1.128;   /* batas jarak SFace (mirip tolerance 0.6 di face_recognition) */

    while (1) {
        if (!video_capture_read()) break;

        if (process_this_frame) {
            /* resize 0.25 + deteksi + koordinat x4 + encoding */
            face_count = face_locations_and_encodings(0.25, face_locations, face_encodings, MAX_FACES);

            for (int i = 0; i < face_count; i++) {
                const char *name = "Unknown";

                /* face_distance + argmin */
                int best_match_index = -1;
                double best_distance = 1e9;
                for (int j = 0; j < known_count; j++) {
                    double distance = face_distance(face_encodings[i], known_face_encodings[j]);
                    if (distance < best_distance) {
                        best_distance = distance;
                        best_match_index = j;
                    }
                }
                /* compare_faces */
                if (best_match_index >= 0 && best_distance <= TOLERANCE) {
                    name = known_face_names[best_match_index];
                }
                face_names[i] = name;
            }
        }

        process_this_frame = !process_this_frame;

        for (int i = 0; i < face_count; i++) {
            int left   = face_locations[i].x;
            int top    = face_locations[i].y;
            int right  = face_locations[i].x + face_locations[i].width;
            int bottom = face_locations[i].y + face_locations[i].height;

            draw_rectangle(left, top, right, bottom, 0, 0, 255, 2);
            draw_rectangle(left, bottom - 35, right, bottom, 0, 0, 255, CV_FILLED);
            put_text(face_names[i], left + 6, bottom - 6, 1.0, 255, 255, 255, 1);
        }

        imshow_frame("Video");

        if ((wait_key(1) & 0xFF) == 'q') break;
    }

    video_capture_release();
    destroy_all_windows();
    return 0;
}