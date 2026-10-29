#include <iostream>
#include <string>
#include <limits>
#include <ctime>

using namespace std;

// --- ESTRUCTURAS ---
struct LecturaSensor {
    int id;
    string zona;
    float temperatura;
    float humedad;
    string estadoRiego;
    LecturaSensor* siguiente;
};

struct SolicitudRiego {
    int correlativo;
    int idSensor;
    int duracion;
    SolicitudRiego* siguiente;
};

struct RegistroHistorial {
    int codigoEvento;
    string detalle;
    string hora;
    RegistroHistorial* siguiente;
};

// --- PUNTEROS GLOBALES ---
LecturaSensor* inicio = NULL;
SolicitudRiego* frente = NULL;
SolicitudRiego* final = NULL;
RegistroHistorial* cima = NULL;

int contadorEventos = 1;
int contadorSolicitudes = 1;

// --- FUNCIONES AUXILIARES ---
string obtenerHoraActual() {
    time_t ahora = time(0);
    string fechaHora = ctime(&ahora);
    fechaHora.pop_back(); // Eliminar salto de linea
    return fechaHora;
}

void apilarHistorial(string detalle) {
    RegistroHistorial* nuevoNodo = new RegistroHistorial;
    nuevoNodo->codigoEvento = contadorEventos++;
    nuevoNodo->detalle = detalle;
    nuevoNodo->hora = obtenerHoraActual();
    nuevoNodo->siguiente = cima;
    cima = nuevoNodo;
}

// --- PILAS (Historial) ---
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

// --- COLAS (Riego) ---
void encolarRiego(int idSensor) {
    SolicitudRiego* nuevoNodo = new SolicitudRiego;
    nuevoNodo->correlativo = contadorSolicitudes++;
    nuevoNodo->idSensor = idSensor;
    nuevoNodo->duracion = 15; // 15 minutos por defecto
    nuevoNodo->siguiente = NULL;

    if (frente == NULL) {
        frente = nuevoNodo;
        final = nuevoNodo;
    } else {
        final->siguiente = nuevoNodo;
        final = nuevoNodo;
    }
    cout << "Solicitud de riego encolada para el Sensor ID: " << idSensor << "\n";
}

void atenderSolicitudRiego() {
    if (frente == NULL) {
        cout << "No hay procesos de riego pendientes.\n";
        return;
    }
    SolicitudRiego* temp = frente;
    int idAtendido = temp->idSensor;
    
    frente = frente->siguiente;
    if (frente == NULL) {
        final = NULL;
    }
    delete temp;
    
    string detalle = "Riego completado en Sensor ID: " + to_string(idAtendido);
    apilarHistorial(detalle);
    cout << "Proceso atendido: " << detalle << "\n";
}

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

// --- LISTAS (Sensores) ---
void insertarSensor(int id, string zona, float temp, float hum) {
    LecturaSensor* nuevoNodo = new LecturaSensor;
    nuevoNodo->id = id;
    nuevoNodo->zona = zona;
    nuevoNodo->temperatura = temp;
    nuevoNodo->humedad = hum;
    nuevoNodo->estadoRiego = (hum < 30.0) ? "Necesita riego" : "No necesita riego";
    
    nuevoNodo->siguiente = inicio;
    inicio = nuevoNodo;
    
    apilarHistorial("Sensor agregado: ID " + to_string(id));
    cout << "Sensor insertado correctamente.\n";
}

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

void actualizarSensor(int id, float nuevaTemp, float nuevaHum) {
    LecturaSensor* actual = inicio;
    while (actual != NULL) {
        if (actual->id == id) {
            actual->temperatura = nuevaTemp;
            actual->humedad = nuevaHum;
            actual->estadoRiego = (nuevaHum < 30.0) ? "Necesita riego" : "No necesita riego";
            apilarHistorial("Sensor actualizado: ID " + to_string(id));
            cout << "Datos actualizados correctamente.\n";
            return;
        }
        actual = actual->siguiente;
    }
    cout << "Sensor no encontrado.\n";
}

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

void ordenarSensoresPorHumedad() {
    if (inicio == NULL || inicio->siguiente == NULL) return;
    bool intercambiado;
    LecturaSensor* actual;
    LecturaSensor* ultimo = NULL;
    do {
        intercambiado = false;
        actual = inicio;
        while (actual->siguiente != ultimo) {
            if (actual->humedad > actual->siguiente->humedad) {
                // Intercambio de valores (no de nodos para simplificar)
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

// --- MENU PRINCIPAL ---
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
            continue;
        }

        switch (opcion) {
            case 1: {
                int id; string zona; float temp, hum;
                cout << "ID del sensor: "; cin >> id;
                cout << "Zona (sin espacios): "; cin >> zona;
                cout << "Temperatura (C): "; cin >> temp;
                cout << "Humedad (%): "; cin >> hum;
                if (hum < 0 || hum > 100) {
                    cout << "Error: Humedad fuera de rango (0-100).\n";
                } else {
                    insertarSensor(id, zona, temp, hum);
                }
                break;
            }
            case 2: mostrarSensores(); break;
            case 3: {
                int id; cout << "ID a buscar: "; cin >> id;
                buscarSensor(id);
                break;
            }
            case 4: {
                int id; float temp, hum;
                cout << "ID a actualizar: "; cin >> id;
                cout << "Nueva temperatura: "; cin >> temp;
                cout << "Nueva humedad: "; cin >> hum;
                if (hum < 0 || hum > 100) cout << "Error: Humedad fuera de rango.\n";
                else actualizarSensor(id, temp, hum);
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

    return 0;
}
