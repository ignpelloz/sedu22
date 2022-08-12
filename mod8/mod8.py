import requests
import time
import MySQLdb
import sys

# ThingSpeak API key
API_KEY = "151QABP1286D3OQT"
CLEAR_API_KEY = "QGCZVKW1TTJ8M0US"

# Datos de la BD
DB_HOST = "127.0.0.1" # localhost"
DB_USER = "root"
DB_PASS = "pi"
DB_NAME = "SEDU22"

# Tiempo que pasará entre checkeos de la BD (y por consiguiente escrituras)
latenciaEntreLecturas = 5

def run_query(query=""):
    datos = [DB_HOST, DB_USER, DB_PASS, DB_NAME]
    conn = MySQLdb.connect(*datos) # TODO: por que la estrella?
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

    # TODO: monitorear la base de datos, y en cuanto se produzca una nueva escritura, hacer un query y lo que se obtenga se envia a ThingSpeak
    if os.path.isfile("/tmp/nuevoRegistroEnDB") is True:
        nuevaEntrada = run_query("SELECT * FROM sensores ORDER BY id DESC LIMIT 1;") # TODO: esto devuelve el id, los 6 sensores y la hora

        # TODO: usar la hora devuelta por la consulta a la BD para el grafico (necesito cambiar de GET a POST para eso)
        writeURL = "https://api.thingspeak.com/update?api_key=%s&field1=%s&field2=%s&field3=%s&field4=%s&field5=%s&field6=%s" \
            % (API_KEY, nuevaEntrada[1], nuevaEntrada[2], nuevaEntrada[3], nuevaEntrada[4], nuevaEntrada[5], nuevaEntrada[6])

        # TODO: checkear respuesta y en caso de error repetir?
        resp = requests.get(writeURL)

        print(resp.text)

        # Delete
        os.remove("/tmp/nuevoRegistroEnDB")

    time.sleep(latenciaEntreLecturas)
