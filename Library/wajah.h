#ifndef WAJAH_H
#define WAJAH_H

#ifdef __cplusplus
extern "C" {
#endif

#define WAJAH_DIM 128   /* panjang encoding satu wajah */
#define CV_FILLED  -1   /* ketebalan -1 = kotak diisi penuh */

/* Kotak posisi wajah (pengganti cv::Rect) */
typedef struct {
    int x, y, width, height;
} FaceRect;

/* = FaceDetectorYN::create + FaceRecognizerSF::create. 1 = berhasil */
int model_init(const char *model_deteksi, const char *model_kenali);

/* = imread + face_locations_of + face_encoding_of(row(0))
   Return: 1 = berhasil, 0 = tidak ada wajah, -1 = file tidak ditemukan */
int load_face_encoding(const char *file, float encoding[WAJAH_DIM]);

/* = cv::VideoCapture */
int  video_capture_open(int index);   /* 1 = berhasil */
int  video_capture_read(void);        /* 1 = ada frame */
void video_capture_release(void);

/* = resize + face_locations_of + (x4) + face_encoding_of untuk frame kamera.
   Return: jumlah wajah */
int face_locations_and_encodings(double scale, FaceRect locations[],
                                 float encodings[][WAJAH_DIM], int max_faces);

/* = cv::rectangle, cv::putText, cv::imshow, cv::waitKey, cv::destroyAllWindows */
void draw_rectangle(int left, int top, int right, int bottom,
                    int b, int g, int r, int thickness);
void put_text(const char *text, int x, int y, double scale,
              int b, int g, int r, int thickness);
void imshow_frame(const char *window_name);
int  wait_key(int delay_ms);
void destroy_all_windows(void);

#ifdef __cplusplus
}
#endif

#endif /* WAJAH_H */