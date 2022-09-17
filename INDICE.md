# Índice

Este directorio contiene los siguientes archivos (además de `INDICE.md` y `README.md`):

## `./arduino/arduino.ino`

Código del microcontrolador.

## `./mod7/bd_cmds.sh`

Script que contiene comandos para instalar y configurar el servidor de base de datos y lanzar su servicio.

Además, contiene comandos del CLI de SQL para crear la base de datos y la tabla dentro de esta.

## `./mod7/mod7.py`

Script en Python referente al módulo 7.

## `./mod7/service.sh`

Script en Bash que contiene los comandos para definir, arrancar y mostrar logs del servicio referente al módulo 7, es decir, el que solicita periódicamente a la placa Arduino lecturas de sensores y mueve los actuadores (script `./mod7/mod7.py`).

## `./mod8/mod8.py`

Script en Python referente al módulo 8.

## `./mod8/service.sh`

Script en Bash que contiene los comandos para definir, arrancar y mostrar logs del servicio referente al módulo 8, es decir, el que lee periódicamente registros en la base de datos local y los envía a la nube ThingSpeak (script `./mod8/mod8.py`).
