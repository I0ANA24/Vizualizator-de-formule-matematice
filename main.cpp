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

// cateva constante pentru grafica
#define SPATIU_OPERATOR 10  // spatiul in pixeli intre operatori si operanzi
#define INALTIME_TEXT 20    // inaltimea aproximativa a textului
#define MARIME_FONT 3      // marimea fontului


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
                strcmp(t, "abs") == 0) {
                listaTokeni[nrTokeni].tip = TIP_FUNCTIE;
                listaTokeni[nrTokeni].prioritate = 5;
            }
            else {
                listaTokeni[nrTokeni].tip = TIP_VARIABILA;
            }
            nrTokeni++;
        }

        // 4. operator sau paranteza
        else if (esteOperator(expresie[i]) || expresie[i] == '(' || expresie[i] == ')') {
            listaTokeni[nrTokeni].text[0] = expresie[i];
            listaTokeni[nrTokeni].text[1] = '\0';

            if (expresie[i] == '(') {
                listaTokeni[nrTokeni].tip = TIP_PARANTEZA_ST;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else if (expresie[i] == ')') {
                listaTokeni[nrTokeni].tip = TIP_PARANTEZA_DR;
                listaTokeni[nrTokeni].prioritate = 0;
            }
            else {
                listaTokeni[nrTokeni].tip = TIP_OPERATOR;
                if (expresie[i] == '+' || expresie[i] == '-') listaTokeni[nrTokeni].prioritate = 1;
                else if (expresie[i] == '*' || expresie[i] == '/') listaTokeni[nrTokeni].prioritate = 2;
                else if (expresie[i] == '^') listaTokeni[nrTokeni].prioritate = 3;
            }
            nrTokeni++;
            i++;
        }
        else {
            i++; // Caractere necunoscute
        }
    }
}

// Functie validare
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
    // Nu putem incepe cu )
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
            // Nu poate urma alt operator sau paranteza inchisa doar + sau -
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
            // Verificam daca urmeaza operator
            if (urmator.tip == TIP_OPERATOR) {
                // daca operatorul e + sau - e ok altfel nu
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

    return 1; // Totul e OK
}

NodArbore* construiesteArbore(int st, int dr) {
    if (st > dr) return NULL;
    if (st == dr) return nodNou(listaTokeni[st]);

    int minPrio = 999;
    int pozitieOperator = -1;
    int paranteze = 0;

    for (int i = dr; i >= st; i--) {
        Token t = listaTokeni[i];
        if (t.tip == TIP_PARANTEZA_DR) paranteze++;
        else if (t.tip == TIP_PARANTEZA_ST) paranteze--;
        else if (paranteze == 0 && t.tip == TIP_OPERATOR) {
            if (t.prioritate < minPrio) {
                minPrio = t.prioritate;
                pozitieOperator = i;
            }
            // la prioritate 1 ne oprim imediat
            if (minPrio == 1) break;
        }
    }

    if (pozitieOperator != -1) {
        NodArbore* radacina = nodNou(listaTokeni[pozitieOperator]);
        if (pozitieOperator == st) { // pt operator unar
            radacina->stanga = NULL;
            radacina->dreapta = construiesteArbore(st + 1, dr);
        }
        else {
            radacina->stanga = construiesteArbore(st, pozitieOperator - 1);
            radacina->dreapta = construiesteArbore(pozitieOperator + 1, dr);
        }
        return radacina;
    }

    // daca nu e operator verificam daca parantezele exterioare sunt pereche
    if (listaTokeni[st].tip == TIP_PARANTEZA_ST && listaTokeni[dr].tip == TIP_PARANTEZA_DR) {
        int k = 0;
        bool suntPereche = true;
        // verif daca paranteza de la st se inchide chiar la dr
        for (int i = st; i < dr; i++) {
            if (listaTokeni[i].tip == TIP_PARANTEZA_ST) k++;
            if (listaTokeni[i].tip == TIP_PARANTEZA_DR) k--;
            if (k == 0) {
                suntPereche = false;
                break;
            }
        }

        if (suntPereche) {
            return construiesteArbore(st + 1, dr - 1);
        }
    }

    // Daca e functie
    if (listaTokeni[st].tip == TIP_FUNCTIE) {
        NodArbore* radacina = nodNou(listaTokeni[st]);
        radacina->stanga = NULL;
        radacina->dreapta = construiesteArbore(st + 1, dr);
        return radacina;
    }

    return NULL;
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
    if(nod == NULL) return;

    calculeazaDimensiuni(nod->stanga);
    calculeazaDimensiuni(nod->dreapta);

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, MARIME_FONT);

    // calculam dimensiunea nodului curent in functie de tip

    // 1. frunza (numar sau variabila)
    if (nod->stanga == NULL && nod->dreapta == NULL) {
        nod->latime = textwidth(nod->info);
        nod->inaltime = textwidth(nod->info);
    }

    // 2. operator unar/binar (+, -, *) - afisare liniara
    else if (nod->info[0] == '+' || nod->info[0] == '-' || nod->info[0] == '*') {
        int wSt = nod->stanga ? nod->stanga->latime : 0;
        int wDr = nod->dreapta ? nod->dreapta->latime : 0;
        int hSt = nod->stanga ? nod->stanga->inaltime : 0;
        int hDr = nod->dreapta ? nod->dreapta->inaltime : 0;

        int wOp = textwidth(nod->info);

        nod->latime = wSt + wOp + wDr + 2 * SPATIU_OPERATOR;
        nod-> inaltime = max(hSt, hDr);
    }

    // 3. fractie (/) - afisare verticala
    else if (nod->info[0] == '/') {

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

        // --- Partea Grafica ---

        
    }
    else {
        cout << "Expresie invalida" << endl;
    }

    return 0;
}