#define BLYNK_PRINT Serial 

// 1. IDENTITAS BLYNK
#define BLYNK_TEMPLATE_ID "TMPL6pmeO7rfj"
#define BLYNK_TEMPLATE_NAME "Aeroponik Project"
#define BLYNK_AUTH_TOKEN "MgTZsvrHgmhzyYK3MPm2M5hcVnAaQj2e" 

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"

// 2. KONEKSI WIFI 
char ssid[] = "EOM";     
char pass[] = "ACAITNTREDRED"; 

#define DHTPIN 4      
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);

#define PIN_KIRIM_PERINTAH 5 

BlynkTimer timer;

// VARIABEL PENYIMPAN DATA
float suhuSekarang = 0.0;
float lembapSekarang = 0.0;
bool modeManual = false; 

// VARIABEL SIKLUS OTOMATIS
unsigned long waktuMulaiSiklus = 0;
unsigned long durasiNyala = 5000;   
unsigned long jedaMati = 300000;    
bool pompaOtomatisNyala = false;
bool baruNyala = true; // Penanda sistem baru pertama kali dihidupkan

// 3. FUNGSI BACA SENSOR & TAMPILAN (Tiap 2 Detik)
void kirimDataSensor() {
  lembapSekarang = dht.readHumidity();
  suhuSekarang = dht.readTemperature();

  if (!isnan(suhuSekarang)) {
    Blynk.virtualWrite(V0, suhuSekarang); 
    Blynk.virtualWrite(V1, lembapSekarang); 

    Serial.print("📊 [SENSOR] Suhu: ");
    Serial.print(suhuSekarang);
    Serial.print("°C | Lembap: ");
    Serial.print(lembapSekarang);
    Serial.println("%");
  } else {
    Serial.println("⚠️ [ERROR] Gagal membaca sensor DHT22!");
  }
}

// 4. MANUAL OVERRIDE (TOMBOL BLYNK)
BLYNK_WRITE(V2) {
  int statusTombol = param.asInt(); 
  
  if (statusTombol == 1) {
    modeManual = true; 
    digitalWrite(PIN_KIRIM_PERINTAH, HIGH); 
    Serial.println("\n=====================================");
    Serial.println("👆 [MANUAL] Mode Aktif! Pompa NYALA.");
    Serial.println("=====================================\n");
  } 
  else {
    modeManual = false; 
    digitalWrite(PIN_KIRIM_PERINTAH, LOW);  
    pompaOtomatisNyala = false; 
    waktuMulaiSiklus = millis(); 
    Serial.println("\n=====================================");
    Serial.println("👆 [MANUAL] Berhenti. Kembali ke OTOMATIS.");
    Serial.println("=====================================\n");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nMemulai Sistem Aeroponik Adaptif...");
  
  dht.begin();
  
  pinMode(PIN_KIRIM_PERINTAH, OUTPUT);
  digitalWrite(PIN_KIRIM_PERINTAH, LOW); 
  Blynk.virtualWrite(V2, 0); 
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(2000L, kirimDataSensor);
  
  // Catat waktu pertama kali alat menyala
  waktuMulaiSiklus = millis();
}

void loop() {
  Blynk.run();
  timer.run();

  if (modeManual == false && !isnan(suhuSekarang)) {
    unsigned long waktuSekarang = millis();

    // --- FASE 1: POMPA MATI (MENUNGGU JEDA) ---
    if (pompaOtomatisNyala == false) {
      
      // Info khusus saat alat baru pertama kali dicolok listrik
      if (baruNyala) {
         if (suhuSekarang > 30.0) jedaMati = 180000; else jedaMati = 300000;
         Serial.print("⏳ Sistem baru menyala. Menunggu fase jeda awal selama ");
         Serial.print(jedaMati / 1000);
         Serial.println(" detik sebelum semprotan pertama...");
         baruNyala = false; 
      }

      // Cek apakah waktu jeda sudah habis?
      if (waktuSekarang - waktuMulaiSiklus >= jedaMati) {
        
        Serial.println("\n-------------------------------------");
        // Penentuan Aturan Misting
        if (suhuSekarang > 30.0) {
          durasiNyala = 10000;  // 10 detik
          jedaMati = 180000;    // 3 menit
          Serial.print("🔥 [MISTING] Suhu PANAS (");
        } else {
          durasiNyala = 5000;   // 5 detik
          jedaMati = 300000;    // 5 menit
          Serial.print("❄️ [MISTING] Suhu NORMAL (");
        }
        
        Serial.print(suhuSekarang);
        Serial.print("°C) -> POMPA NYALA selama ");
        Serial.print(durasiNyala / 1000);
        Serial.println(" detik.");
        Serial.println("-------------------------------------");

        // Eksekusi Pompa Nyala
        digitalWrite(PIN_KIRIM_PERINTAH, HIGH); 
        Blynk.virtualWrite(V2, 1); 
        
        pompaOtomatisNyala = true;
        waktuMulaiSiklus = waktuSekarang; 
      }
    } 
    
    // --- FASE 2: POMPA NYALA (SEDANG MENYEMPROT) ---
    else {
      // Cek apakah durasi nyemprot sudah habis?
      if (waktuSekarang - waktuMulaiSiklus >= durasiNyala) {
        
        // Eksekusi Pompa Mati
        digitalWrite(PIN_KIRIM_PERINTAH, LOW); 
        Blynk.virtualWrite(V2, 0); 
        
        pompaOtomatisNyala = false;
        waktuMulaiSiklus = waktuSekarang; 
        
        Serial.println("\n-------------------------------------");
        Serial.print("✅ [MISTING SELESAI] Pompa MATI. Masuk Fase ISTIRAHAT selama ");
        Serial.print(jedaMati / 1000); 
        Serial.println(" detik.");
        Serial.println("-------------------------------------\n");
      }
    }
  }
}