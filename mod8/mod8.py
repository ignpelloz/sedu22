import requests
import time
import MySQLdb
import sys
import os
import json

# ThingSpeak API key
API_KEY = "151QABP1286D3OQT"
CLEAR_API_KEY = "QGCZVKW1TTJ8M0US"

# Datos de la BD
DB_HOST = "localhost" # Para Docker: 127.0.0.1
DB_USER = "root"
DB_PASS = "root"
DB_NAME = "SEDU22"

# Tiempo que pasará entre checkeos de la BD (y por consiguiente escrituras)
latenciaEntreLecturas = 5

def run_query(query=""):
    conn = MySQLdb.connect(DB_HOST, DB_USER, DB_PASS, DB_NAME)
    cursor = conn.cursor()
    cursor.execute(query)
    res = cursor.fetchone()
    conn.commit()
    cursor.close()
    conn.close()
    return res

def eliminarDatos():
    """Esta funcion elimina todos los datos del canal. """
    res = requests.delete("https://api.thingspeak.com/channels/1828914/feeds.json",
          data=r'api_key=%s' % CLEAR_API_KEY,
          headers = {'Content-Type': 'application/x-www-form-urlencoded'})

while(1):

    # Si se produce una nueva escritura en la BD, hacer un query y lo que se obtenga se envia a ThingSpeak
    if os.path.isfile("/tmp/nuevoRegistroEnDB") is True:
        nuevaEntrada = run_query("SELECT * FROM sensores ORDER BY id DESC LIMIT 1;") # Esto devuelve el id, los 6 sensores y la hora (ultimo registro)

        resp = requests.post("https://api.thingspeak.com/update.json",
              data=json.dumps({"api_key": API_KEY,
                      "field1": float(nuevaEntrada[1]),
                      "field2": float(nuevaEntrada[2]),
                      "field3": float(nuevaEntrada[3]),
                      "field4": float(nuevaEntrada[4]),
                      "field5": float(nuevaEntrada[5]),
                      "created_at": nuevaEntrada[6]}, default=str), # TODO: la fecha debe ser unica, no puede existir ya. Ademas, no debe haber 0s a la izq (por ejemplo, nu usar 02, 03, etc). Tambien, comprobar porque no respeta la fecha utilizada. UPDATE: parece que no tengo que hacer nada especial con la fecha, comprobar!
              headers = {'Content-Type': 'application/json'})

        # TODO: checkear respuesta y en caso de error repetir?
        print(resp.text)

        # Se elimina el flag
        os.remove("/tmp/nuevoRegistroEnDB")

    time.sleep(latenciaEntreLecturas)
