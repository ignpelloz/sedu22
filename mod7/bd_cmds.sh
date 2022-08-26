# Instalar servidor de base de datos
sudo apt install mariadb-server mariadb-client

# Configurar servidor
sudo mysql_secure_installation

# Lanzar servicio
sudo /etc/init.d/mysql start

# Docker (despues de hacer esto, exec en el contenedor y seguir los pasos: crear BD, crear tabla, etc)
sudo docker run -e MYSQL_ROOT_PASSWORD=root -e MYSQL_DATABASE=SEDU22 --net=host mysql

# Para conectarse deberás ejecutar el siguiente comando:
mysql -u root -p # password is root too

# Crear una nueva base de datos
create database SEDU22;

# Mostrar todas las bases de datos creadas en el servidor
show databases;

# Indicar la base de datos que vamos a usar
use SEDU22;

# Con el siguiente comando se crea la tabla (incluye, ademas de valores para los 5 sensores, ID y fecha)
CREATE TABLE sensores(
    ID INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    luminosidad DECIMAL(10,5),
    humedad DECIMAL(10,5),
    temperatura DECIMAL(10,5),
    imux DECIMAL(10,5),
    imuy DECIMAL(10,5),
    fecha_y_hora TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

# Listar tablas de la base de datos
show tables;

# Salir del shell interactivo
quit;
