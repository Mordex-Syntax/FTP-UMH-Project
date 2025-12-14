# Cliente FTP 
Este programa implementa un cliente FTP en c que se conecta al servidor vsftpd utilizando por defecto localhost en el puerto 21.
En caso se desee cambiar la ubicación del host o del puerto se deben pasar como primer y segundo argumento respectivamente.
## Requisitos
* Compilador C
* Biblioteca de hilos POSIX: pthread
* Servidor vsftpd o similiar en ejecución
* Archivos auxiliares:
	*connectTcp.c
	*connectsock.c
	*errexit.c
	*passiveTCP.c

## Compilación
Es posible utilizar el archivo Makefile-clienteftp, o en su defecto ejecutar el siguiente comando:
gcc clienteftp.c connectTCP.c connectsock.c errexit.c passiveTCP.c -o clienteftp -lpthread
## Ejecución
Al ejecutar el programa utilizar 
./clienteftp 
O en su defecto especificar las direcciones de host y puerto correspondientes
./clienteftp [host] [puerto]

## Al iniciar el programa
1. Se conecta al servidor FTP.
2. Muestra el mensaje de bienvenida al servidor.
3. Solicita
	* Usuario (USER)
	* Contraseña (PASS)
Estos datos se usan para la sesión principal del cliente.
### Menú de opciones
Una vez autenticado, el programa muestra el siguiente menú:
==== Menu FTP ====
1. PWD (Ver directorio actual)
2. LIST (Listar archivos)
3. RETR (Descargar archivo en modo pasivo)
4. STOR (Subir archivo en modo pasivo)
5. QUIT (Salir)
6. PASV (Probar conexión en modo pasivo)
7. STOR concurrente (subida múltiple en modo pasivo, con hilos)
8. RETR concurrente (descarga múltiple en modo pasivo, con hilos)
9. PORT (Modo activo, solo prueba de conexión)
10. Comando personalizado (enviar cualquier comando FTP)
11. STOR (modo activo con PORT)
12. RETR (modo activo con PORT)

#### Descripción rápida de opciones importantes
* 1 – PWD
1 – PWD
* 2 – LIST
Entra en modo pasivo (PASV), envía LIST y muestra el listado del directorio remoto.
* 3 – RETR
Entra en modo pasivo y descarga un archivo desde el servidor (modo interactivo).
* 4 – STOR
Entra en modo pasivo y sube un archivo al servidor.
* 7 – STOR concurrente
Permite subir varios archivos en paralelo usando hilos.
Cada hilo:

Establece su propia conexión FTP.

Envía USER, PASS.

Usa PASV y STOR para enviar el archivo.

* 8– RETR concurrente
Permite descargar varios archivos en paralelo usando hilos.
Cada hilo:

Establece su propia conexión FTP.

Envía USER, PASS.

Usa PASV y RETR para descargar el archivo.
* 9– PORT
Configura el modo activo (el cliente abre un listen y envía el comando PORT al servidor).
* 11 – STOR (modo activo con PORT)
Usa PORT, espera la conexión del servidor y sube un archivo en modo activo.
* 12 – RETR (modo activo con PORT)
Usa PORT, espera la conexión del servidor y descarga un archivo en modo activo.
* 10 – Comando personalizado
Permite escribir cualquier comando FTP manualmente (por ejemplo NOOP, HELP, DELE archivo.txt, etc.).
## Configuración del servidor FTP (vsftpd)
Para las pruebas de este cliente se usó vsftpd con una configuración similar a la siguiente en /etc/vsftpd.conf:
### Puntos claves de la configuración
* Solo usuarios locales (no anónimo):
anonymous_enable=NO
local_enable=YES
* Permitir escritura (subidas, borrados, etc.):
write_enable=YES
* Chroot del usuario a su directorio (lo “encierra” en su carpeta):
chroot_local_user=YES
allow_writeable_chroot=YES
* Directorio raíz del usuario local (en este caso):
local_root=/home/ubuntu/ftp
* Uso de hora local:
use_localtime=YES
* Logging de transferencias:
xferlog_enable=YES
* SSL desactivado (conexión FTP simple, sin cifrado):
ssl_enable=NO
* Usuario del sistema
En esta configuración se asume un usuario Linux, por ejemplo:

Usuario: ubuntu

Directorio raíz para FTP: /home/ubuntu/ftp
### Pasos típicos:
* Crear una carpeta ftp si no existe
mkdir -p /home/ubuntu/ftp
chown ubuntu:ubuntu /home/ubuntu/ftp

### Notas adicionales
* Asegurarse de reiniciar el servicio vsftpd después de cambiar el archivo de configuración:
sudo systemctl restart vsftpd
* Los archivos a subir deben existir en el directorio donde se ejecute clienteftp (o usar rutas completas).
* Los archivos descargados se guardan en el directorio actual con el mismo nombre del archivo remoto.

## Ejemplo de ejecución

Compilación (opción 1: Makefile):
```bash
make
# o manual:
gcc GarcesB-clienteFTP.c connectTCP.c connectsock.c errexit.c passiveTCP.c -o clienteftp -lpthread
```

Ejecución básica:
```bash
./clienteftp localhost 21
Conectado al servidor localhost en el puerto 21
Servidor: 220 (vsFTPd 3.0.3)
Bienvenido al servicio FTP
Ingrese el nombre de usuario: ubuntu
Servidor: 331 Please specify the password.
Ingrese la contrasena: ********
Servidor: 230 Login successful.
==== Menu FTP ====
1. PWD ...
Seleccione una opcion: 1
Servidor: 257 "/home/ubuntu/ftp" is the current directory
```

Listar archivos (opción 2):
```bash
Seleccione una opcion: 2
Servidor: 227 Entering Passive Mode (127,0,0,1,195,44)
Dirección PASV detectada: 127.0.0.1:50004
Conexion PASV establecida correctamente
Servidor: 150 Here comes the directory listing.
---- Contenido del directorio remoto ----
test.txt
readme.txt
img.png
---- Fin del listado ----
Servidor: 226 Directory send OK.
```

Subir archivo (opción 4 - STOR pasivo):
```bash
Seleccione una opcion: 4
Ingrese el nombre del archivo a subir: test.txt
Servidor: 227 Entering Passive Mode (...)
Subiendo archivo...
Servidor: 150 Ok to send data.
Servidor: 226 Transfer complete.
Archivo 'test.txt' subido correctamente.
```

Descargar archivo (opción 3 - RETR pasivo):
```bash
Seleccione una opcion: 3
Ingrese el nombre del archivo a descargar: readme.txt
Servidor: 227 Entering Passive Mode (...)
Servidor: 150 Opening BINARY mode data connection.
Servidor: 226 Transfer complete.
Archivo readme.txt descargado correctamente.
```

Subida concurrente (opción 7):
```bash
Seleccione una opcion: 7
Coloque la cantida de archivos que se desea enviar a la vez: 2
Nombre del archivo 1: a.txt
Nombre del archivo 2: b.txt
Hilo a.txt subiendo archivo...
Hilo b.txt subiendo archivo...
Hilo a.txt Envio completado
Hilo b.txt Envio completado
Todos los hilos han terminado
```

Descarga concurrente (opción 8):
```bash
Seleccione una opcion: 8
Coloque la cantidad de archivos que se desea descargar a la vez: 2
Nombre del archivo remoto 1: readme.txt
Nombre del archivo remoto 2: test.txt
Hilo readme.txt descargando el archivo ...
Hilo test.txt descargando el archivo ...
Hilo readme.txt ha completado la descarga
Hilo test.txt ha completado la descarga
Todas las descargas han terminado.
```

Modo activo (PORT + STOR) (opción 11):
```bash
Seleccione una opcion: 11
Ingrese el nombre del archivo a subir (modo activo): big.bin
Enviando comando: PORT 127,0,0,1,31,144
Servidor: 200 PORT command successful.
Subiendo archivo 'big.bin' en modo activo...
Servidor: 150 Ok to send data.
Servidor: 226 Transfer complete.
Archivo 'big.bin' subido correctamente (modo activo).
```

Salir (opción 5):
```bash
Seleccione una opcion: 5
Servidor: 221 Goodbye.
```

Notas:
* Los códigos 150 y 226 indican transferencia iniciada y finalizada correctamente.
* En concurrencia cada hilo establece su propia sesión (USER/PASS).
* Asegurar que los archivos locales existan antes de STOR.

