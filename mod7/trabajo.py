import serial # pip install PySerial
import time
import MySQLdb # sudo apt-get install python3-dev libmysqlclient-dev && sudo pip3 install mysqlclient

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

caracterDelimitador = "/"
latenciaEntreLecturas = 5
umbralLDR = 100 # cubierto vale menos de 10, abierto vale ~340
umbralTemperatura = 40
puerto = "/dev/ttyACM0" # Para conocer el puerto, ejecutar: ls /dev/tty* antes y despues de conectar el arduino al pc. El dispositivo que falte en el primer ls es nuestro puerto
motorActivado = False
servoActivado = False

# Comunicacion por el puerto serie. El constructor Serial recibe: puerto en el que se encuentra la placa conectada y baudios
micro = serial.Serial(puerto, 9600, timeout=1)

# Bastante habitual necesitar un tiempo de setup
time.sleep(2)

def procesarLectura(cadena):
    "Convierte el array de bytes en string, elimina caracteres de inicio y fin y divide los datos. "
    return cadena.decode("utf-8").lstrip("[").rstrip("]\r\n").split(caracterDelimitador)

def comprobarChecksum(separadas):
    checksumObtenido = 0.0
    for valor in separadas[:-1]:
        checksumObtenido += float(valor)
    checksumObtenido = str(checksumObtenido).split('.')[0][-1:]
    if checksumObtenido != separadas[-1:][0]:
        return False
    return True

while(1):
    # Ejecutar solo si el puerto serie esta abierto
    if(micro.isOpen()):
        # Mandar comando de lectura de sensores. Nota: en python los puertos series no transmiten strings si no array de bytes
        micro.write(b"[S]")

        # Lectura de la respuesta del controlador
        separadas = procesarLectura(micro.readline())

        # Si el checksum es incorrecto, se ignora el resto de la iteracion
        if comprobarChecksum(separadas) is False:
            continue

        # Se muestran los datos (a excepcion del checksum)
        print(separadas[:-1])

        # Se inserta lo obtenido en la BD
        hora_actual = time.now()

        # Se debe crear la tabla con el siguiente comando
        """
        CREATE TABLE sensores(
            ID INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
            luminosidad DECIMAL(10,5),
            humedad DECIMAL(10,5),
            temperatura DECIMAL(10,5),
            imux DECIMAL(10,5),
            imuy DECIMAL(10,5),
            sonido DECIMAL(10,5),
            fecha_y_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        """

        query = "INSERT INTO sensores (luminosidad,humedad,temperatura,imux,imuy,sonido,fecha_y_hora) \
                 VALUES ('%s','%s','%s','%s','%s','%s','%s')" % (separadas[0],separadas[1],separadas[2],separadas[3],separadas[4],separadas[5])

        run_query(query)

        # Se coloca el servo en su posicion 180 si se llega al umbral del LDR (si es inferior se coloca en su posicion 0, en el caso de que este en 180)
        if separadas[0] >= umbralLDR and servoActivado is False:
            micro.write(b"[A,0,9]")
            servoActivado = True
        elif separadas[0] < umbralLDR and servoActivado is True:
            micro.write(b"[A,0,0]")
            servoActivado = False

        # Se activa el motor si se llega al umbral del sensor de temperatura (si es inferior se apaga, en el caso de que este activado)
        if separadas[2] >= umbralTemperatura and motorActivado is False:
            micro.write(b"[A,1,9]")
            motorActivado = True
        elif separadas[2] < umbralTemperatura and motorActivado is True:
            micro.write(b"[A,1,0]")
            motorActivado = False

    time.sleep(latenciaEntreLecturas)
