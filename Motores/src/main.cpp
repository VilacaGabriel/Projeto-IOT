#include <Arduino.h>
#include <AccelStepper.h>

// ==== MOTOR 1 ====
#define IN1_M1  16
#define IN2_M1  17
#define IN3_M1  18
#define IN4_M1  19

AccelStepper motor1(AccelStepper::FULL4WIRE, IN1_M1, IN3_M1, IN2_M1, IN4_M1);

const long stepsPerRevolution = 2048; // ajuste se necessário
const int testMaxSpeed = 1000;        // velocidade alvo (passos/s) durante os testes
const unsigned long noProgressTimeout = 700; // ms sem progresso => assume travamento
const unsigned long pauseBetweenTests = 2000; // ms entre testes

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(1); }
  Serial.println();
  Serial.println("=== TESTE DE ACELERACAO ===");
  Serial.print("Velocidade alvo (maxSpeed): ");
  Serial.print(testMaxSpeed);
  Serial.println(" passos/s");
  Serial.println();

  motor1.setMaxSpeed(testMaxSpeed);

  // Loop de testes de aceleração: aumenta e testa até encontrar limite
  for (int accel = 100; accel <= 4000; accel += 100) {
    Serial.println("----------------------------------------");
    Serial.print("Testando aceleração = ");
    Serial.print(accel);
    Serial.println(" passos/s^2");

    motor1.setAcceleration(accel);
    motor1.moveTo(motor1.currentPosition() + stepsPerRevolution);

    unsigned long startTime = millis();
    unsigned long lastProgressTime = startTime;
    long lastDistance = motor1.distanceToGo();
    bool stalled = false;

    // Executa movimento até completar ou detectar travamento
    while (motor1.distanceToGo() != 0) {
      motor1.run();

      // Detecta mudança na distância restante
      long dist = motor1.distanceToGo();
      if (dist != lastDistance) {
        lastDistance = dist;
        lastProgressTime = millis();
      } else {
        // sem progresso
        if (millis() - lastProgressTime > noProgressTimeout) {
          stalled = true;
          Serial.println("⚠️  Sem progresso detectado (possível travamento/pulo de passos). Abortando este teste.");
          // Para o motor de forma segura
          motor1.stop();
          motor1.runToPosition(); // força atualização do posicionamento interno
          break;
        }
      }
    }

    unsigned long elapsed = millis() - startTime;

    if (!stalled) {
      Serial.print("✅ Concluído em ");
      Serial.print(elapsed / 1000.0, 3);
      Serial.println(" s");
      Serial.println("Sem travamento detectado.");
    } else {
      Serial.print("⛔ Teste falhou em aceleração = ");
      Serial.print(accel);
      Serial.println(" passos/s^2");
    }

    Serial.println("Aguardando antes do próximo teste...");
    delay(pauseBetweenTests);
  }

  Serial.println();
  Serial.println("🏁 Todos os testes de aceleração foram concluídos.");
  Serial.println("Recomendações: escolha a maior aceleração que não travou repetidamente.");
}

void loop() {
  // Nenhuma ação contínua necessária — tudo é testado no setup()
}
