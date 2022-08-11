import serial # requires pip install
import time
import MySQLdb

# Acceder a la BD. Nota: antes de esto, se debe instalar y configurar la base de datos
DB_HOST = "localhost"
DB_USER = "root"
DB_PASS = "pi"
DB_NAME = "SEDU22"

def run_query(query=""):
    datos = [DB_HOST, DB_USER, DB_PASS, DB_NAME]
    conn = MySQLdb.connect(*datos) # TODO: por que la estrella?
    cursor = conn.cursor()
    cursor.execute(query)
    conn.commit()
    cursor.close()
    conn.close()
    return

# TODO: usar comillas simples en lugar de dobles?

# Para conocer el puerto, ejecutar: ls /dev/tty* antes y despues de conectar el arduino al pc. El dispositivo que falte en el primer ls es nuestro puerto
puerto = "/dev/ttyACM0"

# Comunicacion por el puerto serie. El constructor Serial recibe: puerto en el que se encuentra la placa conectada y baudios
micro = serial.Serial(puerto, 9600, timeout=1)

# Bastante habitual necesitar un tiempo de setup
time.sleep(2)

# Tarea repetitiva
while(1):
    # Ejecutar solo si el puerto serie esta abierto
    if(micro.isOpen()):
        # Mandar comando de lectura de sensores. Nota: en python los puertos series no transmiten strings si no array de bytes
        micro.write(b"[S]")

        # Lectura de la respuesta del controlador (arduino), es decir, lectura de los sensores. Nota: lo que se recibe es un array de bytes (mismo tipo que se envia con la funcion write())
        cadena = micro.readline()
        cortada = cadena.decode("utf-8")
        cortada = cortada.lstrip("[")
        cortada = cortada.rstrip("]\r\n")

        separadas = cortada.split(",") # En este caso, el caracter delimitador es ','

        # Se inserta lo obtenido en la BD
        query = "INSERT INTO sensores (x,y,z) VALUES ('%s','%s','%s')" % (separadas[0],separadas[1],separadas[2])
        run_query(query)

        # TODO: comprobar checksum

    # Delay para no ser muy pesado
    time.sleep(1)
