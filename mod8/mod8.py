import requests
import time
import MySQLdb
import sys
import os
import json

# ThingSpeak
API_KEY = "VWPF296PD37V9M4P"
CLEAR_API_KEY = "QGCZVKW1TTJ8M0US"
channelID = "1838807"
bulkUpdateURL = "https://api.thingspeak.com/channels/%s/bulk_update.json" % channelID

# Datos de la BD
DB_HOST = "localhost"
DB_USER = DB_PASS = "root"
DB_NAME = "SEDU22"

# Tiempo que pasará entre checkeos de la BD (y por consiguiente escrituras)
latenciaEntreLecturas = 20

def run_query(query=""):
    conn = MySQLdb.connect(DB_HOST, DB_USER, DB_PASS, DB_NAME)
    cursor = conn.cursor()
    cursor.execute(query)
    res = cursor.fetchall()
    conn.commit()
    cursor.close()
    conn.close()
    return res

def eliminarDatos():
    """Esta funcion elimina todos los datos del canal. """
    res = requests.delete("https://api.thingspeak.com/channels/%s/feeds.json" % channelID,
          data=r'api_key=%s' % CLEAR_API_KEY,
          headers = {'Content-Type': 'application/x-www-form-urlencoded'})
    print(res.text)

while(1):
    print("--------------------")
    # Se consulta la BD: ultimas 8 filas (las mas nuevas)
    nuevasEntradas = run_query("SELECT * FROM sensores ORDER BY id DESC LIMIT 8;") # Esto devuelve el id, los 6 sensores y la hora (ultimo registro)

    updates = []
    for nuevaEntrada in nuevasEntradas:
        newObject = {"field1": float(nuevaEntrada[1]),
                   "field2": float(nuevaEntrada[2]),
                   "field3": float(nuevaEntrada[3]),
                   "field4": float(nuevaEntrada[4]),
                   "field5": float(nuevaEntrada[5]),
                   "created_at": str(nuevaEntrada[6]) + " +0200"}
        updates.append(newObject)

    resp = requests.post(bulkUpdateURL,
      data=json.dumps({"write_api_key": API_KEY, "updates": updates}),
      headers = {'Content-Type': 'application/json'})
    print(resp.text)

    time.sleep(latenciaEntreLecturas)
