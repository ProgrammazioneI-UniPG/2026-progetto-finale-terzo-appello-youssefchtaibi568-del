#include "gamelib.h"

int main() {
    // Inizializzazione del generatore di numeri casuali
    srand((unsigned)time(NULL));
    
    int input_utente = 0;
    
    do {
        // stampo il menu principale
        printf("\n=============================\n");
        printf("   COSE STRANE - IL GIOCO    \n");
        printf("=============================\n");
        printf("[1] Nuova Partita / Imposta\n");
        printf("[2] Inizia Avventura\n");
        printf("[3] Esci dal Gioco\n");
        printf("[4] Info e Crediti\n");
        printf("-----------------------------\n");
        printf("Scegli un'opzione: ");
        
        // leggo la scelta dell'utente e vedo se è un numero valido
        if (scanf("%d", &input_utente) != 1) {
            // Pulisco il buffer se scrive roba sbagliata
            while(getchar() != '\n'); 
            input_utente = -1; // input sbagliato
        }
        
        //eseguo l'azione corrispondente alla scelta
        switch(input_utente) {
            case 1:
                imposta_gioco();// configurazione nuova partita
                break;
            case 2:
                gioca();// avvio gioco
                break;
            case 3:
                termina_gioco();// chiusura gioco
                break;
            case 4:
                crediti();// mostro crediti
                break;
            default:
                printf("\n>>> Errore: Opzione non valida. Riprova. <<<\n");
                break;
        }
        
    } while(input_utente != 3); //continu a finché non sceglie di uscire

    return 0;
}