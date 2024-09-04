// Servidor de la capa de enlace de 
#include "headers/server.h"
#include "src/headers/socket.h"

struct Socket socketConexion;
struct Socket socketCliente;
struct Winsock2 winsock2;

int main(int argc, char *argv[]) {
    socketConexion.servidor = TRUE;
    socketConexion.socket = INVALID_SOCKET;

    inicializar_winsock(&winsock2,DEBUG);
    configurar_direccion(&socketConexion,&winsock2,NULL,PUERTO,DEBUG);
    crear_socket(&socketConexion,&winsock2,DEBUG);
    vincular_socket(&socketConexion,&winsock2,DEBUG);
    aceptar_socket(&socketConexion,&winsock2,DEBUG);
    aceptar_conexion(&socketConexion,&socketCliente,&winsock2,DEBUG);
    return 0;
}