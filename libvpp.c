#include "libvpp.h"
#define INICIO_ARQS 12


//cria archive vazio
void criaArchiver(char *caminho)
{
    FILE* arq = fopen(caminho, "w");
    int numArq = 0; off_t localDir = INICIO_ARQS;
    fwrite(&numArq, sizeof(int), 1, arq);
    fwrite(&localDir, sizeof(size_t), 1, arq);
    fclose(arq);
}

//retorna uma struct com as informacoes do aquivo
infoArq_t *coletaInformacoes(char *caminho)
{
    infoArq_t *info = malloc(sizeof(infoArq_t));
    struct stat s;
    stat(caminho , &s);
    info->caminho = caminho;
    info->pos = 0;
    info->userID = s.st_uid;
    info->permissoes = s.st_mode;
    info->tamanho = s.st_size;
    info->data = s.st_mtime;
    return info;
}

//printa as informacoes do arquivo
void printaInfo(infoArq_t *infoArq)
{
    printf("Arquivo: %s \n", infoArq->caminho);
    printf("Posição no archive: %d \n", infoArq->pos);
    printf("Localizacao no archiver: %ld bytes apos inicio\n", infoArq->local);
    printf("UserID: %d \n", infoArq->userID);
    printf("Bits de permissao: 0x%X \n", infoArq->permissoes);
    printf("Tamanho em bytes: %ld \n", infoArq->tamanho);
    struct tm *data = localtime(&infoArq->data);
    printf("Ultima modificacao: %2d/%2d/%4d \n", data->tm_mday, data->tm_mon + 1, data->tm_year + 1900);
}

//exibe o diretorio 
void printaDiretorio(infoArq_t **diretorio, int tam)
{
    for (int i = 0; i < tam; i++) {
        printaInfo(diretorio[i]);
        printf("----------\n");
    }
}

//libera memoria do diretorio
void liberaDiretorio(infoArq_t **diretorio, int tam)
{
    if (!diretorio)
        return;
    for (int i = 0; i < tam; i++) 
        free(diretorio[i]);
    free(diretorio);  
    return;
}

//recebe um arquivo .vpp, retorna seu diretorio, o numero de arquivos e o local do diretorio
infoArq_t **lerDiretorio(char* caminhoArq, int *tam, off_t *localDir)
{
    FILE *arq = fopen(caminhoArq, "r");
    if (!arq)
        return NULL;
    
    fread(tam, sizeof(int), 1, arq);
    fread(localDir, sizeof(off_t),1 , arq);

    if (tam == 0)
        return NULL;

    fseek(arq, *localDir, SEEK_SET);
    infoArq_t** diretorio = malloc(*tam * (sizeof(infoArq_t*)));
    infoArq_t* novo;
    char *caminhoLido;

    for (int i = 0; i < *tam; i++) {
        novo = malloc(sizeof(infoArq_t));

        //le caminho do arquvivo (string terminada em \0)
        int tamCaminho = 1;
        caminhoLido = malloc(tamCaminho * sizeof(char));
        fread(&caminhoLido[tamCaminho - 1], sizeof(char), 1, arq);
        while (caminhoLido[tamCaminho - 1] != '\0') {
            tamCaminho++;
            caminhoLido = realloc(caminhoLido, tamCaminho * sizeof(char));
            fread(&caminhoLido[tamCaminho - 1], sizeof(char), 1, arq);
        }
        novo->caminho = caminhoLido;

        //le demais informacoes do arquivo
        fread(&novo->pos, sizeof(novo->pos), 1, arq);
        fread(&novo->local, sizeof(novo->local), 1, arq);
        fread(&novo->userID, sizeof(novo->userID), 1, arq);
        fread(&novo->permissoes, sizeof(novo->permissoes), 1 , arq);
        fread(&novo->tamanho, sizeof(novo->tamanho), 1 , arq);
        fread(&novo->data, sizeof(novo->data), 1, arq);
        diretorio[i] = novo;
    }
    fclose(arq);
    return diretorio;
}

//escreve diretorio no arquivo passado
void escreverDiretorio(FILE* arq, infoArq_t** diretorio, int tam)
{
    infoArq_t *atual;
    for (int i = 0; i < tam; i++) {
        atual = diretorio[i];
        fwrite(atual->caminho, strlen(atual->caminho), 1, arq);
        fwrite("\0", 1, 1, arq);
        fwrite(&atual->pos, sizeof(atual->pos), 1, arq);
        fwrite(&atual->local, sizeof(atual->local), 1, arq);
        fwrite(&atual->userID, sizeof(atual->userID), 1, arq);
        fwrite(&atual->permissoes, sizeof(atual->permissoes), 1, arq);
        fwrite(&atual->tamanho, sizeof(atual->tamanho), 1, arq);
        fwrite(&atual->data, sizeof(atual->data), 1 , arq);
    }
}

//exibe as informacoes dos arquivos contidos em um archive
void printaInfoArquivos(char *caminhoArchive)
{
    int tam; off_t localDir;
    infoArq_t** dir = lerDiretorio(caminhoArchive, &tam, &localDir);
    printaDiretorio(dir, tam);
    liberaDiretorio(dir, tam);
}

//insere novo arquivo no diretorio de tamanho tam na posicao pos
infoArq_t **insereNoDiretorio(infoArq_t **diretorio, int tam, int pos, char *caminhoArquivo)
{
    infoArq_t *novoArq = coletaInformacoes(caminhoArquivo);
    tam++;
    if (tam == 1) 
        diretorio = malloc(sizeof(infoArq_t*));
    else {
        diretorio = realloc(diretorio, tam * sizeof(infoArq_t*));
        for (int i = tam - 1; i > pos; i--) {
            diretorio[i] = diretorio[i - 1];
            diretorio[i]->pos = diretorio[i]->pos + 1;
            diretorio[i]->local = diretorio[i]->local + novoArq->tamanho;
        }
    }
    if (pos == 0)
        novoArq->local = INICIO_ARQS;
    else
        novoArq->local = diretorio[pos - 1]->local + diretorio[pos - 1]->tamanho;

    novoArq->pos = pos + 1;
    diretorio[pos] = novoArq;
    return diretorio;
}

//exclui o arquivo na posicao pos de um diretorio de tamanho tam
infoArq_t **excluiDoDiretorio(infoArq_t **diretorio, int tam, int pos)
{
    int tamExcluido = diretorio[pos]->tamanho;
    for (int i = pos; i < tam - 1; i++) {
        diretorio[i]->caminho = diretorio[i + 1]->caminho;
        diretorio[i]->local = diretorio[i + 1]->local - tamExcluido;
        diretorio[i]->userID = diretorio[i + 1]->userID;
        diretorio[i]->permissoes = diretorio[i + 1]->permissoes;
        diretorio[i]->tamanho = diretorio[i + 1]->tamanho;
        diretorio[i]->data = diretorio[i + 1]->data;
    }
    tam--;
    diretorio = realloc(diretorio, tam * sizeof(infoArq_t*));
    return diretorio;
}

//copia n bytes do original para copia em blocos de 1024 bytes
void copiaEmBlocos(FILE* original, FILE* copia, size_t n) {
    int numBlocos = n / 1024;
    size_t resto = n % 1024;
    void *buff = malloc(1024);
    for (int i = 0; i < numBlocos; i++) {
        fread(buff, 1024, 1, original);
        fwrite(buff, 1024, 1, copia);
    }
    buff = realloc(buff, resto);
    fread(buff, resto, 1, original);
    fwrite(buff, resto, 1, copia);
    free(buff);
}

//insere novo arquivo no fim do archive, removendo o antigo se for preciso
void inserir(char *caminhoArchiver, char *caminhoNovoArquivo, int flagA)
{
    //cria archive caso ele nao exista
    if (access(caminhoArchiver, F_OK) != 0)
        criaArchiver(caminhoArchiver);

    //le diretorio
    int tam; off_t localDir;
    infoArq_t** diretorio = lerDiretorio(caminhoArchiver, &tam, &localDir);

    //remove antigo, seguindo a indicacao da flag a
    for (int i = 0; (tam > 0) && (i < tam); i++) {
        if (strcmp(diretorio[i]->caminho, caminhoNovoArquivo) == 0){
            time_t dataNovo = coletaInformacoes(caminhoNovoArquivo)->data;
            time_t dataAntigo = diretorio[i]->data;
            if ((!flagA) || dataNovo > dataAntigo)
                excluir(caminhoArchiver, caminhoNovoArquivo);
            else {
                liberaDiretorio(diretorio, tam);
                return;
            }    
            liberaDiretorio(diretorio, tam);
            diretorio = lerDiretorio(caminhoArchiver, &tam, &localDir);
        }
    }

    //insere novo arquivo ao fim do diretorio
    diretorio = insereNoDiretorio(diretorio, tam , tam, caminhoNovoArquivo);

    //abre arquivos
    FILE* novoArq = fopen(caminhoNovoArquivo, "r");
    FILE* arch = fopen(caminhoArchiver, "r+a");
    if (!arch || !novoArq) {
        liberaDiretorio(diretorio, tam);
        printf("impossivel abrir archive!\n");
        return;
    }

    //reescreve numero de arquivos e local do diretorio
    tam++;
    off_t novoLocalDir = diretorio[tam - 1]->local + diretorio[tam - 1]->tamanho;
    fseek(arch, 0, SEEK_SET);
    fwrite(&tam, sizeof(int), 1, arch);
    fwrite(&novoLocalDir, sizeof(off_t), 1, arch);

    //sobrescreve diretorio antigo com o novo arquivo e novo diretorio
    fseek(arch, localDir, SEEK_SET);
    copiaEmBlocos(novoArq, arch, diretorio[tam - 1]->tamanho);
    escreverDiretorio(arch, diretorio, tam);

    fclose(arch);
    fclose(novoArq);
    liberaDiretorio(diretorio, tam);
}

//extrai arquivo sem exclui-lo do archive
void extrairArquivo (char* caminhoArch, char* caminhoExtraido) {
    //le num arquivos, local do diretorio e diretorio
    int tam; off_t localDir;
    infoArq_t **diretorio = lerDiretorio(caminhoArch, &tam, &localDir);

    //busca arquivo a ser extraido
    int pos = 0;
    for (pos = 0; (pos < tam) && (strcmp(diretorio[pos]->caminho, caminhoExtraido) != 0); pos++);
    if (pos >= tam) {
        liberaDiretorio(diretorio, tam);
        printf("Arquivo nao encontrado!\n");
        return;
    }

    //extrai arquivo
    FILE* extraido = fopen(caminhoExtraido, "w");
    if (!extraido) {
        liberaDiretorio(diretorio, tam);
        printf("Impossivel abrir arquivo!\n");
        return;
    }
    FILE* arch = fopen(caminhoArch, "r");
    if (!arch) {
        liberaDiretorio(diretorio, tam);
        printf("impossivel abrir archive!\n");
        return;
    }
    fseek(arch, diretorio[pos]->local, SEEK_SET);
    copiaEmBlocos(arch, extraido, diretorio[pos]->tamanho);
    fclose(arch);
    fclose(extraido);

    //altera stats do arquivo extraido
    struct utimbuf novaHora;
    novaHora.modtime = diretorio[pos]->data;
    novaHora.actime = time(NULL);
    utime(caminhoExtraido, &novaHora);
    chown(caminhoExtraido, diretorio[pos]->userID, -1);
    chmod(caminhoExtraido, diretorio[pos]->permissoes);
    liberaDiretorio(diretorio, tam);
}

//extrai todos os arquivos sem exclui-los do archive
void extrairTodos(char *caminhoArchive)
{
    int tam; off_t localDir;
    infoArq_t** diretorio = lerDiretorio(caminhoArchive, &tam, &localDir);
    FILE *arch = fopen(caminhoArchive, "r");
    for (int i = 0; i < tam; i++){
        FILE* novo = fopen(diretorio[i]->caminho, "w");
        if (!novo) {
            liberaDiretorio(diretorio, tam);
            printf("Impossivel abrir arquivo!\n");
            return;
        }
        fseek(arch, diretorio[i]->local, SEEK_SET);
        copiaEmBlocos(arch, novo, diretorio[i]->tamanho);
        fclose(novo);
        struct utimbuf novaHora;
        novaHora.modtime = diretorio[i]->data;
        novaHora.actime = time(NULL);
        utime(diretorio[i]->caminho, &novaHora);
        chown(diretorio[i]->caminho, diretorio[i]->userID, -1);
        chmod(diretorio[i]->caminho, diretorio[i]->permissoes);
    }
    fclose(arch);
    liberaDiretorio(diretorio, tam);
}

//exclui arquivo do archive
void excluir(char *caminhoArchive, char *caminhoExtraido) {
    //le numero de arquivos, local do diretorio e diretorio
    int tam; off_t localDir;
    infoArq_t **diretorio = lerDiretorio(caminhoArchive, &tam, &localDir);

    //busca arquivo a ser excluido
    int pos = 0;
    for (pos = 0; (pos < tam) && (strcmp(diretorio[pos]->caminho, caminhoExtraido) != 0); pos++);
    if (pos == tam) {
        printf("Arquivo nao encontrado!\n");
        liberaDiretorio(diretorio, tam);
        return;
    }

    //exlcui arquivo do diretorio
    size_t tamExcluido = diretorio[pos]->tamanho;
    off_t localExcluido = diretorio[pos]->local;
    diretorio = excluiDoDiretorio(diretorio, tam, pos);

    //cria achive temporario
    tam--;
    off_t novoLocalDir = localDir - tamExcluido;
    FILE *tmp = fopen("tmp.vpp", "w");
    fwrite(&tam, sizeof(int), 1, tmp);
    fwrite(&novoLocalDir, sizeof(off_t), 1, tmp);

    //copia o archive para o archive temporario
    FILE *arch = fopen(caminhoArchive, "r");
    if (!arch) {
        printf("impossivel abrir archive\n");
        liberaDiretorio(diretorio, tam);
        return;
    }
    size_t numeroBytesAntesArquivo = localExcluido - INICIO_ARQS;
    size_t numeroBytesDepoisArquivo = localDir - localExcluido - tamExcluido;
    fseek(arch, INICIO_ARQS, SEEK_SET);
    copiaEmBlocos(arch, tmp, numeroBytesAntesArquivo);
    fseek(arch, tamExcluido, SEEK_CUR);
    copiaEmBlocos(arch, tmp, numeroBytesDepoisArquivo);
    escreverDiretorio(tmp, diretorio, tam);

    //exclui antigo archive e renomea o novo para substituir o antigo
    remove(caminhoArchive);
    rename("tmp.vpp", caminhoArchive);
    fclose(arch);
    fclose(tmp);
    liberaDiretorio(diretorio, tam);
}

//insere aquivo apos outro no archive
void inserirApos(char *caminhoArch, char *caminhoNovo, char *caminhoAnt)
{
    int tam; off_t localDir;
    infoArq_t** diretorio = lerDiretorio(caminhoArch, &tam, &localDir);
    int posAnterior = 0;
    for (posAnterior = 0; (posAnterior < tam) && (strcmp(diretorio[posAnterior]->caminho, caminhoAnt) != 0); posAnterior++);
    if (posAnterior == tam) {
        printf("Arquivo nao encontrado!\n");
        liberaDiretorio(diretorio, tam);
        return;
    }

    infoArq_t* infoNovo = coletaInformacoes(caminhoNovo);

    //criar temp, escrever novo tam e novo localDir
    FILE *tmp = fopen("tmp.vpp", "w");
    FILE *arch = fopen(caminhoArch, "r");
    FILE* novo = fopen(caminhoNovo, "r");
    tam++;
    off_t novoLocalDir = localDir + infoNovo->tamanho;
    fwrite(&tam, sizeof(int), 1, tmp);
    fwrite(&novoLocalDir, sizeof(off_t), 1, tmp);

    //copiar tudo ate o arq da pos
    fseek(arch, INICIO_ARQS, SEEK_SET);
    off_t numBytesAntes = diretorio[posAnterior]->local + diretorio[posAnterior]->tamanho - INICIO_ARQS;
    copiaEmBlocos(arch, tmp, numBytesAntes);

    //copiar novo arquivo
    copiaEmBlocos(novo, tmp, infoNovo->tamanho);

    //copiar resto do archive
    off_t numBytesDepois = localDir - numBytesAntes - INICIO_ARQS;
    copiaEmBlocos(arch, tmp, numBytesDepois);

    //escrever diretorio
    diretorio = insereNoDiretorio(diretorio, tam - 1, posAnterior + 1, caminhoNovo);
    escreverDiretorio(tmp, diretorio, tam);

    //substituir antigo archive pelo novo
    fclose(arch);
    fclose(tmp);
    fclose(novo);
    remove(caminhoArch);
    rename("tmp.vpp", caminhoArch);
    liberaDiretorio(diretorio, tam);
}
