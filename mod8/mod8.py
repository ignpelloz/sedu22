import requests
import time
import MySQLdb
import sys
import os
import json

# ThingSpeak
API_KEY = "EH9P4GJU85EWW33E"
CLEAR_API_KEY = "QGCZVKW1TTJ8M0US"
channelID = "1828914"
bulkUpdateURL = "https://api.thingspeak.com/channels/%s/bulk_update.json" % channelID # URL del canal (para API)

# Datos de la BD
DB_HOST = "localhost"
DB_USER = DB_PASS = "root"
DB_NAME = "SEDU22"

# Tiempo que pasará entre checkeos de la BD (y por consiguiente escrituras)
latenciaEntreLecturas = 20

def run_query(query=""):
    """Recibe un string que representa una query para SQL y la ejecuta.
       Devuelve lo obtenido al ejecutar fetchall(), que en el caso de
       queries SELECT seran los registros seleccionados. """
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
    # Se consulta la BD: ultimas 10 filas (las mas nuevas)
    nuevasEntradas = run_query("SELECT * FROM sensores ORDER BY id DESC LIMIT 10;") # Esto devuelve un listado, cada elemento tiene: id, valor de los 5 sensores y la fecha & hora de insercion

    updates = []
    # Para cada elemento devuelto por el servidor de la BD, se crea un ubjeto con el formato adecuado para la API de ThingSpeak
    for nuevaEntrada in nuevasEntradas:
        newObject = {"field1": float(nuevaEntrada[1]), # Luminosidad
                   "field2": float(nuevaEntrada[2]), # Humedad
                   "field3": float(nuevaEntrada[3]), # Temperatura
                   "field4": float(nuevaEntrada[4]), # IMU x
                   "field5": float(nuevaEntrada[5]), # IMU y
                   "created_at": str(nuevaEntrada[6]) + " +0200"} # Fecha y hora de insercion (por tanto, de medida)
        # Cada objeto se aniade a una lista que sera enviada a ThingSpeak
        updates.append(newObject)

    # Se envia una peticion de tipo POST a ThingSpeak con la lista de objetos en el cuerpo
    resp = requests.post(bulkUpdateURL,
                         data=json.dumps({"write_api_key": API_KEY, "updates": updates}),
                         headers = {'Content-Type': 'application/json'})

    # Se imprime la respuesta al POST
    print(resp.text)

    # Se espera un tiempo determinado entre dos inserciones
    time.sleep(latenciaEntreLecturas)
