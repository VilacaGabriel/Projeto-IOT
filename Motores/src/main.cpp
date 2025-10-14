#include <Arduino.h>
#include <AccelStepper.h>

// ==== MOTOR 1 ====
#define IN1_M1  16
#define IN2_M1  17
#define IN3_M1  18
#define IN4_M1  19

// ==== MOTOR 2 ====
#define IN1_M2  21
#define IN2_M2  22
#define IN3_M2  23
#define IN4_M2  25

// Tipo de driver: 4 pinos
AccelStepper motor1(AccelStepper::FULL4WIRE, IN1_M1, IN3_M1, IN2_M1, IN4_M1);
AccelStepper motor2(AccelStepper::FULL4WIRE, IN1_M2, IN3_M2, IN2_M2, IN4_M2);

bool motor1Ligado = false;
bool motor2Ligado = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Controle de motores iniciado!");
  Serial.println("Comandos:");
  Serial.println("  M1 ON  -> ligar motor 1");
  Serial.println("  M1 OFF -> parar motor 1");
  Serial.println("  M2 ON  -> ligar motor 2");
  Serial.println("  M2 OFF -> parar motor 2");

  motor1.setMaxSpeed(1000);
  motor1.setAcceleration(300);

  motor2.setMaxSpeed(1000);
  motor2.setAcceleration(300);
}

void loop() {
  // Verifica se há comando recebido pela serial
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("M1 ON")) {
      motor1Ligado = true;
      Serial.println("Motor 1 ligado (sentido horário)");
    } else if (cmd.equalsIgnoreCase("M1 OFF")) {
      motor1Ligado = false;
      Serial.println("Motor 1 parado");
    } else if (cmd.equalsIgnoreCase("M2 ON")) {
      motor2Ligado = true;
      Serial.println("Motor 2 ligado (sentido horário)");
    } else if (cmd.equalsIgnoreCase("M2 OFF")) {
      motor2Ligado = false;
      Serial.println("Motor 2 parado");
    }
  }

  // Faz os motores girarem continuamente se estiverem ligados
  if (motor1Ligado) {
    motor1.setSpeed(500);  // Velocidade positiva = horário
    motor1.runSpeed();
  }

  if (motor2Ligado) {
    motor2.setSpeed(500);
    motor2.runSpeed();
  }
}
