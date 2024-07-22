#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "mpc_c.h"
#include "text-viewer.h"

#define TEXT_LINES 28
#define TEXT_COLUMNS 90

typedef struct
{
    char line[TEXT_COLUMNS];
} linha;

typedef struct
{
    int tam;
    linha *text;
} full_text;

char tela[APP_LINES][APP_COLUMNS];
char texto_na_tela[TEXT_LINES][TEXT_COLUMNS];
full_text texto;
bool text_active = false;
double paginas = 1.0;
int pagina_atual = 0;
bool click = false;
bool tecla = false;
bool tema_escuro = false;

// armazena o id das imagens
int trianguloUp, trianguloDown, trianguloUpDark, trianguloDownDark, lightMode, darkMode;

int countLines(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
        return -1;

    int lines = 0;
    int colunas = 0;
    char ch;
    int tamanhoPalavra = 0;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == ' ' || ch == '\n')
        {
            if (tamanhoPalavra > 0)
            { // ha uma palavra no array
                if (colunas + tamanhoPalavra >= TEXT_COLUMNS)
                {            // checa se ha espaco na linha
                    lines++; // move para a proxima linha
                    colunas = 0;
                }

                colunas += tamanhoPalavra;
                tamanhoPalavra = 0; // reseta
            }

            if (ch == '\n')
            {
                lines++;
                colunas = 0;
            }
            else
            {
                colunas++; // adiciona o espaco depois da palavra
            }
        }
        else
        { // continua construindo a palavra
            if (tamanhoPalavra < TEXT_COLUMNS)
            { // garante que o tamanho n transborde
                tamanhoPalavra++;
            }
        }

        if (colunas >= TEXT_COLUMNS)
        { // nao ultrapassa os limites
            lines++;
            colunas = 0;
        }
    }

    fclose(file);

    // calcula o resto, para saber se vai haver uma pagina pela metade no final
    double conta = (double)lines / 28;
    double frac = modf(conta, &paginas);
    paginas += (frac == 0.0 ? 0 : 1);

    return lines;
}

void loadTextFromFile(const char *filename)
{

    // conta qtas linhas tera o texto e o espaco livre da �ltima p�gina (caso necess�rio)
    texto.tam = countLines(filename);
    int linhas_em_pags_completas = TEXT_LINES * (paginas - 1);
    int linhas_na_pag_metade = texto.tam - linhas_em_pags_completas;
    int espacoLivre = (linhas_na_pag_metade == 0 ? 0 : TEXT_LINES - linhas_na_pag_metade);

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("File opening failed");
        return;
    }

    texto.text = (linha *)malloc((texto.tam + espacoLivre) * sizeof(linha));
    memset(texto.text, ' ', (texto.tam + espacoLivre) * TEXT_COLUMNS);

    int lin = 0, col = 0;
    char ch;
    char palavra[TEXT_COLUMNS]; // guarda palavras temporariamente
    int tamanhoPalavra = 0;

    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == ' ' || ch == '\n')
        {
            if (tamanhoPalavra > 0)
            { // ha uma palavra no array
                if (col + tamanhoPalavra >= TEXT_COLUMNS)
                {          // checa se ha espaco na linha
                    lin++; // move para a proxima linha
                    col = 0;
                }

                // copia a palavra para o texto
                for (int i = 0; i < tamanhoPalavra; i++)
                {
                    texto.text[lin].line[col++] = palavra[i];
                }
                tamanhoPalavra = 0; // reseta
            }

            if (ch == '\n')
            {
                lin++;
                col = 0;
            }
            else
            {
                texto.text[lin].line[col++] = ' '; // adiciona o espaco depois da palavra
            }
        }
        else
        { // continua construindo a palavra
            if (tamanhoPalavra < TEXT_COLUMNS)
            { // garante que o tamanho n transborde
                palavra[tamanhoPalavra++] = ch;
            }
        }

        if (col >= TEXT_COLUMNS || lin >= texto.tam)
        { // n ultrapassa os limites
            lin++;
            col = 0;
        }

        if (lin >= texto.tam)
            break; // caso exceder o numero de linhas
    }

    // aciona o texto, ou seja, fala que a variavel texto_na_tela pode comecar a receber texto
    text_active = !text_active;

    fclose(file);
}

// inicializacao
void initMpc(void)
{
    // mpc configuration
    mpcSetSize(APP_LINES, APP_COLUMNS);

    // mpc callbacks
    mpcSetMouseFunc(cbMouse);
    mpcSetUpdateFunc(cbUpdate);
    mpcSetKeyboardFunc(cbKeyboard);

    // cor do cursor
    mpcSetCursorColor(RED_3);

    // inicializa as matrizes com espacos, padrao de "vazio" no MPC
    memset(tela, ' ', APP_LINES * APP_COLUMNS);
    memset(texto_na_tela, ' ', TEXT_LINES * TEXT_COLUMNS);

    // posicao e tamanho da 'janela' onde as IMAGENS poderao aparecer
    mpcSetClippingArea(0, 0, APP_LINES, APP_COLUMNS);

    // carrega as imagens
    trianguloUp = mpcLoadBmp("./resources/triangleu.bmp");
    trianguloDown = mpcLoadBmp("./resources/triangled.bmp");
    trianguloUpDark = mpcLoadBmp("./resources/triangleuescuro.bmp");
    trianguloDownDark = mpcLoadBmp("./resources/triangledescuro.bmp");
    lightMode = mpcLoadBmp("./resources/light.bmp");
    darkMode = mpcLoadBmp("./resources/dark.bmp");


    mpcAbout();
}

void atualizaTextoNaTela()
{

    if (!text_active)
    {
        memset(texto_na_tela, ' ', TEXT_LINES * TEXT_COLUMNS); // se n ha texto ativo, exibe apenas a tela em branco
    }
    else
    {
        for (int l = 0; l < TEXT_LINES; l++)
        {
            for (int c = 0; c < TEXT_COLUMNS; c++)
            {
                texto_na_tela[l][c] = texto.text[l + (TEXT_LINES * pagina_atual)].line[c];
            }
        }
    }
}

void imprimeTextoNaTela()
{
    for (int l = 5; l < TEXT_LINES + 5; l++)
    {
        for (int c = 3; c < TEXT_COLUMNS + 3; c++)
        {
            if (!tema_escuro)
            {
                mpcSetChar(l, c, texto_na_tela[l - 5][c - 3], F_STD, BLACK, WHITE, 1);
            }
            else
            {
                mpcSetChar(l, c, texto_na_tela[l - 5][c - 3], F_STD, WHITE, GRAY_5, 1);
            }
        }
    }
}

void imprimeHora()
{

    for (int i = 0; i < APP_COLUMNS; i++)
    {
        if (!tema_escuro)
            {
                 mpcSetChar(0, i, ' ', F_STD, BLACK, BLACK, 1);
            }
            else
            {
                 mpcSetChar(0, i, ' ', F_STD, GRAY_5, GRAY_5, 1);
            }
    }

    // obtem hora
    time_t now = time(NULL);

    // converte
    struct tm *local_time = localtime(&now);

    // formata
    char hora[20];
    strftime(hora, sizeof(hora), "%d-%m-%Y %H:%M:%S", local_time);

    // imprime
    for (int i = 40; i < (sizeof(hora) + 39); i++)
    {
        if (!tema_escuro)
        {
                    mpcSetChar(0, i, hora[i - 40], F_STD, WHITE, BLACK, 1);

        }
        else
        {
                    mpcSetChar(0, i, hora[i - 40], F_STD, WHITE, GRAY_5, 1);

        }
    }
}

void desenhaBotaoTema() {
    // desenha as imagens
        if (!tema_escuro)
        {
            mpcShowImg(2, 85, darkMode, 1);
        }
        else
        {
                mpcShowImg(2, 85, lightMode, 1);
        }
}

void desenhaTela()
{
    for (int l = 0; l < APP_LINES; l++)
    {
        for (int c = 0; c < APP_COLUMNS; c++)
        {
            if (!tema_escuro)
            {
                mpcSetChar(l, c, tela[l][c], F_STD, BLACK, GRAY_1, 1);
            }
            else
            {
                mpcSetChar(l, c, tela[l][c], F_STD, WHITE, BLACK, 1);
            }
        }
    }

    desenhaBotaoTema();
    imprimeHora();

    if (!tema_escuro) {
        // desenha o botao load
    mostraTexto(2, 7, "LOAD");
    mpcVLine(23, 17, 60, BLACK, 1);
    mpcVLine(122, 17, 60, BLACK, 1);
    mpcHLine(17, 23, 122, BLACK, 1);
    mpcHLine(60, 23, 122, BLACK, 1);

    // desenha a caixa de texto
    mpcVLine(23, 73, 495, BLACK, 1);
    mpcVLine(745, 73, 495, BLACK, 1);
    mpcHLine(73, 23, 745, BLACK, 1);
    mpcHLine(495, 23, 745, BLACK, 1);
    }
    else
    {
        // desenha o botao load
    mostraTexto(2, 7, "LOAD");
    mpcVLine(23, 17, 60, WHITE, 1);
    mpcVLine(122, 17, 60, WHITE, 1);
    mpcHLine(17, 23, 122, WHITE, 1);
    mpcHLine(60, 23, 122, WHITE, 1);

    // desenha a caixa de texto
    mpcVLine(23, 73, 495, WHITE, 1);
    mpcVLine(745, 73, 495, WHITE, 1);
    mpcHLine(73, 23, 745, WHITE, 1);
    mpcHLine(495, 23, 745, WHITE, 1);
    }

    // se ha mais de uma pagina, sera necessario o scroll, logo ele sera exibido
    if (paginas > 1 && text_active)
    {

        // desenha as imagens
        if (!tema_escuro)
        {
            mpcShowImg(4, 94, trianguloUp, 1);
            mpcShowImg(31, 94, trianguloDown, 1);
        }
        else
        {
                mpcShowImg(4, 94, trianguloUpDark, 1);
        mpcShowImg(31, 94, trianguloDownDark, 1);
        }

        // desenha o scroll
        char b = ' ';
        int tamanho_scroll = 24;
        int inicio_scroll = 7;
        int tamanho_barrinha = tamanho_scroll / paginas;
        for (int i = 0; i < tamanho_barrinha; i++)
        {
            if (!tema_escuro)
            {
                mpcSetChar(inicio_scroll + (pagina_atual * tamanho_barrinha) + i, 96, b, F_STD, BLACK, BLACK, 1);
                mpcSetChar(inicio_scroll + (pagina_atual * tamanho_barrinha) + i, 97, b, F_STD, BLACK, BLACK, 1);
            }
            else
            {
                mpcSetChar(inicio_scroll + (pagina_atual * tamanho_barrinha) + i, 96, b, F_STD, WHITE, WHITE, 1);
                mpcSetChar(inicio_scroll + (pagina_atual * tamanho_barrinha) + i, 97, b, F_STD, WHITE, WHITE, 1);
            }
        }
    }
}

// funcao em loop
void displayApp(void)
{
    desenhaTela();
    atualizaTextoNaTela();
    imprimeTextoNaTela();
}

// funcao auxiliar
void mostraTexto(int l, int c, char *msg)
{
    for (int cont = 0; cont < strlen(msg); cont++)
    {
        if (!tema_escuro)
        {
            mpcSetChar(l, c + cont, msg[cont], F_STD, BLACK, GRAY_1, 1);
        }
        else
        {
            mpcSetChar(l, c + cont, msg[cont], F_STD, WHITE, BLACK, 1);
        }
    }
}

// callbacks
void cbMouse(int lin, int col, int button, int state)
{
    mpcSetCursorPos(lin, col);

    if (!click)
    {
        // carrega o texto
        if (state == 0 && lin > 0 && lin < 4 && col > 2 && col < 15)
        {
            loadTextFromFile("./resources/texto.txt");
            click = true;
        }

        // controla o scroll
        if (state == 0 && lin > 29 && lin < 33 && col > 94 && col < 99 && pagina_atual < (paginas - 1))
        {
            pagina_atual++;
            click = true;
        }
        else if (state == 0 && lin > 3 && lin < 7 && col > 94 && col < 99 && pagina_atual > 0)
        {
            pagina_atual--;
            click = true;
        }

        //controla o tema
        if (state == 0 && lin > 1 && lin < 4 && col > 84 && col < 95)
        {
            tema_escuro = !tema_escuro;
            click = true;
        }
    }

    // desativa o click => debouncer
    if (state == 1)
        click = false;

    // torna o cursor visivel apenas dentro da tela de texto
    if (lin > 4 && lin < 33 && col > 2 && col < 93)
    {
        mpcSetCursorVisible(true);
    }
    else
    {
        mpcSetCursorVisible(false);
    }
}

void cbKeyboard(int key, int modifier, bool special, bool up)
{
    if (!up && !tecla)
    {
        if (key == 101 && pagina_atual > 0)
        {
            pagina_atual--;
            tecla = true;
        }
        else if (key == 103 && pagina_atual < (paginas - 1))
        {
            pagina_atual++;
            tecla = true;
        }
        else if (key == 13)
        {
            loadTextFromFile("./resources/texto.txt");
            tecla = true;
        }
    }

    // debouncer
    if (up)
        tecla = false;
}

void cbUpdate(void)
{
    displayApp();
}
