#include "gamelib.h"

// VARIABILI GLOBALI 
static struct Zona_mondoreale* testa_mr = NULL;
static struct Zona_soprasotto* testa_ss = NULL;
static struct Giocatore* array_giocatori[4] = {NULL, NULL, NULL, NULL};
static int n_giocatori = 0;
static int gioco_pronto = 0; // 1 se la mappa è valida

// Funzioni di supporto 

// Generatore numeri casuali [min, max] 
static int get_random(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Stampe per i nomi della mappa 
static void print_zona(Tipo_zona z) {
    const char* nomi[] = {
        "Bosco Tetro", "Liceo Hawkins", "Laboratorio Nazionale", "Grotta Buia",
        "Strada Principale", "Giardino Pubblico", "Starcourt Mall", "Centrale Enel",
        "Deposito Rifiuti", "Distretto Polizia"
    };
    if (z >= 0 && z <= 9) printf("%s", nomi[z]);
}

// stampa il nome del nemico
static void print_nemico(Tipo_nemico n) {
    if (n == nessun_nemico) printf("Nessuno");
    else if (n == billi) printf("Billi (Il Bagnino)"); 
    else if (n == democane) printf("Un Democane Ringhiante"); 
    else if (n == demotorzone) printf(">>> IL DEMOTORZONE <<<");
}

static void print_oggetto(Tipo_oggetto o) {
    const char* obj_names[] = {"(Vuoto)", "Bicicletta Arrugginita", "T-Shirt Hellfire", "Vecchia Bussola", "Chitarra Elettrica"};
    if (o >= 0 && o <= 4) printf("%s", obj_names[o]);
}

// GESTIONE DELLA MEMORIA

// Funzione helper per pulire solo le mappe (utile per rigenerare senza cancellare giocatori)
static void svuota_solo_mappe() {
    struct Zona_mondoreale* cursore_mr = testa_mr;
    while (cursore_mr != NULL) {
        struct Zona_mondoreale* tmp = cursore_mr;
        cursore_mr = cursore_mr->avanti;
        free(tmp);
    }
    testa_mr = NULL;

    struct Zona_soprasotto* cursore_ss = testa_ss;
    while (cursore_ss != NULL) {
        struct Zona_soprasotto* tmp = cursore_ss;
        cursore_ss = cursore_ss->avanti;
        free(tmp);
    }
    testa_ss = NULL;
}

// libera tutta la memoria allocata e rimette il gioco a zero
static void svuota_memoria() {
    svuota_solo_mappe();

    // elimino Giocatori
    for (int i = 0; i < 4; i++) {
        if (array_giocatori[i] != NULL) {
            free(array_giocatori[i]);
            array_giocatori[i] = NULL;
        }
    }
    n_giocatori = 0;
    gioco_pronto = 0;
}

// LOGICA MAPPA

// Conta le zone attuali
static int conta_zone() {
    int c = 0;
    struct Zona_mondoreale* p = testa_mr;
    while(p) { c++; p = p->avanti; }
    return c;
}

// Funzione interna per allocare e collegare i nodi in una posizione specifica
// FIX CRITICO: Ora accetta una posizione 'pos' invece di mettere sempre in coda
static void inserisci_zona_in_posizione(int pos, Tipo_zona t, Tipo_nemico n_mr, Tipo_nemico n_ss, Tipo_oggetto o_mr) {
    struct Zona_mondoreale* nodo_mr = (struct Zona_mondoreale*)malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* nodo_ss = (struct Zona_soprasotto*)malloc(sizeof(struct Zona_soprasotto));

    // Assegnazione dati
    nodo_mr->tipo = t; nodo_mr->nemico = n_mr; nodo_mr->oggetto = o_mr;
    nodo_ss->tipo = t; nodo_ss->nemico = n_ss;

    // Link verticali
    nodo_mr->link_soprasotto = nodo_ss;
    nodo_ss->link_mondoreale = nodo_mr;

    // CASO 1: Lista vuota o inserimento in testa (pos 0)
    if (testa_mr == NULL || pos <= 0) {
        nodo_mr->avanti = testa_mr;
        nodo_mr->indietro = NULL;
        if (testa_mr != NULL) testa_mr->indietro = nodo_mr;
        testa_mr = nodo_mr;

        nodo_ss->avanti = testa_ss;
        nodo_ss->indietro = NULL;
        if (testa_ss != NULL) testa_ss->indietro = nodo_ss;
        testa_ss = nodo_ss;
    } 
    // CASO 2: Inserimento in mezzo o in coda
    else {
        struct Zona_mondoreale* curr_mr = testa_mr;
        struct Zona_soprasotto* curr_ss = testa_ss;
        int index = 0;

        // Scorro fino alla posizione precedente a quella desiderata
        while (curr_mr->avanti != NULL && index < pos - 1) {
            curr_mr = curr_mr->avanti;
            curr_ss = curr_ss->avanti;
            index++;
        }

        // Collegamento Mondo Reale
        nodo_mr->avanti = curr_mr->avanti;
        nodo_mr->indietro = curr_mr;
        if (curr_mr->avanti != NULL) curr_mr->avanti->indietro = nodo_mr;
        curr_mr->avanti = nodo_mr;

        // Collegamento Soprasotto
        nodo_ss->avanti = curr_ss->avanti;
        nodo_ss->indietro = curr_ss;
        if (curr_ss->avanti != NULL) curr_ss->avanti->indietro = nodo_ss;
        curr_ss->avanti = nodo_ss;
    }
}

// genera automaticamente una mappa
static void genera_mappa_auto() {
    // FIX CRITICO: Pulisce la mappa precedente se esiste 
    if (testa_mr != NULL) {
        printf(" [Sistema] Cancellazione mappa precedente...\n");
        svuota_solo_mappe();
    }

    // creo 15 zone 
    for (int i = 0; i < 15; i++) {
        Tipo_zona t = (Tipo_zona)get_random(0, 9);
        
        Tipo_nemico nem_reale = nessun_nemico;
        int dado = get_random(1, 100);
        if (dado > 60 && dado <= 80) nem_reale = billi;
        if (dado > 80) nem_reale = democane;

        Tipo_nemico nem_sotto = nessun_nemico;
        dado = get_random(1, 100);
        if (dado > 50 && dado <= 90) nem_sotto = democane;
        if (dado > 90) nem_sotto = demotorzone;

        Tipo_oggetto obj = nessun_oggetto;
        if (get_random(0, 1) == 1) obj = (Tipo_oggetto)get_random(1, 4);

        // Uso la nuova funzione passando 'i' per accodare
        inserisci_zona_in_posizione(i, t, nem_reale, nem_sotto, obj);
    }

    // CONTROLLO BOSS (Logica mantenuta perché valida)
    int count_boss = 0;
    struct Zona_soprasotto* scorri = testa_ss;
    while(scorri) {
        if (scorri->nemico == demotorzone) count_boss++;
        scorri = scorri->avanti;
    }

    if (count_boss == 0) {
        scorri = testa_ss;
        while(scorri->avanti) scorri = scorri->avanti;
        scorri->nemico = demotorzone;
        printf(" [Sistema] Boss aggiunto in coda.\n");
    } else if (count_boss > 1) {
        scorri = testa_ss;
        int found = 0;
        while(scorri) {
            if (scorri->nemico == demotorzone) {
                if (found == 0) found = 1; 
                else scorri->nemico = democane; 
            }
            scorri = scorri->avanti;
        }
        printf(" [Sistema] Boss duplicati rimossi.\n");
    }
    printf("Nuova mappa generata (15 zone).\n");
}

// aggiunge una zona manualmente
// FIX CRITICO: Ora chiede la posizione 
static void aggiungi_manuale() {
    int pos, t, nm, ns, og;
    int max_zone = conta_zone();
    
    printf("Inserisci in posizione (0 - %d): ", max_zone);
    scanf("%d", &pos);
    if (pos < 0) pos = 0;
    if (pos > max_zone) pos = max_zone;

    printf("Zona (0-9): "); scanf("%d", &t);
    printf("Nemico MR (0=No, 1=Billi, 2=Demo): "); scanf("%d", &nm);
    printf("Nemico SS (0=No, 2=Demo, 3=Boss): "); scanf("%d", &ns);
    printf("Oggetto (0-4): "); scanf("%d", &og);

    inserisci_zona_in_posizione(pos, (Tipo_zona)t, (Tipo_nemico)nm, (Tipo_nemico)ns, (Tipo_oggetto)og);
    printf("Zona inserita in posizione %d.\n", pos);
}

// rimuove una zona specifica
// FIX CRITICO: Ora chiede la posizione 'i' e cancella quella 
static void cancella_zona_manuale() {
    if (!testa_mr) {
        printf("Mappa vuota.\n");
        return;
    }

    int pos;
    printf("Posizione da cancellare (0 - %d): ", conta_zone() - 1);
    scanf("%d", &pos);

    struct Zona_mondoreale* del_mr = testa_mr;
    struct Zona_soprasotto* del_ss = testa_ss;

    // Navigo alla posizione
    int i = 0;
    while (del_mr != NULL && i < pos) {
        del_mr = del_mr->avanti;
        del_ss = del_ss->avanti;
        i++;
    }

    if (del_mr == NULL) {
        printf("Posizione non valida.\n");
        return;
    }

    // Scollegamento Mondo Reale
    if (del_mr->indietro) del_mr->indietro->avanti = del_mr->avanti;
    else testa_mr = del_mr->avanti; // Era la testa

    if (del_mr->avanti) del_mr->avanti->indietro = del_mr->indietro;

    // Scollegamento Soprasotto
    if (del_ss->indietro) del_ss->indietro->avanti = del_ss->avanti;
    else testa_ss = del_ss->avanti; // Era la testa

    if (del_ss->avanti) del_ss->avanti->indietro = del_ss->indietro;

    free(del_mr);
    free(del_ss);
    printf("Zona in posizione %d rimossa.\n", pos);
}

// mostra la mappa attuale
static void visualizza_mappa() {
    struct Zona_mondoreale* p = testa_mr;
    int k = 0;
    printf("\n--- VISUALIZZAZIONE MONDI ---\n");
    while(p) {
        printf("[Pos %d] ", k++);
        print_zona(p->tipo);
        printf(" | MR: "); print_nemico(p->nemico);
        printf(" | SS: "); print_nemico(p->link_soprasotto->nemico);
        printf("\n");
        p = p->avanti;
    }
}

// controlla se la mappa è valida (15 zone e 1 boss)
static void finalizza_mappa() {
    int c = conta_zone();
    int boss = 0;
    struct Zona_soprasotto* s = testa_ss;
    while(s) { if(s->nemico == demotorzone) boss++; s = s->avanti; }

    if (c >= 15 && boss == 1) {
        gioco_pronto = 1;
        printf("Tutto pronto. I portali sono aperti. Premi 2 per giocare.\n");
    } else {
        printf("Non ci siamo: Servono 15 zone (hai: %d) e 1 Boss (hai: %d).\n", c, boss);
        gioco_pronto = 0;
    }
}

// LOGICA DI GIOCO

static void fase_combattimento(struct Giocatore* pg) {
    Tipo_nemico nem;
    if (pg->mondo == 0) nem = pg->pos_mondoreale->nemico;
    else nem = pg->pos_soprasotto->nemico;

    if (nem == nessun_nemico) return;

    printf("\n>>> COMBATTIMENTO <<<\nAvversario: "); print_nemico(nem); printf("\n");

    int forza_hostile = 0;
    if (nem == billi) forza_hostile = 15;
    if (nem == democane) forza_hostile = 25;
    if (nem == demotorzone) forza_hostile = 50;

    int tiro_dado = get_random(1, 20);
    int forza_totale = pg->attacco_pischico + (pg->difesa_pischica / 2) + tiro_dado;

    printf("Tua potenza: %d (Base) + %d (Dado) = %d VS Nemico: %d\n", pg->attacco_pischico + pg->difesa_pischica/2, tiro_dado, forza_totale, forza_hostile);

    if (forza_totale >= forza_hostile) {
        printf(">> NEMICO SCONFITTO!\n");
        if (get_random(0, 1)) {
            if (pg->mondo == 0) pg->pos_mondoreale->nemico = nessun_nemico;
            else pg->pos_soprasotto->nemico = nessun_nemico;
            printf(">> La creatura scompare.\n");
        }
        // FIX: Evitiamo magic number 1000 per la vittoria, lo gestiamo nel main loop
        if (nem == demotorzone) {
            printf(">>> HAI SCONFITTO IL BOSS! <<<\n");
            pg->fortuna = 9999; // Segnale di vittoria chiara
        }
    } else {
        printf(">> COLPO SUBITO!\n");
        pg->difesa_pischica -= 5;
    }
}

static int menu_oggetti(struct Giocatore* pg) {
    printf("\n--- INVENTARIO ---\n");
    for(int i=0; i<3; i++) {
        printf("%d) ", i+1); print_oggetto(pg->zaino[i]); printf("\n");
    }
    int sel;
    printf("Cosa usi? (0 esci): ");
    scanf("%d", &sel);

    if (sel < 1 || sel > 3) return 0;
    sel--; 

    Tipo_oggetto usato = pg->zaino[sel];
    if (usato == nessun_oggetto) {
        printf("Slot vuoto.\n");
        return 0;
    }

    printf("Oggetto utilizzato!\n");
    pg->zaino[sel] = nessun_oggetto; 

    if (usato == bicicletta) return 1; 
    
    if (usato == maglietta_fuocoinferno) {
        pg->attacco_pischico += 5;
        printf("Attacco aumentato!\n");
    }
    if (usato == bussola) {
        pg->fortuna += 5;
        printf("Fortuna aumentata!\n");
    }
    if (usato == schitarrata_metallica) {
        pg->difesa_pischica += 10;
        printf("Salute ripristinata!\n");
    }
    return 0;
}

// Funzioni pubbliche (Interfaccia)

void imposta_gioco() {
    svuota_memoria();
    printf("\n--- PREPARAZIONE NUOVA PARTITA ---\n");

    do {
        printf("Numero partecipanti (1-4)? ");
        if (scanf("%d", &n_giocatori) != 1) {
             while(getchar() != '\n'); 
             n_giocatori = 0;
        }
    } while(n_giocatori < 1 || n_giocatori > 4);

    int undici_assegnato = 0;
    for (int i = 0; i < n_giocatori; i++) {
        array_giocatori[i] = (struct Giocatore*)malloc(sizeof(struct Giocatore));
        
        printf("Nome Giocatore %d: ", i+1);
        scanf("%s", array_giocatori[i]->nome);

        array_giocatori[i]->attacco_pischico = get_random(1, 20);
        array_giocatori[i]->difesa_pischica = get_random(1, 20);
        array_giocatori[i]->fortuna = get_random(1, 20);
        array_giocatori[i]->mondo = 0;

        for(int k=0; k<3; k++) array_giocatori[i]->zaino[k] = nessun_oggetto;

        printf("Valori: ATK %d | DEF %d | LUCK %d\n", array_giocatori[i]->attacco_pischico, array_giocatori[i]->difesa_pischica, array_giocatori[i]->fortuna);
        if (!undici_assegnato) {
            int yn;
            printf("Vuoi essere Undici (11.5)? (1=Si, 0=No): ");
            scanf("%d", &yn);
            if (yn == 1) {
                array_giocatori[i]->attacco_pischico += 4;
                array_giocatori[i]->difesa_pischica += 4;
                array_giocatori[i]->fortuna -= 7;
                strcat(array_giocatori[i]->nome, "_11");
                undici_assegnato = 1;
            }
        }
    }

    int cmd = 0;
    while (!gioco_pronto) {
        printf("\nEDITOR MAPPA: 1.Auto 2.Ins(Pos) 3.Canc(Pos) 4.Vedi 5.CONFERMA: ");
        
        if (scanf("%d", &cmd) != 1) {
            while(getchar() != '\n'); 
            cmd = -1;
            printf("Inserire numero valido.\n");
            continue; 
        }

        if (cmd == 1) genera_mappa_auto();
        else if (cmd == 2) aggiungi_manuale();
        else if (cmd == 3) cancella_zona_manuale(); // FIX: Nuova funzione di cancellazione
        else if (cmd == 4) visualizza_mappa();
        else if (cmd == 5) finalizza_mappa();
    }

    // Posizionamento iniziale
    for(int i=0; i<n_giocatori; i++) {
        array_giocatori[i]->pos_mondoreale = testa_mr;
        array_giocatori[i]->pos_soprasotto = testa_ss;
    }
}

void gioca() {
    if (!gioco_pronto) {
        printf("Errore: Mappa non ancora creata!\n");
        return;
    }

    int vittoria_totale = 0;
    int tutti_morti = 0;
   
    while (!vittoria_totale && !tutti_morti) {
        
        int turni_misti[4] = {0, 1, 2, 3};
        for (int i = 0; i < n_giocatori; i++) {
            int r = get_random(0, n_giocatori - 1);
            int tmp = turni_misti[i];
            turni_misti[i] = turni_misti[r];
            turni_misti[r] = tmp;
        }

        for (int k = 0; k < n_giocatori; k++) {
            int idx = turni_misti[k];
            // FIX CRITICO: Controllo puntatore NULL per evitare crash
            if (array_giocatori[idx] == NULL) continue;
            
            struct Giocatore* pg = array_giocatori[idx];

            // FIX CRITICO: Gestione morte corretta 
            if (pg->difesa_pischica <= 0) {
                printf("\n>>> %s E' CADUTO IN BATTAGLIA! <<<\n", pg->nome);
                free(array_giocatori[idx]); // Libero memoria
                array_giocatori[idx] = NULL; // Metto a NULL
                continue; // Passo al prossimo
            }

            if (pg->fortuna >= 9999) { vittoria_totale = 1; break; }

            pg->gia_mosso = 0; 

            printf("\n>>> TURNO DI: %s <<<\n", pg->nome);
            printf("Pos: "); 
            if(pg->mondo == 0) print_zona(pg->pos_mondoreale->tipo);
            else print_zona(pg->pos_soprasotto->tipo);
            printf(" [%s]\n", pg->mondo == 0 ? "Reale" : "SOPRASOTTO");
            printf("HP: %d\n", pg->difesa_pischica);

            int fine_turno = 0;
            int fuga_bici = 0;

            while (!fine_turno) {
                Tipo_nemico nem_qui = (pg->mondo==0) ? pg->pos_mondoreale->nemico : pg->pos_soprasotto->nemico;
                int bloccato = (nem_qui != nessun_nemico);

                printf("\nComandi: 1.Avanti 2.Indietro 3.CambiaDimensione 4.Combatti 5.Prendi 6.Oggetti 7.Stato 8.InfoZona 9.FineTurno\n> ");
                
                int act; 
                if (scanf("%d", &act) != 1) {
                    while(getchar() != '\n'); 
                    act = -1;
                    continue;
                }

                switch(act) {
                    case 1: 
                        if (bloccato && !fuga_bici) {
                            printf("Nemico presente! Devi combattere.\n");
                        } else {
                            int ok = 0;
                            if (pg->mondo == 0 && pg->pos_mondoreale->avanti) {
                                pg->pos_mondoreale = pg->pos_mondoreale->avanti; ok=1;
                            } else if (pg->mondo == 1 && pg->pos_soprasotto->avanti) {
                                pg->pos_soprasotto = pg->pos_soprasotto->avanti; ok=1;
                            }
                            if(ok) {
                                printf("Avanzi nella zona successiva.\n");
                                pg->gia_mosso = 1;
                                if(fuga_bici) fine_turno = 1; 
                            } else printf("Strada chiusa.\n");
                        }
                        break;

                    case 2: 
                        if (bloccato && !fuga_bici) {
                            printf("Nemico presente! Non puoi fuggire.\n");
                        } else {
                            int ok = 0;
                            if (pg->mondo == 0 && pg->pos_mondoreale->indietro) {
                                pg->pos_mondoreale = pg->pos_mondoreale->indietro; ok=1;
                            } else if (pg->mondo == 1 && pg->pos_soprasotto->indietro) {
                                pg->pos_soprasotto = pg->pos_soprasotto->indietro; ok=1;
                            }
                            if(ok) {
                                printf("Torni indietro.\n");
                                pg->gia_mosso = 1;
                            } else printf("Sei all'inizio.\n");
                        }
                        break;

                    case 3: 
                        if (pg->mondo == 0) {
                            if (pg->gia_mosso) printf("Stanco. Non puoi viaggiare ora.\n");
                            else if (bloccato && !fuga_bici) printf("Nemico presente! Impossibile concentrarsi.\n");
                            else {
                                pg->pos_soprasotto = pg->pos_mondoreale->link_soprasotto;
                                pg->mondo = 1;
                                pg->gia_mosso = 1;
                                printf("Entri nel SOPRASOTTO.\n");
                            }
                        } else {
                            printf("Cerchi di fuggire... ");
                            int roll = get_random(1, 20);
                            if (roll < pg->fortuna) {
                                pg->pos_mondoreale = pg->pos_soprasotto->link_mondoreale;
                                pg->mondo = 0;
                                printf("SUCCESSO! Torni al Reale.\n");
                                pg->gia_mosso = 1;
                            } else {
                                printf("FALLITO! Resti qui.\n");
                                pg->gia_mosso = 1;
                            }
                        }
                        break;

                    case 4: 
                        fase_combattimento(pg);
                        // Se muore o vince, il turno finisce subito
                        if (pg->difesa_pischica <= 0 || pg->fortuna >= 9999) fine_turno = 1;
                        break;

                    case 5: 
                        if (bloccato) {
                            printf("Prima sconfiggi il nemico!\n");
                        } else if (pg->mondo == 0 && pg->pos_mondoreale->oggetto != nessun_oggetto) {
                            int slot_ok = -1;
                            for(int j=0; j<3; j++) {
                                if(pg->zaino[j] == nessun_oggetto) { slot_ok = j; break; }
                            }
                            if(slot_ok != -1) {
                                pg->zaino[slot_ok] = pg->pos_mondoreale->oggetto;
                                pg->pos_mondoreale->oggetto = nessun_oggetto;
                                printf("Oggetto preso.\n");
                            } else printf("Zaino pieno.\n");
                        } else printf("Niente da prendere.\n");
                        break;

                    case 6:
                        if (menu_oggetti(pg) == 1) fuga_bici = 1;
                        break;

                    case 7:
                        printf("\nStatus: %s (Dim: %d) | HP: %d | ATK: %d | LUCK: %d\n", pg->nome, pg->mondo, pg->difesa_pischica, pg->attacco_pischico, pg->fortuna);
                        break;
                    
                    case 8:
                        printf("\nZona Attuale:\n");
                        if(pg->mondo == 0) {
                            print_zona(pg->pos_mondoreale->tipo);
                            printf(" | Nemico: "); print_nemico(pg->pos_mondoreale->nemico);
                            printf(" | Oggetto: "); print_oggetto(pg->pos_mondoreale->oggetto);
                        } else {
                            print_zona(pg->pos_soprasotto->tipo);
                            printf(" | Nemico: "); print_nemico(pg->pos_soprasotto->nemico);
                        }
                        printf("\n");
                        break;

                    case 9:
                        fine_turno = 1;
                        break;
                }
            }
            if (vittoria_totale) break;
        }

        // FIX CRITICO: Conta solo i giocatori non-NULL e vivi 
        int vivi = 0;
        for(int i=0; i<n_giocatori; i++) {
            if (array_giocatori[i] != NULL && array_giocatori[i]->difesa_pischica > 0) vivi++;
        }
        if(vivi == 0) tutti_morti = 1;
    }

    if (vittoria_totale) printf("\n*** DEMOTORZONE SCONFITTO! VITTORIA! ***\n");
    if (tutti_morti) printf("\n*** TUTTI I GIOCATORI SONO CADUTI. GAME OVER. ***\n");
    
    svuota_memoria();
}

void termina_gioco() {
    svuota_memoria();
    printf("Uscita... A presto!\n");
}

void crediti() {
    printf("Creato da: Chtaibi Youssef (Matr. 393768) - Prog. Procedurale\n");
}