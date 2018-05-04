#include <stdio.h>
#include <string.h>

typedef enum TipoNodo { 
	PipeT,
	ComandoT
} TipoNodo;

typedef struct Nodo {
	TipoNodo tipo;
} Nodo;

typedef struct Pipe { 
	Nodo base;
	Nodo *a, *b;
} Pipe;


typedef struct Comando { 
	Nodo base;
	char nome[255];
} Comando;


void printaCose (Nodo *n) {
	switch (n->tipo) {
		case PipeT: {
			Pipe *p = (Pipe*) n;
			Comando *a = (Comando*) p->a;
			Comando *b = (Comando*) p->b;
			printf("Questa è una pipe: [\n");
			printf("\tA: ");
			printaCose(p->a);
			printf("\tB: ");
			printaCose(p->a);
			printf("]\n");
			break;
		}

		case ComandoT: {
			Comando *c = (Comando*) n;
			printf("Questo è un comando, nome: %s\n", c->nome);
			break;
		}
	}
}

int main () {

	Pipe p;
	Comando ls, wc;

	p.base.tipo = PipeT;
	p.a = &ls.base;
	p.b = &wc.base;

	ls.base.tipo = ComandoT;
	strcpy(ls.nome, "ls");
	
	wc.base.tipo = ComandoT;
	strcpy(wc.nome, "wc");



	printaCose(&p.base);
	printaCose(&ls.base);
	printaCose(&wc.base);
}

