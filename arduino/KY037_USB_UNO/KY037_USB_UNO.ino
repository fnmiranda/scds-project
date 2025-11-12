// arduino/KY037_USB_UNO.ino — bucket correto + Δt real
const int MIC_PIN = A0;
const int THRESHOLD = 100;
const int HYST = 8;
const unsigned long SEND_EVERY_MS = 100;   // tamanho do bucket
const unsigned long REFRACT_MS    = 60;    // reduza se estiver perdendo tiros

unsigned long lastSend = 0;

int eventsBucket = 0;
int peakBucket   = 0;
bool above       = false;

unsigned long lastShotMs = 0;   // instante do último tiro
unsigned long lastDtMs   = 0;   // Δt do último tiro (ms)

void setup() {
  Serial.begin(115200);
  delay(200);
  lastSend = millis();
}

void loop() {
  int val = analogRead(MIC_PIN);                 // 0..1023
  if (val > peakBucket) peakBucket = val;

  unsigned long now = millis();
  bool canTrigger = (now - lastShotMs) >= REFRACT_MS;

  // gatilho com histerese
  if (!above && val >= THRESHOLD + HYST && canTrigger) {
    above = true;
    eventsBucket++;
    if (lastShotMs != 0) lastDtMs = now - lastShotMs;  // Δt REAL entre tiros
    lastShotMs = now;
  } else if (above && val <= THRESHOLD - HYST) {
    above = false;
  }

  // envia 1 linha por bucket (sempre) e zera o bucket
  if (now - lastSend >= SEND_EVERY_MS) {
    unsigned long duration_ms = now - lastSend;
    lastSend = now;

    int intensityPct = map(peakBucket, 0, 1023, 0, 100);
    
    if(intensityPct >90){

      Serial.print(F("{\"events\":"));          Serial.print(eventsBucket);     // << bucket!
      Serial.print(F(",\"duration_ms\":"));     Serial.print(duration_ms);      // tamanho do bucket
      Serial.print(F(",\"peak_intensity\":"));  Serial.print(intensityPct);     // pico do bucket
      Serial.print(F(",\"dt_ms\":"));           Serial.print(lastDtMs);         // Δt do último tiro
      Serial.print(F(",\"t_ms\":"));            Serial.print(now);              // tempo monotônico
      Serial.println(F("}"));
    }

    eventsBucket = 0;
    peakBucket   = 0;
  }

  delay(1);
}
