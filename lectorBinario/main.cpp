#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

int main()
{
    //VARIABLE A GUARDAR
    char buffer2 [] = "Hola";

    // ESCRITURA EN BINARIO
   ofstream myFile ("escribirBin.bin", std::ios::ate | std::ios::in | ios :: binary);
    myFile << buffer2;

    myFile.close();

    //LEER DEL ARCHIVO BINARIO
    string mensaje;
    ifstream leerArchivo ("escribirBin.bin", ios::in | ios::out | ios::binary);

     while (!leerArchivo.eof()) {
            leerArchivo >> mensaje;
            cout << mensaje << endl;
  }
    //OTRA FORMA DE LECTURA
  //  leerArchivo.seekg (0); //POSICI�N DESDE LA QUE EMPIEZA A LEER
  //  leerArchivo.read (mensaje, 4000);

    leerArchivo.close();


    return 0;
}