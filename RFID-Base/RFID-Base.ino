// DEFINICIÓN DE CONSTANTES
typedef enum
{
  Menu,
  IngreseCodigoMatricial,
  AccesoAutorizadoMatricial,
  AccesoDenegadoMatricial,
  ESTADO_4,
  LectorTarjeta,
  TurnoMatutino,
  TurnoTarde,
  TurnoNoche,
  BusquedaDeHorario,
  AccesoAutorizado,
  AccesoAutorizadoFernan,
  AccesoDenegado,
  FueraDeHorario,
} estadoMEF;

// BIBLIOTECAS
#include <Key.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "RTClib.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

RTC_DS1307 rtc;

#define RST_PIN 49 // Configurable, see typical pin layout above
#define SS_PIN 53  // Configurable, see typical pin layout above
MFRC522 mfrc522(SS_PIN, RST_PIN);

// VARIABLES PARA EL RFID
const byte filasTurnoMatutino = 3;
const byte filasTurnoTarde = 3;
const byte filasTurnoNoche = 3;
byte PosicionDeHorario = 0;
byte F = 0;
byte cotadorDeDatosTarde = 0;
String leer_rfid;
// 04, 144, 116, 234 Pablo
// ADMIN
byte profesorFernan[4] = {4, 38, 72, 146};

// MATRIZ DE TURNOS
String turnoMatutino[filasTurnoMatutino] = {"414896212992128", "41110517816993128", "4729849203"};
String turnoTarde[filasTurnoTarde] = {"47858218197181440", "41282721018892128", "6382719"};
String turnoNoche[filasTurnoNoche] = {"1325721229", "798", "749283019301"};

// MATRIZ DE LOS NOMBRES RESPECTO A LOS TURNOS
String nombresTurnoMatutino[3][1] = {
    {"Omar"},
    {"Jose"},
    {"Emanuel"},
};
String nombresTurnoTarde[3][1] = {
    {"karen"},
    {"Jose"},
    {"Facu"},
};
String nombresTurnoNoche[3][1] = {
    {"Emanuel"},
    {"Juan"},
    {"Tobi"},
};

// VARIABLES PARA EL MATRICIAL
char TECLA;
bool borrador = 0;
bool iniciadorLCD = 0;
const byte FILAS = 4;    // four rows
const byte COLUMNAS = 4; // three columns
int rele = 2;
int i = 0;
int z = 0;
int buzzer = 11;
byte p = 0;
byte a = 0;
char Cont[] = {"14700"};
char comprobanteDeContrasena[5];
char keys[FILAS][COLUMNAS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

byte PinesColumnas[COLUMNAS] = {31, 33, 35, 37};
byte PinesFilas[FILAS] = {23, 25, 27, 29};
                                                                 

Keypad teclado = Keypad(makeKeymap(keys), PinesFilas, PinesColumnas, FILAS, COLUMNAS);

// VARIABLES DEL RTC
byte minute = 0;
byte hour = 0;
struct buleanos{
  bool estado = 0;
  byte horaDeAcceso;
}lector;
int horaMilitar;

// MENU
byte posicion1 = 0;
byte posicion2 = 0;
int opcion = 1;

// DECLARACIÓN DE FUNCIONES
void inicializarMEF(estadoMEF *punteroEstadoActual);
void actualizarMEF(estadoMEF *punteroEstadoActual);
void iniciadorlcd(estadoMEF *punteroEstadoActual);
void profeFer(estadoMEF *punteroEstadoActual);
void limpiadorDePantallaYINPUTDelTeclado();
void Horarios(estadoMEF *punteroEstadoActual);
void cambio_de_tipado(byte *buffer, byte bufferSize);

estadoMEF estadoActual;

// DECLARACIÓN DE VARIABLES GLOBALES
uint16_t miliSegundos;

void setup()
{
  inicializarMEF(&estadoActual);
  Serial.begin(9600);
#ifndef ESP8266
  while (!Serial)
    ; // wait for serial port to connect. Needed for native USB
#endif

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1)
      delay(10);
  }

  if (!rtc.isrunning())
  {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  while (!Serial)
    ;
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  delay(4);           // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  Serial.println("RFID reading UID");
  rtc.begin(); // Inicializamos el RTC
  Serial.println(__TIME__);
}

void loop()
{
  limpiadorDePantallaYINPUTDelTeclado();
  actualizarMEF(&estadoActual);
  delay(1);
  miliSegundos++;
}
void inicializarMEF(estadoMEF *punteroEstadoActual)
{
  *punteroEstadoActual = Menu;
}

void actualizarMEF(estadoMEF *punteroEstadoActual)
{
  switch (*punteroEstadoActual)
  {
  case Menu:
    iniciadorlcd(punteroEstadoActual);
    if (TECLA == 'A')
    {
      posicion1--;
      if(posicion1 > 1){
        posicion2--;
      }
    }
    if (TECLA == 'B')
    {
      posicion1++;
      if(posicion1 > 2){
        posicion2++;
      }
      if(posicion1 >= 4){
        posicion1 = 3;
        posicion2 = 1;
      }
    }
    if (posicion1 >= 255)
    {
      posicion1 = 0;
    }
    if(posicion1 >= 0 && posicion1 < 2){
      opcion = 1;
    }
    if((posicion2 >= 0 && posicion2 < 2) && posicion1 > 1){
      opcion = 2;
    }
    switch(opcion){
      case 1:
      lcd.setCursor(0, posicion1);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("Tarjeta");
      lcd.setCursor(1, 1);
      lcd.print("Teclado");
      if(posicion1 == 0 && TECLA == 'C'){
        miliSegundos = 0;
        borrador = 1;
        *punteroEstadoActual = LectorTarjeta;
        return;
      }
      if(posicion1 == 1 && TECLA == 'C'){
        miliSegundos = 0;
        borrador = 1;
        *punteroEstadoActual = IngreseCodigoMatricial;
        return;
      }
      break;

      case 2:
      lcd.setCursor(0, posicion2);
      lcd.print(">");
      lcd.setCursor(1, 0);
      lcd.print("Nuevo usuario");
      lcd.setCursor(1, 1);
      lcd.print("Excluir usuario");
      if(posicion2 == 0 && TECLA == 'C'){
        miliSegundos = 0;
        borrador = 1;
        *punteroEstadoActual = ESTADO_4;
        return;
      }
      if(posicion2 == 1 && TECLA == 'C'){
        miliSegundos = 0;
        borrador = 1;
        *punteroEstadoActual = ESTADO_4;
        return;
      }
      break;
    }
    break;

  
  case IngreseCodigoMatricial:
    iniciadorlcd(punteroEstadoActual);
    digitalWrite(rele, LOW);
    borrador = 1;
    lcd.setCursor(0, 0);
    lcd.print("Ingrese codigo:");
    for (int z = 0; z < 5; z++)
    {
      comprobanteDeContrasena[z] = 0;
    }
    i = 0;
    while (i < 5)
    {
      char TECLA = teclado.getKey();
      while (TECLA == NO_KEY)
      {
        TECLA = teclado.getKey();
      }
      comprobanteDeContrasena[i] = TECLA;
      lcd.setCursor(i, 1);
      lcd.print("*");
      i++;
    }
    if(i == 5)
    {
      borrador = 1;
      miliSegundos = 0;
      *punteroEstadoActual = AccesoAutorizadoMatricial;
      return;
    }
    break;

  case AccesoAutorizadoMatricial:
    comprobanteDeContrasena[i] = '\0';
    if (strcmp(Cont, comprobanteDeContrasena) == 0)
    {
      lcd.setCursor(0, 0);
      lcd.println("Acceso             ");
      lcd.setCursor(0, 1);
      lcd.println("Autorizado         ");
      digitalWrite(rele, HIGH);
      if (miliSegundos > 200)
      {
        miliSegundos = 0;
        *punteroEstadoActual = Menu;
        posicion1 = 0;
        borrador = 1;
        return;
      }
    }
    else
    {
      borrador = 1;
      miliSegundos = 0;
      *punteroEstadoActual = AccesoDenegadoMatricial;
      return;
    }
    break;

  case AccesoDenegadoMatricial:
    // lcd.setCursor(0,0);
    lcd.setCursor(0, 0);
    lcd.println("Acceso");
    lcd.setCursor(0, 1);
    lcd.println("Denegado");
    if (miliSegundos > 200)
    {
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      borrador = 1;
      return;
    }
    break;

  case LectorTarjeta:
    iniciadorlcd(punteroEstadoActual);
    lcd.setCursor(0, 0);
    lcd.print("Ingrese la ");
    lcd.setCursor(0, 1);
    lcd.print("tarjeta");
    p = 0;
    if(mfrc522.PICC_IsNewCardPresent()){
        if(mfrc522.PICC_ReadCardSerial())
        {
          Serial.print("Tag UID:");
          while(lector.estado == 0){
            cambio_de_tipado(mfrc522.uid.uidByte, mfrc522.uid.size); 
            lector.estado = !lector.estado;
          }
          Horarios(punteroEstadoActual);
        }       
    }
    break;

  case TurnoMatutino:
  F = 0;
  for (F = 0; F <=3 ; F++)
  {
    if(F == 3)
    {
      miliSegundos = 0;
      *punteroEstadoActual = AccesoAutorizadoFernan;
      lector.estado = !lector.estado;
      return;
    }
    if (profesorFernan[F] != mfrc522.uid.uidByte[F])
    {
      break;
    }
  }
    
  // LECTOR DE ACCESO
  do
  {
    if(turnoMatutino[p] == leer_rfid){
      borrador = 1;
      p = 0;
      *punteroEstadoActual = AccesoAutorizado;
      lector.horaDeAcceso = 0;
      lector.estado = !lector.estado;
      return;
    }
    while (p == 2)
    {
      borrador = 1;
      *punteroEstadoActual = BusquedaDeHorario;
      lector.estado = !lector.estado;
      p = 0;
      return;
    }
    p++;
  } while (p < 3);
    break;
  case TurnoTarde:
  F = 0;
  for (F = 0; F <=3 ; F++)
  {
    if(F == 3){
      miliSegundos = 0;
      *punteroEstadoActual = AccesoAutorizadoFernan;
      lector.estado = !lector.estado;
      return;
    }
    if (profesorFernan[F] != mfrc522.uid.uidByte[F]){
      break;
    }
  }
  // LECTOR DE ACCESO
  do
  {
    if(turnoTarde[p] == leer_rfid){
      borrador = 1;
      p = 0;
      *punteroEstadoActual = AccesoAutorizado;
      lector.horaDeAcceso = 1;
      lector.estado = !lector.estado;
      return;
    }
    while (p == 2){
      lcd.clear();
      *punteroEstadoActual = BusquedaDeHorario;
      lector.estado = !lector.estado;
      p = 0;
      return;
    }
    p++;
  } while (p < 3);
    break;

  case TurnoNoche:
    F = 0;
    for (F = 0; F <=3 ; F++)
    {
      if(F == 3){
        miliSegundos = 0;
        *punteroEstadoActual = AccesoAutorizadoFernan;
        lector.estado = !lector.estado;
        return;
      }
      if (profesorFernan[F] != mfrc522.uid.uidByte[F]){
        break;
      }
    }
    // LECTOR DE ACCESO
    do{
      if(turnoNoche[p] == leer_rfid){
        borrador = 1;
        p = 0;
        *punteroEstadoActual = AccesoAutorizado;
        lector.horaDeAcceso = 2;
        lector.estado = !lector.estado;
        return;
      }
      while (p == 2)
      {
        lcd.clear();
        *punteroEstadoActual = BusquedaDeHorario;
        lector.estado = !lector.estado;
        p = 0;
        return;
      }
      p++;
    } while (p < 3);
    break;
  case AccesoAutorizadoFernan:
    lcd.setCursor(0, 0);
    lcd.print("Bien venido/a ");
    lcd.setCursor(0, 1);
    lcd.print("Profe. Fernan");
    if (miliSegundos >= 70)
    {
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      posicion2 = 0;
      borrador = 1;
      return;
    }
    break;
  case AccesoAutorizado:
  switch(lector.horaDeAcceso){
    case 0:
    lcd.setCursor(0, 0);
    lcd.print("Bien venido/a ");
    lcd.setCursor(0, 1);
    lcd.print(nombresTurnoMatutino[p][1]);
    if (miliSegundos >= 70){
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      posicion2 = 0;
      borrador = 1;
      return;
    }
    break;
    case 1:
    lcd.setCursor(0, 0);
    lcd.print("Bien venido/a ");
    lcd.setCursor(0, 1);
    lcd.print(nombresTurnoTarde[p][1]);
    if (miliSegundos >= 70){
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      posicion2 = 0;
      borrador = 1;
      return;
    }
    break;
    case 2:
    lcd.setCursor(0, 0);
    lcd.print("Bien venido/a ");
    lcd.setCursor(0, 1);
    lcd.print(nombresTurnoNoche[p][1]);
    if (miliSegundos >= 70){
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      posicion2 = 0;
      borrador = 1;
      return;
    }
    break;
  }
    
    return;
    break;

  case FueraDeHorario:
    switch(PosicionDeHorario){
      case 0:
      if (miliSegundos <= 70 && miliSegundos >= 0){
        lcd.setCursor(0, 0);
        lcd.print("Disculpe");
        lcd.setCursor(0, 1);
        lcd.print(nombresTurnoNoche[p - 1][1]);
      }
      if (miliSegundos >= 70 && miliSegundos <= 140){
        lcd.setCursor(0, 0);
        lcd.print("Esta fuera");
        lcd.setCursor(0, 1);
        lcd.print("de horario");
      }
      if (miliSegundos > 140){
        miliSegundos = 0;
        *punteroEstadoActual = Menu;
        posicion1 = 0;
        posicion2 = 0;
        borrador = 1;
        return;
      }
      break;
      case 1:
        if (miliSegundos <= 70 && miliSegundos >= 0){
          lcd.setCursor(0, 0);
          lcd.print("Disculpe");
          lcd.setCursor(0, 1);
          lcd.print(nombresTurnoTarde[p - 1][1]);
        }
        if (miliSegundos >= 70 && miliSegundos <= 140){
          lcd.setCursor(0, 0);
          lcd.print("Esta fuera");
          lcd.setCursor(0, 1);
          lcd.print("de horario");
        }
        if (miliSegundos > 140){
          miliSegundos = 0;
          *punteroEstadoActual = Menu;
          posicion1 = 0;
          posicion2 = 0;
          borrador = 1;
          return;
        }
      break;
      case 2:
        if (miliSegundos <= 70 && miliSegundos >= 0){
          lcd.setCursor(0, 0);
          lcd.print("Disculpe");
          lcd.setCursor(0, 1);
          lcd.print(nombresTurnoMatutino[p - 1][1]);
        }
        if (miliSegundos >= 70 && miliSegundos <= 140){
          lcd.setCursor(0, 0);
          lcd.print("Esta fuera");
          lcd.setCursor(0, 1);
          lcd.print("de horario");
        }
        if (miliSegundos > 140)
        {
          miliSegundos = 0;
          *punteroEstadoActual = Menu;
          posicion1 = 0;
          posicion2 = 0;
          borrador = 1;
          return;
        }
      break;
    }
    break;
  case BusquedaDeHorario:
    while(p<=2){
      if(turnoNoche[p] == leer_rfid){
        miliSegundos = 0;
        PosicionDeHorario = 0;
        borrador = 1;
        *punteroEstadoActual = FueraDeHorario;
        return;
      }
      if(turnoTarde[p] == leer_rfid){
        miliSegundos = 0;
        PosicionDeHorario = 1;
        *punteroEstadoActual = FueraDeHorario;
        return;
      }
      if(turnoMatutino[p] == leer_rfid){
        miliSegundos = 0;
        PosicionDeHorario = 2;
        *punteroEstadoActual = FueraDeHorario;
        return;
      }
      p++;
    } 
    if(p == 3 ){
      miliSegundos = 0;
      borrador = 1;
      *punteroEstadoActual = AccesoDenegado;
      return;
    }
    break;
  case AccesoDenegado:
    lcd.setCursor(0, 0);
    lcd.print("Acceso denegado");
    if (miliSegundos >= 70)
    {
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      posicion2 = 0;
      borrador = 1;
      return;
    }
    return;
    break;
  case ESTADO_4:
    lcd.setCursor(0, 0);
    lcd.println("Proximamente en  ");
    lcd.setCursor(0, 1);
    lcd.println("La version 2.0   ");
    if (miliSegundos >= 500)
    {
      miliSegundos = 0;
      *punteroEstadoActual = Menu;
      posicion1 = 0;
      posicion2 = 0;
      borrador = 1;
      return;
    }
    break;
  }
}

void iniciadorlcd(estadoMEF *punteroEstadoActual)
{
  if(TECLA && iniciadorLCD == 0)
  {
    lcd.init();
    miliSegundos = 0;
    TECLA = " ";
    lcd.display();
    lcd.backlight();
    iniciadorLCD = !iniciadorLCD;
  }
  if ((TECLA == NO_KEY && miliSegundos >= 500) && iniciadorLCD == 1)
  {
    lcd.noDisplay();
    lcd.noBacklight();
    posicion1 = 0;
    posicion2 = 0;
    borrador = 1;
    iniciadorLCD = !iniciadorLCD;
    *punteroEstadoActual = Menu;
    return;
  }
  return;
}
//SE ENCARGA DE LIMPIAR LA PANTALLA SOLO UNA VEZ
void limpiadorDePantallaYINPUTDelTeclado(){
  if(borrador == 1)
  {
    lcd.clear();
    borrador = !borrador;
  }
  TECLA = teclado.getKey();
  if (TECLA)
  {
    miliSegundos = 0;
    borrador = 1;
  }
  return;
}

//SE ENCARGA DE VERR EL HORARIO
void Horarios(estadoMEF *punteroEstadoActual){
  DateTime now = rtc.now();
  minute = now.minute();
  hour = now.hour();
  horaMilitar = (hour * 100) + minute;
  Serial.println(horaMilitar);
  if(horaMilitar >= 730 && horaMilitar <= 1210){
    borrador = 1;
    miliSegundos = 0;
    *punteroEstadoActual = TurnoMatutino;
    return;
  }
  if(horaMilitar >= 1330 && horaMilitar <= 1745){
    borrador = 1;
    miliSegundos = 0;
    *punteroEstadoActual = TurnoTarde;
    return;
  }
  if(horaMilitar >= 1800 && horaMilitar <= 2130){
    lcd.clear();
    miliSegundos = 0;
    *punteroEstadoActual = TurnoNoche;
    return;
  }
}
//CAMBIA EL TIPADO DE LA TARJETA A UN STRING
void cambio_de_tipado(byte *buffer, byte bufferSize){  
  Serial.println("Estro a la funcion de cambio de tipado"); 
  leer_rfid="";
  for (byte i = 0; i < bufferSize; i++) {
    leer_rfid = leer_rfid + String(buffer[i]);

  }
}
