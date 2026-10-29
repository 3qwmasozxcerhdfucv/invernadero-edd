#include <iostream>
#include <string>
#include <sstream>
#include <limits>
#include <ctime>
#include <algorithm>

using namespace std;

// ================= ESTRUCTURAS (nodos enlazados) =================
struct LecturaSensor {
    int id;
    string zona;
    float temperatura;
    float humedad;
    string estadoRiego;
    LecturaSensor* siguiente;   // enlace al siguiente sensor
};

struct SolicitudRiego {
    int correlativo;
    int idSensor;
    int duracion;
    SolicitudRiego* siguiente;  // enlace a la siguiente solicitud
};

struct RegistroHistorial {
    int codigoEvento;
    string detalle;
    string hora;
    RegistroHistorial* siguiente; // enlace al evento anterior
};

// ================= PUNTEROS DE ACCESO =================
LecturaSensor* inicio = NULL;     // Lista: primer sensor
SolicitudRiego* frente = NULL;    // Cola: solicitud que se atiende primero
SolicitudRiego* final = NULL;     // Cola: ultima solicitud (aqui se encola)
RegistroHistorial* cima = NULL;   // Pila: ultimo evento registrado

int contadorEventos = 1;
int contadorSolicitudes = 1;

// ================= FUNCIONES AUXILIARES =================

// Convierte un entero a texto (compatible con C++98 / Dev-C++)
string aTexto(int numero) {
    ostringstream ss;
    ss << numero;
    return ss.str();
}

// Devuelve la fecha y hora actual como texto, sin el salto de linea final
string obtenerHoraActual() {
    time_t ahora = time(0);
    string fechaHora = ctime(&ahora);
    if (!fechaHora.empty()) fechaHora.erase(fechaHora.size() - 1);
    return fechaHora;
}

// Lee un entero validando que la entrada sea numerica (RNF-05)
int leerEntero(const string& mensaje) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cout << "Error: ingrese un numero entero valido.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Lee un decimal validando que la entrada sea numerica
float leerDecimal(const string& mensaje) {
    float valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cout << "Error: ingrese un numero valido.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Lee la humedad y la vuelve a pedir hasta que este entre 0 y 100
float leerHumedad(const string& mensaje) {
    float h = leerDecimal(mensaje);
    while (h < 0 || h > 100) {
        cout << "Error: Humedad fuera de rango (0-100).\n";
        h = leerDecimal(mensaje);
    }
    return h;
}

// ================= PILA (Historial) =================

// Apilar: agrega un evento en la cima de la pila
void apilarHistorial(string detalle) {
    RegistroHistorial* nuevoNodo = new RegistroHistorial;
    nuevoNodo->codigoEvento = contadorEventos++;
    nuevoNodo->detalle = detalle;
    nuevoNodo->hora = obtenerHoraActual();
    nuevoNodo->siguiente = cima;   // el nuevo nodo apunta a la antigua cima
    cima = nuevoNodo;              // la cima pasa a ser el nuevo nodo
}

// Muestra el historial desde la cima (mas reciente) hasta la base
void mostrarHistorial() {
    if (cima == NULL) {
        cout << "El historial esta vacio.\n";
        return;
    }
    RegistroHistorial* actual = cima;
    cout << "\n--- HISTORIAL DE EVENTOS ---\n";
    while (actual != NULL) {
        cout << "[" << actual->hora << "] Evt " << actual->codigoEvento
             << ": " << actual->detalle << "\n";
        actual = actual->siguiente;
    }
}

// ================= COLA (Riego) =================

// Encolar: agrega una solicitud de riego al final de la cola
void encolarRiego(int idSensor) {
    SolicitudRiego* nuevoNodo = new SolicitudRiego;
    nuevoNodo->correlativo = contadorSolicitudes++;
    nuevoNodo->idSensor = idSensor;
    nuevoNodo->duracion = 15;          // 15 minutos por defecto
    nuevoNodo->siguiente = NULL;

    if (frente == NULL) {              // cola vacia
        frente = nuevoNodo;
        final = nuevoNodo;
    } else {
        final->siguiente = nuevoNodo;  // enlaza el ultimo con el nuevo
        final = nuevoNodo;
    }
    cout << "Solicitud de riego encolada para el Sensor ID: " << idSensor << "\n";
}

// Desencolar: atiende la solicitud del frente y la registra en el historial
void atenderSolicitudRiego() {
    if (frente == NULL) {
        cout << "No hay procesos de riego pendientes.\n";
        return;
    }
    SolicitudRiego* temp = frente;
    int idAtendido = temp->idSensor;

    frente = frente->siguiente;        // avanza el frente
    if (frente == NULL) {
        final = NULL;                  // la cola quedo vacia
    }
    delete temp;                       // libera la memoria del nodo atendido

    string detalle = "Riego completado en Sensor ID: " + aTexto(idAtendido);
    apilarHistorial(detalle);
    cout << "Proceso atendido: " << detalle << "\n";
}

// Muestra las solicitudes pendientes en orden de llegada
void mostrarColaRiego() {
    if (frente == NULL) {
        cout << "La cola de riego esta vacia.\n";
        return;
    }
    SolicitudRiego* actual = frente;
    cout << "\n--- COLA DE RIEGO ---\n";
    while (actual != NULL) {
        cout << "Correlativo: " << actual->correlativo
             << " | Sensor ID: " << actual->idSensor
             << " | Duracion: " << actual->duracion << " min\n";
        actual = actual->siguiente;
    }
}

// ================= LISTA (Sensores) =================

// Insertar: agrega un sensor al inicio de la lista
void insertarSensor(int id, string zona, float temp, float hum) {
    LecturaSensor* nuevoNodo = new LecturaSensor;
    nuevoNodo->id = id;
    nuevoNodo->zona = zona;
    nuevoNodo->temperatura = temp;
    nuevoNodo->humedad = hum;
    nuevoNodo->estadoRiego = (hum < 30.0) ? "Necesita riego" : "No necesita riego";

    nuevoNodo->siguiente = inicio;     // el nuevo nodo apunta al antiguo primero
    inicio = nuevoNodo;                // inicio pasa a ser el nuevo nodo

    apilarHistorial("Sensor agregado: ID " + aTexto(id));
    cout << "Sensor insertado correctamente.\n";
}

// Recorre la lista y muestra todos los sensores
void mostrarSensores() {
    if (inicio == NULL) {
        cout << "No hay sensores registrados.\n";
        return;
    }
    LecturaSensor* actual = inicio;
    cout << "\n--- SENSORES REGISTRADOS ---\n";
    while (actual != NULL) {
        cout << "ID: " << actual->id << " | Zona: " << actual->zona
             << " | Temp: " << actual->temperatura << "C | Hum: "
             << actual->humedad << "% | Estado: " << actual->estadoRiego << "\n";
        actual = actual->siguiente;
    }
}

// Busqueda lineal de un sensor por su ID
void buscarSensor(int id) {
    LecturaSensor* actual = inicio;
    while (actual != NULL) {
        if (actual->id == id) {
            cout << "Sensor encontrado. ID: " << actual->id
                 << " | Temp: " << actual->temperatura
                 << " | Hum: " << actual->humedad
                 << " | Estado: " << actual->estadoRiego << "\n";
            return;
        }
        actual = actual->siguiente;
    }
    cout << "Sensor no encontrado.\n";
}

// Actualiza temperatura y humedad de un sensor y recalcula su estado de riego
void actualizarSensor(int id, float nuevaTemp, float nuevaHum) {
    LecturaSensor* actual = inicio;
    while (actual != NULL) {
        if (actual->id == id) {
            actual->temperatura = nuevaTemp;
            actual->humedad = nuevaHum;
            actual->estadoRiego = (nuevaHum < 30.0) ? "Necesita riego" : "No necesita riego";
            apilarHistorial("Sensor actualizado: ID " + aTexto(id));
            cout << "Datos actualizados correctamente.\n";
            return;
        }
        actual = actual->siguiente;
    }
    cout << "Sensor no encontrado.\n";
}

// Revisa la lista y encola una solicitud por cada sensor con humedad baja
void revisarHumedad() {
    LecturaSensor* actual = inicio;
    int alertas = 0;
    while (actual != NULL) {
        if (actual->humedad < 30.0) {
            encolarRiego(actual->id);
            alertas++;
        }
        actual = actual->siguiente;
    }
    if (alertas == 0) cout << "No hay sensores que requieran riego actualmente.\n";
}

// Ordena la lista por humedad (burbuja, de menor a mayor)
void ordenarSensoresPorHumedad() {
    if (inicio == NULL || inicio->siguiente == NULL) {
        cout << "No hay suficientes sensores para ordenar.\n";
        return;
    }
    bool intercambiado;
    LecturaSensor* actual;
    LecturaSensor* ultimo = NULL;
    do {
        intercambiado = false;
        actual = inicio;
        while (actual->siguiente != ultimo) {
            if (actual->humedad > actual->siguiente->humedad) {
                // Se intercambian los valores de los nodos
                swap(actual->id, actual->siguiente->id);
                swap(actual->zona, actual->siguiente->zona);
                swap(actual->temperatura, actual->siguiente->temperatura);
                swap(actual->humedad, actual->siguiente->humedad);
                swap(actual->estadoRiego, actual->siguiente->estadoRiego);
                intercambiado = true;
            }
            actual = actual->siguiente;
        }
        ultimo = actual;
    } while (intercambiado);
    cout << "Sensores ordenados por humedad.\n";
    apilarHistorial("Lista de sensores ordenada");
}

// Libera toda la memoria dinamica antes de salir del programa
void liberarMemoria() {
    while (inicio != NULL) {
        LecturaSensor* temp = inicio;
        inicio = inicio->siguiente;
        delete temp;
    }
    while (frente != NULL) {
        SolicitudRiego* temp = frente;
        frente = frente->siguiente;
        delete temp;
    }
    final = NULL;
    while (cima != NULL) {
        RegistroHistorial* temp = cima;
        cima = cima->siguiente;
        delete temp;
    }
}

// ================= MENU PRINCIPAL =================
int main() {
    int opcion;
    do {
        cout << "\n=========================================\n";
        cout << "          INVERNADERO INTELIGENTE        \n";
        cout << "=========================================\n";
        cout << "1. Agregar sensor\n";
        cout << "2. Mostrar sensores\n";
        cout << "3. Buscar sensor\n";
        cout << "4. Actualizar sensor\n";
        cout << "5. Ordenar sensores por humedad\n";
        cout << "6. Revisar humedad y generar riego\n";
        cout << "7. Mostrar cola de riego\n";
        cout << "8. Atender solicitud de riego\n";
        cout << "9. Mostrar historial\n";
        cout << "0. Salir\n";
        cout << "Seleccione una opcion: ";

        if (!(cin >> opcion)) {
            cout << "Error: Entrada invalida.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            opcion = -1;
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (opcion) {
            case 1: {
                int id = leerEntero("ID del sensor: ");
                string zona;
                cout << "Zona (sin espacios): ";
                cin >> zona;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                float temp = leerDecimal("Temperatura (C): ");
                float hum = leerHumedad("Humedad (%): ");
                insertarSensor(id, zona, temp, hum);
                break;
            }
            case 2: mostrarSensores(); break;
            case 3: {
                int id = leerEntero("ID a buscar: ");
                buscarSensor(id);
                break;
            }
            case 4: {
                int id = leerEntero("ID a actualizar: ");
                float temp = leerDecimal("Nueva temperatura: ");
                float hum = leerHumedad("Nueva humedad: ");
                actualizarSensor(id, temp, hum);
                break;
            }
            case 5: ordenarSensoresPorHumedad(); break;
            case 6: revisarHumedad(); break;
            case 7: mostrarColaRiego(); break;
            case 8: atenderSolicitudRiego(); break;
            case 9: mostrarHistorial(); break;
            case 0: cout << "Saliendo...\n"; break;
            default: cout << "Opcion invalida.\n";
        }
    } while (opcion != 0);

    liberarMemoria();
    return 0;
}
