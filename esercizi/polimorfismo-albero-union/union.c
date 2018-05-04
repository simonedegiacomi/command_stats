#include <stdio.h>
#include <string.h>

typedef struct Nodo Nodo;

typedef enum TipoNodo { 
    PipeT,
    ComandoT
    
} TipoNodo;


typedef struct Pipe { 
    Nodo *a, *b;
} Pipe;

typedef struct Comando { 
    char nome[255];
} Comando;


typedef union DatiNodo {
    Pipe pipe;
    Comando comando;
} DatiNodo;


struct Nodo {
    TipoNodo tipo;

    DatiNodo dati;
};


void printaCose (Nodo *n) {
    switch (n->tipo) {
        case PipeT: {
            Pipe p = n->dati.pipe;

            printf("Questa è una pipe: [\n");
            printf("\tA: ");
            printaCose(p.a);
            printf("\tB: ");
            printaCose(p.b);
            printf("]\n");
            break;
        }

        case ComandoT: {
            Comando c = n->dati.comando;
            printf("Questo è un comando, nome: %s\n", c.nome);
            break;
        }
    }
}

int main () {

    Nodo ls, wc, p;

    ls.tipo = wc.tipo = ComandoT;
    p.tipo = PipeT;

    p.dati.pipe.a = &ls;
    p.dati.pipe.b = &wc;
    strcpy(ls.dati.comando.nome, "ls");
    strcpy(wc.dati.comando.nome, "wc");



    printaCose(&p);
}

