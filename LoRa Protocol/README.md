# LoRa Protocol

Questo progetto dimostra la comunicazione tra due dispositivi utilizzando la tecnologia LoRa, sfruttando la libreria `iot_board`. Un dispositivo invia messaggi mentre l'altro li riceve e visualizza le informazioni sul display.

## Hardware Utilizzato

* Due dispositivi compatibili con LoRa (es. ESP32 con modulo LoRa)
* Display OLED (per la visualizzazione dei messaggi e delle statistiche)
* Pulsanti (per interazioni hardware opzionali)
* LED (per feedback visivo)

## Funzionamento

Il codice è suddiviso in due sezioni principali:

1. **Dispositivo mittente** :

* Invia periodicamente messaggi al destinatario.
* Incapsula i dati con un header contenente indirizzi mittente e destinatario, ID messaggio e lunghezza del payload.
* Stampa su seriale ogni messaggio inviato.

1. **Dispositivo ricevente** :

* Riceve i messaggi e li visualizza sul display.
* Estrae e verifica i dati del pacchetto ricevuto.
* Mostra dettagli come ID mittente, RSSI e SNR.

## Struttura del Messaggio

Ogni messaggio LoRa contiene i seguenti campi:

* **Destinazione** (2 byte)
* **Mittente** (2 byte)
* **ID Messaggio** (1 byte)
* **Lunghezza Payload** (1 byte)
* **Dati Payload** (stringa di testo)

## Configurazione degli Indirizzi

* Il dispositivo ricevente ha `localAddress = 0x01`.
* Il dispositivo mittente ha `localAddress = 0x02`.
* La trasmissione è diretta a `destination = 0xFF` (broadcast o indirizzo specifico a seconda dell'implementazione).
* 

## Installazione e Uso

1. Carica il codice del **mittente** su un dispositivo con `localAddress = 0x02`.
2. Carica il codice del **ricevente** su un altro dispositivo con `localAddress = 0x01`.
3. Accendi entrambi i dispositivi.
4. Osserva la comunicazione sulla seriale o sul display OLED.

## Personalizzazioni

* Modifica il valore di `destination` per inviare messaggi a un nodo specifico.
* Cambia la frequenza di invio modificando l'intervallo in `runEvery()`.
* Aggiungi gestione degli errori per migliorare l'affidabilità della comunicazione.

## File

## Requirements

- Visual Studio Code with the [PlatformIO](https://platformio.org/);
- `iot_board` library provided by the course to implement the protocols using the components with Arduino Nano Esp32;

## License

This repository is licensed under the MIT license. See the `LICENSE` file for more details.

## Author

Alessandro Ferrante

Email: [github@alessandroferrante.net](mailto:github@alessandroferrante.net)
