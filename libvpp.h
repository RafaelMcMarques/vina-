#ifndef LIBVPP_H
#define LIBVPP_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <utime.h>

typedef struct {
    char *caminho;
    int pos;
    off_t local; 
    uid_t userID;
    mode_t permissoes;
    size_t tamanho;
    time_t data;
} infoArq_t;

void printaInfoArquivos(char *caminhoArchive);

void inserir(char *caminhoArch, char *caminhoNovoArq, int flagA);

void extrairArquivo (char* caminhoArch, char* caminhoExtraido);

void extrairTodos(char *caminhoArchive);

void excluir(char *caminhoArchive, char *caminhoExtraido);

void inserirApos(char *caminhoArch, char *caminhoNovo, char *caminhoAnt);

#endif