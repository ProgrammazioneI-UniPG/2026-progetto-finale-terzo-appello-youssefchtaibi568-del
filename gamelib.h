#ifndef GAMELIB_H
#define GAMELIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Definizioni dei tipi di dati
typedef enum {
    bosco, scuola, laboratorio, caverna, strada, giardino, supermercato,
    centrale_elettrica, deposito_abbandonato, stazione_polizia
} Tipo_zona;

// tipi di nemici possibili
typedef enum {
    nessun_nemico, billi, democane, demotorzone
} Tipo_nemico;

// tipi di oggetti possibili
typedef enum {
    nessun_oggetto, bicicletta, maglietta_fuocoinferno, bussola, schitarrata_metallica
} Tipo_oggetto;

// Dichiarazioni anticipate delle strutture collegate tra loro
struct Zona_soprasotto;
struct Zona_mondoreale;

// Struttura per le zone del Mondo Reale
struct Zona_mondoreale {
    Tipo_zona tipo; // Tipo di zona
    Tipo_nemico nemico;// Nemico presente nella zona
    Tipo_oggetto oggetto; // Oggetto presente nella zona
    struct Zona_mondoreale* avanti;// Puntatore alla zona successiva
    struct Zona_mondoreale* indietro;// Puntatore alla zona precedente
    struct Zona_soprasotto* link_soprasotto;// Collegamento al Soprasotto
};

// Struttura per le zone del Soprasotto
struct Zona_soprasotto {
    Tipo_zona tipo;   
    Tipo_nemico nemico;
    struct Zona_soprasotto* avanti;
    struct Zona_soprasotto* indietro;
    struct Zona_mondoreale* link_mondoreale;
};

// Struttura del Giocatore
struct Giocatore {
    char nome[100]; // Nome del giocatore
    int mondo; // 0 per Reale, 1 per Soprasotto
    struct Zona_mondoreale* pos_mondoreale;// Posizione nel Mondo Reale
    struct Zona_soprasotto* pos_soprasotto;// Posizione nel Soprasotto
    
    // Caratteristiche del giocatore
    int attacco_pischico;
    int difesa_pischica;
    int fortuna;
    
    // Inventario (max 3 oggetti)
    Tipo_oggetto zaino[3];
    
    // Flag per sapere se il giocatore ha già mosso questo turno
    int gia_mosso; 
};

// Prototipi funzioni principali
void imposta_gioco();// configurazione iniziale
void gioca();// avvio gioco
void termina_gioco();// chiusura gioco
void crediti();// mostra crediti

#endif