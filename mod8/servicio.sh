# Creacion del servicio
cat <<EOT >> /lib/systemd/system/mod8.service
[Unit]
Description="Script Modulo 8"
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/bin/python /home/pi/sedu22/mod8/mod8.py
Restart=on-abort

[Install]
WantedBy=multi-user.target
EOT

# Dar permisos de ejecucion
sudo chmod 644 /lib/systemd/system/mod8.service
chmod +x /home/pi/sedu22/mod8/mod8.py

# Recargar daemon
sudo systemctl daemon-reload

# Habilitar y comenzar servicio
sudo systemctl enable mod8.service
sudo systemctl start mod8.service

# Ver logs del servicio
sudo journalctl -f -u mod8.service
