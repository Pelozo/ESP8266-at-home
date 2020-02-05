from flask import Flask, request, abort, render_template
import time
from datetime import datetime
import json

app = Flask(__name__)

#Trusted IPS
SENSOR_IPS = ['192.168.1.200', '192.168.1.107', '192.168.1.112']

#Sensor names
SENSOR_NAMES = {
    "ESP01-1": "Bedroom Temperature",
    "NodemcuV3-1": "Kitchen stuff"
}

@app.route('/', methods=['GET'])
def index():
    if not data:
        abort(503)
    return json.dumps(data)

@app.route('/updateTemperature', methods=['POST'])
def updateTemperature():
    if request.remote_addr not in SENSOR_IPS:
        abort(403)
    try:
        temperature = float(request.form.get('temperature'))
    except (TypeError, ValueError):
        abort(400)    
    try:
        sensor = SENSOR_NAMES[request.form.get('module')]
    except KeyError:
        print("Request from an unkown sensor ({})\n Add it to the dict you dummy.".format(request.form.get('module')))
        sensor = "unknown"

    
    data['temperature_bedroom'] = {}
    data['temperature_bedroom']['timestamp'] = datetime.timestamp(datetime.now())
    data['temperature_bedroom']['temperature'] = temperature

    #TODO something... maybe sqlite?
    
    saveLog(sensor, temperature)
    saveState()
    return json.dumps(data)

@app.route('/updateRaining', methods=['POST'])
def updateRaining():
    if request.remote_addr not in SENSOR_IPS:
        abort(403)
    try:
        isRaining = bool(request.form.get('isRaining'))
    except (TypeError, ValueError):
        abort(400) 
    try:
        sensor = SENSOR_NAMES[request.form.get('module')]
    except KeyError:
        print("Request from an unkown sensor ({})\n Add it to the dict you dummy.".format(request.form.get('module')))
        sensor = "unknown"

    data['raining'] = isRaining

    #TODO send push notification or whatever
    saveState()
    saveLog(sensor, isRaining)
    return json.dumps(data)

@app.route('/ring', methods=['GET'])
def doorbell():
    abort(503)

@app.route('/testServo', methods=['GET'])
def test():
    return render_template('testServo.html', url='http://192.168.1.112/servo?rotate=')

def loadState():
    try:
        with open("state", encoding='utf-8-sig') as file:
            return json.load(file)
    except:
        return {}

def saveState():
    with(open("state", "w")) as file:
        file.write(json.dumps(data))

def saveLog(fileName, line):
    with(open("logs/" + fileName, 'a+')) as file:
        file.write("{} | {}\n".format(str(datetime.now()), str(line)))


if __name__ == '__main__':
    data = loadState()
    app.run(debug=True, port=1025, host='0.0.0.0')