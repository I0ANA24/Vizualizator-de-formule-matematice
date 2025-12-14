#define _CRT_SECURE_NO_WARNINGS 
#include <iostream>
#include <fstream>
#include <cstring> // pentru strcpy, strlen, strcmp
#include <cctype>  // pentru isdigit, isalpha
#include "graphics.h"
#include "winbgim.h"
using namespace std;

ifstream fin("expresie.in");
ofstream fout("expresie.out");

// folosim aceste numere ca sa stim ce tip are fiecare token
#define TIP_NEDEFINIT 0
#define TIP_NUMAR 1      // ex: 3.14, 100
#define TIP_OPERATOR 2   // ex: +, -, *, /, ^
#define TIP_VARIABILA 3  // ex: x, y, a
#define TIP_FUNCTIE 4    // ex: sin, cos, ln, abs
#define TIP_PARANTEZA_ST 5  // ex: (
#define TIP_PARANTEZA_DR 6  // ex: )
#define TIP_MATRICE_ST 7    // pentru [
#define TIP_MATRICE_DR 8    // pentru ]
#define TIP_VIRGULA 9       // pentru ,
#define TIP_MATRICE 10      // pentru nodul radacina al matricei

// cateva constante pentru grafica
#define SPATIU_OPERATOR 10  // spatiul in pixeli intre operatori si operanzi
#define INALTIME_TEXT 20    // inaltimea aproximativa a textului
#define MARIME_FONT 3      // marimea fontului

#define MAX_LINII 20
#define MAX_COLOANE 20

struct Token {
    char text[50]; // textul efectiv sin sau 3.26
    int tip;       // tipul 1 2 3..... de mai sus
    int prioritate;
};

struct NodArbore {
    char info[50];
    int tip;             // ca sa stim cum il desenam
    NodArbore* stanga;
    NodArbore* dreapta;

    // dimensiunile nodului pentru desenare
    int latime;
    int inaltime;
};

Token listaTokeni[100]; // vectorul de tokeni
int nrTokeni = 0;       // cate elemente am gasit


NodArbore* nodNou(Token t) {
    NodArbore* n = new NodArbore;
    strcpy(n->info, t.text);
    n->tip = t.tip;
    n->stanga = NULL;
    n->dreapta = NULL;
    return n;
}

bool esteOperator(char c) {
    if (strchr("+-*/^", c)) return true;
    return false;
}

void tokenizare(char expresie[]) {
    nrTokeni = 0;
    int n = strlen(expresie);
    int i = 0;

    while (i < n) {
        // 1. spatiu-il ignor
        if (isspace(expresie[i])) {
            i++;
            continue;
        }

        // 2. cifra-citim tot numarul
        if (isdigit(expresie[i])) {
            int k = 0;
            while (i < n && (isdigit(expresie[i]) || expresie[i] == '.')) {
                listaTokeni[nrTokeni].text[k] = expresie[i];
                k++; i++;
            }
            listaTokeni[nrTokeni].text[k] = '\0';
            listaTokeni[nrTokeni].tip = TIP_NUMAR;
            listaTokeni[nrTokeni].prioritate = -1;
            nrTokeni++;
        }

        // 3. litera-citim cuvantul
        // de revazut
        else if (isalpha(expresie[i])) {
            int k = 0;
            while (i < n && isalpha(expresie[i])) {
                listaTokeni[nrTokeni].text[k] = expresie[i];
                k++; i++;
            }
            listaTokeni[nrTokeni].text[k] = '\0';

            char t[50];
            strcpy(t, listaTokeni[nrTokeni].text);

            if (strcmp(t, "sin") == 0 || strcmp(t, "cos") == 0 ||
                strcmp(t, "ln") == 0 || strcmp(t, "sqrt") == 0 ||
                strcmp(t, "abs") == 0 || strcmp(t, "int") == 0) {
                listaTokeni[nrTokeni].tip = TIP_FUNCTIE;
                listaTokeni[nrTokeni].prioritate = 5;
            }
            else {
                listaTokeni[nrTokeni].tip = TIP_VARIABILA;
            }
            nrTokeni++;
        }

        // 4. operator paranteze sau matrice
        else if (strchr("+-*/^,[]()", expresie[i])) {
            listaTokeni[nrTokeni].text[0] = expresie[i];
            listaTokeni[nrTokeni].text[1] = '\0';

            char c = expresie[i];
            if (c == '(') {
                listaTokeni[nrTokeni].tip = TIP_PARANTEZA_ST;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else if (c == ')') {
                listaTokeni[nrTokeni].tip = TIP_PARANTEZA_DR;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else if (c == '[') {
                listaTokeni[nrTokeni].tip = TIP_MATRICE_ST;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else if (c == ']') {
                listaTokeni[nrTokeni].tip = TIP_MATRICE_DR;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else if (c == ',') {
                listaTokeni[nrTokeni].tip = TIP_VIRGULA;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else { // Operatori normali (+ - * / ^)
                listaTokeni[nrTokeni].tip = TIP_OPERATOR;
                if (c == '+' || c == '-') listaTokeni[nrTokeni].prioritate = 1;
                else if (c == '*' || c == '/') listaTokeni[nrTokeni].prioritate = 2;
                else if (c == '^') listaTokeni[nrTokeni].prioritate = 3;
            }
            nrTokeni++;
            i++;
        }
        else {
            i++; // caractere necunoscute
        }
    }
}

// f de validare
bool valideazaExpresie() {

    // paranteze
    int paranteze = 0;
    for (int i = 0; i < nrTokeni; i++) {
        if (listaTokeni[i].tip == TIP_PARANTEZA_ST) paranteze++;
        if (listaTokeni[i].tip == TIP_PARANTEZA_DR) paranteze--;

        if (paranteze < 0) {
            fout << "Eroare: S-a inchis o paranteza care nu a fost deschisa (poz " << i << ")!\n";
            return 0;
        }
    }
    if (paranteze != 0) {
        fout << "Eroare: Numar inegal de paranteze deschise (" << paranteze << " ramase neinchise)!\n";
        return 0;
    }

    // vecini
    // nu incepem cu operator in afara de + sau -
    if (listaTokeni[0].tip == TIP_OPERATOR) {
        if (listaTokeni[0].text[0] != '+' && listaTokeni[0].text[0] != '-') {
            fout << "Eroare: Expresia nu poate incepe cu operatorul " << listaTokeni[0].text << "!\n";
            return 0;
        }
    }
    // nu putem incepe cu )
    if (listaTokeni[0].tip == TIP_PARANTEZA_DR) {
        fout << "Eroare: Expresia nu poate incepe cu paranteza inchisa!\n";
        return 0;
    }

    // nu putem termina cu operator, functie sau paranteza deschisa
    if (listaTokeni[nrTokeni - 1].tip == TIP_OPERATOR ||
        listaTokeni[nrTokeni - 1].tip == TIP_FUNCTIE ||
        listaTokeni[nrTokeni - 1].tip == TIP_PARANTEZA_ST) {
        fout << "Eroare: Expresia este neterminata!\n";
        return 0;
    }

    for (int i = 0; i < nrTokeni - 1; i++) {
        Token curent = listaTokeni[i];
        Token urmator = listaTokeni[i + 1];

        // A. operator
        if (curent.tip == TIP_OPERATOR) {
            // nu alt operator sau paranteza inchisa ci doar + sau -
            if (urmator.tip == TIP_OPERATOR) {
                if (urmator.text[0] != '+' && urmator.text[0] != '-') {
                    fout << "Eroare: Doi operatori consecutivi nepermisi (" << curent.text << " " << urmator.text << ")!\n";
                    return 0;
                }
            }
            if (urmator.tip == TIP_PARANTEZA_DR) {
                fout << "Eroare: Operator urmat de paranteza inchisa!\n";
                return 0;
            }
        }

        // B. functie
        if (curent.tip == TIP_FUNCTIE) {
            if (urmator.tip != TIP_PARANTEZA_ST) {
                fout << "Eroare: Dupa functie (" << curent.text << ") trebuie sa urmeze '(' !\n";
                return 0;
            }
        }

        // C. nr sau variabila
        if (curent.tip == TIP_NUMAR || curent.tip == TIP_VARIABILA) {
            if (urmator.tip == TIP_NUMAR || urmator.tip == TIP_VARIABILA || urmator.tip == TIP_FUNCTIE || urmator.tip == TIP_PARANTEZA_ST) {
                fout << "Eroare: Lipseste operatorul intre elemente la pozitia " << i << "!\n";
                return 0;
            }
        }

        if (curent.tip == TIP_PARANTEZA_ST) {
            // daca urmeaza operator
            if (urmator.tip == TIP_OPERATOR) {
                // tre sa ffie neaparat + sau -
                if (urmator.text[0] != '+' && urmator.text[0] != '-') {
                    fout << "Eroare: Operatorul '" << urmator.text << "' nu poate sta imediat dupa paranteza deschisa!\n";
                    return 0;
                }
            }
            if (urmator.tip == TIP_PARANTEZA_DR) {
                fout << "Eroare: Paranteze goale () !\n";
                return 0;
            }
        }
    }

    return 1; //daca totul e ok
}

NodArbore* construiesteArbore(int st, int dr) {
    if (st > dr) return NULL;
    if (st == dr) return nodNou(listaTokeni[st]);

    int minPrio = 999;
    int pozitieOperator = -1;
    int paranteze = 0; // si pt () si pt []

    // 1. op cu prio minima
    for (int i = dr; i >= st; i--) {
        Token t = listaTokeni[i];

        // Numaram parantezele ca sa sarim peste ce e inauntru
        if (t.tip == TIP_PARANTEZA_DR || t.tip == TIP_MATRICE_DR) paranteze++;
        else if (t.tip == TIP_PARANTEZA_ST || t.tip == TIP_MATRICE_ST) paranteze--;

        // Daca suntem in afara oricaror paranteze
        else if (paranteze == 0) {
            // Verificam operatorii (inclusiv virgula)
            if (t.tip == TIP_OPERATOR || t.tip == TIP_VIRGULA) {
                if (t.prioritate < minPrio) {
                    minPrio = t.prioritate;
                    pozitieOperator = i;
                }
                // Virgula are prioritate 0, deci va fi aleasa ultima (fiind cea mai mica)
            }
        }
    }

    // A. Daca am gasit operator (ex: +, *, sau ,)
    if (pozitieOperator != -1) {
        NodArbore* radacina = nodNou(listaTokeni[pozitieOperator]);
        radacina->stanga = construiesteArbore(st, pozitieOperator - 1);
        radacina->dreapta = construiesteArbore(pozitieOperator + 1, dr);
        return radacina;
    }

    // B. Daca nu e operator, verificam () si le stergem
    if (listaTokeni[st].tip == TIP_PARANTEZA_ST && listaTokeni[dr].tip == TIP_PARANTEZA_DR) {
        //daca sunt pereche
        int k = 0; bool ok = true;
        for (int i = st; i < dr; i++) {
            if (listaTokeni[i].tip == TIP_PARANTEZA_ST) k++;
            if (listaTokeni[i].tip == TIP_PARANTEZA_DR) k--;
            if (k == 0) { ok = false; break; }
        }
        if (ok) return construiesteArbore(st + 1, dr - 1);
    }

    // C. Verificam [] si facem nod matrice
    if (listaTokeni[st].tip == TIP_MATRICE_ST && listaTokeni[dr].tip == TIP_MATRICE_DR) {
        //daca sunt pereche
        int k = 0; bool ok = true;
        for (int i = st; i < dr; i++) {
            if (listaTokeni[i].tip == TIP_MATRICE_ST) k++;
            if (listaTokeni[i].tip == TIP_MATRICE_DR) k--;
            if (k == 0) { ok = false; break; }
        }
        if (ok) {
            NodArbore* matrice = new NodArbore;
            strcpy(matrice->info, "[]");
            matrice->tip = TIP_MATRICE;
            matrice->stanga = NULL;
            matrice->dreapta = construiesteArbore(st + 1, dr - 1);
            return matrice;
        }
    }

    // D. Functii (sin, cos, int)
    if (listaTokeni[st].tip == TIP_FUNCTIE) {
        NodArbore* radacina = nodNou(listaTokeni[st]);
        radacina->stanga = NULL;
        radacina->dreapta = construiesteArbore(st + 1, dr);
        return radacina;
    }

    return NULL;
}

void fMatrice(NodArbore* nod, NodArbore* mat[MAX_LINII][MAX_COLOANE], int& rows, int& cols, int rurent, int curent) {
    if (nod == NULL) return;

    if (nod->tip == TIP_MATRICE) {
        fMatrice(nod->dreapta, mat, rows, cols, rurent, 0);
        if (rurent != -1) rows++;
        return;
    }

    if (nod->tip == TIP_VIRGULA) {
        // daca virgula separa linii din matrrice
        if (nod->stanga->tip == TIP_MATRICE) {
            fMatrice(nod->stanga, mat, rows, cols, rows, 0);
            fMatrice(nod->dreapta, mat, rows, cols, rows, 0);
        }
        // altfel virgula separa elemente de pe o linie adc coloane
        else
            fMatrice(nod->stanga, mat, rows, cols, rurent, curent);
        return;
    }

    // daca e element efectiv il punem pe prima col libera din linia rows
    int c = 0;
    while (mat[rows][c] != NULL && c < MAX_COLOANE) c++;
    if (c < MAX_COLOANE) {
        mat[rows][c] = nod;
        if (c + 1 > cols) cols = c + 1; // act nr maxim de col
    }
}

void extrageElementeLinie(NodArbore* nod, NodArbore* lista[], int& k) {
    if (nod == NULL) return;
    if (nod->tip == TIP_VIRGULA) {
        extrageElementeLinie(nod->stanga, lista, k);
        extrageElementeLinie(nod->dreapta, lista, k);
    }
    else {
        lista[k++] = nod;
    }
}

void construiesteMat(NodArbore* radacinaMatrice, NodArbore* grid[MAX_LINII][MAX_COLOANE], int& rows, int& cols) {
    for (int i = 0; i < MAX_LINII; i++)
        for (int j = 0; j < MAX_COLOANE; j++) grid[i][j] = NULL;
    rows = 0; cols = 0;

    NodArbore* continut = radacinaMatrice->dreapta;

    // 1. Colectam randurile
    NodArbore* listaLinii[MAX_LINII];
    int nrLinii = 0;
    extrageElementeLinie(continut, listaLinii, nrLinii);
    // Atentie: extrageElementeLinie va pune in listaLinii pointeri la nodurile TIP_MATRICE (randuri)

    rows = nrLinii;

    // 2. Pentru fiecare linie, extragem elementele
    for (int i = 0; i < rows; i++) {
        if (listaLinii[i]->tip == TIP_MATRICE) {
            NodArbore* elemente[MAX_COLOANE];
            int nrElem = 0;
            extrageElementeLinie(listaLinii[i]->dreapta, elemente, nrElem);

            if (nrElem > cols) cols = nrElem;

            for (int j = 0; j < nrElem; j++) {
                grid[i][j] = elemente[j];
            }
        }
        else {
            // Cazul vector simplu [1, 2, 3] (o singura linie, nu are sub-matrici)
            // Aici listaLinii[i] este direct elementul
            grid[0][i] = listaLinii[i];
            if (i == 0) { rows = 1; cols = nrLinii; } // Setam o singura data
        }
    }
}

void afiseazaArbore(NodArbore* r, int nivel) {
    if (r == NULL) return;
    afiseazaArbore(r->dreapta, nivel + 1);
    for (int i = 0; i < nivel; i++) fout << "    ";
    fout << r->info << endl;
    afiseazaArbore(r->stanga, nivel + 1);
}

int getLatimeText(char* text) {
    return textwidth(text);
}

// Functie pentru calcularea dimensiunilor textului din fiecare nod
// Postordine, de jos in sus
void calculeazaDimensiuni(NodArbore* nod) {
    if (nod == NULL) return;

    calculeazaDimensiuni(nod->stanga);
    calculeazaDimensiuni(nod->dreapta);

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, MARIME_FONT);

    // calculam dimensiunea nodului curent in functie de tip

    // 1. frunza (numar sau variabila)
    if (nod->stanga == NULL && nod->dreapta == NULL) {
        nod->latime = textwidth(nod->info);
        nod->inaltime = textheight(nod->info);
    }

    // 2. operator unar/binar (+, -, *) - afisare liniara
    else if (nod->info[0] == '+' || nod->info[0] == '-' || nod->info[0] == '*') {
        int wSt = nod->stanga ? nod->stanga->latime : 0;
        int wDr = nod->dreapta ? nod->dreapta->latime : 0;
        int hSt = nod->stanga ? nod->stanga->inaltime : 0;
        int hDr = nod->dreapta ? nod->dreapta->inaltime : 0;

        int wOp = textwidth(nod->info);

        nod->latime = wSt + wOp + wDr + 2 * SPATIU_OPERATOR;
        nod->inaltime = max(hSt, hDr);
    }

    // 3. fractie (/) - afisare verticala
    else if (nod->info[0] == '/') {
        int wSt = nod->stanga->latime;
        int wDr = nod->dreapta->latime;

        nod->latime = max(wSt, wDr) + 10;
        nod->inaltime = nod->stanga->inaltime + nod->dreapta->inaltime + 10;
    }

    // 4. putere (^)
    else if (nod->info[0] == '^') {
        int wBaza = nod->stanga->latime;
        int wExp = nod->dreapta->latime;

        nod->latime = wBaza + wExp;
        nod->inaltime = nod->stanga->inaltime + (nod->dreapta->inaltime / 2);
    }

    // 5. functie (sin, cos, etc)
    else if (nod->tip == TIP_FUNCTIE) {
        if (strcmp(nod->info, "int") == 0) {
            int wArg = nod->dreapta->latime;
            int hArg = nod->dreapta->inaltime;

            nod->latime = 15 + wArg + 10 + textwidth("dx");
            nod->inaltime = max(textheight("A") * 2, hArg + 10); //putin mai inalt decat expresia
        }
        else {
            int wNume = textwidth(nod->info);
            int wArg = nod->dreapta->latime;

            nod->latime = wNume + wArg + 20; // +20 pentru paranteze
            nod->inaltime = max(textheight("A"), nod->dreapta->inaltime);
        }
    }
    else if (nod->tip == TIP_MATRICE) {
        // Folosim gridul pentru a calcula dimensiunile
        NodArbore* mat[MAX_LINII][MAX_COLOANE];
        int rows, cols;
        construiesteMat(nod, mat, rows, cols);

        // Vectori pentru dimensiunile maxime
        int colW[MAX_COLOANE] = { 0 };
        int rowH[MAX_LINII] = { 0 };

        // Pas 1: Calculam dimensiunile fiecarui element recursiv
        // si determinam maximele pe linii si coloane
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                if (mat[i][j]) {
                    calculeazaDimensiuni(mat[i][j]); // Recursiv pt elemente
                    if (mat[i][j]->latime > colW[j]) colW[j] = mat[i][j]->latime;
                    if (mat[i][j]->inaltime > rowH[i]) rowH[i] = mat[i][j]->inaltime;
                }
            }
        }

        // Pas 2: Calculam totalul
        int totalW = 0;
        for (int j = 0; j < cols; j++) totalW += colW[j];
        totalW += (cols - 1) * 15; // Spatiu intre coloane
        totalW += 30; // Spatiu pentru paranteze [ ]

        int totalH = 0;
        for (int i = 0; i < rows; i++) totalH += rowH[i];
        totalH += (rows - 1) * 10; // Spatiu intre linii
        totalH += 10; // Padding sus/jos

        nod->latime = totalW;
        nod->inaltime = totalH;
    }
}

void deseneazaRecursiv(NodArbore* nod, int x, int y, bool arataParanteze = true) //x,y centrul zonei de desenare
{
    if (nod == NULL) return;

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, MARIME_FONT);
    setcolor(WHITE);

    // 1. frunza (numar sau variabila)
    if (nod->stanga == NULL && nod->dreapta == NULL)
        outtextxy(x - nod->latime / 2, y - nod->latime / 2, nod->info);

    // 2. operator unar/binar (+, -, *) - afisare liniara
    else if (nod->info[0] == '+' || nod->info[0] == '-' || nod->info[0] == '*') {
        // desenam operatorul la mijloc
        //outtextxy(x - textwidth(nod->info) / 2, y - textheight(nod->info) / 2, nod->info);
        //outtextxy(x - textwidth(nod->info) / 2, y - textheight("+") / 5, nod->info);
        outtextxy(x - textwidth(nod->info) / 2, y - 5, nod->info);

        // calculam pozitiile fiilor
        int wSt = (nod->stanga) ? nod->stanga->latime : 0;
        int wDr = (nod->dreapta) ? nod->dreapta->latime : 0;
        int wOp = textwidth(nod->info);

        // x-ul pentru stanga: scadem jumatate din operator si jumatate din latimea stanga
        if (nod->stanga)
            deseneazaRecursiv(nod->stanga, x - wOp / 2 - SPATIU_OPERATOR - wSt / 2, y);

        // x-ul pentru dreapta
        if (nod->dreapta)
            deseneazaRecursiv(nod->dreapta, x + wOp / 2 + SPATIU_OPERATOR + wDr / 2, y);
    }


    // 3. fractie (/) - afisare verticala
    else if (nod->info[0] == '/') {
        // desenam linia de fractie
        int lungimeLinie = nod->latime;
        //line(x - lungimeLinie / 2, y + textheight("+") / 5, x + lungimeLinie / 2, y + textheight("+") / 5);
        line(x - lungimeLinie / 2, y + 7, x + lungimeLinie / 2, y + 7);

        // desenam numaratorul
        int hSt = nod->stanga->inaltime;
        deseneazaRecursiv(nod->stanga, x, y - hSt / 2 - 5);

        // 3. desenam numitorul
        int hDr = nod->dreapta->inaltime;
        deseneazaRecursiv(nod->dreapta, x, y + hDr / 2 + 5);
    }


    // 4. putere (^)
    else if (nod->info[0] == '^') {
        int wBaza = nod->stanga->latime;
        int wExp = nod->dreapta->latime;

        // desenam baza putin mai la stanga
        deseneazaRecursiv(nod->stanga, x - wExp / 2, y + 10);

        // desenam exponentul mai sus si mai mic
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, MARIME_FONT - 1);
        deseneazaRecursiv(nod->dreapta, x + wBaza / 2, y - 15);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, MARIME_FONT); // revenim la font normal
    }


    // 5. functie (sin, cos, etc)
    else if (nod->tip == TIP_FUNCTIE) {
        if (strcmp(nod->info, "int") == 0) {
            int hTotal = nod->inaltime;
            int xSimbol = x - nod->latime / 2 + 5;

            //carlig sus
            arc(xSimbol, y - hTotal / 2 + 5, 0, 180, 5);
            //carlig jos
            arc(xSimbol, y + hTotal / 2 - 5, 180, 360, 5);
            //linia
            line(xSimbol - 5, y - hTotal / 2 + 5, xSimbol + 5, y + hTotal / 2 - 5); // linie usor oblica

            //expresia de dupa semnul de integrala
            int wArg = nod->dreapta->latime;
            deseneazaRecursiv(nod->dreapta, xSimbol + 15 + wArg / 2, y);

            // dx la final
            outtextxy(x + nod->latime / 2 - textwidth("dx"), y - textheight("d") / 2, "dx");
        }
        else {
            int wNume = textwidth(nod->info);
            int wArg = nod->dreapta->latime;

            // numele functiei
            int xStart = x - nod->latime / 2;
            outtextxy(xStart, y - textheight("A") / 2, nod->info);

            // desen (
            outtextxy(xStart + wNume, y - textheight("A") / 2, "(");

            // arg functiei
            deseneazaRecursiv(nod->dreapta, xStart + wNume + 10 + wArg / 2, y);

            // desen )
            outtextxy(xStart + wNume + 10 + wArg + 5, y - textheight("A") / 2, ")");
        }
    }
    // matrice
    else if (nod->tip == TIP_MATRICE) {
        NodArbore* mat[MAX_LINII][MAX_COLOANE];
        int rows, cols;
        construiesteMat(nod, mat, rows, cols);

        int w = nod->latime;
        int h = nod->inaltime;

        // 1. Desenam Parantezele Mari (O SINGURA DATA)
        // Stanga [
        line(x - w / 2, y - h / 2, x - w / 2 + 8, y - h / 2);
        line(x - w / 2, y - h / 2, x - w / 2, y + h / 2);
        line(x - w / 2, y + h / 2, x - w / 2 + 8, y + h / 2);
        // Dreapta ]
        line(x + w / 2, y - h / 2, x + w / 2 - 8, y - h / 2);
        line(x + w / 2, y - h / 2, x + w / 2, y + h / 2);
        line(x + w / 2, y + h / 2, x + w / 2 - 8, y + h / 2);

        // Recalculam latimile coloanelor si inaltimile liniilor pentru pozitionare
        int colW[MAX_COLOANE] = { 0 };
        int rowH[MAX_LINII] = { 0 };
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (mat[i][j]) {
                    if (mat[i][j]->latime > colW[j]) colW[j] = mat[i][j]->latime;
                    if (mat[i][j]->inaltime > rowH[i]) rowH[i] = mat[i][j]->inaltime;
                }

        // 2. Desenam Elementele
        // Calculam Y-ul de start (coltul stanga-sus al continutului)
        int startY = y - h / 2 + 5;

        for (int i = 0; i < rows; i++) {
            // Y-ul curent pentru centrul randului
            int curentY = startY + rowH[i] / 2;

            int startX = x - w / 2 + 15; // X-ul de start

            for (int j = 0; j < cols; j++) {
                // X-ul curent pentru centrul coloanei
                int curentX = startX + colW[j] / 2;

                if (mat[i][j]) {
                    // Desenam elementul centrat in celula sa
                    deseneazaRecursiv(mat[i][j], curentX, curentY);
                }

                // Avansam X-ul
                startX += colW[j] + 15; // + spatiu intre coloane
            }

            // Avansam Y-ul
            startY += rowH[i] + 10; // + spatiu intre linii
        }
    }
}

int main() {
    char expresie[1001];

    if (!fin) {
        cout << "Eroare citire" << endl;
        return 1;
    }

    fin.getline(expresie, 1001);

    tokenizare(expresie);

    if (valideazaExpresie()) {
        cout << "Valid. Se genereaza arborele in expresie.out" << endl;
        NodArbore* radacina = construiesteArbore(0, nrTokeni - 1);
        afiseazaArbore(radacina, 0);

        //grafica
        initwindow(1200, 600, "Vizualizator de formule matematice");
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, MARIME_FONT);

        calculeazaDimensiuni(radacina);

        outtextxy(50, 50, "Expresia randata grafic: ");
        deseneazaRecursiv(radacina, 600, 300); //centrul ecranului

        getch(); //taste pt inchideree
        closegraph();

    }
    else {
        cout << "Expresie invalida" << endl;
    }

    return 0;
}