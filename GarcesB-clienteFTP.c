#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <pthread.h>

int errexit(const char *format, ...);
int connectTCP(const char *host, const char *service);
void mostrarMenu();
void enviarComandoSimple(int s,const char *comando);
int comandoPASV(int s);
void comandoSTOR(int s);
void comandoLIST(int s);
void comandoRETR(int s);
void *hiloEnvio(void *arg);
void *hiloDescarga(void *arg);
int comandoPORT(int s);
int passiveTCP(const char *service, int qlen);

struct thread_args{
char host[64];
char service[16];
char filename[128];
char user[64];
char pass[64];
};

#define LINELEN 128
char usuario_global [64];
char contrasena_global [64];

int main (int argc, char *argv[]){
const char *host = "localhost";
const char *service =  "21";
char buf [LINELEN + 1];
char temp_input [LINELEN];
int s, n,opcion;
switch (argc){
case 1:
break;
case 2:
host = argv[1];
break;
case 3:
host = argv[1];
service = argv[2];
break;
default:
errexit("No se pudo conectar con el servidor %s:%s\n",host,service);

}
s = connectTCP(host,service);
if(s<0){
errexit("No se puede conectar al servidor %s: %s\n",host,service);
}
printf("Conectado al servidor %s en el puerto %s\n",host,service);
n = read(s,buf,LINELEN);
if(n<0){
errexit("Error al leer el mensaje de bienvenida : %s\n", strerror(errno));
}
buf[n] = '\0';
printf("Servidor: %s",buf);
printf("Bienvenido al servicio FTP \n");
printf("Ingrese el nombre de usuario: ");
strcpy(buf, "USER ");
if(fgets(temp_input,LINELEN,stdin) !=NULL){
temp_input[strcspn(temp_input, "\n")] = 0;
strcpy(usuario_global, temp_input);
strcat(buf, temp_input);
strcat(buf, "\r\n");
if(write(s,buf,strlen(buf))<0){
errexit("Error al enviar USER");
}
n=read(s,buf,LINELEN);
if (n<=0){
errexit("No se pudo leer la respuesta de USER");
}
buf[n] = '\0';
printf("Servidor: %s \n", buf);
}
printf("Ingrese la contrasena: ");
strcpy(buf, "PASS ");
if(fgets(temp_input, LINELEN, stdin) != NULL){
temp_input[strcspn(temp_input, "\n")]= 0;
strcpy(contrasena_global, temp_input);
strcat(buf, temp_input);
strcat(buf, "\r\n");
if(write(s,buf,strlen(buf))<0){
errexit("Error al enviar PASS");
}
n= read(s,buf,LINELEN);
if(n<=0){
errexit("No se pudo leer la respuesta de PASS");
}
buf[n] = '\0';
printf("Servidor: %s", buf);
}
while (1){
mostrarMenu();
scanf("%d",&opcion);
getchar();
switch(opcion){
case 1:
enviarComandoSimple(s,"PWD");
break;
case 2:
comandoLIST(s);
break;
case 3:
comandoRETR(s);
break;
case 4:
comandoSTOR(s);
break;
case 5:
enviarComandoSimple(s,"QUIT");
exit(0);
break;
case 6:
int dataSock= comandoPASV(s);
if(dataSock>0){
printf("Conexion de datos lista (socket %d)\n",dataSock);
close(dataSock);
}else{
printf("No se pudo establecer la conexions PASV.\n");
}

break;
case 7:{
int num;
printf("Coloque la cantida de archivos que se desea enviar a la vez");
scanf("%d",&num);
getchar();
pthread_t hilos[num];
struct thread_args args[num];
for(int i=0; i< num; i++){
printf("Nombre del achivo %d",i+1);
fgets(args[i].filename, sizeof(args[i].filename),stdin);
args[i].filename[strcspn(args[i].filename,"\n")]=0;
strcpy(args[i].host, host);
strcpy(args[i].service, service);
strcpy(args[i].user, usuario_global); // cambiar dependiendo del caso
strcpy(args[i].pass, contrasena_global); // cambiar dependiendo del caso

}
for(int i =0; i<num; i++){
pthread_create(&hilos[i],NULL,hiloEnvio,&args[i]);
//sleep(1);
}

for(int i =0; i<num;i++){
pthread_join(hilos[i],NULL);
}
printf("Todos los hilos han terminado");
break;}
case 8: {
int num;
printf("Coloque la cantidad de archivos que se desea descargar a la vez");
scanf("%dig", &num);
getchar();
pthread_t hilos[num];
struct thread_args args[num];
for(int i =0; i < num; i++){
printf("Nombre del archivo remoto %d", i+1);
fgets(args[i].filename, sizeof(args[i].filename),stdin);
args[i].filename[strcspn(args[i].filename, "\n")] =0;
strcpy(args[i].host, host);
strcpy(args[i].service, service);
strcpy(args[i].user, "ubuntu"); // cambiar dependiendo del caso
strcpy(args[i].pass, "ubuntu");
}
for(int i =0; i < num; i++){
pthread_create(&hilos[i], NULL, hiloDescarga, &args[i]);
//sleep(1);
}

for (int i = 0; i < num; i++) {
pthread_join(hilos[i], NULL);
}
printf("Todas las descargas han terminado.\n");
break;

}
case 9:{
int dataSock = comandoPORT(s);
if (dataSock > 0){
printf("Conexion activa (PORT) lista en sockent %d\n", dataSock);
close(dataSock);
}else{
printf("No se pudo establecer la conexion activa.\n");
}
break;
}
case 10: {
 char comando[256];
 char buf[512];
 int n;
printf("Ingrese el comando FTP que desea enviar (por ejemplo: NOOP, HELP, DELE archivo.txt):\n> ");
fgets(comando, sizeof(comando), stdin);
comando[strcspn(comando, "\n")] = 0;
strcat(comando, "\r\n");
if (write(s, comando, strlen(comando)) < 0) {
perror("Error al enviar comando personalizado");
break;
}
n = read(s, buf, sizeof(buf) - 1);
if (n > 0) {
buf[n] = '\0';
printf("Servidor: %s\n", buf);
} else {
printf("No se recibió respuesta del servidor.\n");
}

break;
}

case 11: {
    char filename[128];
    char buf[512];
    int listenSock, dataSock, n;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    FILE *f;

    printf("Ingrese el nombre del archivo a subir (modo activo): ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    listenSock = comandoPORT(s);
    if (listenSock < 0) break;

    snprintf(buf, sizeof(buf), "STOR %s\r\n", filename);
    write(s, buf, strlen(buf));

    dataSock = accept(listenSock, (struct sockaddr *)&cli_addr, &cli_len);
    if (dataSock < 0) {
        perror("Error en accept()");
        close(listenSock);
        break;
    }

    f = fopen(filename, "rb");
    if (!f) {
        perror("No se pudo abrir el archivo local");
        close(dataSock);
        close(listenSock);
        break;
    }

    printf("Subiendo archivo '%s' en modo activo...\n", filename);
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        write(dataSock, buf, n);
    }

    fclose(f);
    close(dataSock);
    close(listenSock);

    n = read(s, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Servidor: %s\n", buf);
    }

    printf("Archivo '%s' subido correctamente (modo activo).\n", filename);
    break;
}


case 12: {
    char filename[128];
    char buf[512];
    int listenSock, dataSock, n;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    FILE *f;

    printf("Ingrese el nombre del archivo a descargar (modo activo): ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    listenSock = comandoPORT(s);
    if (listenSock < 0) break;

    snprintf(buf, sizeof(buf), "RETR %s\r\n", filename);
    write(s, buf, strlen(buf));

    dataSock = accept(listenSock, (struct sockaddr *)&cli_addr, &cli_len);
    if (dataSock < 0) {
        perror("Error en accept()");
        close(listenSock);
        break;
    }

    f = fopen(filename, "wb");
    if (!f) {
        perror("No se pudo crear el archivo local");
        close(dataSock);
        close(listenSock);
        break;
    }

    printf("Descargando archivo '%s' en modo activo...\n", filename);
    while ((n = read(dataSock, buf, sizeof(buf))) > 0) {
        fwrite(buf, 1, n, f);
    }

    fclose(f);
    close(dataSock);
    close(listenSock);

    n = read(s, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Servidor: %s\n", buf);
    }

    printf("Archivo '%s' descargado correctamente (modo activo).\n", filename);
    break;
}


default:
printf("No se ha codificado esa opcion");
break;

}
}
}

void mostrarMenu(){
printf("\n==== Menu FTP ====\n");
printf("1. PWD (Ver directorio actual)\n");
printf("2. LIST (listar archivos)\n");
printf("3. RETR (Descargar archivo)\n");
printf("4. STOR (Subir archivos)\n");
printf("5. QUIT (SALIR)\n");
printf("6. PASV (Modo pasivo)\n");
printf("7. STOR concurrente\n");
printf("8. RETR concurrente\n");
printf("9. PORT (Modo activo)\n");
printf("10. Comando personalizado (enviar lo que desees)\n");
printf("11. STOR (modo activo con PORT)\n");
printf("12. RETR (modo activo con PORT)\n");
printf("Seleccione una opcion: ");

}


void enviarComandoSimple(int s, const char *comando) {
    char buf[256];
    int n;
    snprintf(buf, sizeof(buf), "%s\r\n", comando);
    if (write(s, buf, strlen(buf)) < 0) {
        perror("Error al enviar comando");
        return;
    }
    n = read(s, buf, sizeof(buf) - 1);
    if (n <= 0) {
        perror("Error al leer respuesta");
        return;
    }

    buf[n] = '\0';
    printf("Servidor: %s\n", buf);
}


int comandoPASV(int s){
char buf[256];
int n;
int h1,h2,h3,h4,p1,p2;
char ip_str[64];
int port;
int dataSock;

if(write(s, "PASV\r\n",6)<0){
errexit("Error al enviar PASV");
return -1;
}
n= read(s,buf,sizeof(buf)-1);
if(n<=0){
errexit("Error al leer la respuesta de PASV");
return -1;
}
buf[n]='\0';
printf("Servidor: %s", buf);
if (sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
               &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        printf("No se pudo interpretar correctamente la respuesta PASV.\n");
        return -1;
}
sprintf(ip_str, "%d.%d.%d.%d", h1, h2, h3, h4);
port = p1 * 256 + p2;
printf("Dirección PASV detectada: %s:%d\n", ip_str, port);
char puerto [20];
sprintf(puerto,"%d",port);
dataSock = connectTCP(ip_str,puerto);
if(dataSock<0){
errexit("Error al conectar al puerto de datos PASV\n");
return -1;
}
printf("Conexion PASV establecida correctamente\n");
return dataSock;
}

void comandoSTOR(int s) {
char buf[512];
char filename[128];
int n;
FILE *f = NULL;
int dataSock = -1;

dataSock = comandoPASV(s);
if (dataSock < 0) {
errexit("No se pudo establecer la conexión PASV\n");
return;
}

printf("Ingrese el nombre del archivo a subir: ");
if (fgets(filename, sizeof(filename), stdin) == NULL) {
close(dataSock);
return;
}
filename[strcspn(filename, "\n")] = 0;

f = fopen(filename, "rb");
if (!f) {
errexit("Error al abrir el archivo local");
close(dataSock);
return;
}

snprintf(buf, sizeof(buf), "STOR %s\r\n", filename);
if (write(s, buf, strlen(buf)) < 0) {
errexit("Error al enviar comando STOR");
fclose(f);
close(dataSock);
return;
}

n = read(s, buf, sizeof(buf) - 1);
if (n > 0) {
buf[n] = '\0';
printf("Servidor: %s", buf);
}

printf("Subiendo archivo...\n");
while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
if (write(dataSock, buf, n) < 0) {
errexit("Error al enviar datos");
break;
}
}

fclose(f);
close(dataSock);

n = read(s, buf, sizeof(buf) - 1);
if (n > 0) {
buf[n] = '\0';
printf("Servidor: %s", buf);
} else if (n < 0) {
errexit("Error al leer respuesta final");
}

printf("Archivo '%s' subido correctamente.\n", filename);
}

void comandoLIST(int s) {
char buf[512];
int n;
int dataSock;
dataSock = comandoPASV(s);
if (dataSock < 0) {
printf("No se pudo establecer la conexión PASV.\n");
return;
}
if (write(s, "LIST\r\n", 6) < 0) {
perror("Error al enviar comando LIST");
close(dataSock);
return;
}
n = read(s, buf, sizeof(buf) - 1);
if (n > 0) {
buf[n] = '\0';
printf("Servidor: %s", buf);
}
printf("\n---- Contenido del directorio remoto ----\n");
while ((n = read(dataSock, buf, sizeof(buf) - 1)) > 0) {
buf[n] = '\0';
printf("%s", buf);
}
printf("---- Fin del listado ----\n");
close(dataSock);
n = read(s, buf, sizeof(buf) - 1);
if (n > 0) {
buf[n] = '\0';
printf("Servidor: %s", buf);
}
}

void comandoRETR(int s){
char buf[512];
char filename[128];
int n;
FILE *f = NULL;
int dataSock=-1;
dataSock = comandoPASV(s);
if(dataSock < 0){
errexit("No se pudo realizar la conexion PASV\n");
return;
}
comandoLIST(s);
printf("Ingrese el nombre del archivo a descargar");
if (fgets(filename, sizeof(filename),stdin)== NULL){
close(dataSock);
return;
}

filename[strcspn(filename,"\n")] = 0;
snprintf(buf, sizeof(buf),"RETR %s\r\n",filename);
if(write(s,buf,strlen(buf))<0){
errexit("Error al enviar la instruccion RETR");
close(dataSock);
return;
}
n= read(s, buf, sizeof(buf)-1);
if (n>0){
buf[n] = '\0';
printf("Servidor: %s", buf);
}
printf("Archivo %s descargado correctamente.\n",filename);


}

void *hiloEnvio(void *arg){
struct thread_args *data = (struct thread_args *)arg;
char buf[512];
int s,n;
FILE *f;
int dataSock;
printf("Iniciando envio a traves de un hilo de %s\n",data -> filename);
s = connectTCP(data ->host, data -> service);
if(s<0){
errexit("Error al conectar TCP para el hilo");
pthread_exit(NULL);
}
n = read(s, buf, sizeof(buf)-1);
buf[n] = '\0';
printf("Hilo %s servidor %s\n",data -> filename, buf);
snprintf(buf, sizeof(buf), "USER %s\r\n",data -> user);
write(s,buf,strlen(buf));
n= read(s, buf, sizeof(buf)-1);
buf[n]='\0';
printf("Hilo %s %s",data -> filename, buf);
snprintf(buf,sizeof(buf),"PASS %s\r\n",data -> pass);
write(s,buf,strlen(buf));
n= read(s,buf, sizeof(buf)-1);
buf[n]='\0';
printf("HILO %s %s", data -> filename, buf);
dataSock = comandoPASV(s);
if (dataSock < 0){
printf("Hilo %s Error en PASV\n", data -> filename);
close(s);
pthread_exit(NULL);
}
snprintf(buf, sizeof(buf),"STOR %s\r\n",data -> filename);
write(s, buf, strlen(buf));
n = read(s, buf, sizeof(buf)-1);
buf[n]='\0';
printf("Hilo %s %s", data -> filename, buf);
f = fopen(data -> filename, "rb");
if(!f){
errexit("Error al abrir el archivo local");
close(dataSock);
close(s);
pthread_exit(NULL);
}
printf("Hilo %s subiendo achivo\n",data-> filename);
while((n = fread(buf,1,sizeof(buf),f))>0){
write(dataSock, buf, n);
}
fclose(f);
close(dataSock);
n= read(s, buf, sizeof(buf)-1);
if(n>0){
buf[n] = '\0';
printf("Hilo %s Envio completado", data -> filename);
}
write(s, "QUIT\r\n",6);
n = read(s, buf, sizeof(buf)-1);
if(n>0){
buf[n] = '\0';
printf("Hilo %s %s", data -> filename,buf);
}
close(s);
printf("Hilo %s conexion cerrada", data -> filename);
pthread_exit(NULL);
}

void *hiloDescarga(void *arg){
struct thread_args *data = (struct thread_args *) arg;
char buf[512];
int s, n;
FILE *f;
int dataSock;
printf("Hilo de descarga, iniciando descarga de %s\n", data -> filename);
s = connectTCP(data -> host, data -> service);
if (s<0){
errexit("Error en connectTCP");
pthread_exit(NULL);
}
n = read(s, buf, sizeof(buf)-1);
if(n>0){
buf[n] = '\0';
printf("Hilo [%s] Servidor: %s", data->filename, buf);
}
snprintf(buf, sizeof(buf), "USER %s\r\n",data -> user);
write(s, buf, strlen(buf));
n = read(s, buf, sizeof(buf)-1);
buf[n] = '\0';
printf("Hilo [%s] %s", data->filename, buf);
snprintf(buf, sizeof(buf), "PASS %s\r\n", data -> pass);
write(s, buf, strlen(buf));
n = read(s, buf, sizeof(buf)-1);
buf[n] = '\0';
printf("Hilo [%s] %s", data->filename, buf);
dataSock = comandoPASV(s);
if (dataSock < 0){
errexit("Error en el hilo %s con PASV\n", data -> filename);
close(s);
pthread_exit(NULL);

}

snprintf(buf, sizeof(buf), "RETR %s\r\n", data->filename);
write(s, buf, strlen(buf));
n = read(s, buf, sizeof(buf)-1);
if (n > 0){
buf[n] = '\0';
printf("Hilo [%s] %s", data->filename, buf);
}
f = fopen(data -> filename, "wb");
if (!f){
errexit("Error al abrir el archivo");
close(dataSock);
close(s);
pthread_exit(NULL);

}
printf("Hilo %s descargando el archivo ... ", data -> filename);
while((n = read(dataSock, buf, sizeof(buf)))> 0){
fwrite(buf, 1,n,f);
}
fclose(f);
close(dataSock);
n = read(s, buf, sizeof(buf)-1);
if(n> 0){
buf[n] = '\0';
printf("Hilo [%s] %s", data->filename, buf);
}

printf("Hilo %s, ha completado la descarga", data -> filename);

write(s, "QUIT\r\n",6);
n = read(s, buf, sizeof(buf)-1);
if (n> 0){
buf[n] = '\0';
printf("Hilo [%s] %s", data->filename, buf);
}

close(s);
printf("Hilo %s conexion cerrada", data -> filename);
pthread_exit(NULL);
}


int comandoPORT(int s) {
    char buf[256], ip_str[64];
    int port, h1, h2, h3, h4, p1, p2;
    int listenSock;
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int n;

    listenSock = passiveTCP("8090", 1);
    if (listenSock < 0) {
        perror("Error al crear socket de escucha (PORT)");
        return -1;
    }

    if (getsockname(listenSock, (struct sockaddr *)&cli_addr, &cli_len) < 0) {
        perror("Error en getsockname()");
        close(listenSock);
        return -1;
    }

    port = ntohs(cli_addr.sin_port);
    strcpy(ip_str, "127.0.0.1");
    sscanf(ip_str, "%d.%d.%d.%d", &h1, &h2, &h3, &h4);
    p1 = port / 256;
    p2 = port % 256;

    snprintf(buf, sizeof(buf), "PORT %d,%d,%d,%d,%d,%d\r\n",
             h1, h2, h3, h4, p1, p2);
    printf("Enviando comando: %s", buf);
    write(s, buf, strlen(buf));

    n = read(s, buf, sizeof(buf) - 1);
    if (n <= 0) {
        perror("Error leyendo la respuesta del servidor");
        close(listenSock);
        return -1;
    }

    buf[n] = '\0';
    printf("Servidor: %s", buf);
    printf("Socket de escucha listo en puerto %d (modo activo)\n", port);

    listen(listenSock, 1);
    return listenSock;
}

