#include <stdio.h>
#include <stdlib.h>
#include "libvpp.h"
#define MAX 50

int main(int argc, char *argv[])
{
    if ((argc < 3) && (strcmp(argv[1], "-h") != 0)) {
        printf("Uso: vina++ <opção> <archive> [membro1 membro2 ...]\n");
        return 0;
    }

    char* caminhoArchive;
    char* caminhoArquivo;
    char* caminhoArqAnterior;
    char* op = argv[1];

    //insere arquivos
    if (strcmp(op, "-i") == 0) {
        caminhoArchive = argv[2];
        for (int i = 3; i < argc; i++ ) {
            caminhoArquivo = argv[i];
            inserir(caminhoArchive, caminhoArquivo, 0);
        }
        return 0;
    }

    //insere com flag a
    if (strcmp(op, "-a") == 0) {
        caminhoArchive = argv[2];
        for (int i = 3; i < argc; i++ ) {
            caminhoArquivo = argv[i];
            inserir(caminhoArchive, caminhoArquivo, 1);
        }
        return 0;
    }

    //move um membro para depois de outro
    if (strcmp(op, "-m") == 0) {
        caminhoArqAnterior = argv[2];
        caminhoArchive = argv[3];
        caminhoArquivo = argv[4];
        excluir(caminhoArchive, caminhoArquivo);
        inserirApos(caminhoArchive, caminhoArquivo, caminhoArqAnterior);
        return 0;
    }

    //extrai
    if (strcmp(op, "-x") == 0) {
        caminhoArchive = argv[2];
        if (argc == 3)
            extrairTodos(caminhoArchive);
        else {
            for (int i = 3; i < argc; i++) {
            caminhoArquivo = argv[i];
            extrairArquivo(caminhoArchive, caminhoArquivo);
            }
        }
        return 0;
    }

    //remove
    if (strcmp(op, "-r") == 0) {
        caminhoArchive = argv[2];
        for (int i = 3; i < argc; i++){
            caminhoArquivo = argv[i];
            excluir(caminhoArchive, caminhoArquivo);
        }
        return 0;
    }

    //lista
    if (strcmp(op, "-c") == 0) {
        caminhoArchive = argv[2];
        printaInfoArquivos(caminhoArchive);
        return 0;
    }

    //ajuda
    else if (strcmp(op, "-h") == 0) {
        printf("Uso: vina++ <opção> <archive> [membro1 membro2 ...]\n");
        printf("Opcoes: \n-i para inserir\n-a para atualizar\n-m para mover\n-x para extrair\n-r para remover\n-c para listar\n");
        return 0;
    }
    
    else
        printf("Uso: vina++ <opção> <archive> [membro1 membro2 ...]\n");

    return 0;
}
