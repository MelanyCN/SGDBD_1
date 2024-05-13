#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <regex>

using namespace std;

class DiscoDuro {
private:
    int cant_platos;
    int cant_pistas;
    int cant_sectores;
    int capacidad_sector;

    unsigned long long capacidad_total;
    int capacidad_utilizada;

    string baseDirectory = "/home/melany/prueba_vs/sectores"; 

public:
    DiscoDuro(){
        cant_platos = 8;
        cant_pistas = 16384;
        cant_sectores = 128;
        capacidad_sector = 4096;

        capacidad_total = 8LL * 16384 * 128 * 4096 * 2;
        capacidad_utilizada = 0;

        inicializarDisco();
    }

    DiscoDuro(int _cant_platos, int _cant_pistas, int _cant_sectores, int _capacidad_sector){
        cant_platos = _cant_platos;
        cant_pistas = _cant_pistas;
        cant_sectores = _cant_sectores;
        capacidad_sector = _capacidad_sector;

        capacidad_total = _cant_platos * 2 * _cant_pistas * _cant_sectores * _capacidad_sector;
        capacidad_utilizada = 0;

        inicializarDisco();
    }

    void inicializarDisco() {
        std::filesystem::create_directories(baseDirectory); 

        for (int plato = 0; plato < cant_platos; plato++) {
            for (int superficie = 0; superficie < 2; superficie++) {
                for (int pista = 0; pista < cant_pistas; pista++) {
                    for (int sector = 0; sector < cant_sectores; sector++) {
                        ostringstream oss;
                        oss << baseDirectory << "/plato" << plato << "_superficie" << superficie
                            << "_pista" << pista << "_sector" << sector << ".txt";
                        std::ofstream file(oss.str());
                        if (!file) {
                            std::cerr << "Error al crear el archivo de sector: " << oss.str() << endl;
                        }
                        file.close();
                    }
                }
            }
        }
    }
    
    void almacenarRegistroEnSector(const string& registro, int sectorIndex) {
        std::ostringstream oss;
        oss << baseDirectory << "/plato" << (sectorIndex / (cant_sectores * 2 * cant_pistas)) 
            << "_superficie" << ((sectorIndex / (cant_sectores * cant_pistas)) % 2) 
            << "_pista" << ((sectorIndex / cant_sectores) % cant_pistas) 
            << "_sector" << (sectorIndex % cant_sectores) << ".txt";
        
        std::ofstream file(oss.str(), std::ios::app); 
        if (!file) {
            std::cerr << "Error al abrir el archivo de sector: " << oss.str() << endl;
            return;
        }
        file << registro << endl;
        file.close();
    }

    int getTotalSectors() const {
        return cant_platos * 2 * cant_pistas * cant_sectores; // 2 superficies por plato
    }
    
    void almacenarRegistro(string registro) {
        capacidad_utilizada = capacidad_utilizada + registro.size();
    }

    int espacioLibre() {
        return capacidad_total - capacidad_utilizada;
    }

    unsigned long long getCapacidadTotal() {
        return capacidad_total;
    }

    int getCapacidadDelSector() {
        return capacidad_sector;
    }
};

class BaseDeDatos {
private:
    DiscoDuro *disco;
    ofstream esquema;
    int sectorIndex = 0;

public:
    BaseDeDatos(DiscoDuro *disco) : disco(disco) {}

    /*
    int calcularTamanoDelRegistro(const string& esquema) {
        ifstream file("esquema.txt")
        string token;
        int numInts = 0; 
        getline(iss, token, '#');  // nombre de la relacion

        while (getline(iss, token, '#')) {
            string nombreColumna = token; 
            if (getline(iss, token, '#')) { 
                if (token == "int") {
                    numInts++;
                }
            }
        }

        int tamanoRegistro = numInts * 4; // int ocupa 4 bytes
        return tamanoRegistro;
    } 
    */

    string obtenerTipoDato(const string& palabra) {
        string tipo_dato;
        do {
            cout << palabra << ": ";
            cin >> tipo_dato;
            if (tipo_dato != "int" && tipo_dato != "string" && tipo_dato != "float") {
                cout << "Tipo de dato ingresado incorrectamente.\n";
            }
        } while (tipo_dato != "int" && tipo_dato != "string" && tipo_dato != "float");
        return tipo_dato;
    }

    void procesarEsquema(const string& linea) {
        stringstream ss(linea);
        string palabra;
        string tipo_dato;

        cout << "Nombre de la relacion: ";
        string relacion;
        cin >> relacion;
        relacion = relacion + "#";

        cout << "Tipo de dato (int, string, float): \n";
        while (getline(ss, palabra, ',')) {
            tipo_dato = obtenerTipoDato(palabra);
            relacion += palabra + "#" + tipo_dato + "#";
        }

        escribirEsquema(relacion);
    }

    void cargarDatosDesdeArchivos(string archivoCSV){
        ifstream file(archivoCSV);
        string linea;

        // PARA EL ESQUEMA ------------------------------------ 
        esquema.open("esquema.txt");
        if (getline(file, linea)) {
            while (!linea.empty() && isspace(linea.back())) {
                linea.pop_back();
            }
            linea = linea + ",";

            procesarEsquema(linea);
        }

        // PARA LA RELACION (por defecto del csv) ------------
        while (getline(file, linea)) {
            string registro;
            parseCSVLine(linea, registro);
            disco->almacenarRegistroEnSector(registro, sectorIndex++); 
            if (sectorIndex >= disco->getTotalSectors()) {
                sectorIndex = 0;
            }
        }

        file.close();
    }

    void parseCSVLine(const string& linea, string& registro) {
        stringstream ss(linea);
        string item;

        while (getline(ss, item, ',')) {
            if (item.front() == '"' && item.back() == '"') { 
                item = item.substr(1, item.size() - 2);
            }
            registro += item + "#";
        }

        if (!registro.empty()) {
            registro.pop_back();  // Eliminar el último '#'
        }
    }

    /*

    void agregarRegistro(){
        string query;
        getline(cin, query);

        string palabra;
        string registro;
        string linea;

        cout << "Nombre de la tabla a agregar: ";
        string nombre_relacion;
        cin >> nombre_relacion;
        nombre_relacion = nombre_relacion + ".txt";
        ifstream relacion(nombre_relacion);
        ifstream esquema_lec("esquema.txt");

        if (relacion.is_open()) { // la relacion existe
            if (getline(esquema_lec, linea)) {
                //string linea_relacion;
                while (!linea.empty() && isspace(linea.back())) {
                    linea.pop_back();
                }
                linea = linea + ",";

                procesarEsquema(linea);
            }
            ifstream relacion(esquema);
            while (getline(ss, palabra, ',')) {
                registro = obtenerRegistro(palabra);
                relacion += palabra + "#" + registro + "#";
            }
            relacion.close(); // Cerrar el archivo después de usarlo
        } else {
            cout << "Error: No existe la relacion." << endl;
        }


        cout << "Tipo de dato (int, string, float): \n";
        while (getline(ss, palabra, ',')) {
            tipo_dato = obtenerTipoDato(palabra);
            relacion += palabra + "#" + tipo_dato + "#";
        }

        escribirEsquema(relacion);
    }
    */

    void cerrarArchivoSalida() {
        esquema.close();
    }

    void escribirEsquema(string palabra) {
        esquema << palabra << endl;
        //disco->almacenarRegistro(palabra);
    }

    void evaluarQuery(string query){
        //PASO 1 : lee el esquema 
        //esquema.open("esquema.txt");
        cout << "-----------------------------------------------------------------------------" << endl; 
        regex patron("&\\s+(SELECT)\\s+((\\*)|([a-zA-Z0-9_]+))\\s+(FROM)\\s+([a-zA-Z0-9_]+)(?:\\s+(WHERE)\\s+([a-zA-Z0-9_]+)\\s*(=|<|>|<=|>=)\\s*([0-9]+))?\\s*\\|\\s*([a-zA-Z0-9_]+)\\s*(#)+");
        
        string linea;   // para iterar en las lineas del archivo
        string palabra; 
        
        stringstream consulta(query); 

        //PASO 2 : chequear la condicion
        smatch matches;
        
        if (regex_match(query, matches, patron)) {
            ifstream file_esquema("esquema.txt");

            while (getline(file_esquema, linea)) {
                stringstream registro(linea);
                int pos=0;

                while (getline(registro, palabra, '#')) {     // esquema  
                    cout << matches[4] << endl;
                    //cout << "-------------------------------------------------------------------" << endl;  
                    string nombreTabla;  
                    
                    if (pos==0 && palabra == matches[6]){
                        nombreTabla = matches[6];
                        nombreTabla = nombreTabla + ".txt";
                        continue;

                    } else if (pos!=0 && pos%2==0 && palabra == matches[2]){
                        ifstream tabla(nombreTabla);
                        string lineaR;
                        string palabraR;
                        while (getline(tabla, lineaR)){
                            stringstream rt(lineaR);
                            int contador = 0;
                            while (getline(rt, palabraR, '#')){
                                if (contador == pos/2)
                                    cout << palabraR << endl;
                                contador ++;
                            }

                        }
                        tabla.close();
                    } 
                    pos = pos+1;
                } 
            }
            file_esquema.close();

        } 
    }

    int capacidadTotalDisco() {
        return disco->getCapacidadTotal();
    }

    int capacidadUtilizadaDeSector() {
        return disco->getCapacidadDelSector();
    }

    int espacioLibreDisco() {
        return disco->espacioLibre();
    }
};

int main() {

    string inicioDB;
    /* 
    // Determinar tamaño del disco con datos de entrada
    cout << "Cantidad de platos: \n";
    int cant_discos; cin >> cant_discos; // cantidad de platos 2
    cout << "Cantidad de pistas: \n";
    int cant_pistas; cin >> cant_pistas;
    cout << "Cantidad de sectores: \n";
    int cant_sectores; cin >> cant_sectores;
    cout << "Capacidad del sector: \n";
    int capacidad_sector; cin >> capacidad_sector;
    DiscoDuro *disco = new DiscoDuro(cant_discos, cant_pistas, cant_sectores, capacidad_sector);
    */

    DiscoDuro *disco = new DiscoDuro(1,2,15,10);
    cout << "inicia: ";
    inicioDB = "MEGATRON3000";
    cout << "Welcome TO MEGATRON 3000!" << endl;
    BaseDeDatos *baseDeDatos = new BaseDeDatos(disco);

    cout << "Se cargo la base de datos existente (csv)...";

    cout << "Nombre de la relacion de la bd existente: ";
    string nombre_relacion;
    cin >> nombre_relacion;
    baseDeDatos->cargarDatosDesdeArchivos("Titanic_original.csv");

    while (true){
        if (inicioDB == "MEGATRON3000"){
            int opcion; 
            cout << "1) Crear Relacion \n"; 
            cout << "2) Agregar registro \n";
            cout << "3) Realizar consulta \n";
            cout << "4) Capacidad total del disco \n";
            cin >> opcion;

            if (opcion == 1){
                cout << "Nombre de la relacion: ";
                cin >> nombre_relacion;

            } else if (opcion == 2){
                //baseDeDatos->agregarRegistro();

            } else if (opcion == 3){
                string query;
                getline(cin, query);

                baseDeDatos->evaluarQuery(query);
            } else if (opcion == 4){
                unsigned long long capacidad_disco = disco->getCapacidadTotal(); 
                cout << "Capacidad del disco: "<< capacidad_disco << endl;
            }
        
            baseDeDatos->cerrarArchivoSalida();

            delete baseDeDatos;
        } 

    }

    delete disco;

    return 0;
}

