#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <cstring> // strcpy, strlen, strcmp
#include <cctype>  // isdigit, isalpha
#include "graphics.h"
#include "winbgim.h"
using namespace std;

ifstream fin("expresie2.in");
ofstream fout("expresie2.out");

// definim tipurile de elemente posibile
#define tip_nedefinit 0
#define tip_numar 1      // ex: 3.14, 100
#define tip_operator 2   // +, -, *, /, ^
#define tip_variabila 3  // x, y, a
#define tip_functie 4    // sin, cos, ln, abs
#define tip_par_st 5     // (
#define tip_par_dr 6     // )
#define tip_mat_st 7     // [
#define tip_mat_dr 8     // ]
#define tip_virgula 9    // ,
#define tip_matrice 10   // radacina pentru matrice

// constante grafica
#define spatiu_op 10   // spatiu pixeli intre operatori
#define inalt_txt 20   // inaltime text
#define marime_font 3  // font size

#define max_linii 20
#define max_col 20

struct token
{
    char text[50]; // textul efectiv (sin, 3.14 etc)
    int tip;       // tipul definit mai sus
    int prioritate;
};

struct nod
{
    char info[50];
    int tip;
    nod* st; // stanga
    nod* dr; // dreapta

    // dimensiuni pentru desenare
    int lat;   // latime
    int inalt; // inaltime
};

token tokeni[100]; // vectorul de elemente
int nr_tok = 0;    // cate elemente am gasit

// functie pentru crearea unui nod nou
nod* nod_nou(token t)
{
    nod* n = new nod;
    strcpy(n->info, t.text);
    n->tip = t.tip;
    n->st = NULL;
    n->dr = NULL;
    return n;
}

bool e_operator(char c)
{
    if (strchr("+-*/^", c)) return true;
    return false;
}

// impartim textul in bucatele (tokeni)
void tokenizare(char s[])
{
    nr_tok = 0;
    int n = strlen(s);
    int i = 0;

    while (i < n)
    {
        // 1. sarim peste spatii
        if (isspace(s[i]))
        {
            i++;
            continue;
        }

        // 2. daca e numar
        if (isdigit(s[i]))
        {
            int k = 0;
            while (i < n && (isdigit(s[i]) || s[i] == '.'))
            {
                tokeni[nr_tok].text[k] = s[i];
                k++;
                i++;
            }
            tokeni[nr_tok].text[k] = '\0';
            tokeni[nr_tok].tip = tip_numar;
            tokeni[nr_tok].prioritate = -1;
            nr_tok++;
        }

        // 3. daca e litera (functie sau variabila)
        else if (isalpha(s[i]))
        {
            int k = 0;
            while (i < n && isalpha(s[i]))
            {
                tokeni[nr_tok].text[k] = s[i];
                k++;
                i++;
            }
            tokeni[nr_tok].text[k] = '\0';

            char t[50];
            strcpy(t, tokeni[nr_tok].text);

            // verificam daca e functie cunoscuta
            if (strcmp(t, "sin") == 0 || strcmp(t, "cos") == 0 ||
                    strcmp(t, "ln") == 0 || strcmp(t, "sqrt") == 0 ||
                    strcmp(t, "abs") == 0 || strcmp(t, "int") == 0)
            {
                tokeni[nr_tok].tip = tip_functie;
                tokeni[nr_tok].prioritate = 5;
            }
            else
            {
                tokeni[nr_tok].tip = tip_variabila;
            }
            nr_tok++;
        }

        // 4. operatori, paranteze, virgule
        else if (strchr("+-*/^,[]()", s[i]))
        {
            tokeni[nr_tok].text[0] = s[i];
            tokeni[nr_tok].text[1] = '\0';

            char c = s[i];
            if (c == '(')
            {
                tokeni[nr_tok].tip = tip_par_st;
                tokeni[nr_tok].prioritate = 0;
            }
            else if (c == ')')
            {
                tokeni[nr_tok].tip = tip_par_dr;
                tokeni[nr_tok].prioritate = 0;
            }
            else if (c == '[')
            {
                tokeni[nr_tok].tip = tip_mat_st;
                tokeni[nr_tok].prioritate = 0;
            }
            else if (c == ']')
            {
                tokeni[nr_tok].tip = tip_mat_dr;
                tokeni[nr_tok].prioritate = 0;
            }
            else if (c == ',')
            {
                tokeni[nr_tok].tip = tip_virgula;
                tokeni[nr_tok].prioritate = 0;
            }
            else   // operatori matematici
            {
                tokeni[nr_tok].tip = tip_operator;
                if (c == '+' || c == '-') tokeni[nr_tok].prioritate = 1;
                else if (c == '*' || c == '/') tokeni[nr_tok].prioritate = 2;
                else if (c == '^') tokeni[nr_tok].prioritate = 3;
            }
            nr_tok++;
            i++;
        }
        else
        {
            i++; // caractere necunoscute, le ignor
        }
    }
}

// verificam daca expresia e scrisa corect
bool validare()
{
    // verific parantezele
    int par = 0;
    for (int i = 0; i < nr_tok; i++)
    {
        if (tokeni[i].tip == tip_par_st) par++;
        if (tokeni[i].tip == tip_par_dr) par--;

        if (par < 0)
        {
            fout << "Eroare: S-a inchis o paranteza care nu a fost deschisa (poz " << i << ")!\n";
            return 0;
        }
    }
    if (par != 0)
    {
        fout << "Eroare: Numar inegal de paranteze deschise (" << par << " ramase neinchise)!\n";
        return 0;
    }

    // vecini
    // nu incepem cu operator in afara de + sau -
    if (tokeni[0].tip == tip_operator)
    {
        if (tokeni[0].text[0] != '+' && tokeni[0].text[0] != '-')
        {
            fout << "Eroare: Expresia nu poate incepe cu operatorul " << tokeni[0].text << "!\n";
            return 0;
        }
    }
    // nu putem incepe cu )
    if (tokeni[0].tip == tip_par_dr)
    {
        fout << "Eroare: Expresia nu poate incepe cu paranteza inchisa!\n";
        return 0;
    }

    // nu putem termina cu operator, functie sau paranteza deschisa
    if (tokeni[nr_tok - 1].tip == tip_operator ||
            tokeni[nr_tok - 1].tip == tip_functie ||
            tokeni[nr_tok - 1].tip == tip_par_st)
    {
        fout << "Eroare: Expresia este neterminata!\n";
        return 0;
    }

    // verific vecinii fiecarui element
    for (int i = 0; i < nr_tok - 1; i++)
    {
        token crt = tokeni[i];
        token urm = tokeni[i + 1];

        // A.operatori
        if (crt.tip == tip_operator)
        {
            if (urm.tip == tip_operator)
            {
                if (urm.text[0] != '+' && urm.text[0] != '-')
                {
                    fout << "Eroare: Doi operatori consecutivi nepermisi (" << crt.text << " " << urm.text << ")!\n";
                    return 0;
                }
            }
            if (urm.tip == tip_par_dr)
            {
                fout << "Eroare: Operator urmat de paranteza inchisa!\n";
                return 0;
            }
        }

        // B.functii
        if (crt.tip == tip_functie)
        {
            if (urm.tip != tip_par_st)
            {
                fout << "Eroare: Dupa functie (" << crt.text << ") trebuie sa urmeze '(' !\n";
                return 0;
            }
        }

        // C.nr sau variabila
        if (crt.tip == tip_numar || crt.tip == tip_variabila)
        {
            if (urm.tip == tip_numar || urm.tip == tip_variabila || urm.tip == tip_functie || urm.tip == tip_par_st)
            {
                fout << "Eroare: Lipseste operatorul intre elemente la pozitia " << i << "!\n";
                return 0;
            }
        }

        // D. Paranteze deschise
        if (crt.tip == tip_par_st)
        {
            if (urm.tip == tip_operator)
            {
                // tre sa ffie neaparat + sau -
                if (urm.text[0] != '+' && urm.text[0] != '-')
                {
                    fout << "Eroare: Operatorul '" << urm.text << "' nu poate sta imediat dupa paranteza deschisa!\n";
                    return 0;
                }
            }
            if (urm.tip == tip_par_dr)
            {
                fout << "Eroare: Paranteze goale () !\n";
                return 0;
            }
        }
    }
    return 1; // totul e ok
}

nod* const_arbore(int st, int dr)
{
    if (st > dr) return NULL;
    if (st == dr) return nod_nou(tokeni[st]);

    int prio_min = 999;
    int poz_op = -1;
    int par = 0; //si pt () si pt []

    // 1.op cu prio minima
    for (int i = dr; i >= st; i--)
    {
        token t = tokeni[i];

        // Numaram parantezele ca sa sarim peste ce e inauntru
        if (t.tip == tip_par_dr || t.tip == tip_mat_dr) par++;
        else if (t.tip == tip_par_st || t.tip == tip_mat_st) par--;

        // daca suntem in afara parantezelor
        else if (par == 0)
        {
            if (t.tip == tip_operator || t.tip == tip_virgula)
            {
                if (t.prioritate < prio_min)
                {
                    prio_min = t.prioritate;
                    poz_op = i;
                }
                // virgula are prioritate 0, deci va fi aleasa ultima
            }
        }
    }

    // A. Daca am gasit operator
    if (poz_op != -1)
    {
        nod* rad = nod_nou(tokeni[poz_op]);
        rad->st = const_arbore(st, poz_op - 1);
        rad->dr = const_arbore(poz_op + 1, dr);
        return rad;
    }

    // B. Daca e intre paranteze (), le eliminam
    if (tokeni[st].tip == tip_par_st && tokeni[dr].tip == tip_par_dr)
    {
        int k = 0;
        bool ok = true;
        for (int i = st; i < dr; i++)
        {
            if (tokeni[i].tip == tip_par_st) 
                k++;
            if (tokeni[i].tip == tip_par_dr) 
                k--;
            if (k == 0)
            {
                ok = false;
                break;
            }
        }
        if (ok) return const_arbore(st + 1, dr - 1);
    }

    // C. Daca e matrice intre [], facem nod de tip matrice
    if (tokeni[st].tip == tip_mat_st && tokeni[dr].tip == tip_mat_dr)
    {
        int k = 0;
        bool ok = true;
        for (int i = st; i < dr; i++)
        {
            if (tokeni[i].tip == tip_mat_st) k++;
            if (tokeni[i].tip == tip_mat_dr) k--;
            if (k == 0)
            {
                ok = false;
                break;
            }
        }
        if (ok)
        {
            nod* n_mat = new nod;
            strcpy(n_mat->info, "[]");
            n_mat->tip = tip_matrice;
            n_mat->st = NULL;
            n_mat->dr = const_arbore(st + 1, dr - 1);
            return n_mat;
        }
    }

    // D. Daca e functie
    if (tokeni[st].tip == tip_functie)
    {
        nod* rad = nod_nou(tokeni[st]);
        rad->st = NULL;
        rad->dr = const_arbore(st + 1, dr);
        return rad;
    }

    return NULL;
}

// functie ajutatoare pentru matrice
void f_matrice(nod* n, nod* mat[max_linii][max_col], int& l, int& c, int l_crt, int c_crt)
{
    if (n == NULL) return;

    if (n->tip == tip_matrice)
    {
        f_matrice(n->dr, mat, l, c, l_crt, 0);
        if (l_crt != -1) l++;
        return;
    }

    if (n->tip == tip_virgula)
    {
        // daca virgula separa linii
        if (n->st->tip == tip_matrice)
        {
            f_matrice(n->st, mat, l, c, l, 0);
            f_matrice(n->dr, mat, l, c, l, 0);
        }
        // daca virgula separa coloane
        else
            f_matrice(n->st, mat, l, c, l_crt, c_crt);
        return;
    }

    // pun elementul in prima pozitie libera
    int k = 0;
    while (mat[l][k] != NULL && k < max_col) k++;
    if (k < max_col)
    {
        mat[l][k] = n;
        if (k + 1 > c) c = k + 1;
    }
}

void extrage_linii(nod* n, nod* lista[], int& k)
{
    if (n == NULL) return;
    if (n->tip == tip_virgula)
    {
        extrage_linii(n->st, lista, k);
        extrage_linii(n->dr, lista, k);
    }
    else
    {
        lista[k++] = n;
    }
}

// transformam arborele matricei intr-un grid efectiv
void const_matrice(nod* rad_mat, nod* mat[max_linii][max_col], int& l, int& c)
{
    for (int i = 0; i < max_linii; i++)
        for (int j = 0; j < max_col; j++) mat[i][j] = NULL;
    l = 0;
    c = 0;

    nod* continut = rad_mat->dr;

    // 1. Luam randurile
    nod* lista_linii[max_linii];
    int nr_linii = 0;
    extrage_linii(continut, lista_linii, nr_linii);
    l = nr_linii;

    // 2. Pentru fiecare linie luam elementele
    for (int i = 0; i < l; i++)
    {
        if (lista_linii[i]->tip == tip_matrice)
        {
            nod* elem[max_col];
            int nr_elem = 0;
            extrage_linii(lista_linii[i]->dr, elem, nr_elem);

            if (nr_elem > c) c = nr_elem;

            for (int j = 0; j < nr_elem; j++)
            {
                mat[i][j] = elem[j];
            }
        }
        else
        {
            // vector simplu [1, 2, 3]
            mat[0][i] = lista_linii[i];
            if (i == 0)
            {
                l = 1;
                c = nr_linii;
            }
        }
    }
}

void afisare_arbore(nod* r, int nivel)
{
    if (r == NULL) return;
    afisare_arbore(r->dr, nivel + 1);
    for (int i = 0; i < nivel; i++) fout << "    ";
    fout << r->info << endl;
    afisare_arbore(r->st, nivel + 1);
}

// calculam dimensiunile grafice ale fiecarui nod (lat si inalt)
void calc_dim(nod* n)
{
    if (n == NULL) return;

    calc_dim(n->st);
    calc_dim(n->dr);

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, marime_font);

    // 1. frunza (numar/variabila)
    if (n->st == NULL && n->dr == NULL)
    {
        n->lat = textwidth(n->info);
        n->inalt = textheight(n->info);
    }

    // 2. operatori simpli (+, -, *)
    else if (n->info[0] == '+' || n->info[0] == '-' || n->info[0] == '*')
    {
        int lat_st = n->st ? n->st->lat : 0;
        int lat_dr = n->dr ? n->dr->lat : 0;
        int inalt_st = n->st ? n->st->inalt : 0;
        int inalt_dr = n->dr ? n->dr->inalt : 0;

        int lat_op = textwidth(n->info);

        n->lat = lat_st + lat_op + lat_dr + 2 * spatiu_op;
        n->inalt = max(inalt_st, inalt_dr);
    }

    // 3. impartire (/) - desenat ca fractie
    else if (n->info[0] == '/')
    {
        int lat_st = n->st->lat;
        int lat_dr = n->dr->lat;

        n->lat = max(lat_st, lat_dr) + 10;
        n->inalt = n->st->inalt + n->dr->inalt + 10;
    }

    // 4. putere (^)
    else if (n->info[0] == '^')
    {
        int lat_baza = n->st->lat;
        int lat_exp = n->dr->lat;

        n->lat = lat_baza + lat_exp;
        n->inalt = n->st->inalt + (n->dr->inalt / 2);
    }

    // 5. functii
    else if (n->tip == tip_functie)
    {
        if (strcmp(n->info, "int") == 0)
        {
            int lat_arg = n->dr->lat;
            int inalt_arg = n->dr->inalt;

            n->lat = 15 + lat_arg + 10 + textwidth("dx");
            n->inalt = max(textheight("A") * 2, inalt_arg + 10);
        }
        else
        {
            int lat_nume = textwidth(n->info);
            int lat_arg = n->dr->lat;

            n->lat = lat_nume + lat_arg + 20;
            n->inalt = max(textheight("A"), n->dr->inalt);
        }
    }
    // 6. matrice
    else if (n->tip == tip_matrice)
    {
        nod* mat[max_linii][max_col];
        int l, c;
        const_matrice(n, mat, l, c);

        int col_w[max_col] = { 0 };
        int row_h[max_linii] = { 0 };

        // calculam maximele pe linii si coloane
        for (int i = 0; i < l; i++)
        {
            for (int j = 0; j < c; j++)
            {
                if (mat[i][j])
                {
                    calc_dim(mat[i][j]);
                    if (mat[i][j]->lat > col_w[j]) col_w[j] = mat[i][j]->lat;
                    if (mat[i][j]->inalt > row_h[i]) row_h[i] = mat[i][j]->inalt;
                }
            }
        }

        int total_w = 0;
        for (int j = 0; j < c; j++) total_w += col_w[j];
        total_w += (c - 1) * 15; // spatiu intre col
        total_w += 30; // paranteze

        int total_h = 0;
        for (int i = 0; i < l; i++) total_h += row_h[i];
        total_h += (l - 1) * 10; // spatiu intre linii
        total_h += 10;

        n->lat = total_w;
        n->inalt = total_h;
    }
}

void deseneaza(nod* n, int x, int y)
{
    if (n == NULL) return;

    settextstyle(SANS_SERIF_FONT, HORIZ_DIR, marime_font);
    setcolor(WHITE);

    // 1. frunza (nr sau variabia)
    if (n->st == NULL && n->dr == NULL)
        outtextxy(x - n->lat / 2, y - n->lat / 2, n->info);

    // 2. operatori
    else if (n->info[0] == '+' || n->info[0] == '-' || n->info[0] == '*')
    {
        outtextxy(x - textwidth(n->info) / 2, y - 5, n->info);
        
        //poz fiilor
        int lat_st = (n->st) ? n->st->lat : 0;
        int lat_dr = (n->dr) ? n->dr->lat : 0;
        int lat_op = textwidth(n->info);

        if (n->st)
            deseneaza(n->st, x - lat_op / 2 - spatiu_op - lat_st / 2, y);

        if (n->dr)
            deseneaza(n->dr, x + lat_op / 2 + spatiu_op + lat_dr / 2, y);
    }

    // 3. fractie
    else if (n->info[0] == '/')
    {
        int lungime = n->lat;
        line(x - lungime / 2, y + 7, x + lungime / 2, y + 7);//linia de fractie

        int inalt_st = n->st->inalt;
        deseneaza(n->st, x, y - inalt_st / 2 - 5);//numarator

        int inalt_dr = n->dr->inalt;
        deseneaza(n->dr, x, y + inalt_dr / 2 + 5);//numitor
    }

    // 4. putere
    else if (n->info[0] == '^')
    {
        int lat_baza = n->st->lat;
        int lat_exp = n->dr->lat;

        deseneaza(n->st, x - lat_exp / 2, y + 10);

        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, marime_font - 1); //exponentul mai sus si mai mic
        deseneaza(n->dr, x + lat_baza / 2, y - 15);
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, marime_font);//revenim la font normal
    }

    // 5. functii
    else if (n->tip == tip_functie)
    {
        if (strcmp(n->info, "int") == 0)
        {
            int h = n->inalt;
            int x_simbol = x - n->lat / 2 + 5;

            arc(x_simbol, y - h / 2 + 5, 0, 180, 5);//carlig sus
            arc(x_simbol, y + h / 2 - 5, 180, 360, 5);//carlig jos
            line(x_simbol - 5, y - h / 2 + 5, x_simbol + 5, y + h / 2 - 5);//linia

            int lat_arg = n->dr->lat;//expresia de dupa semnul de integrala
            deseneaza(n->dr, x_simbol + 15 + lat_arg / 2, y);
            // dx la final
            outtextxy(x + n->lat / 2 - textwidth("dx"), y - textheight("d") / 2, "dx");
        }
        else
        {
            int lat_nume = textwidth(n->info);
            int lat_arg = n->dr->lat;
            int x_start = x - n->lat / 2;

            outtextxy(x_start, y - textheight("A") / 2, n->info);// numele functiei
            outtextxy(x_start + lat_nume, y - textheight("A") / 2, "(");// desen (
            deseneaza(n->dr, x_start + lat_nume + 10 + lat_arg / 2, y);// arg functiei
            outtextxy(x_start + lat_nume + 10 + lat_arg + 5, y - textheight("A") / 2, ")");// desen )
        }
    }
    // 6. matrice
    else if (n->tip == tip_matrice)
    {
        nod* mat[max_linii][max_col];
        int l, c;
        const_matrice(n, mat, l, c);

        int w = n->lat;
        int h = n->inalt;

        // Desenam parantezele mari
        // Stanga [
        line(x - w / 2, y - h / 2, x - w / 2 + 8, y - h / 2);
        line(x - w / 2, y - h / 2, x - w / 2, y + h / 2);
        line(x - w / 2, y + h / 2, x - w / 2 + 8, y + h / 2);
        // Dreapta ]
        line(x + w / 2, y - h / 2, x + w / 2 - 8, y - h / 2);
        line(x + w / 2, y - h / 2, x + w / 2, y + h / 2);
        line(x + w / 2, y + h / 2, x + w / 2 - 8, y + h / 2);

        int col_w[max_col] = { 0 };
        int row_h[max_linii] = { 0 };
        for (int i = 0; i < l; i++)
            for (int j = 0; j < c; j++)
                if (mat[i][j])
                {
                    if (mat[i][j]->lat > col_w[j]) col_w[j] = mat[i][j]->lat;
                    if (mat[i][j]->inalt > row_h[i]) row_h[i] = mat[i][j]->inalt;
                }

        int start_y = y - h / 2 + 5;//y in coltul stanga sus

        for (int i = 0; i < l; i++)
        {
            int crt_y = start_y + row_h[i] / 2;//y curent pentru centrul randului
            int start_x = x - w / 2 + 15;// x  de start

            for (int j = 0; j < c; j++)
            {
                int crt_x = start_x + col_w[j] / 2;//x curent pentru centrul coloanei
                if (mat[i][j])
                {
                    deseneaza(mat[i][j], crt_x, crt_y);
                }
                start_x += col_w[j] + 15;
            }
            start_y += row_h[i] + 10;
        }
    }
}

int main()
{
    char expresie[1001];

    if (!fin)
    {
        cout << "Eroare citire" << endl;
        return 1;
    }

    fin.getline(expresie, 1001);

    tokenizare(expresie);

    if (validare())
    {
        cout << "Valid. Se genereaza arborele in expresie.out" << endl;
        nod* radacina = const_arbore(0, nr_tok - 1);
        afisare_arbore(radacina, 0);

        // partea grafica
        initwindow(1200, 600, "Vizualizator de formule matematice");
        settextstyle(SANS_SERIF_FONT, HORIZ_DIR, marime_font);

        calc_dim(radacina);

        outtextxy(50, 50, "Formula arata asa: ");
        deseneaza(radacina, 600, 300);

        getch();//tasta pt inchidere
        closegraph();
    }
    else
    {
        cout << "Expresie invalida" << endl;
    }

    return 0;
}
