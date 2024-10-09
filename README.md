# Data-Network-Course
Universidad Tecnológica Nacional - Facultad Regional Mendoza
Redes de Datos - 3ro año - 2024

# Tabla de Contenidos
- [Introducción](#introducción)
- [Ejecución](#Ejecución)
    + [Capa de Enlace](#Capa-de-Enlace)
        + [Configuración](#Cliente-Servidor-Configuración)

# Introducción
Este repositorio contiene los trabajos prácticos de la materia Redes de Datos de la carrera de Ingeniería en Sistemas de Información de la UTN FRM.

# Ejecución

### Capa de Enlace
El modelo de Capa de Enlace de Data-Network-Course se encuentra en la carpeta Link-Layer.

Para ejecutar dicho modelo, el cual se basa en cliente-servidor, se debe clonar el repositorio, compilar y ejecutar los archivos de Cliente y Servidor.

* `git clone https://github.com/CondoriJP/Data-Network-Course/tree/main/Link-Layer` clone el repositorio
* `gcc Link-Layer/Cliente.cpp -o Cliente` compilar el archivo Cliente
* `gcc Link-Layer/Servidor.cpp -o Servidor` compilar el archivo Servidor
* `./Servidor` ejecutar el servidor
* `./Cliente` ejecutar el cliente

(Solo valido para sistemas operativos Linux)

### Configuración
Para configurar el modelo de Capa de Enlace, se debe modificar el archivo `Link-Layer/Cliente.cpp` y `Link-Layer/Servidor.cpp` con los siguientes parámetros:

Cliente y Servidor:

`#define PORT_TX 8080`          Puerto del Servidor
`#define IP_TX "127.0.0.1"`     IP de la máquina del Servidor
`#define PORT_RX 8081`          Puerto del Cliente
`#define IP_RX "127.0.0.1"`     IP de la máquina del Cliente


Cliente:

`#define MAX`                   Tamaño del buffer de datos
`#define TAM`                   Tamaño de la ventana
`clockTimer(RETARDO, TICKS)`    Retardo en milisegundos y cantidad de TICKS de RETARDO para el time out

Servidor:
`agregarDato(RETARDO)`          Retardo en milisegundos para la recepción de datos
