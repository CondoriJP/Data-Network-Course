#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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
 */

typedef unsigned int Byte;
#define PORT_TX 8080
#define IP_TX "127.0.0.1"
#define PORT_RX 8081
#define IP_RX "127.0.0.1"

#define MAX 1024
#define TAM 32

struct Trama {
    Byte id;
    Byte data;
    Byte crc;
};

struct Datos {
    Byte num_trama;
    bool libre;
    bool enviado;
    bool ack;
    bool nak;
    bool clock;
    Byte data;
    Byte ticks;
};

struct Buffer_Primario {
    struct Datos datos[TAM];
    Byte proximo;
    Byte cantLibres;
};

struct Buffer_Secundario {
    Byte datos[MAX];
    Byte inicio;
    Byte fin;
    Byte cantidad;
};

struct Buffer_Primario buffer_primario;
struct Buffer_Secundario buffer_secundario;
char buffer[MAX];
Byte ID_UNICA = 0;
bool transmitiendo = false;


// Funciones
void b_S_limpiar();
void b_S_set(Byte dato);
Byte b_S_get();
void b_S_get_rollback();
void b_S_mostrar();
void b_S_ingreso();
Byte id_unica_get();
void id_unica_rollback();
void id_unica_reset();
void b_P_limpiar();
void b_P_limpiar_data(Byte num_trama);
void b_P_actualizar();
Byte b_P_set(Byte num_trama, Byte data);
void b_P_mostrar();
void agregar_datos();
void temporizador(int &time);
void enviar_datos(int &time, int tick, int tickTrama);
void aplicar_ack();
void clockTimer(int tick, int tickTrama);
Byte calcularCRS(Byte data);
void mostrarTrama(Trama trama);
Trama recibirTrama();
void enviarTrama(Trama trama);
int ack_tipo(Trama trama);
void procesar_ACK(Trama trama);
void procesar_NAK(Trama trama);
void recibir_ACK();
void enviar_inicio();
void enviar_fin();

// Implementación

void b_S_limpiar() {
    buffer_secundario.inicio = 0;
    buffer_secundario.fin = 0;
    buffer_secundario.cantidad = 0;
    for (Byte i = 0; i < MAX; i++) {
        buffer_secundario.datos[i] = 0;
    }
}

void b_S_set(Byte dato){
    buffer_secundario.datos[buffer_secundario.fin] = dato;
    buffer_secundario.fin++;
    buffer_secundario.cantidad++;
    if (buffer_secundario.fin == MAX) {
        buffer_secundario.fin = 0;
    }
}

Byte b_S_get() {
    Byte dato = buffer_secundario.datos[buffer_secundario.inicio];
    buffer_secundario.inicio++;
    buffer_secundario.cantidad--;
    if (buffer_secundario.inicio == MAX) {
        buffer_secundario.inicio = 0;
    }
    return dato;
}

void b_S_get_rollback() {
    buffer_secundario.inicio--;
    buffer_secundario.cantidad++;
    if (buffer_secundario.inicio == MAX) {
        buffer_secundario.inicio = 0;
    }
}

void b_S_mostrar() {
    for (Byte i = 0; i < buffer_secundario.cantidad; i++) {
        printf("%c", buffer_secundario.datos[(buffer_secundario.inicio + i) % MAX]);
    }
    printf("\n");
}

void b_S_ingreso() {
    char c;
    printf("[>] Ingrese los datos a transmitir: ");
    while (buffer_secundario.cantidad < MAX) {
        c = getchar();
        if (c == '\n') {
            break;
        }
        b_S_set(c);
    }
    for (Byte i=0; i<MAX; i++){
        buffer[i] = buffer_secundario.datos[i];
    }
}

Byte id_unica_get() {
    ID_UNICA++;
    return ID_UNICA;
}

void id_unica_rollback() {
    ID_UNICA--;
}

void id_unica_reset() {
    ID_UNICA = 0;
}

void b_P_limpiar() {
    buffer_primario.proximo = 0;
    buffer_primario.cantLibres = TAM;
    for (Byte i = 0; i < TAM; i++) {
        buffer_primario.datos[i].num_trama = 0; 
        buffer_primario.datos[i].data = 0;
        buffer_primario.datos[i].libre = true;
        buffer_primario.datos[i].enviado = false;
        buffer_primario.datos[i].ack = false;
        buffer_primario.datos[i].nak = false;
        buffer_primario.datos[i].clock = false;
        buffer_primario.datos[i].ticks = 0;
    }
}

void b_P_limpiar_data(Byte num_trama) {
    for (Byte i = 0; i < TAM; i++) {
        if (buffer_primario.datos[i].num_trama == num_trama) {
            buffer_primario.datos[i].num_trama = 0; 
            buffer_primario.datos[i].data = 0;
            buffer_primario.datos[i].libre = true;
            buffer_primario.datos[i].enviado = false;
            buffer_primario.datos[i].ack = false;
            buffer_primario.datos[i].nak = false;
            buffer_primario.datos[i].clock = false;
            buffer_primario.datos[i].ticks = 0;
        }
    }
}

void b_P_actualizar() {
    buffer_primario.proximo = -1;
    buffer_primario.cantLibres = 0;
    for (Byte i = 0; i < TAM; i++) {
        if (buffer_primario.datos[i].libre) {
            buffer_primario.cantLibres++;
            if (buffer_primario.proximo == -1) {
                buffer_primario.proximo = i;
            }
        }
    }
}

Byte b_P_set(Byte num_trama, Byte data) {
    b_P_actualizar();
    if (buffer_primario.cantLibres > 0) {
        b_P_limpiar_data(num_trama);
        buffer_primario.datos[buffer_primario.proximo].num_trama = num_trama;
        buffer_primario.datos[buffer_primario.proximo].data = data;
        buffer_primario.datos[buffer_primario.proximo].libre = false;
        return num_trama;
    } else if (buffer_primario.cantLibres == 0) {
        return -1;
    } else {
        return -1;
    }
}

void b_P_mostrar() {
    printf("\n[+] Buffer Primario\n");
    for (Byte i = 0; i < TAM; i++) {
        printf(" ID: %d, Data: %c, Clock: %d, ACK: %d, NAK: %d\n", buffer_primario.datos[i].num_trama, buffer_primario.datos[i].data, buffer_primario.datos[i].ticks, buffer_primario.datos[i].ack, buffer_primario.datos[i].nak);
        printf("------------------------------------------\n");
    }
}

void agregar_datos() {
    Byte num_trama;
    Byte data;
    while(buffer_secundario.cantidad > 0) {
        b_P_actualizar();
        num_trama = id_unica_get();
        data = b_S_get();
        if (b_P_set(num_trama, data) == -1) {
            id_unica_rollback();
            b_S_get_rollback();
            break;
        }
    }
    if (buffer_secundario.cantidad == 0) {
        b_P_actualizar();
        if (buffer_primario.cantLibres == TAM) {
            transmitiendo = false;
        }
    }
}

void temporizador(int &time) {
    Byte num_trama;
    Byte data;
    for (Byte i = 0; i < TAM; i++) {
        if (!buffer_primario.datos[i].libre) {
            if (buffer_primario.datos[i].clock) {
                buffer_primario.datos[i].ticks--;
                if (buffer_primario.datos[i].ticks == 0) {
                    data = buffer_primario.datos[i].data;
                    num_trama = buffer_primario.datos[i].num_trama;
                    b_P_limpiar_data(num_trama);
                    b_P_actualizar();
                    b_P_set(num_trama, data);
                    time--;
                }
            }
        }
    }
}

void enviar_datos(int &time, int tick, int tickTrama) {
    Byte num_trama;
    Byte data;
    Trama trama;
    system("clear");
    for (Byte i = 0; i < TAM; i++) {
        if (!buffer_primario.datos[i].libre) {
            if (!buffer_primario.datos[i].enviado) {
                trama.id = buffer_primario.datos[i].num_trama;
                trama.data = buffer_primario.datos[i].data;
                trama.crc = calcularCRS(trama.data);
                enviarTrama(trama);
                buffer_primario.datos[i].enviado = true;
                buffer_primario.datos[i].clock = true;
                buffer_primario.datos[i].ticks = tickTrama+time;
                time++;
                b_P_mostrar();
                std::this_thread::sleep_for(std::chrono::milliseconds(tick));
                system("clear");
            }
        }
    }
}

void aplicar_ack(){
    Byte num_trama;
    Byte data;
    for (Byte i = 0; i < TAM; i++) {
        if (buffer_primario.datos[i].ack) {
            b_P_limpiar_data(buffer_primario.datos[i].num_trama);
        } else if (buffer_primario.datos[i].nak) {
            data = buffer_primario.datos[i].data;
            num_trama = buffer_primario.datos[i].num_trama;
            b_P_limpiar_data(num_trama);
            b_P_actualizar();
            b_P_set(num_trama, data);
        }
    }
}


void clockTimer(int tick, int tickTrama) {
    int time = 0;
    enviar_inicio();
    std::this_thread::sleep_for(std::chrono::milliseconds(tick));
    while (transmitiendo) {
        agregar_datos();
        temporizador(time);
        enviar_datos(time, tick, tickTrama);
        aplicar_ack();
        system("clear");
        b_P_mostrar();
        std::this_thread::sleep_for(std::chrono::milliseconds(tick));
    }
    enviar_fin();
    system("clear");
    printf("[>] Paquete enviado: %s\n", buffer);
}

Byte calcularCRS(Byte data) {
    // Inicializar los bits de paridad
    Byte p1 = 0, p2 = 0, p4 = 0, p8 = 0;

    // Bits de datos (D1-D8)
    Byte d1 = (data >> 0) & 1; // Bit 0
    Byte d2 = (data >> 1) & 1; // Bit 1
    Byte d3 = (data >> 2) & 1; // Bit 2
    Byte d4 = (data >> 3) & 1; // Bit 3
    Byte d5 = (data >> 4) & 1; // Bit 4
    Byte d6 = (data >> 5) & 1; // Bit 5
    Byte d7 = (data >> 6) & 1; // Bit 6
    Byte d8 = (data >> 7) & 1; // Bit 7

    // Calcular los bits de paridad
    p1 = d1 ^ d2 ^ d4 ^ d5 ^ d7; // p1 cubre bits 1, 2, 4, 5, 7
    p2 = d1 ^ d3 ^ d4 ^ d6 ^ d7; // p2 cubre bits 1, 3, 4, 6, 7
    p4 = d2 ^ d3 ^ d4 ^ d8;      // p4 cubre bits 2, 3, 4, 8
    p8 = d5 ^ d6 ^ d7 ^ d8;      // p8 cubre bits 5, 6, 7, 8

    // Combinar los bits de datos y los bits de paridad
    Byte hammingCode = 0;
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

void mostrarTrama(Trama trama){
    printf("[+] Trama recibida: ID=%d, Data=%c, CRS=%d\n", trama.id, trama.data, trama.crc);
}

Trama recibirTrama() {
    // Crear un socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
    }

    // Definir la dirección del servidor
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_RX); // Puerto del servidor
    serv_addr.sin_addr.s_addr = inet_addr(IP_RX); // Dirección del servidor (localhost)

    // Enlazar el socket a la dirección del servidor
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("[!] Error al enlazar el socket\n");
    }

    // Recibir la trama
    Trama trama;
    ssize_t bytes_received = recvfrom(sockfd, &trama, sizeof(trama), 0, NULL, NULL);
    if (bytes_received < 0) {
        printf("[!] Error al recibir la trama\n");
    }

    // Cerrar el socket
    close(sockfd);

    return trama;
}

void enviarTrama(Trama trama) {
    // Crear un socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("[!] Error al crear el socket\n");
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
        printf("[!] Error al enviar la trama\n");
    } else {
        printf("[+] Trama enviada: ID=%d, Data=%c, CRC=%d\n", trama.id, trama.data, trama.crc);
    }

    // Cerrar el socket
    close(sockfd);
}


int ack_tipo(Trama trama){
    if (trama.id==0 && trama.data==0 && trama.crc==60){
        // Trama de inicio de comunicación
        return 0;
    } else if (trama.id==0 && trama.data==0 && trama.crc==195) {
        // Trama de fin de comunicación
        return 1;
    } else if (trama.data==0 && trama.crc==127) {
        // Trama de ACK
        return 2;
    } else if (trama.data==0) {
        // Trama de NAK
        return 3;
    } else {
        // Trama de datos
        return 4;
    }
}

void procesar_ACK(Trama trama){
    Byte id = trama.id;
    for (Byte i = 0; i < TAM; i++) {
        if (buffer_primario.datos[i].num_trama == id) {
            buffer_primario.datos[i].ack = true;
            buffer_primario.datos[i].clock = false;
            buffer_primario.datos[i].ticks = 0;
            break;
        }
    }
}

void procesar_NAK(Trama trama){
    Byte id = trama.id;
    for (Byte i = 0; i < TAM; i++) {
        if (buffer_primario.datos[i].num_trama == id) {
            buffer_primario.datos[i].nak = true;
            buffer_primario.datos[i].clock = false;
            buffer_primario.datos[i].ticks = 0;
            break;
        }
    }
}


void recibir_ACK(){
    while (true) {
        if (transmitiendo) {
            Trama trama = recibirTrama();
            mostrarTrama(trama);
            Byte tipo = ack_tipo(trama);
            if (tipo == 2) {
                // ACK
                procesar_ACK(trama);
            } else if (tipo == 3) {
                // NAK
                procesar_NAK(trama);
            }
        }
    }
}

void enviar_inicio(){
    Trama trama;
    trama.id = 0;
    trama.data = 0;
    trama.crc = 60;
    enviarTrama(trama);
}

void enviar_fin(){
    Trama trama;
    trama.id = 0;
    trama.data = 0;
    trama.crc = 195;
    enviarTrama(trama);
}

using namespace std;
int main (int argc, char *argv[]) {
    system("clear");
    thread t1(recibir_ACK);
    t1.detach();
    while(true){
        id_unica_reset();
        b_S_limpiar();
        b_P_limpiar(); 
        b_S_ingreso();
        system("clear");
        transmitiendo = true;
        clockTimer(25, 2);
    }
    return 0;
}
