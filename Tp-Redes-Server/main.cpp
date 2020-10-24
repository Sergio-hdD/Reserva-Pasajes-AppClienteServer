#include <iostream>
#include <winsock2.h>
#include <string>
#include <conio.h>
#include <clocale>//es para usar � y acento
#include <fstream> //Lib. para trabajar con archivos
#include <ctime> //Lib. para trabajar con fechas / tiempos
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <windns.h>

#define TAMANIO_I  5
#define TAMANIO_J  21

using namespace std;

string nombreArchivo;

void registrarServerLog(string evento, string aRegistrar);


class Server{
public:
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];
    Server()
    {
        WSAStartup(MAKEWORD(2,0), &WSAData);
        server = socket(AF_INET, SOCK_STREAM, 0);

        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(5555);

        bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
        listen(server, 0);

        registrarServerLog("Inicia servidor", "Socket creado. Puerto de escucha:4747");

        cout << "Escuchando para conexiones entrantes." << endl;

        int clientAddrSize = sizeof(clientAddr);
        if((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
        {
            cout << "Cliente conectado!" << endl;
        }
    }

    string Recibir()
    {
        fd_set fds ;
        struct timeval tv ;

        tv.tv_sec = 120 ;
        tv.tv_usec = 0 ;

        FD_ZERO(&fds) ;
        FD_SET(client, &fds) ;

        int n = select (client, &fds, NULL, NULL, &tv) ;

        if (n == 0){
          printf("Timeout..\n");
          // Cerrar sesion
        }
        recv(client, buffer, sizeof(buffer), 0);

        string buff = buffer;

        memset(buffer, 0, sizeof(buffer));
      return buff;
    }

    void Enviar(string respuesta)
    {
        for(int i=0;i<respuesta.length();i++){
            this->buffer[i]= respuesta[i];
        }

        send(client, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
    }

    void CerrarSocket()
    {
        closesocket(client);
        WSACleanup();
        registrarServerLog("Socket cerrado, cliente desconectado", "");
        cout << "Socket cerrado, cliente desconectado." << endl;
    }
};

void crearArchivoButacas();
void gestionarAsiento(Server *&Servidor, string userName, bool reservar);
void liberar(Server *&Servidor, string userName);
string verificarSolicitud_Y_Responder(Server *&Servidor,vector <string> vectorButacas, string userName, bool reservar);
void marcarButacaComoOcupada(vector <string> vectorButacas, int pos_I, int pos_J, string userName);
void marcarButacaComoLiberada(vector <string> vectorButacas, int pos_I, int pos_J, string userName);
string butacaAString(int pos_I, int pos_J);
vector<string> separarPalabrasPuntoYComa(string str);
string checkUser(Server *&Servidor);
void registrarServerLog(string evento, string aRegistrar);
void registrarUserLog(string evento, string aRegistrar);
void crearArchivoUserLog(string usuario);

void iniciarButacas(char butacas[TAMANIO_I][TAMANIO_J]);
void mostrarButacas(vector <string> vectorButacas);
int asignarValorPosI_A_Letra(char letra);

void darFormato_y_GuardarButacasEnArchivo(string nombreArchivo, string titulo,char butacas[TAMANIO_I][TAMANIO_J]);
void guardarEnArchivo(string lineaAGuardar, string nombreArchivo);
bool verificarSiExisteArchivo(string nombreArchivo);
vector <string> leerArchivoGuardarEnVectorString(string nombreArchivo);
void guardarEnArchivoYaFormateada(string lineaAGuardar, string nombreArchivo);
void marcarEnArchivoReservaRealizada(vector <string> vectorButacas);
string traerSoloButacas(vector <string> vectorButacas);

int numeroDeSentencias(string file);
void manejarPeticion(string userName, Server *&Servidor);
void mostrarRegistro(string userName, Server *&Servidor);



/************************************
         MAIN
***********************************/
int main()
{
    setlocale(LC_CTYPE,"Spanish");// Spanish (de la librer�a locale.h) es para usar � y acento
    Server *Servidor = new Server();

    string userName = checkUser(Servidor);
    crearArchivoButacas();

    manejarPeticion(userName, Servidor);

    Servidor->CerrarSocket();

    system("cls");

    main();
}
/************************************
        FIN  MAIN
***********************************/


/***********************************************************************/
void crearArchivoButacas(){
    nombreArchivo = "Registro_de_butacas";
    if(verificarSiExisteArchivo(nombreArchivo)==false){//solo es paracrear un archivo
        string tituloArchivo = ">>> REGISTRO DE BUTACAS VACIAS Y OCUPADA <<<";
        char butacas[TAMANIO_I][TAMANIO_J];
        iniciarButacas(butacas);
        darFormato_y_GuardarButacasEnArchivo(nombreArchivo,tituloArchivo,butacas);
    }
}
/***********************************************************************/

/***********************************************************************/
void gestionarAsiento(Server *&Servidor, string userName, bool reservar){

    vector <string> vectorButacas = leerArchivoGuardarEnVectorString(nombreArchivo);

    Servidor->Enviar(traerSoloButacas(vectorButacas));

    string salir = "false";

    bool salirWhile = false;

    while(!salirWhile){

        vectorButacas = leerArchivoGuardarEnVectorString(nombreArchivo);
        salir = verificarSolicitud_Y_Responder(Servidor,vectorButacas, userName, reservar);

        if(salir=="true"){
            salirWhile=true;
        }
        else if(salir!="true" && salir!="false"){//es porque cuando se desconecta el cliente  cerrando la ventana llegaba vacio
            salir ="true";
            salirWhile = true;
            system("cls");
        }
    }

}
/***********************************************************************/
void liberar(Server *&Servidor, string userName){

}

/***********************************************************************/

/***********************************************************************/
void manejarPeticion(string userName, Server *&Servidor){
    string peticion="";
    bool salir = false;

    while(!salir){

        peticion = Servidor->Recibir();
        cout<<peticion<<endl;
        //Servidor->Enviar("_");//Envio cualquier cosa para que no d� error

        if(peticion=="Registro"){
            mostrarRegistro(userName,Servidor);
        }
        else if(peticion=="Gestionar"){
            string opcion = Servidor->Recibir();
            if(opcion=="ReservarAsiento"){
                gestionarAsiento(Servidor, userName,true);
                opcion = "";
            }
            else if(opcion=="LiberarAsiento"){
                gestionarAsiento(Servidor, userName, false);
                opcion = "";
            }

        }
        peticion="";
    }
}
/***********************************************************************/


/***********************************************************************/
int numeroDeSentencias(std::string archivo){

        int contador = 0;

        fstream file;

        file.open(archivo);

        if(file.is_open()){
            while(!file.eof()){
                string linea = "";
                getline(file, linea);
                contador++;
            }
        }
        file.close();
    return contador;
};
/***********************************************************************/


/***********************************************************************/
void mostrarRegistro(string userName, Server *&Servidor){

    std::string userFile = userName+".log";
    std::string numero = std::to_string(numeroDeSentencias(userFile));
    Servidor->Enviar(numero);
    fstream file;
    file.open(userFile);

    if(file.is_open()){
        for(int i = 0 ; i < stoi(numero); i++){
            string linea = "";
            getline(file, linea);
            Servidor->Enviar(linea);
        }
    }
    file.close();

}
/***********************************************************************/


/***********************************************************************/
string traerSoloButacas(vector <string> vectorButacas){
    string soloButacas;
cout<<endl;
   for(int i=0;i<11;i++){
        if(i>6&&i!=9){
          for(int j=0;j<43;j++){
              if(j>3 && j%2==0 && j<42){
                 soloButacas=soloButacas+vectorButacas[i][j];
              }else if(j==42){
                 soloButacas=soloButacas+vectorButacas[i][j];
              }//if interno
          }//for j
         }//if i>3
   }//for i
  return soloButacas;
}
/***********************************************************************/

/***********************************************************************/
void darFormato_y_GuardarButacasEnArchivo(string nombreArchivo, string titulo,char butacas[TAMANIO_I][TAMANIO_J]){
    string lineaAGuardar;
    guardarEnArchivo(titulo,nombreArchivo);
    for (int i =0; i<TAMANIO_I;i++){
        if(i==2){guardarEnArchivo("-------------------------------------------",nombreArchivo);}
        if(i==4){guardarEnArchivo("===========================================",nombreArchivo);}
        for (int j=0; j<TAMANIO_J;j++){
           if(i==0&&j<1){lineaAGuardar=lineaAGuardar+"  "+butacas[i][j]+" ";
           }else if(j<1){lineaAGuardar=lineaAGuardar+" "+butacas[i][j]+" |";
           }else{lineaAGuardar=lineaAGuardar+butacas[i][j]+" ";}
        }//cierro el for de columna
        guardarEnArchivo(lineaAGuardar,nombreArchivo);
        lineaAGuardar="";

    }//Cierro el for de renglon

}
/***********************************************************************/

/***********************************************************************/
void guardarEnArchivo(string lineaAGuardar, string nombreArchivo){
   bool NO_EsPrimeraCarga = verificarSiExisteArchivo(nombreArchivo);// Si no existe el archivo quiere decir que va a ser la primera cargaga de datos (ya que cuando lo creo cargo datos)
   nombreArchivo= nombreArchivo+".txt";
   ofstream archivo(nombreArchivo.c_str(),ios::out | ios::app);
        if(NO_EsPrimeraCarga==false){//si es la primera carga
          archivo<<"\n"<<lineaAGuardar<<"\n\n"; //pongo t�tulo y salto de linea al final para dejar un regl�n vacio
        }else{//si NO es la primera carga, pongo salto de linea al comienzo
          archivo<<"\n"<<lineaAGuardar;
        }/*sin este if me generaba una linea dem�s, al comienzo del archivo, por el salto de linea. Si pusiese el salto de linea al final gener�a la linea dem�s al final del
         archivo y no tengo forma manejarlo cuando... en ambos casos al mostrar los registros del archivo me mostrar�a uno dem�s en blanco arriba o abajo*/
   archivo.close();
}
/***********************************************************************/

/***********************************************************************/
bool verificarSiExisteArchivo(string nombreArchivo){
    nombreArchivo= nombreArchivo+".txt";
    bool yaExisteArchivo = true;
    ifstream archivo;//ifstream(tipo de variable para abrir un archivo)...  archivo (nombre de la variable)
    archivo.open(nombreArchivo.c_str(),ios::in);// con archivo.open le digo que quiero abrir un archivo y con ios::in le digo que abro para leerlo
    if(archivo.fail())//si hay un error y no se abre el arvhivo
    {
        yaExisteArchivo = false;
    }
    archivo.close();//cerramos archivo
 return yaExisteArchivo;
}
/***********************************************************************/

/***********************************************************************/
vector <string> leerArchivoGuardarEnVectorString(string nombreArchivo)
{
    vector <string> butacasEnSting;
    nombreArchivo = nombreArchivo+".txt";
    ifstream archivo;//ifstream(tipo de variable para abrir un archivo)...  archivo (nombre de la variable)
    string texto;// variable string

    archivo.open(nombreArchivo.c_str(),ios::in);// con archivo.open le digo que quiero abrir un archivo y con ios::in le digo que abro para leerlo

    if(archivo.fail())//si hay un error y no se abre el arvhivo
    {
        cout<<"No se ha podido abrir archivo"<<endl;//se muestra
        exit(1);//sale
    }
    int i=0;
    //si se abre
    while(!archivo.eof())//mientras no sea el final del archivo
    {
           getline(archivo,texto);//Tomo lo que va encontrando en "archivo" y lo copio en "texto"
           butacasEnSting.push_back(texto);//guardo en una posici�n del vector la lines obtenidad del archivo
           i++;
    }
   archivo.close();//cerramos archivo
   return butacasEnSting;
}
/***********************************************************************/

/***********************************************************************/
void guardarEnArchivoYaFormateada(string lineaAGuardar, string nombreArchivo){
   bool NO_EsPrimeraCarga = verificarSiExisteArchivo(nombreArchivo);// Si no existe el archivo quiere decir que va a ser la primera cargaga de datos (ya que cuando lo creo cargo datos)
   nombreArchivo= nombreArchivo+".txt";
   ofstream archivo(nombreArchivo.c_str(),ios::out | ios::app);
        if(NO_EsPrimeraCarga==false){//si es la primera carga
          archivo<<lineaAGuardar; //solo pongo la linea
        }else{//si NO es la primera carga, pongo salto de linea al comienzo
          archivo<<"\n"<<lineaAGuardar;
        }/*sin este if me generaba una linea dem�s, al comienzo del archivo, por el salto de linea. Si pusiese el salto de linea al final gener�a la linea dem�s al final del
         archivo y no tengo forma manejarlo cuando... en ambos casos al mostrar los registros del archivo me mostrar�a uno dem�s en blanco arriba o abajo*/
   archivo.close();
}
/***********************************************************************/

/***********************************************************************/
string verificarSolicitud_Y_Responder(Server *&Servidor,vector <string> vectorButacas, string userName, bool reservar){
    string mensajePeticion = "";
    string mensajeDelCli="";
    char letra = '\0';
    int pos_J = -1;
    int pos_I = -1;
    bool posicionDisponible = false;

    mensajeDelCli = Servidor->Recibir(); //Se recibe la butaca. EJ: B8

    if(mensajeDelCli!="0"){
        letra = mensajeDelCli[0];
        mensajeDelCli.erase(0,1);//Saco la letra que guerd�
        pos_J =atoi(const_cast< char *>(mensajeDelCli.c_str()));
        pos_J = pos_J*2+2; //es por la diferencia que hay entre la posicion de vista en consola y la del archivo
        pos_I = asignarValorPosI_A_Letra(letra);


        if(vectorButacas[pos_I][pos_J]=='O' && reservar){
           Servidor->Enviar("Disponible");//est� disponible

           marcarButacaComoOcupada(vectorButacas, pos_I, pos_J, userName);
           mensajePeticion = Servidor->Recibir();
        }
        else if(vectorButacas[pos_I][pos_J]=='X' && !reservar){
            Servidor->Enviar("Disponible");//est� disponible

            marcarButacaComoLiberada(vectorButacas, pos_I, pos_J, userName);
            mensajePeticion = Servidor->Recibir();
        }
        else{
            Servidor->Enviar("NoDisponible");//est� disponible
            //mostrarButacas(vectorButacas);
            mensajePeticion = Servidor->Recibir();
        }
    }
    else{
        mensajePeticion = Servidor->Recibir();
    }

    return mensajePeticion;
}
/***********************************************************************/


/***********************************************************************/
int asignarValorPosI_A_Letra(char letra){
    int pos_I = -1;

    if(letra=='A' || letra == 'a'){
        pos_I=7;
    }
    else if(letra=='B' || letra == 'b'){
        pos_I=8;
    }
    else {
        pos_I=10;
    }

    return pos_I;
}
/***********************************************************************/


/***********************************************************************/
void iniciarButacas(char butacas[TAMANIO_I][TAMANIO_J]){
    char numeros[20] = {'1','2','3','4','5','6','7','8','9','0','1','2','3','4','5','6','7','8','9','0'};
    char letras[5] = {' ',' ','A','B','C'};
    for (int i =0; i<TAMANIO_I;i++){
        for (int j=1; j<TAMANIO_J;j++){
           if(j<=9){butacas[0][j] = letras[0];}else if(j<=19){butacas[0][j] = '1';}else{butacas[0][j] = '2';}
           butacas[1][j] = numeros[j-1];
           butacas[i][0] = letras[i];
           butacas[i][j] = 'O';
        }
    }
}
/***********************************************************************/


/***********************************************************************/
void mostrarButacas(vector <string> vectorButacas){
    for(int i=0;i<11;i++){
        if(i>3){
         cout<<vectorButacas[i]<<endl;
        }
    }
}
/***********************************************************************/

/**********************************************************************/
void marcarEnArchivoReservaRealizada(vector <string> vectorButacas){
    ofstream archivoAuxiliar;
    archivoAuxiliar.open("auxiliar.txt",ios::out);
    if(archivoAuxiliar.is_open()){
       for(int i=0;i<11;i++){
         archivoAuxiliar<<vectorButacas[i]<<"\n";
       }//Fin for
    }else{
        cout<<"No se pudo abrir el archivo o aun no ha sido creado"<<endl;
    }
    archivoAuxiliar.close();
    remove("Registro_de_butacas.txt");
    rename("auxiliar.txt","Registro_de_butacas.txt");
}
/**********************************************************************/

/**********************************************************************/
string butacaAString(int pos_I, int pos_J){
    char letra = '\0';

    pos_J = pos_J/2-2;
    pos_J++;
    std::string posJ_str = std::to_string(pos_J);

    if(pos_I == 7) letra = 'A';
    else if(pos_I == 8) letra = 'B';
    else letra = 'C';

    string butaca = letra+posJ_str;

    return butaca;
}
/**********************************************************************/

/***********************************************************************/
void marcarButacaComoOcupada(vector <string> vectorButacas, int pos_I, int pos_J, string userName){

        vectorButacas[pos_I][pos_J] = 'X';
        marcarEnArchivoReservaRealizada(vectorButacas);

        string butaca = butacaAString(pos_I, pos_J);

        string reserva = "Reserva_";

        reserva+=butaca;

        registrarUserLog(reserva, userName);
        system("cls");
        mostrarButacas(vectorButacas);
        cout<<"************************************"<<endl;
        cout<<"** Butaca reservada exitosamente. **"<<endl;
        cout<<"************************************"<<endl;
 }

 void marcarButacaComoLiberada(vector <string> vectorButacas, int pos_I, int pos_J, string userName){

        vectorButacas[pos_I][pos_J] = 'O';
        marcarEnArchivoReservaRealizada(vectorButacas);

        string butaca = butacaAString(pos_I, pos_J);

        string libera = "Libera_";
        libera+=butaca;

        registrarUserLog(libera, userName);
        system("cls");
        mostrarButacas(vectorButacas);
        cout<<"************************************"<<endl;
        cout<<"** Butaca liberada exitosamente. **"<<endl;
        cout<<"************************************"<<endl;
 }
/***********************************************************************/

vector<string> separarPalabrasPuntoYComa(string str) {

        int posInit = 0;
        int posFound = 0;
        string splitted;
        char pattern = ';';
        vector<string> resultados;

        while(posFound >= 0){
            posFound = str.find(pattern, posInit);
            splitted = str.substr(posInit, posFound - posInit);
            posInit = posFound + 1;
            resultados.push_back(splitted);
        }

        return resultados;
}

string checkUser(Server *&Servidor)
{
    string usuarioEncontrado = "false";
    char delimitador = ';';
    vector<string> resultados;
    vector<string> userAndPass;
    int contador = 0;
    string loggedUser = "";


    while(contador<3 && usuarioEncontrado == "false"){

        string linea;
        fstream file;

        file.open("users.dat", ios::in);

        userAndPass = separarPalabrasPuntoYComa(Servidor->Recibir());

        if(file.is_open())
        {
            while(!file.eof()){

                getline(file, linea);

                resultados = separarPalabrasPuntoYComa(linea);

                if(resultados[0] == userAndPass[0] && resultados[1] == userAndPass[1]){
                        usuarioEncontrado = "true";
                        registrarServerLog("Usuario autenticado", resultados[0]);
                        crearArchivoUserLog(resultados[0]);
                        registrarUserLog("Inicia sesion", resultados[0]);
                        loggedUser = resultados[0];
                }

            }
        }

        file.close();

        usuarioEncontrado == "true" ? cout<<"Usuario Encontrado"<<endl<<endl : cout<<"Crendenciales invalidas..."<<endl<<endl<<"Por favor ingrese sus datos nuevamente (Le quedan " << " intentos)"<<endl<<endl;

        contador++;

        if(usuarioEncontrado == "true") {
            contador = 4;
        }

        Servidor->Enviar(usuarioEncontrado+";"+to_string(contador));

    }
  return loggedUser;
}



void registrarServerLog(string evento, string aRegistrar){
    std::ofstream serverLog("server.txt", std::ios::ate | std::ios::in);
    if(serverLog.fail()){ //Si el archivo no se encuentra o no esta disponible o presenta errores
            cout<<"No se pudo abrir el archivo server log"; //Muestra el error
                        }
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    serverLog<<buf;
    serverLog<<": "<<evento<<" - "<<aRegistrar<<endl;
    serverLog<<": ==================================="<<endl;
    serverLog.close();
}


void crearArchivoUserLog(string usuario){
    string nombreArchivo = usuario + ".log";
  std::ifstream userLog( nombreArchivo );
  if(userLog.fail()){
    //EL ARCHIVO NO EXISTE
    std::ofstream userLogCrear( nombreArchivo );
  }

  userLog.close();
}



void registrarUserLog(string evento, string aRegistrar){
    string nombreArchivo = aRegistrar + ".log";
    std::ofstream userLog( nombreArchivo , std::ios::ate | std::ios::in);
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    userLog<<buf;
    userLog<<": "<<evento<<" - "<<aRegistrar<<endl;
    userLog<<": ==================================="<<endl;
    userLog.close();


}
