# Docker (despues de hacer esto, exec en el contenedor y seguir los pasos del PDF)
sudo docker run -e MYSQL_ROOT_PASSWORD=pi -e MYSQL_DATABASE=SEDU22 --net=host mysql

# Con el siguiente comando se crea la tabla (incluye, ademas de valores para los 6 sensores, ID y fecha)
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
