import paho.mqtt.client as mqtt
from datetime import datetime
import urllib3
import json
import sqlite3
from pymongo import MongoClient
http   = urllib3.PoolManager()

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("ict720/narodomy/esp32/data")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    try:
        dev_name = msg.topic.split('/')[2]
        dev_data = json.loads(msg.payload.decode())['value']
        # ql_cmd = f"INSERT INTO taist (device,data) VALUES ('{dev_name}', {dev_data})"
        c.execute("INSERT INTO taist (device,data) VALUES (?, ?)",
                (dev_name, dev_data)
                )
    except:
        print("Error: can not insert SQLite")

    #insert 
    dev_db = mongo_client.taist
    dev_col = dev_db.logs
    data = {"device": dev_name, "data": dev_data, "timestamp": datetime.now()}

    dev_col.insert_one(data)
    # payload = {"timestamp": datetime.now().isoformat(), "message": msg.payload.decode()}
    # encoded_payload = json.dumps(payload).encode('utf-8')
    # print(encoded_payload)
    # http.request('POST','https://ict720-79595-default-rtdb.asia-southeast1.firebasedatabase.app/narodomy.json', body=encoded_payload, headers={'Content-Type': 'application/json'})

# init SQlite
conn = sqlite3.connect('taist.db')
c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS taist (
          _id INTEGER PRIMARY KEY AUTOINCREMENT,
          timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
          device TEXT,
          data REAL
          )''')
conn.commit()

# init MongoDB
mongo_client = MongoClient('my_docker_project-mongo-1', 27017)  # Use the service name "mongo"


mqttc = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
mqttc.on_connect = on_connect
mqttc.on_message = on_message

print("app.py is working.")

mqttc.connect("mosquitto", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
mqttc.loop_forever()