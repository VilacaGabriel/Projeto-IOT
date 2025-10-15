# camera_stream.py
import cv2
import numpy as np
from config import ESP32_CAM_URL

def get_camera():
    """Abre a conexão com a câmera (ESP32-CAM)."""
    return cv2.VideoCapture(ESP32_CAM_URL)

def generate_frames():
    """Captura frames da câmera e aplica visão computacional:
       - Detecção de movimento
       - Detecção de cor vermelha
    """
    camera = get_camera()

    # Captura dois frames iniciais para detectar movimento
    ret, frame1 = camera.read()
    ret, frame2 = camera.read()

    while True:
        if not ret:
            break

        # --- [1] DETECÇÃO DE MOVIMENTO ---
        diff = cv2.absdiff(frame1, frame2)
        gray = cv2.cvtColor(diff, cv2.COLOR_BGR2GRAY)
        blur = cv2.GaussianBlur(gray, (5, 5), 0)
        _, thresh = cv2.threshold(blur, 20, 255, cv2.THRESH_BINARY)
        dilated = cv2.dilate(thresh, None, iterations=3)
        contours, _ = cv2.findContours(dilated, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

        movimento_detectado = False
        for c in contours:
            if cv2.contourArea(c) < 1000:
                continue
            movimento_detectado = True
            x, y, w, h = cv2.boundingRect(c)
            cv2.rectangle(frame1, (x, y), (x + w, y + h), (0, 255, 0), 2)

        if movimento_detectado:
            cv2.putText(frame1, "Movimento detectado", (10, 30),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # --- [2] DETECÇÃO DE COR VERMELHA ---
        hsv = cv2.cvtColor(frame1, cv2.COLOR_BGR2HSV)

        # Intervalo de vermelho (em HSV)
        lower_red1 = np.array([0, 120, 70])
        upper_red1 = np.array([10, 255, 255])
        mask1 = cv2.inRange(hsv, lower_red1, upper_red1)

        lower_red2 = np.array([170, 120, 70])
        upper_red2 = np.array([180, 255, 255])
        mask2 = cv2.inRange(hsv, lower_red2, upper_red2)

        mask = mask1 + mask2

        # Achar contornos de regiões vermelhas
        contours, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        for c in contours:
            if cv2.contourArea(c) > 500:
                x, y, w, h = cv2.boundingRect(c)
                cv2.rectangle(frame1, (x, y), (x + w, y + h), (0, 0, 255), 2)
                cv2.putText(frame1, "Vermelho detectado", (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)

        # --- [3] ENVIAR FRAME AO NAVEGADOR ---
        _, buffer = cv2.imencode('.jpg', frame1)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

        # Atualiza frames para o próximo loop
        frame1 = frame2
        ret, frame2 = camera.read()
