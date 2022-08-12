# Creacion del servicio
cat <<EOT >> /lib/systemd/system/mod7.service
[Unit]
Description="Script Modulo 7"
After=multi-user.target

[Service]
Type=simple
ExecStart=/usr/bin/python /home/pi/mod7.py
Restart=on-abort

[Install]
WantedBy=multi-user.target
EOT

# Dar permisos de ejecucion
sudo chmod 644 /lib/systemd/system/mod7.service
chmod +x /home/pi/mod7.py

# Recargar daemon
sudo systemctl daemon-reload

# Habilitar y comenzar servicio
sudo systemctl enable mod7.service
sudo systemctl start mod7.service

# Ver logs del servicio
sudo journalctl -f -u mod7.service
