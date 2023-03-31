#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define N 78

//Struct per le liste
typedef struct nodo_{
    char *parola; 
    struct nodo_ *next;
} nodo_t;

//Struct per i nodi dell'albero
typedef struct nodo{
    char *parola; //usiamo una parola anzichè una chiave
    struct nodo* right;
    struct nodo* left;
}nodo;

//Struct per definire l'albero e la sua radice
typedef struct albero{
    nodo *radice;
}albero;

//Struct per gestire i vincoli che si scoprono durante il gioco
typedef struct vincolo_{
    char appartenenza; //v --> appartiene e f --> non-appartiene e p --> potrebbe appartenere
    char *corrispondenza; //v --> è in quella pos e f --> non è in quella posizione e p --> potrebbe essere in quella posizione
    short num_min;
    short num_esatto;
} vincolo_t;

nodo_t* inserisciInTesta(nodo_t*,char*,short);
nodo_t* inserisciInCoda(nodo_t*,char*,short);
nodo_t* inserisciInOrdine(nodo_t*,char*,short);
nodo_t* aggiorna_parole_ammissibili(vincolo_t**,nodo_t*,albero*,int*,short,char*, char*);
nodo_t* postorder_insert(vincolo_t**,albero*,nodo*,nodo_t*,int*,short,char*,char*);
nodo_t* confronta(albero*,short,int*,char*,char*,short*,nodo_t*,vincolo_t**,char*);
short get_indice(char);
void inorder(albero*,nodo*);
void visualizza(nodo_t*);
int tree_search(albero*,nodo*,char*);
void BST_Insert(albero*,char*,short);
int nuova_partita(albero*,short,int*,vincolo_t**,char*);
short compatibile_parola(vincolo_t**,char*,short,char*, char*);
short max_num(short,short);
void freeList(nodo_t*);
void inorder_free(albero*,nodo*);
nodo_t* rimuovi_nodo(nodo_t*,nodo_t*,nodo_t**);
nodo_t* merge(nodo_t*,nodo_t*,short);
nodo_t* newNode(char*,short);


int main(){
    short k; //indica la lunghezza della parola
    int i; 
    int *num_parole; //numero parole inserite nel vocabolario
    albero *T = malloc(sizeof(albero)); //struttura dati che contiene il vocabolario
    char *parola; // parola usata per le prime letture del codice
    int massimo; // intero per verificare la lunghezza max della stringa
    vincolo_t *vincolo[N]; //struttura per i vincoli
    char *indovinate_already; //stringa di char che uso per tenere le lettere trovate nelle posizioni i-esime
    short is_new_match = 1;
    int supporto = 0;
    
    num_parole = &supporto;

    if(T != NULL){
        //Inizializzazione Albero
        T->radice = NULL;
        if(scanf("%hd", &k)){} //leggo la lunghezza della parole
        massimo = max_num(k, 17);
        parola = malloc(sizeof(char)*(massimo+1)); //alloco la parola di massimo+1 char per evitari conflitti
        indovinate_already = malloc(sizeof(char)*k+1);

        //inizializzo indovinte_already
        for(i=0;i<k;i++){
            indovinate_already[i] = '*';
        }
        indovinate_already[k] = '\0';

        //inizializzo per la prima volta la struttura
        for(i=0;i<N; i++){
            vincolo[i] = malloc(sizeof(vincolo_t));
            vincolo[i]->corrispondenza = malloc(sizeof(char)*(k+1));
            for(int j=0;j<k;j++){
                vincolo[i]->corrispondenza[j] = 'p';
            }
            vincolo[i]->corrispondenza[k] = '\0';
            vincolo[i]->appartenenza = 'p';
            vincolo[i]->num_min = 0;
            vincolo[i]->num_esatto = -1;
        }

        if(parola != NULL){
            while(1){
                //Riempo l'albero
                if(scanf("%s", parola)){}

                //Entro se ho ricevuto un comando
                if(parola[0] == '+'){
                    // Se scrivo "+nuova_partita" inzia il gioco
                    if(strcmp("+nuova_partita", parola)==0){
                        while(is_new_match){
                            is_new_match = nuova_partita(T, k, num_parole, vincolo, indovinate_already);
                        }
                        break;
                    }
                    // Se voglio fare "+inserisci_inizio" prima di avere fatto una partita
                    if(strcmp(parola, "+inserisci_inizio")==0){
                        while(1){
                            if(scanf("%s", parola)){}
                            if(strcmp("+inserisci_fine", parola)==0){
                                break;
                            }
                            BST_Insert(T, parola, k);
                            *num_parole = *num_parole + 1;
                        }
                        continue;
                    }
                }

                //incremento il numero di parole e inserisco la parola nell'albero 
                *num_parole = *num_parole + 1;
                BST_Insert(T, parola, k);
            }

            for(i=0;i<N;i++){
                free(vincolo[i]);
            }
            free(parola);
            free(indovinate_already);
        }
    }
    inorder_free(T, T->radice);
    free(T);

    return 0;
}


//Una semplice funzione che restituisce l'indice corrispondente del carattere passato
short get_indice(char c){
    short indice;
    indice = (int)c - (int)'-';
    return indice;
}

//Questa funzione si occupa della creazione della nuova partita e della gestione di tutto ciò che ne deriva
int nuova_partita(albero *T, short k, int* num_parole_voc, vincolo_t** vincolo, char* indovinate_already){
    int n, i; //nunero di tentativi
    char *indovina;
    char *parola;
    short occ_indovina[N] = {0};
    int *num_parole_amm;
    nodo_t *parole_ammissibili = NULL;
    int max; //Salvo il numero di parole inserite nel vocabolario
    short massimo; //Massimo tra 17 e k
    nodo_t *support_list = NULL; //lista di supporto per il '+inserisci_inizio'

    //Setto max come il numero di parole del vocabolario 
    max = *num_parole_voc;
    num_parole_amm = &max;

    //Alloco le parole di 18 char se k<18 in modo di non scrivere su aree di memoria non allocate
    massimo = max_num(k, 17);

    indovina = malloc(sizeof(char)*(k+1));
    parola = malloc(sizeof(char)*(massimo+1));

    //Inizia la vera e propria partita
    if(indovina!=NULL && parola!=NULL){
        //parola di riferimento
        if(scanf("%s", indovina)){}

        //riempo occ_indovina
        for(i=0;i<k;i++){
            occ_indovina[get_indice(indovina[i])]++;
        }

        //numero di tentativi
        if(scanf("%d", &n)){}

        //inizia la partita
        for(i=0; i<n+1; i++){
            if(scanf("%s", parola)){}

            // Controllo se ho finito l'inserimento -> Se si finisce la partita
            if(feof(stdin)){
                free(indovina);
                free(parola);
                freeList(parole_ammissibili);
                break;
            }

            //Se il primo carattere è un + allora è un comando
            if(parola[0] == '+'){
                //se inserisco +stampa_filtrate
                if(strcmp(parola, "+stampa_filtrate")==0){
                    //Se parole ammissibili == NULL scorro direttamente l'albero, perchè non ho effettuato confronti
                    if(parole_ammissibili != NULL){
                        visualizza(parole_ammissibili);
                    }else{
                        inorder(T, T->radice);
                    }
                    i--;
                    continue;
                }

                //se voglio fare una nuova partita
                if(strcmp(parola, "+nuova_partita")==0){
                    //Svuoto la struttura dei vincoli prima di una nuova partita
                    for(int j=0; j<N; j++){
                        for(int m=0;m<k;m++){
                            vincolo[j]->corrispondenza[m] = 'p';
                            indovinate_already[m] = '*';
                        }
                        vincolo[j]->appartenenza = 'p';
                        vincolo[j]->num_min = 0;
                        vincolo[j]->num_esatto = -1;
                    }
                    freeList(parole_ammissibili);
                    free(indovina);
                    free(parola);
                    return 1;
                }

                //se voglio inserire delle altre parole al vocabolario o alle parole ammissibili
                if(strcmp(parola, "+inserisci_inizio")==0){
                    while(1){
                        if(scanf("%s", parola)){}
                        if(strcmp("+inserisci_fine", parola)==0){
                            break;
                        }
                        BST_Insert(T, parola, k);
                        //se parole ammissibili != NULL vuol dire che ho almeno fatto un confronto
                        if(parole_ammissibili != NULL){
                            if(compatibile_parola(vincolo, parola, k, indovina, indovinate_already)){
                                support_list = inserisciInOrdine(support_list, parola, k);
                                *num_parole_amm = *num_parole_amm + 1;
                            }
                        }else{
                            *num_parole_amm = *num_parole_amm + 1;
                        }
                        *num_parole_voc = *num_parole_voc +1;
                    }
                    if(parole_ammissibili!=NULL && support_list != NULL){
                        parole_ammissibili = merge(parole_ammissibili, support_list, k);
                        freeList(support_list);
                        support_list = NULL;
                    }
                    i--;
                    continue;
                }
            }

            //se le stringhe sono uguali stampo "ok" e ritorno la lista
            if(strcmp(parola, indovina)==0){
                printf("ok\n");
                continue;
            }

            //controlla se la parola inserita appartiene al vocabolario, se si esegue il confronto
            if(tree_search(T, T->radice, parola)){
                parole_ammissibili = confronta(T, k, num_parole_amm, indovina, parola, occ_indovina, parole_ammissibili, vincolo, indovinate_already);
                if(i==(n-1)){
                    printf("ko\n");
                }
            }else{
                //Entro se la parola non è contenuta nell'albero e quindi se la parola non è all'interno del vocabolario
                printf("not_exists\n");
                i--;
            }
        }
    }
    return 0;
}

//Funzione che controlla e confronta la parola rispetto ad i vicnoli precedenti
//e sistema i vincoli in base a quanto scoperto con la nuova parola inserita
nodo_t* confronta(albero *T, short k, int *num_parole_amm, char *indovina, char *inserita, short *occ_indovina, nodo_t* parole_ammissibili, vincolo_t **vincolo, char *indovinate_already){
    int i;
    char *risultato;
    short corr_inserita[N] = {0};
    short sbag_inserita[N] = {0};
    short occ_parola[N] = {0};
    short char_idx;


    risultato = malloc(sizeof(char)*k+1);
    risultato[k] = '\0';

    if(risultato != NULL){

        //riempo corr_inserita e temp_occ_indovina
        for(i=0;i<k;i++){
            char_idx = get_indice(inserita[i]);
            occ_parola[char_idx]++;
            if(inserita[i]==indovina[i]){
                corr_inserita[char_idx]++;
            }
        }

        //Confronto
        for(i=0;i<k;i++){
            char_idx = get_indice(inserita[i]);
            //caso in cui le due lettere sono uguali e nella stessa posizione
            if(inserita[i]==indovina[i]){
                risultato[i] = '+';
                indovinate_already[i] = inserita[i];
                vincolo[char_idx]->appartenenza = 'v';
                vincolo[char_idx]->corrispondenza[i] ='v';
                continue;
            }

            //caso in cui la lettera in questione non è proprio contenuta in indovina
            if(occ_indovina[char_idx]==0){
                vincolo[char_idx]->appartenenza = 'f';
                risultato[i] = '/';
                sbag_inserita[char_idx]++;
                continue;
            }

            //caso in cui char_idx è inserita in una pos scorretta e abbiamo terminato il num. di occ disponibili rispetto alle sbagliate
            if(sbag_inserita[char_idx] >= occ_indovina[char_idx] - corr_inserita[char_idx]){
                risultato[i] = '/';
                sbag_inserita[char_idx]++;
                vincolo[char_idx]->corrispondenza[i] ='f';
                continue;
            }else{
                //caso in cui la lettera è contenuta ma in una pos. sbagliata
                risultato[i] = '|';
                sbag_inserita[char_idx]++;
                vincolo[char_idx]->appartenenza = 'v';
                vincolo[char_idx]->corrispondenza[i] ='f';
                continue;
            }

        }
        
        //gestione di numero esatto e numero minimo
        for(i=0; i<k; i++){
            char_idx = get_indice(inserita[i]);
            if(occ_parola[char_idx]>occ_indovina[char_idx]){
                vincolo[char_idx]->num_esatto = occ_indovina[char_idx];
                vincolo[char_idx]->num_min = vincolo[char_idx]->num_esatto;
            }else if(occ_parola[char_idx] == occ_indovina[char_idx]){
                vincolo[char_idx]->num_min = occ_indovina[char_idx];
            }else if(occ_parola[char_idx] < occ_indovina[char_idx]){
                if(vincolo[char_idx]->num_min < occ_parola[char_idx]){
                    vincolo[char_idx]->num_min = occ_parola[char_idx];
                }
            }
        }

        parole_ammissibili = aggiorna_parole_ammissibili(vincolo, parole_ammissibili, T, num_parole_amm, k, indovina, indovinate_already);

        printf("%s\n", risultato);
        printf("%d\n", *num_parole_amm);

        free(risultato);
    }

    return parole_ammissibili;
}

//Funzione che aggiorna le parole ammissibili in base alle informazioni raccolte dall'ultimo inserimento
nodo_t* aggiorna_parole_ammissibili(vincolo_t** vincolo, nodo_t* parole_ammissibili, albero *T, int *num_parole, short k, char *indovina, char *indovinate_already){
    nodo_t *tmp = NULL;
    nodo_t *prec = NULL;
    short char_idx;
    short rimossa = 0;
    int max = *num_parole;
    short occ_parola[N];
    int i, j;

    //Entro qui se è il primo confronto
    if(parole_ammissibili == NULL){
        parole_ammissibili = postorder_insert(vincolo, T, T->radice, parole_ammissibili, num_parole, k, indovina, indovinate_already);
        return parole_ammissibili;
    }

    tmp = parole_ammissibili;
    prec = tmp;
    //Dopo il primo confronto entro qui
    for(i=0; i<max && tmp; i++){

        //Se sono sulla parola da indovinare la lascio e vado avanti
        if(strcmp(tmp->parola, indovina) == 0){
            prec = tmp;
            tmp = tmp->next;
            continue;
        }

        //Svuoto le occorrenze della parola 
        for(j=0; j<N; j++){
            occ_parola[j] = 0;
        }

        //inizializzo le occorenze della parola
        for(j=0; j<k; j++){
            occ_parola[get_indice(tmp->parola[j])]++;
        }

        //Gestione delle parole ammissibili
        for(j=0; j<k; j++){

            //Se conosco l'elemento in posizione j-esima verifico se tmp->parola lo contiene
            if(indovinate_already[j] != '*' && indovinate_already[j] != tmp->parola[j]){
                tmp = rimuovi_nodo(prec, tmp, &parole_ammissibili);
                *num_parole= *num_parole - 1;
                rimossa = 1;
                break;
            }

            //Prendo l'indice
            char_idx = get_indice(tmp->parola[j]);

            //Se la lettere non appartiene alla parola la scarto
            if(vincolo[char_idx]->appartenenza == 'f'){
                tmp = rimuovi_nodo(prec, tmp, &parole_ammissibili);
                *num_parole= *num_parole - 1;
                rimossa = 1;
                break;

            }else if(vincolo[char_idx]->appartenenza == 'v'){
                //Se la lettera appartiene alla parola controllo i vincoli
                if(vincolo[char_idx]->corrispondenza[j]=='f'){
                    tmp = rimuovi_nodo(prec, tmp, &parole_ammissibili);
                    *num_parole= *num_parole - 1;
                    rimossa = 1;
                    break;
                }
            }

            //Controllo numero esatto o numero minimo
            if(vincolo[char_idx]->num_esatto!=-1){
                if(occ_parola[char_idx] != vincolo[char_idx]->num_esatto){
                    tmp = rimuovi_nodo(prec, tmp, &parole_ammissibili);
                    *num_parole= *num_parole - 1;
                    rimossa = 1;
                    break;
                }
            }else{
                //Se non conosciamo num_esatto controllo il num_minimo
                if(occ_parola[char_idx] < vincolo[char_idx]->num_min){
                    tmp = rimuovi_nodo(prec, tmp, &parole_ammissibili);
                    *num_parole= *num_parole - 1;
                    rimossa = 1;
                    break;
                }
            }

            char_idx = get_indice(indovina[j]);
            //Se la lettera della pos. j è stata controllata controllo se la parola inserita la contiene
            if(vincolo[char_idx]->appartenenza == 'v'){
                if(occ_parola[char_idx] == 0){
                    tmp = rimuovi_nodo(prec, tmp, &parole_ammissibili);
                    *num_parole= *num_parole - 1;
                    rimossa = 1;
                    break;
                }
            }
        }

        //Se rimuovo la parola entro qui
        if(rimossa){
            rimossa = 0;
            continue;
        }else{
            prec = tmp;
            tmp = tmp->next;
        }

    }
    return parole_ammissibili;
}

//Controlla se la parola è compatibile con i vincoli trovati o se è la parola da indovinare
short compatibile_parola(vincolo_t** vincolo, char *parola, short k, char *indovina, char *indovinate_already){
    short char_idx;
    short flag = 1;
    short occ_parola[N] = {0};
    short m;

    //se la parola è quella da indovinare la inserisco senza fare controlli
    if(strcmp(parola, indovina) == 0){
        return flag;
    }

    //riempo le occorrenze della parola inserita
    for(m=0; m<k; m++){
        occ_parola[get_indice(parola[m])]++;
    }

    //Fase di Controllo
    for(int j=0; j<k; j++){
        //se conosco il contenuto dell'i-esimo posizione controllo che la parola lo abbia
        if(indovinate_already[j] != '*' && indovinate_already[j] != parola[j]){
            flag = 0;
            break;
        }

        char_idx = get_indice(parola[j]);

        //se la lettere non appartiene alla parola la scarto
        if(vincolo[char_idx]->appartenenza == 'f'){
            flag = 0;
            break;
            //se la lettera appartiene alla parola controllo i vincoli
        }else if(vincolo[char_idx]->appartenenza == 'v'){
            if(vincolo[char_idx]->corrispondenza[j]=='f'){
                flag = 0;
                break;
            }
        }

        //Controllo il numero esatto ed il numero minimo
        if(vincolo[char_idx]->num_esatto!=-1){
            if(occ_parola[char_idx] != vincolo[char_idx]->num_esatto){
                flag = 0;
                break;
            }
        }else{
            //Se non conosciamo il num_esatto controllo il num_min
            if(occ_parola[char_idx]<vincolo[char_idx]->num_min){
                flag = 0;
                break;
            }
        }

        char_idx = get_indice(indovina[j]);
        //se abbiamo trovato la lettera in pos. j-esima di indovina controllo se la parola la contiene
        if(vincolo[char_idx]->appartenenza == 'v'){
            if(occ_parola[char_idx] == 0 ){
                flag = 0;
                break;
            }
        }

    }
    return flag;
}

//Funzione di inserimento in testa per la lista
nodo_t* inserisciInTesta(nodo_t* lista, char *parola, short k){
    nodo_t *tmp;
    tmp = malloc(sizeof(nodo_t));
    tmp->parola = malloc(sizeof(char)*(k+1));
    if(tmp != NULL){
        //copio la stringa
        for(int i=0; i<k;i++){
            tmp->parola[i] = parola[i];
        }
        tmp->parola[k] = '\0';
        tmp->next = lista;
        lista = tmp;
    }else
        printf("Memoria esaurita!\n");
    return lista;
}

//Funzione di inserimento in coda per la lista
nodo_t* inserisciInCoda(nodo_t* lista, char *parola, short k){
    nodo_t *prec;
    nodo_t *tmp;
    tmp = malloc(sizeof(nodo_t));
    tmp->parola = malloc(sizeof(char)*(k+1));
    if(tmp != NULL){
        tmp->next = NULL;
        //copio la stringa
        for(int i=0; i<k;i++){
            tmp->parola[i] = parola[i];
        }
        tmp->parola[k] = '\0';
        if(lista == NULL)
            lista = tmp;
        else{
            for(prec=lista;prec->next!=NULL;prec=prec->next);
            prec->next = tmp;
        }
    }else
        printf("Memoria esaurita!\n");
    return lista;
}

//Funzione di inserimento in ordine per la lista
nodo_t* inserisciInOrdine(nodo_t* lista, char *parola, short k){
    nodo_t *current = NULL;
    nodo_t *tmp = NULL;

    if(lista == NULL || strcmp(lista->parola, parola)>0){
        lista = inserisciInTesta(lista, parola, k);
        return lista;
    }

    tmp = malloc(sizeof(nodo_t));
    tmp->parola = malloc(sizeof(char)*(k+1));
    //copio la parola
    for(int i=0; i<k;i++){
        tmp->parola[i] = parola[i];
    }
    tmp->parola[k] = '\0';
    tmp->next = NULL;

    current = lista;
    while (current->next != NULL && strcmp(current->next->parola, parola) < 0){
        current = current->next;
    }

    tmp->next = current->next;
    current->next = tmp;

    return lista;
}

//Funzione di inserimento per le parole nel BST
void BST_Insert(albero *T, char *z, short k){
    nodo *temp = malloc(sizeof(nodo));
    temp->parola = malloc(sizeof(char)*k+1);
    nodo *y = NULL;
    nodo *x = T->radice;
    //copio la parola
    for(int i=0; i<k;i++){
        temp->parola[i] = z[i];
    }
    temp->parola[k] = '\0';
    while(x!= NULL){
        y = x;
        if(strcmp(temp->parola, x->parola)<0)
            x = x->left;
        else
            x = x->right;
    }
    if(y == NULL){
        T->radice = temp;  //Entro qui se l'albero era vuoto
    }else if(strcmp(temp->parola, y->parola)< 0){
        y->left = temp;
    }else{
        y->right = temp;
    }
    temp->left = NULL;
    temp->right = NULL;
}

//Funzione che cerca una parola nell'BST e mi dice se la parola è presente oppure no
int tree_search(albero *T, nodo *nodo, char *parola){
    if(nodo == NULL)
        return 0;
    else if(strcmp(nodo->parola, parola)==0)
        return 1;

    if(strcmp(parola, nodo->parola)<0)
        return tree_search(T, nodo->left, parola);
    else
        return tree_search(T, nodo->right, parola);
}

//Funzione che stampa tutte le parole presenti nella lista passata
void visualizza(nodo_t* lista){
    while(lista){
        printf("%s\n", lista->parola);
        lista = lista->next;
    }
}

//Funzione chiamata dalla BST_Insert per sistemare l'albero
void inorder(albero *T, nodo *z){
    if(z != NULL){
        inorder(T, z->left);
        printf("%s\n", z->parola);
        inorder(T, z->right);
    }
}

//Funzione chiamata dalla BST_Insert per sistemare l'albero
nodo_t* postorder_insert(vincolo_t** vincolo, albero *T, nodo *z, nodo_t *parole_ammissibili, int *num_parole, short k, char* indovina, char* indovinate_already){
    if(z!=NULL){
        parole_ammissibili = postorder_insert(vincolo, T, z->right, parole_ammissibili, num_parole, k, indovina, indovinate_already);
        if(compatibile_parola(vincolo, z->parola, k, indovina, indovinate_already)){
            parole_ammissibili = inserisciInTesta(parole_ammissibili, z->parola, k);
        }else{
            *num_parole= *num_parole - 1;
        }
        parole_ammissibili = postorder_insert(vincolo, T, z->left, parole_ammissibili, num_parole, k, indovina, indovinate_already);
    }
    return parole_ammissibili;
}

//Funzione che restituisce il massimo tra due numeri
short max_num(short n1, short n2){
    if(n1>n2)
        return n1;
    return n2;
}

//Funzione che fa la free di una lista
void freeList(nodo_t* head){
    nodo_t* tmp;
    while (head != NULL){
        tmp = head;
        head = head->next;
        free(tmp->parola);
        free(tmp);
    }
}

//Funzione che fa la free di tutto un albero
void inorder_free(albero *T, nodo *z){
    if(z != NULL){
        inorder_free(T, z->left);
        inorder_free(T, z->right);
        free(z->parola);
        free(z);
    }
}

//Funzione che rimuove un nodo da una lista
nodo_t* rimuovi_nodo(nodo_t* prec, nodo_t* curr, nodo_t** lst){
    nodo_t *tmp;
    
    //Rimuovi la testa
    if(curr == *lst){
        tmp = curr;
        curr = curr->next;
        *lst = curr;
    }else{
    //Rimuovi un elemento che non è la testa
        tmp = curr;
        curr = curr->next;
        prec->next = curr;
    }
    
    free(tmp->parola);
    free(tmp);
    
    return curr;
}


//Funzione che unisce due liste 
nodo_t* merge(nodo_t *lista, nodo_t *newelem, short k){
    nodo_t* tmp1 = NULL;
    nodo_t* tmp2 = NULL;
    nodo_t* prec = NULL;
    nodo_t* save = NULL;
    
    tmp2 = newelem;
    
    if(strcmp(tmp2->parola, lista->parola)<0){
        lista = inserisciInTesta(lista, tmp2->parola, k);
        tmp2 = tmp2->next;
    }
    
    if(tmp2 != NULL){
        prec = lista;
        for(tmp1 = lista->next; tmp1 && tmp2; prec = tmp1, tmp1 = tmp1->next){
            if(strcmp(tmp2->parola, tmp1->parola)<0){
                save = newNode(tmp2->parola, k);
                prec->next = save;
                save->next = tmp1;
                prec = save;
                tmp2 = tmp2->next;
                while(tmp2 && strcmp(tmp2->parola, tmp1->parola)<0){
                    save = newNode(tmp2->parola, k);
                    prec->next = save;
                    save->next = tmp1;
                    prec = save;
                    tmp2 = tmp2->next;
                }
            }
        }
        if(tmp1 == NULL && tmp2 != NULL){
            while(tmp2){
                lista = inserisciInCoda(lista, tmp2->parola, k);
                tmp2 = tmp2->next;
            }
        }
    }
    return lista;
}

//Funzione che crea un nuovo nodo di una lista
nodo_t* newNode(char* parola, short k){
    nodo_t *newNode = NULL;

    newNode = malloc(sizeof(nodo_t));
    newNode->parola = malloc(sizeof(char)*k+1);

    for(int i=0; i<k;i++){
           newNode->parola[i] = parola[i];
        }
        newNode->parola[k] = '\0';

    return newNode;
}