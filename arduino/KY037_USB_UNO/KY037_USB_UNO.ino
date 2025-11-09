// arduino/KY037_USB_UNO.ino — bucket correto + refratário
// Envia JSON a cada 500 ms com eventos e pico do PRÓXIMO MEIO SEGUNDO

const int MIC_PIN = A0;                 // KY-037 saída analógica no A0
const int THRESHOLD = 100;              // ajuste conforme ruído (0..1023)
const int HYST = 8;                    // histerese (saída do estado acima)
const unsigned long SEND_EVERY_MS = 250;     // tamanho do bucket (1 s)
const unsigned long REFRACT_MS    = 90;       // ignora eco por ~90 ms
const unsigned long WINDOW_MS_TEST = 10000UL; // janela de teste (debug)

unsigned long lastSend = 0;
unsigned long winStart = 0;

int eventsTotal = 0;    // total acumulado (debug)
int eventsBucket = 0;   // eventos NESTE bucket
int peakBucket = 0;     // pico NESTE bucket

bool above = false;
unsigned long lastEventMs = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("KY037 UNO (bucket + refratario) @115200"));
  winStart = lastSend = millis();
}

void loop() {
  int val = analogRead(MIC_PIN); // 0..1023

  // pico do bucket
  if (val > peakBucket) peakBucket = val;

  // detecção com histerese + período refratário
  unsigned long now = millis();
  bool canTrigger = (now - lastEventMs) >= REFRACT_MS;

  if (!above && val >= THRESHOLD + HYST && canTrigger) {
    above = true;
    eventsTotal++;     // acumulado (debug)
    eventsBucket++;    // conta apenas neste bucket
    lastEventMs = now;
  } else if (above && val <= THRESHOLD - HYST) {
    above = false;
  }

  // debug a cada 500 ms
  static unsigned long lastDbg = 0;
  if (now - lastDbg >= 300) {
    lastDbg = now;
    Serial.print(F("DBG val="));  Serial.print(val);
    Serial.print(F(" peakB="));   Serial.print(peakBucket);
    Serial.print(F(" evB="));     Serial.print(eventsBucket);
    Serial.print(F(" evT="));     Serial.println(eventsTotal);
  }

  // envia JSON por bucket e ZERA contadores do bucket
  if (now - lastSend >= SEND_EVERY_MS) {
    unsigned long dt = (lastSend == 0) ? 0 : (now - lastSend);
    lastSend = now;

    int intensityPct = map(peakBucket, 0, 1023, 0, 100);
    if (intensityPct >= 91){
      Serial.print(F("{\"events\":"));
      Serial.print(eventsTotal);                 // << SOMENTE os eventos do bucket
      Serial.print(F(",\"duration_ms\":"));
      Serial.print(dt);
      Serial.print(F(",\"peak_intensity\":"));
      Serial.print(intensityPct);                 // << pico do bucket
      Serial.println(F("}"));
    }
    // zera para o próximo bucket
    eventsBucket = 0;
    peakBucket   = 0;
  }

  // opcional: zera janela de teste a cada 10 s (apenas debug)
  if (now - winStart >= WINDOW_MS_TEST) {
    winStart = now;
    eventsTotal = 0;
    above = false;
  }

  delay(1);
}
