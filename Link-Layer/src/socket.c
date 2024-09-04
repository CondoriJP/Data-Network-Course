#include "headers/socket.h"

void inicializar_winsock(struct Winsock2 *winsock2,boolean debug) {
    if (debug) printf("[+] Inicializando Winsock\n");
    int iResultado;
    iResultado = WSAStartup(MAKEWORD(2, 2), &winsock2->wsaData);
    if (iResultado != 0) {
        if (debug) printf("[X] Error al inicializar Winsock\n");
        exit(1);
    }
}

void configurar_direccion(struct Socket *mySocket,struct Winsock2 *winsock2,char *dir_ip,char *puerto,boolean debug) {
    if (debug) printf("[+] Configurando direccion\n");
    int iResultado;
    winsock2->hints = (struct addrinfo *)malloc(sizeof(struct addrinfo));
    if (winsock2->hints == NULL) {
        if (debug) ("[X] Error al reservar memoria para hints\n");
        exit(1);
    }
    winsock2->hints->ai_family = AF_INET;
    winsock2->hints->ai_socktype = SOCK_STREAM;
    winsock2->hints->ai_protocol = IPPROTO_TCP;
    if(mySocket->servidor) winsock2->hints->ai_flags = AI_PASSIVE;
    strcpy(mySocket->puerto, puerto);
    iResultado = getaddrinfo(dir_ip, mySocket->puerto, winsock2->hints, &winsock2->result);
    if (iResultado != 0) {
        if (debug) printf("[X] Error al obtener informacion de la direccion\n");
        WSACleanup();
        exit(1);
    }
}

void vincular_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug) {
    if (debug) printf("[+] Vinculando socket\n");
    int iResultado;
    iResultado = bind(mySocket->socket, winsock2->result->ai_addr, (int)winsock2->result->ai_addrlen);
    if (iResultado == SOCKET_ERROR) {
        if (debug) printf("[X] Error al vincular el socket\n");
        freeaddrinfo(winsock2->result);
        closesocket(mySocket->socket);
        WSACleanup();
        exit(1);
    }
}

void crear_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug) {
    if (debug) printf("[+] Creando socket\n");
    mySocket->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mySocket->socket == INVALID_SOCKET) {
        if (debug) printf("[X] Error al crear el socket\n");
        freeaddrinfo(winsock2->result);
        WSACleanup();
        exit(1);
    }
}

void conectar_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug) {
    if (debug) printf("[+] Conectando socket\n");
    int iResultado;
    iResultado = connect(mySocket->socket, winsock2->result->ai_addr, (int)winsock2->result->ai_addrlen);
    if (iResultado == SOCKET_ERROR) {
        if (debug) printf("[X] Error al conectar el socket\n");
        freeaddrinfo(winsock2->result);
        closesocket(mySocket->socket);
        WSACleanup();
        exit(1);
    }
}

void aceptar_socket(struct Socket *mySocket,struct Winsock2 *winsock2,boolean debug) {
    if (debug) printf("[+] Aceptando socket\n");
    int iResultado;
    iResultado = listen(mySocket->socket, SOMAXCONN);
    if (iResultado == SOCKET_ERROR) {
        if (debug) printf("[X] Error al aceptar el socket\n");
        freeaddrinfo(winsock2->result);
        closesocket(mySocket->socket);
        WSACleanup();
        exit(1);
    }
}

void aceptar_conexion(struct Socket *mySocket,struct Socket *socketCliente,struct Winsock2 *winsock2,boolean debug) {
    if (debug) printf("[+] Aceptando conexion\n");
    int iResultado;
    socketCliente->socket = accept(mySocket->socket, NULL, NULL);
    if (socketCliente->socket == INVALID_SOCKET) {
        if (debug) printf("[X] Error al aceptar la conexion\n");
        freeaddrinfo(winsock2->result);
        closesocket(mySocket->socket);
        WSACleanup();
        exit(1);
    }
}