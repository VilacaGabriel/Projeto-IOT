import cv2
import numpy as np
import os

BASE_DIR = "static/anomalias/classificados"

GRUPOS = ["rabisco", "rasgo", "amassado", "outros"]

# ==============================================================
# EXTRAI CARACTERÍSTICAS DE UMA IMAGEM
# ==============================================================
def extrair_caracteristicas(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    edges = cv2.Canny(gray, 40, 160)
    densidade_borda = np.sum(edges > 0) / edges.size

    lap = cv2.Laplacian(gray, cv2.CV_64F)
    variancia_laplace = lap.var()

    contornos, _ = cv2.findContours(edges, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    maior_contorno = max([cv2.contourArea(c) for c in contornos]) if contornos else 0

    return np.array([densidade_borda, variancia_laplace, maior_contorno])


# ==============================================================
# CARREGA BASE DE DADOS DOS GRUPOS
# E CALCULA MÉDIAS E DESVIOS
# ==============================================================
def carregar_base():
    estatisticas = {}

    for grupo in GRUPOS:
        pasta = os.path.join(BASE_DIR, grupo)
        valores = []

        if not os.path.exists(pasta):
            estatisticas[grupo] = None
            continue

        arquivos = os.listdir(pasta)
        for nome in arquivos:
            caminho = os.path.join(pasta, nome)
            img = cv2.imread(caminho)

            if img is None:
                continue

            car = extrair_caracteristicas(img)
            valores.append(car)

        if len(valores) == 0:
            estatisticas[grupo] = None
        else:
            arr = np.array(valores)
            estatisticas[grupo] = {
                "media": arr.mean(axis=0),
                "desvio": arr.std(axis=0) + 1e-6  # evita divisão por zero
            }

    return estatisticas


# ==============================================================
# CLASSIFICA UMA NOVA IMAGEM COM BASE NAS MÉDIAS
# ==============================================================
def classificar_imagem(caminho):
    img = cv2.imread(caminho)
    if img is None:
        return "outros"

    nova = extrair_caracteristicas(img)

    # carrega dataset aprendido
    base = carregar_base()

    melhor_grupo = "outros"
    menor_distancia = float("inf")

    for grupo, dados in base.items():
        if dados is None:
            continue

        media = dados["media"]
        desvio = dados["desvio"]

        # distância normalizada (Z-score)
        distancia = np.sum(np.abs(nova - media) / desvio)

        if distancia < menor_distancia:
            menor_distancia = distancia
            melhor_grupo = grupo

    return melhor_grupo
