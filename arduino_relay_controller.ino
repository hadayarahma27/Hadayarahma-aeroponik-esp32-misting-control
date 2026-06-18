#define PIN_DARI_ESP 2
#define RELAY_PIN 8

void setup() {
  pinMode(PIN_DARI_ESP, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Kondisi awal: Pompa MATI (Active Low)
}

void loop() {
  int perintah = digitalRead(PIN_DARI_ESP);
  
  // Jika ESP32 mengirim sinyal ON (HIGH), Uno mengaktifkan Relay (LOW)
  if (perintah == HIGH) {
    digitalWrite(RELAY_PIN, LOW);  
  } else {
    digitalWrite(RELAY_PIN, HIGH); 
  }
}