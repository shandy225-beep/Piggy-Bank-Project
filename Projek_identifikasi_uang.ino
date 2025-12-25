/*
 * ========================================================================
 * SISTEM IDENTIFIKASI UANG OTOMATIS
 * Kelompok: MILKY WAY
 * ========================================================================
 * Deskripsi:
 * Sistem ini menggunakan ESP32 untuk mendeteksi dan mengidentifikasi
 * uang kertas dan koin secara otomatis menggunakan:
 * - Sensor warna TCS3200 untuk identifikasi uang kertas
 * - Load cell HX711 untuk identifikasi koin berdasarkan berat
 * - IR sensor untuk trigger deteksi
 * - Blynk IoT untuk monitoring dan reset
 * ========================================================================
 */

#define BLYNK_PRINT Serial

/* ========================================================================
 * KONFIGURASI BLYNK
 * ======================================================================== */
#define BLYNK_TEMPLATE_ID "TMPL6p9414P_W"
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "X2bUA0-WZzVD06ZHwlkYLBfGNefhEgxF"

/* ========================================================================
 * LIBRARY
 * ======================================================================== */
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HX711_ADC.h>

/* ========================================================================
 * KONFIGURASI WIFI
 * ======================================================================== */
char ssid[] = "realme 10";     // Ganti dengan SSID WiFi Anda
char pass[] = "12345678";       // Ganti dengan password WiFi Anda

/* ========================================================================
 * KONFIGURASI PIN
 * ======================================================================== */
// Pin TCS3200 Color Sensor
#define S0            23
#define S1            22
#define S2            32
#define S3            33
#define sensorOut     4

// Pin IR Sensor
#define IR_SENSOR_PIN     18    // Sensor untuk deteksi uang kertas
#define IR_SENSOR2_PIN    25    // Sensor untuk reset (interrupt)

// Pin HX711 Load Cell
const int HX711_dout = 26;
const int HX711_sck  = 27;

/* ========================================================================
 * KALIBRASI SENSOR WARNA TCS3200
 * Nilai ini didapat dari hasil kalibrasi untuk uang rupiah
 * ======================================================================== */
int redMin   = 167,  redMax   = 1400;
int greenMin = 163,  greenMax = 1420;
int blueMin  = 138,  blueMax  = 1170;

/* ========================================================================
 * INISIALISASI OBJEK
 * ======================================================================== */
HX711_ADC loadcell(HX711_dout, HX711_sck);
BlynkTimer timer;

/* ========================================================================
 * VARIABEL GLOBAL
 * ======================================================================== */
// Variabel untuk sensor IR dan warna
int currentIRState, prevIRState = HIGH;
int redPW = 0, greenPW = 0, bluePW = 0;     // Output raw pulse dari sensor
int reds = 0, greens = 0, blues = 0;        // Nilai RGB yang sudah di-mapping

// Variabel untuk total uang dan deteksi koin
int uangTotal = 0;
bool koinTerdeteksi = false;

// Variabel untuk debounce dan interrupt
volatile bool isrTriggered = false;
volatile unsigned long lastInterruptTime = 0;
unsigned long lastKoinTime = 0;
const unsigned long debounceKoinTime = 2000;    // Debounce 2 detik untuk koin

/* ========================================================================
 * FUNGSI ISR (INTERRUPT SERVICE ROUTINE)
 * Fungsi ini dipanggil ketika IR sensor 2 mendeteksi objek (FALLING edge)
 * Digunakan untuk reset total uang
 * ======================================================================== */
void IRAM_ATTR isr() {
  unsigned long interruptTime = millis();
  
  // Debounce 200ms untuk mencegah trigger berulang
  if (interruptTime - lastInterruptTime > 200) {
    isrTriggered = true;
    lastInterruptTime = interruptTime;
  }
}

/* ========================================================================
 * FUNGSI UPDATE BLYNK
 * Update nilai total uang ke Blynk Virtual Pin V0
 * ======================================================================== */
void updateBlynk() {
  Blynk.virtualWrite(V0, uangTotal);
}

/* ========================================================================
 * FUNGSI SETUP
 * Inisialisasi semua komponen dan koneksi
 * ======================================================================== */
void setup() {
  // Inisialisasi Serial Monitor
  Serial.begin(9600);
  Serial.println("\n========================================");
  Serial.println("  SISTEM IDENTIFIKASI UANG OTOMATIS");
  Serial.println("  Kelompok: MILKY WAY");
  Serial.println("========================================\n");

  // Inisialisasi Load Cell HX711
  Serial.println("Inisialisasi Load Cell...");
  loadcell.begin();
  loadcell.start(2000);
  loadcell.setCalFactor(1034.61);  // Faktor kalibrasi (sesuaikan jika perlu)
  Serial.println("Load Cell siap.\n");

  // Konfigurasi pin TCS3200 Color Sensor
  Serial.println("Inisialisasi TCS3200 Color Sensor...");
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  // Set frekuensi output TCS3200 ke 20%
  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);
  Serial.println("TCS3200 siap.\n");

  // Inisialisasi IR Sensor
  Serial.println("Inisialisasi IR Sensor...");
  pinMode(IR_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR2_PIN), isr, FALLING);
  Serial.println("IR Sensor siap.\n");

  // Koneksi ke Blynk
  Serial.println("Menghubungkan ke Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Terhubung ke Blynk!\n");

  // Setup timer untuk update Blynk setiap 1 detik
  timer.setInterval(1000L, updateBlynk);

  Serial.println("========================================");
  Serial.println("  SISTEM SIAP DIGUNAKAN!");
  Serial.println("========================================\n");
}

/* ========================================================================
 * FUNGSI MEMBACA WARNA
 * Membaca nilai RGB dari sensor TCS3200 dan mapping ke rentang 0-255
 * ======================================================================== */
void readColor() {
  // Baca komponen warna MERAH
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  redPW = pulseIn(sensorOut, LOW);
  reds = constrain(map(redPW, redMin, redMax, 255, 0), 0, 255);

  // Baca komponen warna HIJAU
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  greenPW = pulseIn(sensorOut, LOW);
  greens = constrain(map(greenPW, greenMin, greenMax, 255, 0), 0, 255);

  // Baca komponen warna BIRU
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  bluePW = pulseIn(sensorOut, LOW);
  blues = constrain(map(bluePW, blueMin, blueMax, 255, 0), 0, 255);

  // Tampilkan nilai RGB ke Serial Monitor
  Serial.print("RGB: [");
  Serial.print(reds);
  Serial.print(", ");
  Serial.print(greens);
  Serial.print(", ");
  Serial.print(blues);
  Serial.println("]");
}

/* ========================================================================
 * FUNGSI DETEKSI UANG KERTAS
 * Mengidentifikasi nominal uang kertas berdasarkan warna RGB
 * 
 * Parameter:
 *   r - Nilai merah (0-255)
 *   g - Nilai hijau (0-255)
 *   b - Nilai biru (0-255)
 * 
 * Return:
 *   Nominal uang (100000, 50000, 20000) atau 0 jika tidak dikenali
 * ======================================================================== */
int detectUang(int r, int g, int b) {
  // Deteksi uang Rp 100.000 (dominan MERAH)
  if (r > g && r > b && r > 170 && g > 155 && b > 155) {
    return 100000;
  }
  
  // Deteksi uang Rp 50.000 (dominan BIRU)
  if (b > r && b > g && r > 150 && g > 170 && b > 175) {
    return 50000;
  }
  
  // Deteksi uang Rp 20.000 (dominan HIJAU)
  if (g > r && g > b && r > 150 && g > 170 && b > 155) {
    return 20000;
  }
  
  // Tidak dikenali
  return 0;
}

/* ========================================================================
 * FUNGSI LOOP UTAMA
 * ======================================================================== */
void loop() {
  // Jalankan Blynk dan timer
  Blynk.run();
  timer.run();
  
  // Update data dari load cell
  loadcell.update();

  // ======================================================================
  // PROSES INTERRUPT RESET
  // ======================================================================
  if (isrTriggered) {
    isrTriggered = false;
    uangTotal = 0;
    
    // Update Blynk
    Blynk.virtualWrite(V1, "Reset");
    Blynk.virtualWrite(V0, uangTotal);
    
    Serial.println("\n*** RESET TOTAL UANG ***");
    Serial.println("Total uang: Rp 0\n");
  }

  // ======================================================================
  // PROSES DETEKSI KOIN MENGGUNAKAN LOAD CELL
  // ======================================================================
  float berat = loadcell.getData();

  // Debounce untuk mencegah pembacaan berulang
  if (millis() - lastKoinTime > debounceKoinTime) {
    
    // Deteksi koin baru berdasarkan berat
    if (!koinTerdeteksi && berat > 3) {
      koinTerdeteksi = true;
      
      // Identifikasi nominal koin berdasarkan berat
      if (berat >= 3.25 && berat <= 5.5) {
        // Koin Rp 1000 (berat: 3.25 - 5.5 gram)
        uangTotal += 1000;
        Serial.println("\n[KOIN] Rp 1.000 terdeteksi");
        Serial.print("Berat: ");
        Serial.print(berat, 2);
        Serial.println(" gram");
        
      } else if (berat >= 2 && berat < 3.2) {
        // Koin Rp 500 (berat: 2 - 3.2 gram)
        uangTotal += 500;
        Serial.println("\n[KOIN] Rp 500 terdeteksi");
        Serial.print("Berat: ");
        Serial.print(berat, 2);
        Serial.println(" gram");
        
      } else {
        Serial.println("\n[KOIN] Tidak dikenali");
        Serial.print("Berat: ");
        Serial.print(berat, 2);
        Serial.println(" gram");
      }

      Serial.print("Total uang: Rp ");
      Serial.println(uangTotal);
      Serial.println();
      
      lastKoinTime = millis();
    }

    // Deteksi koin diangkat
    if (koinTerdeteksi && berat < 1) {
      koinTerdeteksi = false;
      Serial.println("[INFO] Koin telah diangkat.\n");
    }
  }
  currentIRState = digitalRead(IR_SENSOR_PIN);

  // ======================================================================
  // PROSES DETEKSI UANG KERTAS MENGGUNAKAN SENSOR WARNA
  // ======================================================================
  // Trigger deteksi saat IR sensor mendeteksi uang kertas (FALLING edge)
  if (currentIRState == LOW && currentIRState != prevIRState) {
    Serial.println("\n[KERTAS] Uang kertas terdeteksi!");
    Serial.println("Membaca warna...");
    
    // Baca nilai warna
    readColor();
    
    // Identifikasi nominal berdasarkan warna
    int nilaiUang = detectUang(reds, greens, blues);

    if (nilaiUang > 0) {
      uangTotal += nilaiUang;
      Serial.print("[KERTAS] Nominal: Rp ");
      Serial.println(nilaiUang);
    } else {
      Serial.println("[KERTAS] Tidak dikenali");
    }
    
    Serial.print("Total uang: Rp ");
    Serial.println(uangTotal);
    Serial.println();
    
    delay(500); // Debounce
  }
  
  prevIRState = currentIRState;
}