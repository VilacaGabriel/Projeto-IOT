# app.py
from flask import Flask, render_template, Response
from camera_stream import generate_frames
from config import ESP32_WROOM_API

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html', ESP_IP=ESP32_WROOM_API)

@app.route('/video')
def video():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    app.run(host="192.168.1.9", debug=True)
