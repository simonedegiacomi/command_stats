#include <stdio.h>
#include <string.h>

typedef enum TipoNodo { 
	PipeT,
	ComandoT
} TipoNodo;

typedef struct Nodo {
	TipoNodo tipo;
	void *valore;
} Nodo;

typedef struct Pipe { 
	Nodo *a, *b;
} Pipe;


typedef struct Comando { 
	char nome[255];
} Comando;


void printaCose (Nodo *n) {
	switch (n->tipo) {
		case PipeT: {
			Pipe *p = (Pipe*) n->valore;
			printf("Questa è una pipe: [\n");
			printf("\tA: ");
			printaCose(p->a);
			printf("\tB: ");
			printaCose(p->b);
			printf("]\n");
			break;
		}

		case ComandoT: {
			Comando *c = (Comando*) n->valore;
			printf("Questo è un comando, nome: %s\n", c->nome);
			break;
		}
	}
}

int main () {

	Nodo nodoP;
	Pipe p;
	Nodo nodoLs, nodoWc;

	nodoP.tipo = PipeT;
	nodoP.valore = (void *) &p;
	p.a = &nodoLs;
	p.b = &nodoWc;

	Comando ls, wc;

	nodoLs.tipo = ComandoT;
	nodoLs.valore = (void *) &ls;
	strcpy(ls.nome, "ls");
	
	nodoWc.tipo = ComandoT;
	nodoWc.valore = (void *) &wc;
	strcpy(wc.nome, "wc");



	printaCose(&nodoP);
	printaCose(&nodoLs);
	printaCose(&nodoWc);
}

