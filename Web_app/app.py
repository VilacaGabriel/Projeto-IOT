# app.py
from flask import Flask, render_template, Response, request, jsonify
from camera_stream import generate_frames
from config import ESP32_WROOM_API
from flask import send_from_directory
import os
import shutil

BASE = "static/anomalias"

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html', ESP_IP=ESP32_WROOM_API)

@app.route('/video')
def video():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route("/anomalias")
def listar_anomalias():
    files = os.listdir("anomalias")
    files = sorted(files, reverse=True)
    return render_template("anomalias.html", imagens=files)

@app.route("/registro")
def registro():
    base_classificados = "static/anomalias/classificados"
    base_nao_classificado = "static/anomalias/nao_classificado"

    # --------------------------------------
    # 1) Carregar imagens não classificadas
    # --------------------------------------
    if not os.path.exists(base_nao_classificado):
        os.makedirs(base_nao_classificado)

    nao_classificados = sorted(os.listdir(base_nao_classificado))

    # --------------------------------------
    # 2) Carregar dinamicamente os grupos existentes
    # --------------------------------------
    imagens = {}

    if not os.path.exists(base_classificados):
        os.makedirs(base_classificados)

    for grupo in sorted(os.listdir(base_classificados)):
        pasta = os.path.join(base_classificados, grupo)

        # só aceita pastas (evita arquivos soltos)
        if os.path.isdir(pasta):
            imagens[grupo] = sorted(os.listdir(pasta))

    return render_template(
        "registro.html",
        imagens=imagens,
        nao_classificados=nao_classificados
    )

@app.route("/anomalias/<path:filename>")
def arquivo_anomalia(filename):
    return send_from_directory("anomalias", filename)

@app.post("/classificar_manual")
def classificar_manual():
    dados = request.get_json()
    imagem = dados.get("imagem")
    grupo_novo = dados.get("grupo")
    grupo_atual = dados.get("grupo_atual")  # pode ser '' se vier de nao_classificado

    if not imagem or not grupo_novo:
        return jsonify({"erro": "Dados inválidos"}), 400

    caminho_nao_classificado = os.path.join(BASE, "nao_classificado")
    caminho_classificados = os.path.join(BASE, "classificados")

    # --- Descobrir de onde vem a imagem ---
    if grupo_atual:
        # vem de um grupo já classificado
        origem = os.path.join(caminho_classificados, grupo_atual, imagem)
    else:
        # vem da pasta de não classificados
        origem = os.path.join(caminho_nao_classificado, imagem)

    # --- Criar pasta do novo grupo se não existir ---
    pasta_destino = os.path.join(caminho_classificados, grupo_novo)
    os.makedirs(pasta_destino, exist_ok=True)

    destino = os.path.join(pasta_destino, imagem)

    try:
        shutil.move(origem, destino)
    except Exception as e:
        return jsonify({"erro": str(e)}), 500

    return jsonify({
        "status": "ok",
        "imagem_nova": imagem,
        "grupo": grupo_novo
    })
BASE_NAO_CLASSIFICADO = "static/anomalias/nao_classificado"
BASE_CLASSIFICADOS = "static/anomalias/classificados"

@app.route("/excluir_imagem", methods=["POST"])
def excluir_imagem():
    data = request.get_json()
    imagem = data.get("imagem")
    grupo_atual = data.get("grupo_atual")  # “” se estiver em não_classificado

    try:
        # ===============================
        # CASO 1 — IMAGEM SEM GRUPO
        # ===============================
        if not grupo_atual:
            caminho = os.path.join(BASE_NAO_CLASSIFICADO, imagem)

            if os.path.exists(caminho):
                os.remove(caminho)
            else:
                return jsonify({"status": "erro", "erro": "Arquivo não encontrado"}), 404

            return jsonify({"status": "ok"})


        # ===============================
        # CASO 2 — IMAGEM CLASSIFICADA
        # ===============================
        pasta_grupo = os.path.join(BASE_CLASSIFICADOS, grupo_atual)
        caminho = os.path.join(pasta_grupo, imagem)

        if os.path.exists(caminho):
            os.remove(caminho)
        else:
            return jsonify({"status": "erro", "erro": "Arquivo não encontrado"}), 404

        # Se a pasta ficar vazia, apagar também
        if len(os.listdir(pasta_grupo)) == 0:
            shutil.rmtree(pasta_grupo)

        return jsonify({"status": "ok"})

    except Exception as e:
        return jsonify({"status": "erro", "erro": str(e)}), 500


if __name__ == '__main__':
    app.run(debug=True)
