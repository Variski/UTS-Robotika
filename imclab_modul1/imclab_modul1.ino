/*
  Modul 1 - Pemrograman Basic iMCLab (Arduino-ESP32 core 3.x)
  Kontrol kecepatan (PWM) dan arah motor DC, baca RPM (jika sensor diisi), LED indikator.

  Perintah Serial Monitor (115200 baud, line ending Newline):
    VER        -> versi firmware
    PWM 60     -> kecepatan 0-100 persen
    DIR 1      -> arah 1 = maju, 0 = mundur
    RPM        -> kecepatan putar (butuh PIN_ENC diisi)
    LED 1      -> LED biru onboard nyala (1) atau mati (0)
    X          -> stop motor
*/

// ===== Pin motor (terkonfirmasi dari tes motor) =====
const int PIN_EN  = 12;   // enable / PWM kecepatan
const int PIN_IN1 = 27;   // arah 1
const int PIN_IN2 = 26;   // arah 2
const int PIN_LED = 2;    // LED biru onboard ESP32

// ===== Isi dari hasil motor_test (pin KANDIDAT sensor putaran), -1 = belum ada =====
const int   PIN_ENC = 13;
const float PULSES_PER_REV = 1.0;   // jumlah pulsa per putaran cakram

const int PWM_FREQ = 30000;
const int PWM_RES  = 8;             // 0-255

volatile unsigned long pulses = 0;
void IRAM_ATTR onPulse() { pulses++; }

int pwmPct = 0;
int dirNow = 1;
float rpm = 0;
unsigned long lastCalc = 0;

void applyMotor() {
  // dir 1 = maju: IN1 LOW, IN2 HIGH (sama dengan contoh kode modul)
  digitalWrite(PIN_IN1, dirNow ? LOW : HIGH);
  digitalWrite(PIN_IN2, dirNow ? HIGH : LOW);
  ledcWrite(PIN_EN, pwmPct * 255 / 100);
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  ledcAttach(PIN_EN, PWM_FREQ, PWM_RES);   // sintaks core 3.x

  if (PIN_ENC >= 0) {
    pinMode(PIN_ENC, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_ENC), onPulse, RISING);
  } else {
    Serial.println("PIN_ENC belum diisi: perintah RPM akan menampilkan 0.");
  }

  applyMotor();
  lastCalc = millis();
  Serial.println("iMCLab Basic siap. Contoh: PWM 60, DIR 0, RPM, X");
}

void loop() {
  unsigned long now = millis();
  if (PIN_ENC >= 0 && now - lastCalc >= 500) {
    noInterrupts();
    unsigned long p = pulses;
    pulses = 0;
    interrupts();
    rpm = (p / PULSES_PER_REV) * 60000.0 / (now - lastCalc);
    lastCalc = now;
  }

  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    line.toUpperCase();

    if (line == "VER") {
      Serial.println("iMCLab Basic 1.0");
    } else if (line == "RPM") {
      Serial.println(rpm, 1);
    } else if (line == "X") {
      pwmPct = 0;
      applyMotor();
      Serial.println("Stop");
    } else if (line.startsWith("PWM ")) {
      pwmPct = constrain(line.substring(4).toInt(), 0, 100);
      applyMotor();
      Serial.println(pwmPct);
    } else if (line.startsWith("DIR ")) {
      dirNow = line.substring(4).toInt() ? 1 : 0;
      applyMotor();
      Serial.println(dirNow);
    } else if (line.startsWith("LED ")) {
      int v = line.substring(4).toInt();
      digitalWrite(PIN_LED, v ? HIGH : LOW);
      Serial.println(v ? 1 : 0);
    } else if (line.length() > 0) {
      Serial.println("Perintah tidak dikenal");
    }
  }
}
