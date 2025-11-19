# camera_stream.py
import cv2
import numpy as np
from config import ESP32_CAM_URL

def get_camera():
    """Abre a conexão com a câmera ESP32-CAM."""
    return cv2.VideoCapture(ESP32_CAM_URL)

# Criador de trackers MOSSE (compatível com OpenCV normal e legacy)
def create_tracker():
    try:
        return cv2.legacy.TrackerMOSSE_create()
    except:
        return cv2.TrackerMOSSE_create()

def generate_frames():
    """Gera frames com detecção de anomalias (riscos/manchas) e envia ao navegador."""
    camera = get_camera()

    # === Captura do primeiro frame para referência ===
    ret, prev_frame = camera.read()
    while not ret or prev_frame is None:
        print("⚠️ Falha inicial na câmera. Reconectando...")
        camera.release()
        camera = get_camera()
        ret, prev_frame = camera.read()

    prev_gray = cv2.cvtColor(prev_frame, cv2.COLOR_BGR2GRAY)
    prev_gray = cv2.GaussianBlur(prev_gray, (7, 7), 0)

    trackers = []

    while True:
        ret, frame = camera.read()

        # === Reconexão automática se o stream cair ===
        if not ret or frame is None:
            print("⚠️ Falha na leitura da câmera. Tentando reconectar...")
            camera.release()
            camera = get_camera()
            continue

        # ===============================
        # PROCESSAMENTO DO FRAME
        # ===============================
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(gray, (7, 7), 0)

        # -------------------------------
        # [1] DETECÇÃO DE ANOMALIAS
        # -------------------------------
        diff = cv2.absdiff(prev_gray, gray)
        blur = cv2.GaussianBlur(diff, (5, 5), 0)
        _, thresh = cv2.threshold(blur, 25, 255, cv2.THRESH_BINARY)

        # limpar ruído mas manter contorno fiel
        thresh = cv2.erode(thresh, None, iterations=1)
        thresh = cv2.dilate(thresh, None, iterations=1)

        contours, _ = cv2.findContours(
            thresh, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE
        )

        new_boxes = []
        for c in contours:
            area = cv2.contourArea(c)
            if area < 20:  # detectar riscos/manchas pequenas
                continue

            x, y, w, h = cv2.boundingRect(c)

            # Encolher box para ficar justo
            pad = 2
            x = max(0, x + pad)
            y = max(0, y + pad)
            w = max(1, w - 2 * pad)
            h = max(1, h - 2 * pad)

            # MOSSE precisa de janela mínima
            MIN_SIZE = 10
            if w < MIN_SIZE: w = MIN_SIZE
            if h < MIN_SIZE: h = MIN_SIZE

            new_boxes.append((x, y, w, h))

        # Criar trackers para novas manchas
        for (x, y, w, h) in new_boxes:
            tracker = create_tracker()
            try:
                tracker.init(frame, (x, y, w, h))
                trackers.append(tracker)
            except:
                print("⚠️ Tracker ignorado: bounding box muito pequeno")

        # -------------------------------
        # [2] ATUALIZAR TRACKERS EXISTENTES
        # -------------------------------
        updated_trackers = []
        for trk in trackers:
            ok, box = trk.update(frame)
            if ok:
                updated_trackers.append(trk)
                (x, y, w, h) = [int(v) for v in box]

                cv2.rectangle(frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
                cv2.putText(frame, "Anomalia", (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 0, 0), 2)

        trackers = updated_trackers

        # -------------------------------
        # [3] ENVIAR FRAME PARA O NAVEGADOR
        # -------------------------------
        _, buffer = cv2.imencode('.jpg', frame)
        frame_jpg = buffer.tobytes()

        yield (
            b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n' + frame_jpg + b'\r\n'
        )

        # atualizar frame anterior para a próxima detecção
        prev_gray = gray
