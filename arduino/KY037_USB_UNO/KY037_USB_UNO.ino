// KY-037 + Arduino UNO com interrupção no D0 (INT0)
// Envia 1 JSON por bucket (SEND_EVERY_MS): {"events":E,"duration_ms":T,"peak_intensity":P}

const uint8_t MIC_A_PIN = A0;     // analógico para medir pico (opcional)
const uint8_t MIC_D_PIN = 2;      // D0 do KY-037 -> INT0 (UNO)

const unsigned long SEND_EVERY_MS   = 1000;    // tamanho do bucket (telemetria)
const unsigned long REFRACT_US      = 90000UL; // 90 ms: ignora eco/rebote
const unsigned long WINDOW_MS_TEST  = 10000UL; // zera janela de teste (debug)

volatile unsigned long isrLastUs = 0;          // última borda válida
volatile uint16_t      isrBucketEvents = 0;    // eventos válidos no bucket atual

// pico do bucket (analógico)
int    peakBucket = 0;
int    peakWindow = 0;

// temporização
unsigned long lastSendMs = 0;
unsigned long winStartMs = 0;

void onMicRising() {
  // Interrupção na borda de subida do comparador (D0)
  unsigned long now = micros();
  if (now - isrLastUs >= REFRACT_US) {   // período refratário para matar ecos
    isrBucketEvents++;                   // conta 1 evento no bucket atual
    isrLastUs = now;
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);

  pinMode(MIC_D_PIN, INPUT);             // alguns módulos exigem INPUT_PULLUP
  attachInterrupt(digitalPinToInterrupt(MIC_D_PIN), onMicRising, RISING);

  lastSendMs = millis();
  winStartMs = lastSendMs;

  Serial.println(F("KY-037 UNO (interrupt) iniciado @115200"));
}

void loop() {
  // Mede o pico analógico dentro do bucket (opcional, só para intensidade)
  int v = analogRead(MIC_A_PIN);         // 0..1023
  if (v > peakBucket) peakBucket = v;
  if (v > peakWindow) peakWindow = v;

  unsigned long now = millis();

  // Envia 1 linha de JSON por bucket
  if (now - lastSendMs >= SEND_EVERY_MS) {
    lastSendMs = now;

    // Leitura atômica do contador da ISR
    noInterrupts();
    uint16_t eventsBucket = isrBucketEvents;
    //isrBucketEvents = 0;                 // zera para o próximo bucket
    interrupts();

    // Captura e zera o pico do bucket
    int peak = peakBucket;
    peakBucket = 0;

    // Normaliza pico para 0..100 (percentual aproximado)
    int intensityPct = map(peak, 0, 1023, 0, 100);

    // Emite JSON
    Serial.print(F("{\"events\":"));
    Serial.print(eventsBucket);
    Serial.print(F(",\"duration_ms\":"));
    Serial.print((int)SEND_EVERY_MS);
    Serial.print(F(",\"peak_intensity\":"));
    Serial.print(intensityPct);
    Serial.println(F("}"));
  }

  // Opcional: zera janela de teste a cada 10s
  if (now - winStartMs >= WINDOW_MS_TEST) {
    winStartMs = now;
    peakWindow = 0;
  }
}
