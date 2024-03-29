import serial
import time
import MySQLdb
import os

# Datos para conectar con BD
DB_HOST = "localhost"
DB_USER = DB_PASS = "root"
DB_NAME = "SEDU22"

def run_query(query=""):
    """Recibe un string que representa una query para SQL y la ejecuta. """
    conn = MySQLdb.connect(DB_HOST, DB_USER, DB_PASS, DB_NAME)
    cursor = conn.cursor()
    cursor.execute(query)
    conn.commit()
    cursor.close()
    conn.close()
    return

def procesarLectura(cadena):
    """Recibe un array de bytes y lo convierte en string, elimina caracteres de inicio y fin, 'O' y divide los datos. """
    return cadena.decode("utf-8").lstrip("[O").rstrip("]\r\n").split(caracterDelimitador)

def comprobarChecksum(separadas):
    """Recibe los datos ya separados (medidas de los sensores y checksum) y comprueba si el checksum es correcto. """
    checksumObtenido = 0.0
    for valor in separadas[:-1]:
        checksumObtenido += float(valor) # Sumatorio de los valores de los sensores
    checksumObtenido = str(checksumObtenido).split('.')[0][-1:] # Se toma el entero a la izquierda del punto (el sumatorio es un float)
    if checksumObtenido != separadas[-1:][0]:
        return False
    return True

def checkTEntreUmbrales(temperatura, umbral0, umbral1):
    """Comprueba si una medicion de temperatura se encuentra entre dos valores. """
    if temperatura >= umbral0 and temperatura < umbral1:
        return True
    return False

def fanActivadoSet(umbral):
    """Actualiza la variable fanActivado, utilizada para conocer a que velocidad gira el ventilador. """
    global fanActivado
    fanActivado = {"umbralU0":False,
                   "umbral01":False,
                   "umbral12":False,
                   "umbral23":False,
                   "umbral34":False,
                   "umbral45":False,
                   "umbral56":False,
                   "umbralO6":False}
    fanActivado[umbral] = True

def fanControl(temperatura):
    """Recibe un valor de temperatura, comprueba en que posicion se encuentra entre los umbrales
       y en funcion de esto activa el ventilador con mayor o menor velocidad. """
    if temperatura <= umbralesTemp[0] and fanActivado["umbralU0"] is False:
        micro.write(b"[A,1,0]")
        fanActivadoSet("umbralU0")
    elif checkTEntreUmbrales(temperatura,umbralesTemp[0],umbralesTemp[1]) is True and fanActivado["umbral01"] is False:
        micro.write(b"[A,1,3]")
        fanActivadoSet("umbral01")
    elif checkTEntreUmbrales(temperatura,umbralesTemp[1],umbralesTemp[2]) is True and fanActivado["umbral12"] is False:
        micro.write(b"[A,1,4]")
        fanActivadoSet("umbral12")
    elif checkTEntreUmbrales(temperatura,umbralesTemp[2],umbralesTemp[3]) is True and fanActivado["umbral23"] is False:
        micro.write(b"[A,1,5]")
        fanActivadoSet("umbral23")
    elif checkTEntreUmbrales(temperatura,umbralesTemp[3],umbralesTemp[4]) is True and fanActivado["umbral34"] is False:
        micro.write(b"[A,1,6]")
        fanActivadoSet("umbral34")
    elif checkTEntreUmbrales(temperatura,umbralesTemp[4],umbralesTemp[5]) is True and fanActivado["umbral45"] is False:
        micro.write(b"[A,1,7]")
        fanActivadoSet("umbral45")
    elif checkTEntreUmbrales(temperatura,umbralesTemp[5],umbralesTemp[6]) is True and fanActivado["umbral56"] is False:
        micro.write(b"[A,1,8]")
        fanActivadoSet("umbral56")
    elif temperatura >= umbralesTemp[6] and fanActivado["umbralO6"] is False:
        micro.write(b"[A,1,9]")
        fanActivadoSet("umbralO6")

def getPuerto():
    """ Devuelve la ruta del dispositivo, por ejemplo ttyACM0. """
    for dispositivo in os.listdir("/dev"):
        if "ACM" in dispositivo:
            return dispositivo

caracterDelimitador = "/"
latenciaEntreLecturas = 5
umbralLDR = 100 # cubierto vale menos de 10, abierto vale ~340
umbralesTemp = [22,23,24,25,26,27,28]
puerto = "/dev/%s" % getPuerto()
servoActivado = False
fanActivado = {"umbralU0":False,
               "umbral01":False,
               "umbral12":False,
               "umbral23":False,
               "umbral34":False,
               "umbral45":False,
               "umbral56":False,
               "umbralO6":False}

# Comunicacion por el puerto serie. El constructor Serial recibe: puerto en el que se encuentra la placa conectada y baudios
micro = serial.Serial(puerto, 9600, timeout=1)

# Esperar un tiempo de setup
time.sleep(2)

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
        run_query("INSERT INTO sensores (luminosidad,humedad,temperatura,imux,imuy) \
                  VALUES ('%s','%s','%s','%s','%s');" % (separadas[0],separadas[1],separadas[2],separadas[3],separadas[4]))

        # Se coloca el servo en su posicion 180 si se llega al umbral del LDR (si es inferior se coloca en su posicion 0, en el caso de que este en 180)
        if float(separadas[0]) >= umbralLDR and servoActivado is False:
            micro.write(b"[A,0,9]")
            servoActivado = True
        elif float(separadas[0]) < umbralLDR and servoActivado is True:
            micro.write(b"[A,0,0]")
            servoActivado = False

        # Se activa el motor en funcion de los umbrales definidos
        fanControl(float(separadas[2]))

    # Se espera un tiempo determinado entre dos lecturas
    time.sleep(latenciaEntreLecturas)
