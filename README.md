# TODO

- Slow blink of L means task run out of memory and fast blink means the total requested stack size (from all the 4 xTaskCreate) exceeds the heap size?


- Aumentar configTOTAL_HEAP_SIZE en /data/universidad/master/1ro/2do_semestre/SEDU/arduino-1.8.19/libraries/Arduino_FreeRTOS/src/FreeRTOSConfig.h resolvio los problemas. Sin embargo, por qué? debo ser capaz de explicar eso (y tambien como se relaciona con los stack sizes usados en los 4 xTaskCreate) en la memoria final del trabajo


- Debo reducir la memoria utilizada:
  - https://www.megunolink.com/articles/three-methods-for-reducing-arduino-ram-usage/
  - https://create.arduino.cc/projecthub/john-bradnam/reducing-your-memory-usage-26ca05
