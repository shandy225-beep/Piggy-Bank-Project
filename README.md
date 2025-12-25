# Sistem Identifikasi Uang Otomatis

##  Deskripsi Proyek

Sistem identifikasi uang otomatis menggunakan ESP32 yang mampu mendeteksi dan mengidentifikasi uang kertas dan koin secara otomatis. Sistem ini dirancang oleh **Kelompok MILKY WAY** sebagai proyek akhir.

### Fitur Utama
-  Deteksi uang kertas (Rp 20.000, Rp 50.000, Rp 100.000) menggunakan sensor warna TCS3200
-  Deteksi koin (Rp 500, Rp 1.000) menggunakan load cell HX711
-  Monitoring real-time melalui aplikasi Blynk IoT
-  Fungsi reset total uang melalui sensor IR atau aplikasi Blynk
-  Display total uang terakumulasi

---

##  Komponen Hardware

| No | Komponen | Jumlah | Fungsi |
|----|----------|--------|--------|
| 1 | ESP32 Development Board | 1 | Mikrokontroler utama |
| 2 | TCS3200 Color Sensor | 1 | Mendeteksi warna uang kertas |
| 3 | HX711 Load Cell Amplifier | 1 | Membaca data dari load cell |
| 4 | Load Cell (5kg atau 10kg) | 1 | Menimbang koin |
| 5 | IR Sensor | 2 | Trigger deteksi & reset |
| 6 | Kabel Jumper | Secukupnya | Koneksi antar komponen |
| 7 | Breadboard | 1 | Prototipe sirkuit |
| 8 | Power Supply 5V | 1 | Sumber daya |

---

##  Diagram Koneksi Pin

### TCS3200 Color Sensor
\\\
TCS3200        ESP32
--------------------------------
S0             GPIO 23
S1             GPIO 22
S2             GPIO 32
S3             GPIO 33
OUT            GPIO 4
VCC            5V
GND            GND
\\\

### HX711 Load Cell
\\\
HX711          ESP32
--------------------------------
DT (DOUT)      GPIO 26
SCK            GPIO 27
VCC            5V
GND            GND
\\\

### IR Sensor
\\\
IR Sensor      ESP32
--------------------------------
IR #1 OUT      GPIO 18 (Deteksi uang kertas)
IR #2 OUT      GPIO 25 (Reset/Interrupt)
VCC            5V
GND            GND
\\\

### Blok Diagram
<img width="1295" height="606" alt="image" src="https://github.com/user-attachments/assets/91088de5-9c24-48d7-9147-b59d9bbaffdd" />

---

##  Library yang Dibutuhkan

Install library berikut melalui Arduino IDE Library Manager:

1. **WiFi** (Built-in ESP32)
2. **BlynkSimpleEsp32** - Untuk integrasi dengan Blynk IoT
   - Cara install: Library Manager  Search "Blynk"  Install
3. **HX711_ADC** - Untuk membaca data load cell
   - Cara install: Library Manager  Search "HX711_ADC"  Install by Olav Kallhovd

---

##  Konfigurasi

### 1. Konfigurasi WiFi

Edit bagian berikut di kode:
\\\cpp
char ssid[] = "realme 10";      // Ganti dengan SSID WiFi Anda
char pass[] = "12345678";        // Ganti dengan password WiFi Anda
\\\

### 2. Konfigurasi Blynk

1. Buat akun di [Blynk.cloud](https://blynk.cloud/)
2. Buat template baru dengan nama "ESP32"
3. Salin **Template ID** dan **Auth Token**
4. Edit bagian berikut di kode:
\\\cpp
#define BLYNK_TEMPLATE_ID "TMPL6p9414P_W"           // Ganti dengan Template ID Anda
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "X2bUA0-WZzVD06ZHwlkYLBfGNefhEgxF"  // Ganti dengan Auth Token Anda
\\\

### 3. Setup Blynk Datastreams

Buat datastream di Blynk Console:
- **V0** (Virtual Pin 0): Display total uang (Integer)
- **V1** (Virtual Pin 1): Status reset (String)

### 4. Kalibrasi Sensor

#### Kalibrasi TCS3200 (Sensor Warna)
Jalankan program kalibrasi TCS3200 untuk mendapatkan nilai min-max RGB. Edit nilai berikut:
\\\cpp
int redMin   = 167,  redMax   = 1400;
int greenMin = 163,  greenMax = 1420;
int blueMin  = 138,  blueMax  = 1170;
\\\

#### Kalibrasi HX711 (Load Cell)
1. Jalankan program kalibrasi HX711
2. Gunakan beban standar (misalnya 100g, 500g)
3. Dapatkan calibration factor
4. Edit nilai berikut:
\\\cpp
loadcell.setCalFactor(1034.61);  // Ganti dengan hasil kalibrasi Anda
\\\

#### Kalibrasi Deteksi Uang Kertas
Sesuaikan threshold RGB di fungsi \detectUang()\ berdasarkan hasil pembacaan warna uang Anda:
\\\cpp
// Rp 100.000 (Merah)
if (r > g && r > b && r > 170 && g > 155 && b > 155) return 100000;

// Rp 50.000 (Biru)
if (b > r && b > g && r > 150 && g > 170 && b > 175) return 50000;

// Rp 20.000 (Hijau)
if (g > r && g > b && r > 150 && g > 170 && b > 155) return 20000;
\\\

#### Kalibrasi Deteksi Koin
Sesuaikan range berat koin jika diperlukan:
\\\cpp
// Koin Rp 1000: 3.25 - 5.5 gram
if (berat >= 3.25 && berat <= 5.5) uangTotal += 1000;

// Koin Rp 500: 2 - 3.2 gram
else if (berat >= 2 && berat < 3.2) uangTotal += 500;
\\\

---

##  Cara Penggunaan

### Upload Program
1. Buka file \Projek_identifikasi_uang.ino\ di Arduino IDE
2. Pilih board: **ESP32 Dev Module**
3. Pilih port COM yang sesuai
4. Upload program ke ESP32

### Menjalankan Sistem
1. Buka **Serial Monitor** (baud rate: 9600)
2. Tunggu hingga ESP32 terhubung ke WiFi dan Blynk
3. Sistem siap digunakan!

### Deteksi Uang Kertas
1. Letakkan uang kertas di atas sensor warna TCS3200
2. Pastikan IR sensor mendeteksi keberadaan uang
3. Sistem akan otomatis membaca warna dan mengidentifikasi nominal
4. Total uang akan bertambah

### Deteksi Koin
1. Letakkan koin di atas load cell
2. Sistem akan menimbang dan mengidentifikasi nominal berdasarkan berat
3. Total uang akan bertambah
4. Angkat koin untuk deteksi berikutnya

### Reset Total Uang
Ada 2 cara untuk reset:
1. **Manual**: Trigger IR sensor ke-2 (GPIO 25)
2. **Via Blynk**: Tekan tombol reset di aplikasi Blynk

---

##  Output Serial Monitor

Contoh output saat sistem berjalan:
\\\
========================================
  SISTEM IDENTIFIKASI UANG OTOMATIS
  Kelompok: MILKY WAY
========================================

Inisialisasi Load Cell...
Load Cell siap.

Inisialisasi TCS3200 Color Sensor...
TCS3200 siap.

Inisialisasi IR Sensor...
IR Sensor siap.

Menghubungkan ke Blynk...
Terhubung ke Blynk!

========================================
  SISTEM SIAP DIGUNAKAN!
========================================

[KOIN] Rp 1.000 terdeteksi
Berat: 3.45 gram
Total uang: Rp 1000

[KERTAS] Uang kertas terdeteksi!
Membaca warna...
RGB: [185, 162, 158]
[KERTAS] Nominal: Rp 100000
Total uang: Rp 101000
\\\

---

##  Troubleshooting

### ESP32 Tidak Terhubung ke WiFi
- Pastikan SSID dan password benar
- Cek jarak ESP32 dengan router WiFi
- Pastikan WiFi menggunakan 2.4GHz (ESP32 tidak support 5GHz)

### Sensor Warna Tidak Akurat
- Lakukan kalibrasi ulang TCS3200
- Pastikan pencahayaan konsisten
- Sesuaikan threshold RGB di fungsi \detectUang()\

### Load Cell Tidak Stabil
- Lakukan kalibrasi ulang HX711
- Pastikan load cell terpasang dengan benar
- Hindari getaran pada permukaan load cell

### Blynk Tidak Update
- Cek koneksi internet
- Pastikan Auth Token benar
- Cek quota Blynk (free account memiliki limit)

### Deteksi Ganda (Double Detection)
- Sesuaikan waktu debounce
- Pastikan sensor IR bekerja dengan baik

---

##  Pengembangan Lebih Lanjut

Beberapa ide untuk pengembangan proyek:
- [ ] Tambah support untuk nominal uang lainnya (Rp 1.000, Rp 2.000, Rp 5.000, Rp 10.000)
- [ ] Implementasi LCD display untuk menampilkan total uang
- [ ] Tambah buzzer untuk notifikasi audio
- [ ] Implementasi machine learning untuk deteksi uang lebih akurat
- [ ] Tambah fitur sorting otomatis berdasarkan nominal
- [ ] Database logging untuk histori transaksi

---

##  Kelompok MILKY WAY

Proyek ini dikembangkan sebagai Tugas Akhir oleh Kelompok MILKY WAY.

---

##  Lisensi

Proyek ini dibuat untuk keperluan edukasi.

---

##  Kontak & Dukungan

Jika ada pertanyaan atau masalah, silakan hubungi tim pengembang.

---

**Terakhir diupdate:** Desember 2025

