# camera_stream.py
import cv2
import numpy as np
import time
import os
import shutil
from datetime import datetime
from config import ESP32_CAM_URL

# ============================================================
# ⚙ Funções utilitárias de câmera / tracker
# ============================================================
def get_camera():
    cam = cv2.VideoCapture(ESP32_CAM_URL)
    # tenta reduzir latência do buffer
    try:
        cam.set(cv2.CAP_PROP_BUFFERSIZE, 1)
    except:
        pass
    return cam

def create_tracker():
    try:
        return cv2.legacy.TrackerMOSSE_create()
    except Exception:
        return cv2.TrackerMOSSE_create()

# ============================================================
# 🎚 Parâmetros de sensibilidade (ajuste aqui)
# ============================================================
SENSIBILIDADE = 0.5
SCALE = max(0.0, min(1.0, SENSIBILIDADE))

MIN_CONTOUR_AREA = max(3, int(15 - 10 * SCALE))   # min área do contorno
MIN_BOX_SIZE = max(4, int(12 - 7 * SCALE))       # min largura/altura

# MOG2 params
MOG2_HISTORY = int(250 - 200 * SCALE)
MOG2_VAR_THRESHOLD = int(18 - 12 * SCALE)

ACCUMULATE_WEIGHT = 0.01 + SCALE * 0.03
THRESH_MOG2 = int(230 - 100 * SCALE)
THRESH_ACCUM = int(30 - 20 * SCALE)
THRESH_LAPLACE = int(40 - 25 * SCALE)

IOU_DUPLICATE_THRESHOLD = 0.35
MIN_MOVIMENTO_PX = 3

# ============================================================
# Estado / caches
# ============================================================
ultimo_centro = {}       # id_obj -> (cx, cy)
contador_frames = {}     # id_obj -> contagem
ultimo_tamanho = {}      # id_obj -> (w,h)
ultimo_salvamento = {}   # id_obj -> timestamp

# ============================================================
# PATHS (salva em static para front)
# ============================================================
SAVE_DIR = os.path.join("static", "anomalias")
CLASSIFICADOS_DIR = os.path.join(SAVE_DIR, "classificados")
NAO_CLASSIFICADO_DIR = os.path.join(SAVE_DIR, "nao_classificado")

os.makedirs(SAVE_DIR, exist_ok=True)
os.makedirs(CLASSIFICADOS_DIR, exist_ok=True)
os.makedirs(NAO_CLASSIFICADO_DIR, exist_ok=True)

# ============================================================
# Funções utilitárias
# ============================================================
def iou(boxA, boxB):
    xA = max(boxA[0], boxB[0])
    yA = max(boxA[1], boxB[1])
    xB = min(boxA[0] + boxA[2], boxB[0] + boxB[2])
    yB = min(boxA[1] + boxA[3], boxB[1] + boxB[3])

    interW = max(0, xB - xA)
    interH = max(0, yB - yA)
    interArea = interW * interH

    areaA = boxA[2] * boxA[3]
    areaB = boxB[2] * boxB[3]
    union = areaA + areaB - interArea
    if union <= 0:
        return 0.0
    return interArea / union

def gerar_nome_simples(pasta, extensao=".jpg"):
    """Gera anomaliaN.jpg incremental na pasta informada."""
    os.makedirs(pasta, exist_ok=True)
    maior = 0
    for nome in os.listdir(pasta):
        if nome.startswith("anomalia") and nome.endswith(extensao):
            corpo = nome[len("anomalia"):-len(extensao)]
            try:
                n = int(corpo)
                if n > maior:
                    maior = n
            except:
                pass
    return f"anomalia{maior + 1}{extensao}"

# ============================================================
# Salva imagem completa com classificação e nome simples
# ============================================================
def salvar_anomalia(frame, id_obj):
    agora = time.time()
    # evita spam por tracker (2s)
    if id_obj in ultimo_salvamento and agora - ultimo_salvamento[id_obj] < 2.0:
        return
    ultimo_salvamento[id_obj] = agora

    # salva temporário (único por id_obj)
    nome_temp = f"temp_anomalia_{id_obj}_{int(agora*1000)}.jpg"
    caminho_temp = os.path.join(SAVE_DIR, nome_temp)
    try:
        ok = cv2.imwrite(caminho_temp, frame)
        if not ok:
            print("❌ cv2.imwrite retornou False:", caminho_temp)
            return
    except Exception as e:
        print("❌ Erro escrevendo temp:", e)
        return

    # chama classificador dinamicamente (protegido)
    try:
        from classificador import classificar_imagem
        categoria = classificar_imagem(caminho_temp)
    except Exception as e:
        print("❌ Erro no classificador:", e)
        categoria = None

    if not categoria or not isinstance(categoria, str):
        # se classificador falhar, coloca em não_classificado
        categoria = None

    # se categoria válida -> move para classificados/<categoria>
    if categoria:
        destino_dir = os.path.join(CLASSIFICADOS_DIR, categoria)
        os.makedirs(destino_dir, exist_ok=True)
    else:
        destino_dir = NAO_CLASSIFICADO_DIR
        os.makedirs(destino_dir, exist_ok=True)

    # gera nome simples definitivo
    filename = gerar_nome_simples(destino_dir, ".jpg")
    caminho_final = os.path.join(destino_dir, filename)

    try:
        shutil.move(caminho_temp, caminho_final)
        print(f"📸 Salvo: {caminho_final}  (categoria: {categoria or 'nao_classificado'})")
    except Exception as e:
        print("❌ Erro movendo arquivo:", e)
        # tenta rename fallback
        try:
            os.rename(caminho_temp, caminho_final)
            print("✅ fallback: rename ok")
        except Exception as e2:
            print("❌ Erro no fallback rename:", e2)
            # tenta remover temp para não acumular lixo
            try:
                os.remove(caminho_temp)
            except:
                pass

# ============================================================
# Detecta se o objeto se moveu (retorna boolean, dx)
# ============================================================
def objeto_se_moveu(id_objeto, caixa):
    global ultimo_centro
    x, y, w, h = caixa
    cx = x + w // 2
    cy = y + h // 2
    if id_objeto not in ultimo_centro:
        ultimo_centro[id_objeto] = (cx, cy)
        return False, 0
    cx_prev, cy_prev = ultimo_centro[id_objeto]
    dx = cx - cx_prev
    dy = cy - cy_prev
    ultimo_centro[id_objeto] = (cx, cy)
    dist = np.hypot(dx, dy)
    return dist >= MIN_MOVIMENTO_PX, dx

# ============================================================
# 🔵 DETECTOR DE FAIXA VERTICAL CENTRAL
# ============================================================
capturou_faixa = {}  # id_obj -> bool

def tocou_area_vertical(frame_width, bbox, largura_area=30):
    """
    Retorna True se QUALQUER parte do bounding box tocar a faixa
    vertical central da imagem.
    """
    x, y, w, h = bbox

    x1 = x
    x2 = x + w

    centro = frame_width // 2
    faixa_esq  = centro - largura_area // 2
    faixa_dir  = centro + largura_area // 2

    # Colisão com a faixa?
    return not (x2 < faixa_esq or x1 > faixa_dir)
# ============================================================
# LOOP principal (gera frames para a rota /video)
# ============================================================
def generate_frames():
    camera = get_camera()

    bg = cv2.createBackgroundSubtractorMOG2(
        history=max(60, MOG2_HISTORY),
        varThreshold=max(8, MOG2_VAR_THRESHOLD),
        detectShadows=True
    )

    accumulated = None
    trackers = []  # lista de (tracker, last_box)

    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))

    while True:
        ret, frame = camera.read()
        if not ret or frame is None:
            # tenta reconectar
            print("⚠️ get_camera() retornou False — tentando reconectar...")
            try:
                camera.release()
            except:
                pass
            time.sleep(0.5)
            camera = get_camera()
            continue

        # processamento leve (gray + blur)
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        small = cv2.GaussianBlur(gray, (3, 3), 0)

        # MOG2
        mog_mask = bg.apply(small)
        _, mog_mask = cv2.threshold(mog_mask, max(100, THRESH_MOG2 - 80), 255, cv2.THRESH_BINARY)

        # Laplaciano (borda)
        lap = cv2.Laplacian(small, cv2.CV_8U)
        _, lap = cv2.threshold(lap, max(10, THRESH_LAPLACE - 25), 255, cv2.THRESH_BINARY)

        # Acúmulo
        if accumulated is None:
            accumulated = small.astype("float")
        cv2.accumulateWeighted(small, accumulated, max(0.01, ACCUMULATE_WEIGHT))
        diff_accum = cv2.absdiff(small, cv2.convertScaleAbs(accumulated))
        _, diff_accum = cv2.threshold(diff_accum, max(8, THRESH_ACCUM - 20), 255, cv2.THRESH_BINARY)

        # Combina máscaras
        combined = cv2.bitwise_or(mog_mask, lap)
        combined = cv2.bitwise_or(combined, diff_accum)
        combined = cv2.morphologyEx(combined, cv2.MORPH_CLOSE, kernel, iterations=1)

        # Contornos
        contours, _ = cv2.findContours(combined, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        # Novas caixas candidatas
        new_boxes = []
        for c in contours:
            area = cv2.contourArea(c)
            if area < MIN_CONTOUR_AREA:
                continue
            x, y, w, h = cv2.boundingRect(c)
            if w < MIN_BOX_SIZE or h < MIN_BOX_SIZE:
                continue
            new_boxes.append((x, y, w, h))

        # Evita duplicados usando IOU contra trackers existentes
        existing_boxes = [t[1] for t in trackers]
        for box in new_boxes:
            if any(iou(box, eb) > IOU_DUPLICATE_THRESHOLD for eb in existing_boxes):
                continue
            trk = create_tracker()
            try:
                trk.init(frame, box)
                trackers.append((trk, box))
            except Exception as e:
                # falha ao iniciar tracker: ignora
                # print("⚠️ falha init tracker:", e)
                continue

        # Atualiza trackers — cria nova lista de trackers ativos
        updated = []
        # track_ids usados — para limpar contadores de trackers mortos
        usados_ids = set()

        for trk, last_box in trackers:
            try:
                ok, box = trk.update(frame)
            except Exception:
                # se tracker falhar, ignora esse tracker
                continue  

            if not ok:
                continue

            x, y, w, h = map(int, box)
            obj_id = id(trk)
            usados_ids.add(obj_id)

            # filtro de estabilidade de tamanho (reduz ruído)
            if obj_id in ultimo_tamanho:
                lw, lh = ultimo_tamanho[obj_id]
                if abs(w - lw) > 50 or abs(h - lh) > 50:
                    # possível ruído grande — ignora esse tracker por agora
                    continue
            ultimo_tamanho[obj_id] = (w, h)

            # detecta movimento real
            moveu, dx = objeto_se_moveu(obj_id, (x, y, w, h))
            if not moveu:
                continue

            # incrementa contador de frames (pode ser usado para estabilidade)
            contador_frames[obj_id] = contador_frames.get(obj_id, 0) + 1

            # desenha no frame
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 200, 255), 2)
            cv2.putText(frame, "Anomalia", (x, y - 6), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 200, 255), 1)

            # SALVA (com proteção contra spam)
            # --- captura somente quando tocou a faixa central ---
            frame_h, frame_w = frame.shape[:2]

            # se a faixa não existir ainda no dicionário, inicializa
            if obj_id not in capturou_faixa:
                capturou_faixa[obj_id] = False

            # detectou colisão?
            if tocou_area_vertical(frame_w, (x, y, w, h)):
                if not capturou_faixa[obj_id]:
                    salvar_anomalia(frame, obj_id)
                    capturou_faixa[obj_id] = True  # evita repetir
            else:
                # objeto saiu da área → permite capturar de novo
                capturou_faixa[obj_id] = False

            updated.append((trk, (x, y, w, h)))

        # limpar contadores de trackers que sumiram
        # (evita crescimento de dicts)
        chaves_atuais = set(usados_ids)
        for k in list(contador_frames.keys()):
            if k not in chaves_atuais:
                contador_frames.pop(k, None)
        for k in list(ultimo_tamanho.keys()):
            if k not in chaves_atuais:
                ultimo_tamanho.pop(k, None)
        for k in list(ultimo_centro.keys()):
            if k not in chaves_atuais:
                ultimo_centro.pop(k, None)

        trackers = updated

        # encode e yield frame para /video
        try:
            _, buffer = cv2.imencode(".jpg", frame)
            yield (b"--frame\r\nContent-Type: image/jpeg\r\n\r\n" + buffer.tobytes() + b"\r\n")
        except Exception as e:
            print("❌ Erro ao encondear frame:", e)
            continue
