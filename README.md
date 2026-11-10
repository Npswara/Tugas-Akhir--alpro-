# FACE ID: Pengenalan Wajah dalam Bahasa C

Program pengenalan wajah *real-time* dari kamera. Program utamanya ditulis dalam **bahasa C**, sedangkan **OpenCV** (C++) dipakai lewat sebuah file jembatan kecil.

Program membaca foto wajah orang yang ingin dikenali, lalu membuka kamera. Setiap wajah yang terlihat diberi kotak merah dan nama orangnya, atau tulisan `Unknown` jika wajah itu tidak dikenal.

Alurnya meniru contoh Python `face_recognition` yang populer, tetapi deteksi dan pengenalannya memakai dua model bawaan OpenCV:

| Tugas | Model |
|---|---|
| Menemukan posisi wajah | **YuNet** (`face_detection_yunet.onnx`) |
| Mengenali siapa orangnya | **SFace** (`face_recognition_sface.onnx`) |

---

## Struktur folder

```
latihan/
├── main.c                        Program utama (C murni)
├── wajah.h                       Header C: daftar fungsi OpenCV yang bisa dipanggil dari C
├── wajah.cpp                     Jembatan C++ ke OpenCV (dibungkus extern "C")
├── Makefile                      Perintah build
├── README.md                     Dokumen ini
├── nara.png                      Foto wajah orang yang dikenali
├── face_detection_yunet.onnx     Model deteksi wajah (diunduh)
└── face_recognition_sface.onnx   Model pengenalan wajah (diunduh)
```

### Mengapa ada file C++?
Sejak versi 4, OpenCV tidak lagi menyediakan API bahasa C. Karena itu, `wajah.cpp` membungkus fungsi-fungsi OpenCV dengan `extern "C"` supaya bisa dipanggil dari `main.c` seperti fungsi C biasa. Logika program, yaitu daftar orang, perhitungan jarak, pencarian wajah yang paling mirip, dan perulangan utama, tetap ditulis dalam C di `main.c`.

---

## Kebutuhan

- macOS (Apple Silicon atau Intel)
- Xcode Command Line Tools (`clang`, `clang++`, `make`)
- Homebrew
- OpenCV 5.x (diuji dengan struktur folder Homebrew OpenCV 5.1)
- Kamera (webcam bawaan atau USB)

---

## Instalasi

### 1. Pasang alat build dan OpenCV
```bash
xcode-select --install
```
```bash
brew install opencv
```

### 2. Unduh model
Jalankan dari dalam folder proyek:
```bash
curl -L -o face_detection_yunet.onnx https://media.githubusercontent.com/media/opencv/opencv_zoo/main/models/face_detection_yunet/face_detection_yunet_2023mar.onnx
```
```bash
curl -L -o face_recognition_sface.onnx https://media.githubusercontent.com/media/opencv/opencv_zoo/main/models/face_recognition_sface/face_recognition_sface_2021dec.onnx
```

Cek ukurannya:
```bash
ls -lh *.onnx
```
| File | Ukuran yang benar |
|---|---|
| `face_detection_yunet.onnx` | sekitar 230 KB |
| `face_recognition_sface.onnx` | sekitar 37 MB |

Jika ukurannya hanya ratusan byte, yang terunduh adalah file penunjuk Git LFS, bukan model aslinya. Hapus file itu dan unduh ulang dari alamat `media.githubusercontent.com` di atas.

### 3. Siapkan foto wajah
Taruh foto wajah orang yang ingin dikenali di folder proyek, misalnya `nara.png`. Foto sebaiknya:
- hanya berisi **satu** wajah,
- wajahnya menghadap ke depan dengan cahaya yang cukup,
- berformat `.jpg` atau `.png`.

---

## Build dan menjalankan

```bash
make
```
```bash
./facerec
```

Tekan **`q`** di jendela video untuk keluar.

Saat pertama kali dijalankan, macOS akan meminta izin kamera. Klik **Allow**. Jika kamera tidak terbuka, aktifkan izinnya di **System Settings → Privacy & Security → Camera** untuk Terminal (atau Visual Studio Code), lalu buka ulang aplikasi tersebut.

### Build manual tanpa `make`
```bash
clang -std=c11 -O2 -c main.c -o main.o
```
```bash
clang++ -std=c++17 -O2 -c wajah.cpp -o wajah.o -I/opt/homebrew/opt/opencv/include/opencv5
```
```bash
clang++ main.o wajah.o -o facerec -L/opt/homebrew/opt/opencv/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_highgui -lopencv_videoio -lopencv_objdetect -lopencv_dnn -Wl,-rpath,/opt/homebrew/opt/opencv/lib
```

Langkah terakhir harus memakai `clang++`, bukan `clang`, supaya library standar C++ yang dipakai `wajah.cpp` ikut tertaut.

Untuk menghapus hasil build:
```bash
make clean
```

### Arti opsi compile
| Opsi | Arti |
|---|---|
| `-std=c11` / `-std=c++17` | Standar bahasa yang dipakai. OpenCV 5 membutuhkan C++17 |
| `-O2` | Optimasi supaya program berjalan lebih cepat |
| `-I...` | Folder tempat file header OpenCV (`opencv2/opencv.hpp`) |
| `-L...` | Folder tempat file library OpenCV (`.dylib`) |
| `-lopencv_xxx` | Modul OpenCV yang ditautkan |
| `-Wl,-rpath,...` | Supaya program tahu letak library OpenCV saat dijalankan |

### Modul OpenCV yang dipakai
| Library | Dipakai untuk |
|---|---|
| `opencv_core` | `cv::Mat` dan tipe dasar lainnya |
| `opencv_imgproc` | `resize`, `rectangle`, `putText` |
| `opencv_imgcodecs` | `imread` (membaca foto) |
| `opencv_highgui` | `imshow`, `waitKey` (jendela dan keyboard) |
| `opencv_videoio` | `VideoCapture` (kamera) |
| `opencv_objdetect` | `FaceDetectorYN`, `FaceRecognizerSF` |
| `opencv_dnn` | Menjalankan model `.onnx` |

---

## Menambah orang yang dikenali

Buka `main.c`. Untuk setiap orang baru, tambahkan encoding-nya, lalu masukkan ke kedua daftar dengan **urutan yang sama**:

```c
/* Load sample pictures and learn how to recognize them. */
float biden_face_encoding[WAJAH_DIM];
int status = load_face_encoding("nara.png", biden_face_encoding);
if (status == -1) { fprintf(stderr, "yana tidak ditemukan\n"); return 1; }
if (status == 0)  { fprintf(stderr, "Tidak ada wajah di nara.png\n"); return 1; }

float budi_face_encoding[WAJAH_DIM];
status = load_face_encoding("budi.jpg", budi_face_encoding);
if (status != 1) { fprintf(stderr, "budi.jpg bermasalah\n"); return 1; }

float *known_face_encodings[] = {
    biden_face_encoding,
    budi_face_encoding
};
const char *known_face_names[] = {
    "Yana",
    "Budi"
};
```

Setelah itu, jalankan `make` lagi.

---

## Cara kerja

```
Mulai
  │
  ├─ model_init()           muat YuNet + SFace
  ├─ video_capture_open()   buka kamera
  ├─ load_face_encoding()   foto → 128 angka "sidik jari" wajah
  │
  └─ Ulangi setiap frame:
       ├─ video_capture_read()
       ├─ (selang-seling) face_locations_and_encodings()
       │     perkecil frame 0.25x → deteksi wajah → koordinat x4 → encoding
       ├─ untuk setiap wajah:
       │     hitung face_distance() ke semua wajah terdaftar
       │     ambil jarak terkecil (argmin)
       │     jarak <= TOLERANCE ? nama orang : "Unknown"
       ├─ draw_rectangle() + put_text()
       ├─ imshow_frame()
       └─ tombol 'q' → keluar
  │
video_capture_release() + destroy_all_windows()
```

### Konsep penting
- **Encoding wajah**: SFace mengubah wajah menjadi vektor 128 angka. Wajah orang yang sama menghasilkan vektor yang berdekatan.
- **Jarak Euclidean**: dihitung di `face_distance()` dengan rumus √Σ(aᵢ − bᵢ)². Makin kecil jaraknya, makin mirip wajahnya. Encoding sudah dinormalisasi di `wajah.cpp`, sehingga hasilnya sama dengan `FR_NORM_L2` milik OpenCV.
- **TOLERANCE = 1.128**: batas jarak yang direkomendasikan untuk SFace. Jika jaraknya ≤ 1.128, kedua wajah dianggap orang yang sama.
- **Frame selang-seling**: deteksi hanya dijalankan pada setengah dari jumlah frame (`process_this_frame`) supaya program lebih ringan.
- **Skala 0.25**: deteksi dilakukan pada gambar yang diperkecil supaya cepat, tetapi encoding diambil dari gambar ukuran asli supaya tetap akurat.

---

## Daftar fungsi di `wajah.h`

| Fungsi | Padanan OpenCV | Keterangan |
|---|---|---|
| `model_init(det, rec)` | `FaceDetectorYN::create`, `FaceRecognizerSF::create` | Return 1 jika berhasil |
| `load_face_encoding(file, enc)` | `imread` + `detect` + `alignCrop` + `feature` | Return 1 = berhasil, 0 = tidak ada wajah, -1 = file tidak ditemukan |
| `video_capture_open(i)` | `VideoCapture::open` | Return 1 jika kamera terbuka |
| `video_capture_read()` | `VideoCapture::read` | Return 1 jika ada frame |
| `video_capture_release()` | `VideoCapture::release` | Menutup kamera |
| `face_locations_and_encodings(skala, loc, enc, maks)` | `resize` + `detect` + `feature` | Return jumlah wajah di frame |
| `draw_rectangle(l, t, r, b, B, G, R, tebal)` | `cv::rectangle` | `tebal = CV_FILLED` untuk kotak terisi |
| `put_text(teks, x, y, skala, B, G, R, tebal)` | `cv::putText` | Font `FONT_HERSHEY_DUPLEX` |
| `imshow_frame(nama)` | `cv::imshow` | Menampilkan frame |
| `wait_key(ms)` | `cv::waitKey` | Kode tombol, -1 jika tidak ada |
| `destroy_all_windows()` | `cv::destroyAllWindows` | Menutup jendela |

Warna ditulis dalam urutan **B, G, R** (Biru, Hijau, Merah), mengikuti aturan OpenCV.

---

## Pengaturan

| Yang ingin diubah | Letak |
|---|---|
| Ketelitian pengenalan | `TOLERANCE` di `main.c`. Turunkan (mis. 1.0) agar lebih ketat, naikkan (mis. 1.2) agar lebih longgar |
| Kecepatan atau jangkauan deteksi | Angka `0.25` di `face_locations_and_encodings(0.25, ...)`. Makin besar, wajah yang jauh lebih mudah terdeteksi, tetapi program makin lambat |
| Kamera lain (iPhone, USB) | `video_capture_open(0)` → ganti `0` menjadi `1` |
| Jumlah wajah maksimum per frame | `MAX_FACES` di `main.c` |
| Letak OpenCV | `OPENCV` di `Makefile` (Mac Intel: `/usr/local/opt/opencv`) |

---

## Mengatasi masalah

| Pesan / gejala | Penyebab dan solusi |
|---|---|
| `cannot open source file "opencv2/opencv.hpp"` (VS Code) | VS Code belum tahu letak header. Tambahkan `/opt/homebrew/opt/opencv/include/opencv5` ke `includePath` di `.vscode/c_cpp_properties.json` |
| `opencv2/opencv.hpp file not found` (compile) | Path `-I` salah. Cari dengan `find /opt/homebrew -name opencv.hpp` |
| `Undefined symbols for architecture arm64` | Library belum ditautkan. Pakai `make`, jangan tombol ▶ Run di VS Code |
| `Library not loaded: libopencv_...` | Opsi `-Wl,-rpath,...` tidak ikut saat compile |
| `Model .onnx gagal dimuat` atau error `parseFile` | File model tidak ada atau rusak. Cek ukurannya (lihat bagian Instalasi) |
| `yana tidak ditemukan` | `nara.png` tidak ada di folder tempat program dijalankan |
| `Tidak ada wajah di nara.png` | Ganti dengan foto yang wajahnya lebih jelas dan menghadap depan |
| Jendela hitam atau kamera tidak menyala | Izin kamera belum diberikan (lihat bagian Build dan menjalankan) |
| Semua wajah muncul sebagai `Unknown` | Naikkan `TOLERANCE` sedikit, atau pakai foto referensi yang lebih jelas |
| Orang lain dikenali sebagai Yana | Turunkan `TOLERANCE` |

Jalankan program dari folder proyek (`./facerec`), karena file model dan foto dicari di folder tempat program dijalankan.

---

## Referensi
- OpenCV: https://opencv.org
- Model YuNet dan SFace (OpenCV Zoo): https://github.com/opencv/opencv_zoo
- Konsep alur program: contoh `facerec_from_webcam_faster.py` dari library Python `face_recognition`
