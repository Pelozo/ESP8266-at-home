from flask import Flask, request, abort
import time
from datetime import datetime
import json

app = Flask(__name__)

#Trusted IPS
SENSOR_IPS = ['192.168.1.200', '192.168.1.107', '192.168.1.112']

#Sensor names
SENSOR_NAMES = {
    "ESP01-1":"Bedroom Temperature"
}


data = {}

@app.route('/', methods=['GET'])
def index():
    print(data)
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
    return json.dumps(data)



def saveLog(fileName, line):
    with(open("logs/" + fileName, 'a+')) as file:
        file.write("{} | {}\n".format(str(datetime.now()), str(line)))
        #file.write(str(datetime.now()) + " | " + str(line) + "\n")


if __name__ == '__main__':
    app.run(debug=True, port=1025, host='0.0.0.0')