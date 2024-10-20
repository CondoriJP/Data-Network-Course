// Definir Librerías
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

/*
 * Universidad Tecnológica Nacional
 * Facultad Regional Mendoza
 *
 * Ingeniería en Sistemas de Información
 * Redes de datos 3k10 - 2024
 * Grupo 4
 *
 * Trabajo Práctico N° 2
 * Autor: Juan Pablo Condori Tellez
 * https://github.com/CondoriJP/Data-Network-Course
 *
 * Descripción:
 * Servidor que recibe tramas de datos y las almacena en un buffer secundario.
 * Las tramas de datos contienen un ID, un dato y un CRC.
 *
 * Compilación:
 * g++ -o Servidor Servidor.cpp
 */

// Definir constantes
#define PORT_TX 8081
#define IP_TX "127.0.0.1"
#define PORT_RX 8080
#define IP_RX "127.0.0.1"

// Definir estructuras
struct Trama {
    unsigned int id;
    unsigned int data;
    unsigned int crc;
};


struct Cache {
    struct Trama trama[64];
    unsigned int ultima;
    int cantidad;
};

struct Buffer_Secundario { // Funciona como una pila de 256, solo se introduce en orden asdendente segun el numero de trama
    unsigned int datos[256];
    unsigned int inicio;
    unsigned int fin;
    unsigned int contador;
};

// Variables globales
struct Buffer_Secundario buffer_secundario;
struct Cache cache;

//Trama inicio de datos
// tramaINICIO.id = 0;
// tramaINICIO.data = 0;
// tramaINICIO.crc = 60;

//Trama fin de datos
// tramaFIN.id = 0;
// tramaFIN.data = 0;
// tramaFIN.crc = 195;

//Trama ACK
// tramaACK.id = cualquiera
// tramaACK.data = 0;
// tramaACK.crc = 127;

// Definir funciones
void enviarTrama(Trama trama);
void recibirTrama();
bool bufferSecundarioAgregarPila(unsigned int num_trama, unsigned int data); // Agrega un dato a la pila, si el orden es correcto
                                                                             // retorna true, si no, retorna false y no agrega el dato
bool bufferSecundarioSacarPila(unsigned int &data); // Saca un dato de la pila, si la pila no esta vacia, retorna true y el dato
                                                    // si la pila esta vacia, retorna false y no saca ningun dato

void limpiarBufferSecundario(); // Limpia el buffer secundario, pone el contador en 0, el inicio en 0 y el fin en 0
void limpiarCache(); // Limpia la cache, pone la cantidad en 0 y el ultima en 0
Trama sacarCache(); // Saca la trama de la cache, si la cache no esta vacia, retorna la trama y la elimina de la cache
                   // si la cache esta vacia, retorna una trama con id=0, data=0, crc=0.1
bool agregarCache(Trama trama); // Agrega una trama a la cache, si la cache no esta llena, agrega la trama y retorna true
                                // si la cache esta llena, no agrega la trama y retorna false
void mostrarTrama(Trama trama);
void mostrarBufferSecundario();

void agregarDato(int tick);

// Implementar funciones
using namespace std;

void enviarTrama(Trama trama) {
    // Crear un socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cout << "[!] Error al crear el socket" << endl;
        return;
    }

    // Definir la dirección del servidor
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_TX); // Puerto del servidor
    serv_addr.sin_addr.s_addr = inet_addr(IP_TX); // Dirección del servidor (localhost)

    // Enviar la trama
    ssize_t bytes_sent = sendto(sockfd, &trama, sizeof(trama), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (bytes_sent < 0) {
        cout << "[!] Error al enviar la trama" << endl;
    }

    // Cerrar el socket
    close(sockfd);
}

void recibirTrama() {
    cout << "[+] Iniciando servidor..." << endl << endl;
    while (true){
        // Crear un socket UDP
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            cout << "[!] Error al crear el socket" << endl;
        }

        // Definir la dirección del servidor
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT_RX); // Puerto del servidor
        serv_addr.sin_addr.s_addr = inet_addr(IP_RX); // Dirección del servidor (localhost)

        // Enlazar el socket a la dirección del servidor
        if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            cout << "[!] Error al enlazar el socket" << endl;
        }

        // Recibir la trama
        Trama trama;
        ssize_t bytes_received = recvfrom(sockfd, &trama, sizeof(trama), 0, NULL, NULL);
        if (bytes_received < 0) {
            cout << "[!] Error al recibir la trama" << endl;
        } 

        // Cerrar el socket
        close(sockfd);

        if (trama.id == 0 && trama.data == 0 && trama.crc == 60) {
            cout << endl;
            cout << "[+] Inicio de datos" << endl;
            limpiarCache();
            limpiarBufferSecundario();
        } else if (trama.id == 0 && trama.data == 0 && trama.crc == 195) {
            cout << "[+] Fin de datos" << endl << endl << "[>] Paquete recibido: ";
            unsigned int data;
            while (bufferSecundarioSacarPila(data)) {
                cout << (char)data;
            }
            cout << endl << endl;
        } else {
            if (agregarCache(trama)) {
                cout << "| ID=" << trama.id << ", Data=" << (char)trama.data << ", CRC=" << trama.crc << endl;
            }
        }
    }
}


unsigned int calcularCRS(unsigned int data) {
    // Asegúrate de que 'data' tenga solo 8 bits (0-255)
    if (data > 0xFF) {
        throw std::invalid_argument("El dato debe ser un valor de 8 bits (0-255).");
    }

    // Inicializar los bits de paridad
    unsigned int p1 = 0, p2 = 0, p4 = 0, p8 = 0;

    // Bits de datos (D1-D8)
    unsigned int d1 = (data >> 0) & 1; // Bit 0
    unsigned int d2 = (data >> 1) & 1; // Bit 1
    unsigned int d3 = (data >> 2) & 1; // Bit 2
    unsigned int d4 = (data >> 3) & 1; // Bit 3
    unsigned int d5 = (data >> 4) & 1; // Bit 4
    unsigned int d6 = (data >> 5) & 1; // Bit 5
    unsigned int d7 = (data >> 6) & 1; // Bit 6
    unsigned int d8 = (data >> 7) & 1; // Bit 7

    // Calcular los bits de paridad
    p1 = d1 ^ d2 ^ d4 ^ d5 ^ d7; // p1 cubre bits 1, 2, 4, 5, 7
    p2 = d1 ^ d3 ^ d4 ^ d6 ^ d7; // p2 cubre bits 1, 3, 4, 6, 7
    p4 = d2 ^ d3 ^ d4 ^ d8;      // p4 cubre bits 2, 3, 4, 8
    p8 = d5 ^ d6 ^ d7 ^ d8;      // p8 cubre bits 5, 6, 7, 8

    // Combinar los bits de datos y los bits de paridad
    unsigned int hammingCode = 0;
    hammingCode |= (p1 << 0); // P1 en la posición 1
    hammingCode |= (p2 << 1); // P2 en la posición 2
    hammingCode |= (d1 << 2); // D1 en la posición 3
    hammingCode |= (p4 << 3); // P4 en la posición 4
    hammingCode |= (d2 << 4); // D2 en la posición 5
    hammingCode |= (d3 << 5); // D3 en la posición 6
    hammingCode |= (d4 << 6); // D4 en la posición 7
    hammingCode |= (p8 << 7); // P8 en la posición 8
    hammingCode |= (d5 << 8); // D5 en la posición 9
    hammingCode |= (d6 << 9); // D6 en la posición 10
    hammingCode |= (d7 << 10); // D7 en la posición 11
    hammingCode |= (d8 << 11); // D8 en la posición 12

    return hammingCode;
}


bool bufferSecundarioAgregarPila(unsigned int num_trama, unsigned int data) { // Agrega un dato a la pila, si el orden es correcto
    if (buffer_secundario.contador == 0) {
        buffer_secundario.inicio = num_trama;
        buffer_secundario.fin = num_trama;
        buffer_secundario.datos[num_trama] = data;
        buffer_secundario.contador++;
        return true;
    } else if (num_trama == buffer_secundario.fin + 1) {
        buffer_secundario.fin = num_trama;
        buffer_secundario.datos[num_trama] = data;
        buffer_secundario.contador++;
        return true;
    } else {
        return false;
    }
}

bool bufferSecundarioSacarPila(unsigned int &data) { // Saca un dato de la pila, si la pila no esta vacia, retorna true y el dato
    if (buffer_secundario.contador == 0) {
        return false;
    } else {
        data = buffer_secundario.datos[buffer_secundario.inicio];
        buffer_secundario.inicio++;
        buffer_secundario.contador--;
        return true;
    }
}

void limpiarBufferSecundario() {
    for (int i = 0; i < 256; i++) {
        buffer_secundario.datos[i] = 0;
    }
    buffer_secundario.inicio = 0;
    buffer_secundario.fin = 0;
    buffer_secundario.contador = 0;
}

Trama sacarCache() {
    if (cache.cantidad == 0) {
        return {0, 0, 0};
    } else {
        Trama trama = cache.trama[cache.ultima];
        cache.ultima--;
        cache.cantidad--;
        return trama;
    }
}

bool agregarCache(Trama trama) {
    if (cache.cantidad == 32) {
        cout << "[!] Error 3" << endl;
        return false;
    } else {
        for (int i = 0; i < cache.cantidad; i++) {
            if (cache.trama[i].id == trama.id) {
                cout << "[!] Error 2" << endl;
                return false;
            }
        }
        if ((trama.crc == 127) && (trama.id == 0)) {
            cout << "[!] Error 1" << endl;
            return false;
        }
        unsigned int crc = calcularCRS(trama.data);
        if (crc != trama.crc) {
            cout << "[!] Error en la trama" << endl;
            cout << "CRC: " << crc << ", CRC Trama: " << trama.crc << endl;
            return false;
        }
        cache.ultima++;
        cache.trama[cache.ultima].data = trama.data;
        cache.trama[cache.ultima].id = trama.id;
        cache.trama[cache.ultima].crc = trama.crc;
        cache.cantidad++;
        return true;
    }
}

void limpiarCache() {
    for (int i = 0; i < 64; i++) {
        cache.trama[i].id = 0;
        cache.trama[i].data = 0;
        cache.trama[i].crc = 0;
    }
    cache.ultima = 0;
    cache.cantidad = 0;
}

void mostrarTrama(Trama trama) {
    cout << "ID: " << trama.id << ", Data: " << trama.data << ", CRC: " << trama.crc << endl;
}

void mostrarBufferSecundario() {
    cout << "Buffer Secundario: ";
    for (int i = buffer_secundario.inicio; i <= buffer_secundario.fin; i++) {
        cout << (char)buffer_secundario.datos[i];
    }
    cout << endl;
}

void agregarDato(int tick) {
    while (true){
        Trama tram = sacarCache();
        if (tram.id != 0) {
            bufferSecundarioAgregarPila(tram.id, tram.data);
            Trama ack = {tram.id, 0, 127};
            enviarTrama(ack);
        }
        this_thread::sleep_for(std::chrono::milliseconds(tick));
    }
}

int main (int argc, char *argv[]) {
    system("clear");
    limpiarBufferSecundario();
    thread hilo_recibir(recibirTrama);
    hilo_recibir.detach();
    agregarDato(250);
    return 0;
}

