// Cliente de la capa de enlace de datos
#include "headers/client.h"
#include "src/headers/socket.h"

struct Socket socketConexion;
struct Winsock2 winsock2;

int main(int argc, char *argv[]) {
    socketConexion.servidor = FALSE;
    socketConexion.socket = INVALID_SOCKET;
    
    inicializar_winsock(&winsock2,DEBUG);
    configurar_direccion(&socketConexion,&winsock2,DIR_IP,PUERTO,DEBUG);
    crear_socket(&socketConexion,&winsock2,DEBUG);
    conectar_socket(&socketConexion,&winsock2,DEBUG);
    return 0;
}