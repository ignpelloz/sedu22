# TODO

- Cuando usar define y cuando un simple int? por ejemplo, para definir el entero en el que hay un led conectado, deberia usar define or int?


- Deberia usar un semaforo para la cola? es decir, una tarea no puede escribir si otra esta leyendo


- Finalizar modulos 4 y 5:
  1. Probar mod4/mod4.ino
  2. Probar mod5/trabajo_sinEj4.ino
  3. Cuando ambos funcionen, implementar mod5/trabajo.ino (es el ejercicio 4 del mod5, basicamente combina los ficheros de los pasos 1 y 2). Recordar que tengo 6 sensores y 2 actuadores.


- En los modulos 7 y 8 (cuando entra en juego la raspberry en definitiva), tener en cuenta la version de Python: creo que en los tutoriales usan siempre 2.7 pero el script lo hice usando 3


- Necesito demostrar de alguna manera que implemente poder pasar valores 0-9 a los actuadores. Deberia definir distintos umbrales en el script del mod7 o quizas permitir leer de un YAML en el que indico tramas [A,0,6], [A,1,3], etc. De cara a poder demostrar esto, quizas podria bajar la iteracion de 5 segundos a 2. **En cualquier caso**, podria yo en la raspberry conectarme por el puerto serie al controlador y mandarle tramas? asi podria demostrar que lo he implementado correctamente, en el caso de que lo de los umbrales no sea suficientemente sensible


- Donde puedo incluir un pulsador?


- Slow blink of L means task run out of memory and fast blink means the total requested stack size (from all the 4 xTaskCreate) exceeds the heap size?


- Aumentar configTOTAL_HEAP_SIZE en /data/universidad/master/1ro/2do_semestre/SEDU/arduino-1.8.19/libraries/Arduino_FreeRTOS/src/FreeRTOSConfig.h resolvio los problemas. Sin embargo, por qué? debo ser capaz de explicar eso (y tambien como se relaciona con los stack sizes usados en los 4 xTaskCreate) en la memoria final del trabajo


- Debo reducir la memoria utilizada:
  - https://www.megunolink.com/articles/three-methods-for-reducing-arduino-ram-usage/
  - https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05


- El DHT11 y la IMU no necesitan resistencia, por tanto se conectan muy facilmente, hacerlo ya para el mod 4:
  - https://lastminuteengineers.com/dht11-module-arduino-tutorial/#:~:text=ground%20of%20Arduino.-,Wiring%20DHT11%20Module%20to%20Arduino,-Let%E2%80%99s%20hook%20the
  - https://forum.arduino.cc/t/having-trouble-using-imu-sensor-icm-20689-with-arduino-i2c/692145/10 (viendo los videos que entregue el anio pasado, conecte mi IMU con solo 4 cables, todos a 4 pines adyacentes en un extremo: VCC (5v), GND, SCL y SDA. Estos dos ultimos van conectados a los pines SCL y SDA del arduino, en la parte de pines de "communication")
