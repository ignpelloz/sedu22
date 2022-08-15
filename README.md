# TODO

- Cuando usar define y cuando un simple int? por ejemplo, para definir el entero en el que hay un led conectado, deberia usar define or int?


- Deberia usar un semaforo para la cola? es decir, una tarea no puede escribir si otra esta leyendo


- Si la cola no funciona con elementos de tipo String, usar un struct tal y como recomiendan en el video de teoria?


- Finalizar modulos 4 y 5:
  1. Probar mod4/trabajo/trabajo.ino
  2. Probar mod5/trabajo_sinEj4.ino
  3. Cuando ambos funcionen, implementar mod5/trabajo.ino (es el ejercicio 4 del mod5, basicamente combina los ficheros de los pasos 1 y 2). Recordar que tengo 6 sensores y 2 actuadores.


- En los modulos 7 y 8 (cuando entra en juego la raspberry en definitiva), tener en cuenta la version de Python: creo que en los tutoriales usan siempre 2.7 pero el script lo hice usando 3


- Necesito demostrar de alguna manera que implemente poder pasar valores 0-9 a los actuadores. Deberia definir distintos umbrales en el script del mod7 o quizas permitir leer de un YAML en el que indico tramas [A,0,6], [A,1,3], etc. De cara a poder demostrar esto, quizas podria bajar la iteracion de 5 segundos a 2. **En cualquier caso**, podria yo en la raspberry conectarme por el puerto serie al controlador y mandarle tramas? asi podria demostrar que lo he implementado correctamente, en el caso de que lo de los umbrales no sea suficientemente sensible


- Donde puedo incluir un pulsador?
