#define _CRT_SECURE_NO_WARNINGS // <--- 1. ASTA REZOLVA PROBLEMA CU STRCPY
#include <iostream>
#include <fstream>
#include <cstring> // pentru strcpy, strlen, strcmp
#include <cctype>  // pentru isdigit, isalpha
using namespace std;

ifstream fin("expresie.in");
ofstream fout("expresie.out");

// Folosim aceste numere ca sa stim ce tip are fiecare token
#define TIP_NEDEFINIT 0
#define TIP_NUMAR 1      // ex: 3.14, 100
#define TIP_OPERATOR 2   // ex: +, -, *, /, ^
#define TIP_VARIABILA 3  // ex: x, y, a
#define TIP_FUNCTIE 4    // ex: sin, cos, ln, abs
#define TIP_PARANTEZA_ST 5  // ex: (
#define TIP_PARANTEZA_DR 6  // ex: )

// --- STRUCTURI DE DATE ---

struct Token {
    char text[50]; // Aici tinem textul efectiv, ex: "sin" sau "3.26"
    int tip;       // Aici tinem codul (1, 2, 3...) definit mai sus
    int prioritate;
};

// STRUCTURA PENTRU ARBORE
struct NodArbore {
    char info[50];       // Informatia din nod (+, sin, x, 5)
    int tip;             // Tipul (ca sa stim cum il desenam)
    NodArbore* stanga;   // Copilul stang
    NodArbore* dreapta;  // Copilul drept
};

// Variabile globale
Token listaTokeni[100]; // Vectorul de tokeni
int nrTokeni = 0;       // Cate elemente am gasit

// --- FUNCTII AJUTATOARE ---

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
        // 1. Daca e SPATIU sau TAB, il ignoram
        if (isspace(expresie[i])) { // Am schimbat cu isspace()
            i++;
            continue;
        }

        // 2. Daca e CIFRA -> citim tot numarul
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

        // 3. Daca e LITERA -> citim cuvantul
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

        // 4. Daca e OPERATOR sau PARANTEZA
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

// Functie validare (Simplificata pentru claritate, logica ta era buna)
bool valideazaExpresie() {
    int paranteze = 0;
    for (int i = 0; i < nrTokeni; i++) {
        if (listaTokeni[i].tip == TIP_PARANTEZA_ST) paranteze++;
        if (listaTokeni[i].tip == TIP_PARANTEZA_DR) paranteze--;
        if (paranteze < 0) return 0;
    }
    if (paranteze != 0) return 0;

    // Putem adauga aici restul validarii tale, dar pentru test, e suficient paranteze
    return 1;
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
            // Important: Daca gasim prioritate 1 (+ sau -) ne oprim imediat
            // pentru ca parcurgem de la dreapta la stanga
            if (minPrio == 1) break;
        }
    }

    if (pozitieOperator != -1) {
        NodArbore* radacina = nodNou(listaTokeni[pozitieOperator]);
        if (pozitieOperator == st) { // Operator unar (ex: -5)
            radacina->stanga = NULL;
            radacina->dreapta = construiesteArbore(st + 1, dr);
        }
        else {
            radacina->stanga = construiesteArbore(st, pozitieOperator - 1);
            radacina->dreapta = construiesteArbore(pozitieOperator + 1, dr);
        }
        return radacina;
    }

    // Daca nu e operator, poate e intre paranteze ( ... )
    if (listaTokeni[st].tip == TIP_PARANTEZA_ST && listaTokeni[dr].tip == TIP_PARANTEZA_DR) {
        return construiesteArbore(st + 1, dr - 1);
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

void afiseazaArboreDebug(NodArbore* r, int nivel) {
    if (r == NULL) return;
    afiseazaArboreDebug(r->dreapta, nivel + 1);
    for (int i = 0; i < nivel; i++) fout << "    ";
    fout << r->info << endl;
    afiseazaArboreDebug(r->stanga, nivel + 1);
}

int main() {
    char expresie[1001];

    if (!fin) {
        cout << "Eroare: Nu ai creat fisierul expresie.in!" << endl;
        return 1;
    }

    // Citire sigura
    fin.getline(expresie, 1001);

    tokenizare(expresie);

    // Validare simpla
    if (valideazaExpresie()) {
        cout << "Validare OK. Se genereaza arborele in expresie.out..." << endl;
        NodArbore* radacina = construiesteArbore(0, nrTokeni - 1);
        afiseazaArboreDebug(radacina, 0);
    }
    else {
        cout << "Expresie invalida (verifica parantezele)." << endl;
    }

    return 0;
}