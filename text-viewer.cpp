#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mpc_c.h"
#include "text-viewer.h"

#define TEXT_LINES 28
#define TEXT_COLUMNS 90

typedef struct {
    char line[TEXT_COLUMNS];
} linha;

typedef struct {
    int tam;
    linha *text;
} full_text;


char tela[APP_LINES][APP_COLUMNS];
char texto_na_tela[TEXT_LINES][TEXT_COLUMNS];
full_text texto;
bool textActive = false;
double paginas = 1.0;
int pagina_atual = 0;
bool click = false;

//armazena o id das imagens
int trianguloUp;
int trianguloDown;

int countLines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    int lines = 0;
    int colunas = 0;
    char ch;
    int tamanhoPalavra = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == ' ' || ch == '\n') {
            if (tamanhoPalavra > 0) { // há uma palavra no array
                if (colunas + tamanhoPalavra >= TEXT_COLUMNS) { // checa se há espaço na linha
                    lines++; // move para a próxima linha
                    colunas = 0;
                }

                colunas += tamanhoPalavra;
                tamanhoPalavra = 0; // reseta
            }

            if (ch == '\n') {
                lines++;
                colunas = 0;
            } else {
                colunas++;// adiciona o espaço depois da palavra
            }

        } else { // continua construindo a palavra
            if (tamanhoPalavra < TEXT_COLUMNS) { // garante que o tamanho n transborde
                tamanhoPalavra++;
            }
        }

        if (colunas >= TEXT_COLUMNS) { // não ultrapassa os limites
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

void loadTextFromFile(const char* filename) {

    // conta qtas linhas terá o texto e o espaço livre da última página (caso necessário)
    texto.tam = countLines(filename);
    int linhas_em_pags_completas = TEXT_LINES * (paginas - 1);
    int linhas_na_pag_metade = texto.tam - linhas_em_pags_completas;
    int espacoLivre = (linhas_na_pag_metade == 0 ? 0 : TEXT_LINES - linhas_na_pag_metade);

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("File opening failed");
        return;
    }

    texto.text = (linha *)malloc((texto.tam + espacoLivre) * sizeof(linha));
    memset(texto.text,' ',(texto.tam + espacoLivre)*TEXT_COLUMNS);

    int lin = 0, col = 0;
    char ch;
    char palavra[TEXT_COLUMNS]; // guarda palavras temporariamente
    int tamanhoPalavra = 0;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == ' ' || ch == '\n') {
            if (tamanhoPalavra > 0) { // há uma palavra no array
                if (col + tamanhoPalavra >= TEXT_COLUMNS) { // checa se há espaço na linha
                    lin++; // move para a próxima linha
                    col = 0;
                }

                // copia a palavra para o texto
                for (int i = 0; i < tamanhoPalavra; i++) {
                    texto.text[lin].line[col++] = palavra[i];
                }
                tamanhoPalavra = 0; // reseta
            }

            if (ch == '\n') {
                lin++;
                col = 0;
            } else {
                texto.text[lin].line[col++] = ' '; // adiciona o espaço depois da palavra
            }

        } else { // continua construindo a palavra
            if (tamanhoPalavra < TEXT_COLUMNS) { // garante que o tamanho n transborde
                palavra[tamanhoPalavra++] = ch;
            }
        }

        if (col >= TEXT_COLUMNS || lin >= texto.tam) { // n ultrapassa os limites
            lin++;
            col = 0;
        }

        if (lin >= texto.tam) break; // caso exceder o número de linhas
    }

    // aciona o texto, ou seja, fala que a variavel texto_na_tela pode começar a receber texto
    textActive = !textActive;

    fclose(file);
}


//inicializacao
void initMpc(void) {
   //mpc configuration
   mpcSetSize(APP_LINES, APP_COLUMNS);

   //mpc callbacks
   mpcSetMouseFunc(cbMouse);
   mpcSetUpdateFunc(cbUpdate);
   mpcSetKeyboardFunc(cbKeyboard);

    //cor do cursor
   mpcSetCursorColor(RED_3);

   //inicializa as matrizes com espacos, padrao de "vazio" no MPC
   memset(tela,' ',APP_LINES*APP_COLUMNS);
   memset(texto_na_tela,' ',TEXT_LINES*TEXT_COLUMNS);

   //posição e tamanho da 'janela' onde as IMAGENS poderão aparecer
   mpcSetClippingArea(0, 0, APP_LINES, APP_COLUMNS);

   //carrega as imagens
   trianguloUp = mpcLoadBmp("./resources/triangleu.bmp");
   trianguloDown = mpcLoadBmp("./resources/triangled.bmp");

   mpcAbout();
}

void atualizaTextoNaTela() {

    if (!textActive) {
        memset(texto_na_tela,' ',TEXT_LINES*TEXT_COLUMNS);  // se n há texto ativo, exibe apenas a tela em branco
    } else {
        for (int l = 0; l < TEXT_LINES; l++) {
            for (int c = 0; c < TEXT_COLUMNS; c++) {
                char novo_char = texto.text[l + (TEXT_LINES * pagina_atual)].line[c];
                if (texto_na_tela[l][c] != novo_char) {  // desenha apenas caracteres que mudaram
                    texto_na_tela[l][c] = novo_char;
                    mpcSetChar(5 + l, 3 + c, novo_char, F_STD, BLACK, WHITE, 1);
                }
            }
        }
    }
}


void imprimeTextoNaTela() {
    for (int l = 5; l < TEXT_LINES + 5; l++) {
        for (int c = 3; c < TEXT_COLUMNS + 3; c++)
        {
            mpcSetChar(l, c, texto_na_tela[l - 5][c - 3], F_STD, BLACK, WHITE, 1);
        }
    }
}

void desenhaTela() {
    for (int l = 0; l < APP_LINES; l++) {
        for (int c = 0; c < APP_COLUMNS; c++)
        {
            mpcSetChar(l, c, tela[l][c], F_STD, BLACK, GRAY_1, 1);
        }
    }
    //desenha o botão load
   mostraTexto(2, 7, "LOAD");
   mpcVLine(23, 13, 62, BLACK, 1);
   mpcVLine(122, 13, 62, BLACK, 1);
   mpcHLine(13, 23, 122, BLACK, 1);
   mpcHLine(62, 23, 122, BLACK, 1);

   //desenha a caixa de texto
   mpcVLine(23, 73, 495, BLACK, 1);
   mpcVLine(745, 73, 495, BLACK, 1);
   mpcHLine(73, 23, 745, BLACK, 1);
   mpcHLine(495, 23, 745, BLACK, 1);

    //se há mais de uma página, é necessário o scroll, logo ele é exibido
    if (paginas > 1) {
        //desenha as imagens
        mpcShowImg(4, 94, trianguloUp, 1);
        mpcShowImg(31, 94, trianguloDown, 1);
        //desenha o scroll
        char b = ' ';
        int tamanho_scroll = 24;
        int inicio_scroll = 7;
        int tamanho_barrinha = tamanho_scroll / paginas;
        for (int i = 0; i < tamanho_barrinha; i++) {
            mpcSetChar(inicio_scroll + (pagina_atual * tamanho_barrinha) + i, 96, b, F_STD, BLACK, BLACK, 1);
            mpcSetChar(inicio_scroll + (pagina_atual * tamanho_barrinha) + i, 97, b, F_STD, BLACK, BLACK, 1);
        }
    }
}


//funcao em loop
void displayApp(void)
{
    desenhaTela();
    atualizaTextoNaTela();
    imprimeTextoNaTela();
}

//funcao auxiliar
void mostraTexto(int l, int c, char *msg)
{
   for (int cont = 0; cont < strlen(msg); cont++)
      mpcSetChar(l, c+cont, msg[cont], F_STD, BLACK, GRAY_1, 1);
}


//callbacks
void cbMouse(int lin, int col, int button, int state) {
       printf("\nO Mouse foi movido: %d %d %d %d ", lin, col, button, state);

   mpcSetCursorPos(lin, col);

   //carrega o texto
   if (state == 0 && lin > 0 && lin < 4 && col > 2 && col < 15) {
        loadTextFromFile("./resources/texto.txt");
        click = true;
   }

   //controla o acroll
   if (state == 0 && lin > 29 && lin < 33 && col > 94 && col < 99 && pagina_atual < (paginas - 1)) {
        pagina_atual++;
        click = true;
   } else if(state == 0 && lin > 3 && lin < 7 && col > 94 && col < 99 && pagina_atual > 0) {
        pagina_atual--;
        click = true;
   }

   //desativa o click = debouncer
   if (state == 1) click = false;

   //torna o cursor visivel apenas dentro da tela de texto
   if (lin > 4 && lin < 33 && col > 2 && col < 93) {
        mpcSetCursorVisible(true);
   } else {
        mpcSetCursorVisible(false);
   }
}

void cbKeyboard(int key, int modifier, bool special, bool up) {
   printf("\nAlguma tecla foi pressionada: %d %d %d %d", key, modifier, special,  up);
printf("\nAlguma tecla foi pressionada: %d %d %d %d", key, modifier, special,  up);

   /*if (special ) //se for uma tecla especial cai aqui
   {
       if( up == false ) //pega o evento quando uma seta direcional eh pessionada e nao quando eh solta
       {
            switch(key){
                case 100://seta para esquerda
                    cursorCol--;
                    break;
                case 101://seta para cima
                    cursorLin--;
                    break;
                case 102://seta para direita
                    cursorCol++;
                    break;
                case 103://seta para baixo
                    cursorLin++;
                    break;
            }//switch
       } //up
   }
   else
   {
      if(up == false) //soh pega o caractere quando ele for pressionado, ou seja, quando nao eh up.
      {
          //usa-se as coordenadas do cursor para inserir o texto.
          //mpcSetChar(cursorLin, cursorCol, key, F_STD, BLACK, YELLOW_5, 1 );
          //a cada caractere digitado, move-se o cursor para frente.
          cursorCol++;
          //guarda-se a informacao em uma matriz para reexibicao a cada frame. Senao, nao aparece nada na tela.
          texto[cursorLin][cursorCol] = key;
          //mose-se o cursor
          mpcSetCursorPos(cursorLin, cursorCol);
      }
   }
   //changed = true;
*/
}

void cbUpdate(void) {
   displayApp();
}
