// Cabezera de la clase Socket
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

struct Socket
{
    SOCKET socket;
    char puerto[5];
    boolean servidor;
};

struct Winsock2
{
    WSADATA wsaData;
    struct addrinfo *result;
    struct addrinfo *hints;
};

void inicializar_winsock(struct Winsock2 *winsock2,boolean debug);

void configurar_direccion(struct Socket *mySocket,struct Winsock2 *winsock2,char* dir_ip,char *puerto,boolean debug);

void crear_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug);

void vincular_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug);

void conectar_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug);

void aceptar_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug);

void aceptar_conexion(struct Socket *mySocket,struct Socket *socketCliente,struct Winsock2 *winsock2,boolean debug);