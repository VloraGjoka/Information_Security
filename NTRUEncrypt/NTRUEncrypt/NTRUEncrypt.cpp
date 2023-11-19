#include <iostream>
#include <vector>
#include <map>  
#include <random>
#include <chrono> 
#include <string>
using namespace std;

typedef long long type;

// Public constants
const type N = 23, p = 257, q = 17737, INF = N * 100;

const int d = N / 2;

map<char, int> char_to_int;
map<int, char> int_to_char;

mt19937_64 llrand((int)chrono::steady_clock::now().time_since_epoch().count());

struct polynomial {
    vector<type> coef;

    // Konstruktori i polinomit
    polynomial(vector<type> _coef = {}) : coef(_coef) {}

    // Merr gradën e polinomit
    type degree() {
        if (coef.empty()) return -INF;
        return coef.size() - 1;
    }

    // Operatorë për krahasimin e polinomeve
    bool operator !=(polynomial o) {
        if (coef.size() != o.coef.size())
            return true;
        for (int i = 0; i < coef.size(); i++)
            if (coef[i] != o.coef[i])
                return true;
        return false;
    }

    bool operator ==(polynomial o) {
        return !((*this) != o);
    }

    // Rrit madhësinë e polinomit
    void resize(size_t new_size) {
        size_t old_size = coef.size();
        coef.resize(new_size);
        for (size_t i = old_size; i < new_size; i++)
            coef[i] = 0;
    }

    // Merr elementin e fundit të polinomit
    type back() {
        return coef.back();
    }

    // Merr madhësinë e polinomit
    size_t size() {
        return coef.size();
    }

    // Kontrollon nëse polinomi është bosh
    bool empty() {
        return coef.empty();
    }

    // Hiq elementin e fundit të polinomit
    void pop_back() {
        coef.pop_back();
    }

    // Operator për qasjen në elementet e polinomit
    type& operator[](size_t id) {
        return coef[id];
    }
};

// Struktura e një polinomi
polynomial base;

// Struktura për çelësat e kriptimit
struct key {
    polynomial f, g, fp, fq; // Polinomet përbërëse të çelësave
    polynomial h;            // Pjesë e çelësit publik

    key() {} // Konstruktori i çelësit
};

// Funksioni për të printuar një polinom
void print(polynomial a) {
    bool first = true; // Kontrollon nëse është elementi i parë
    if (a.empty())
        cout << "0"; // Nëse polinomi është bosh, printo "0"
    for (int i = a.size() - 1; i >= 0; i--) { // Kalon nëpër të gjithë elementet e polinomit
        if (a[i] == 0)
            continue; // Anashkalon elementet zero
        // Logjika për të printuar çdo term të polinomit
        if (first) {
            // Rasti i parë kur printohet termi i parë i polinomit
            if (i == 0)
                cout << a[i];
            else if (abs(a[i]) != 1)
                cout << a[i] << "*x";
            else if (a[i] == -1)
                cout << "-x";
            else
                cout << "x";
            if (i > 1)
                cout << "^" << i;
            first = false;
        }
        else {
            // Shtimi i simboleve të plus dhe minus për termat e mëvonshëm
            if (a[i] > 0)
                cout << " + ";
            else
                cout << " - ";
            if (i == 0)
                cout << abs(a[i]);
            else {
                if (abs(a[i]) != 1)
                    cout << abs(a[i]) << "*";
                cout << "x";
                if (i > 1)
                    cout << "^" << i;
            }
        }
    }
    cout << endl;
}

// Funksioni për shtimin e dy polinomeve
polynomial add(polynomial a, polynomial b, type mod) {
    polynomial r; // Polinomi rezultant
    r.resize(max(a.size(), b.size())); // Vendos madhësinë e polinomit rezultant
    // Shtimi i elementëve të dy polinomeve
    for (size_t i = 0; i < r.size(); i++) {
        if (i < a.size())
            r[i] = a[i];
        if (i < b.size())
            r[i] += b[i];
        r[i] %= mod; // Aplikon modulo për çdo term
    }
    // Largon çdo term zero në fund të polinomit
    while (!r.empty() && r.back() == 0)
        r.pop_back();
    return r;
}

// Zbritja e dy polinomeve
polynomial sub(polynomial a, polynomial b, type mod) {
    polynomial r; // Polinomi rezultant
    r.resize(max(a.size(), b.size())); // Cakto madhësinë maksimale të polinomit
    for (size_t i = 0; i < r.size(); i++) {
        if (i < a.size())
            r[i] = a[i]; // Kopjo elementin nga polinomi a
        if (i < b.size())
            r[i] -= b[i]; // Zbrit elementin nga polinomi b
        r[i] %= mod; // Apliko modulo për të mbetur në rangun e modulit
    }
    while (!r.empty() && r.back() == 0)
        r.pop_back(); // Largo elementet zero në fund të polinomit
    return r;
}

// Shumëzimi i dy polinomeve
polynomial multiply(polynomial a, polynomial b, type mod) {
    polynomial r; // Polinomi rezultant
    if (a.degree() + b.degree() >= 0)
        r.resize(a.degree() + b.degree() + 1); // Cakto madhësinë e polinomit rezultant
    for (size_t i = 0; i < a.size(); i++)
        for (size_t j = 0; j < b.size(); j++)
            r[i + j] = (r[i + j] + a[i] * b[j]) % mod; // Shumëzo dhe shto elementet e polinomeve
    while (!r.empty() && r.back() == 0)
        r.pop_back(); // Largo elementet zero në fund të polinomit
    return r;
}

// Funksioni për të gjetur funksionin phi të Euler
type phi(type mod) {
    type ans = mod; // Vlera fillestare e phi
    for (type i = 2; i * i <= mod; i++) {
        if (mod % i == 0) ans -= ans / i; // Zbrit pjesën e përputhshme nëse i është faktor
        while (mod % i == 0) mod /= i; // Ndaj me faktorët e thjeshtë
    }
    if (mod != 1) ans -= ans / mod; // Zbrit pjesën e përputhshme nëse ka mbetur një faktor i thjeshtë
    return ans; // Kthe vlerën e phi
}

// Funksioni për ngritjen në fuqi me modul
type fexp(type x, type y, type mod) {
    type r = 1; // Vlera fillestare
    while (y) {
        if (y & 1) // Nëse biti më i ulët i y është 1
            r = r * x % mod; // Shumëzo me bazën dhe apliko modulin
        x = x * x % mod; // Ngrit bazën në katror dhe apliko modulin
        y >>= 1; // Shifto y djathtas për një bit
    }
    return (r + mod) % mod; // Kthe rezultatin e ngritjes në fuqi
}

// Funksioni për të gjetur inversin modular
type inv(type x, type mod) {
    return fexp(x, phi(mod) - 1, mod); // Përdor ngritjen në fuqi për të gjetur inversin
}

// Funksioni për pjesëtimin e dy polinomeve (duke marrë vetëm pjesën e plotë)
polynomial poly_floor(polynomial a, polynomial b, type mod) {
    polynomial r; // Polinomi rezultant
    if (b.degree() == -INF) { // Kontrollo nëse po ndahet me zero
        cout << "Division by zero\n";
        return r;
    }
    if (b.degree() > a.degree())
        return r; // Nëse gradë e a është më e vogël se e b, kthe polinom bosh
    r.resize(a.degree() - b.degree() + 1); // Cakto madhësinë e polinomit rezultant
    while (a.degree() >= b.degree()) { // Ndërsa gradë e a është më e madhe ose e barabartë me gradë e b
        type mult = (a.back() * inv(b.back(), mod) % mod); // Gjej koeficientin përforcues
        r[a.degree() - b.degree()] = mult; // Shto në polinomin rezultant
        for (int i = 0; i <= b.degree(); i++)
            a[a.degree() - b.degree() + i] = (a[a.degree() - b.degree() + i] - b[i] * mult) % mod; // Llogarit diferencën dhe apliko modulin
        while (!a.empty() && a.back() == 0)
            a.pop_back(); // Largo elementet zero në fund të polinomit
    }
    while (!r.empty() && r.back() == 0)
        r.pop_back(); // Largo elementet zero në fund të polinomit rezultant
    return r;
}

// Funksioni për të marrë mbetjen e ndarjes së dy polinomeve
polynomial remainder(polynomial a, polynomial b, type mod) {
    polynomial r; // Polinomi rezultant
    if (b.degree() == -INF) // Nëse po ndahet me zero
        r = a; // Kthe polinomin a si mbetje
    else
        r = sub(a, multiply(b, poly_floor(a, b, mod), mod), mod); // Përndryshe, gjej mbetjen
    return r;
}

// Funksioni për të gjeneruar një numër të rastësishëm
int myrandom(int i) { return llrand() % i; }

// Funksioni për të gjeneruar një çelës të rastësishëm polinomial
polynomial random_key(int qp, int qn) {
    polynomial key; // Inicializon polinomin për çelësin
    key.coef.resize(N); // Cakton madhësinë e polinomit
    for (int i = 0; i < qp; i++)
        key.coef[i] = 1; // Vendos koeficientët pozitivë
    for (int i = 0; i < qn; i++)
        key.coef[i + qp] = -1; // Vendos koeficientët negativë
    random_shuffle(key.coef.begin(), key.coef.end(), myrandom); // Përzien koeficientët në mënyrë të rastësishme
    while (!key.empty() && key.back() == 0)
        key.pop_back(); // Largon zero në fund të polinomit
    return key; // Kthen çelësin e gjeneruar
}

// Gjen GCD (Greatest Common Divisor) e dy polinomeve
polynomial gcd(polynomial a, polynomial b, polynomial& x, polynomial& y, type mod) {
    if (b.degree() >= 0) {
        polynomial r = gcd(b, remainder(a, b, mod), y, x, mod); // Thirr rekursivisht GCD me mbetjen
        y = sub(y, multiply(x, poly_floor(a, b, mod), mod), mod); // Përditëson polinomin y
        return r; // Kthen GCD
    }
    else {
        x = polynomial({ 1 }); // Inicializon polinomin x
        y = polynomial(); // Inicializon polinomin y
        return a; // Kthen polinomin a si GCD
    }
}

// Gjeneron çelësat publikë dhe privatë për kriptim
key generate_keys() {
    key r; // Inicializon strukturën e çelësve
    polynomial aux; // Polinom ndihmës
    r.f = random_key(d + 1, d); // Gjeneron polinomin f
    // Kontrollon për të siguruar që f është i shkëmbyeshëm me bazën në modulat p dhe q
    while (gcd(r.f, base, r.fp, aux, p) != polynomial({ 1 }) || gcd(r.f, base, r.fq, aux, q) != polynomial({ 1 }))
        r.f = random_key(d + 1, d); // Rixhenron f nëse nuk plotësohen kushtet
    r.g = random_key(d, d); // Gjeneron polinomin g
    r.h = remainder(multiply(r.fq, r.g, q), base, q); // Llogarit polinomin h si mbetjen e f*q dhe g
    return r; // Kthen çelësat e gjeneruar
}

// Konverton një mesazh në një polinom
polynomial msg_to_poly(string message) {
    polynomial m; // Inicializon polinomin për mesazhin
    m.resize(message.size()); // Cakton madhësinë e polinomit
    for (int i = 0; i < message.size(); i++)
        m[i] = char_to_int[message[i]]; // Konverton çdo karakter në një numër dhe e shton në polinom
    return m; // Kthen polinomin e mesazhit
}

// Funksioni për enkriptimin e një mesazhi
polynomial encrypt(string message, polynomial h) {
    polynomial r = random_key(d, d); // Gjeneron një çelës të rastësishëm r
    polynomial m = msg_to_poly(message); // Konverton mesazhin në polinom
    // Printon polinomin e mesazhit dhe çelësin r
    cout << "m = ";
    print(m);
    cout << "r = ";
    print(r);
    for (type& i : r.coef)
        i = i * p % q; // Shumëzon çdo koeficient të r me p dhe aplikohen mod q
    while (!r.empty() && r.back() == 0)
        r.pop_back(); // Largon zero në fund të polinomit r
    return remainder(add(m, multiply(r, h, q), q), base, q); // Kthen mbetjen e shtimit të m dhe shumëzimit të r me h
}

// Funksioni për dekriptimin e një mesazhi të enkriptuar
polynomial decrypt(polynomial e, key k) {
    polynomial a = remainder(multiply(k.f, e, q), base, q); // Shumëzon polinomin e enkriptuar me f dhe aplikohen mod q
    // Rregullon koeficientët e polinomit a për të qenë në rangun e duhur
    for (type& i : a.coef) {
        if (i < -q / 2) i += q;
        if (i > q / 2) i -= q;
    }
    return remainder(multiply(k.fp, a, p), base, p); // Shumëzon a me fp dhe aplikohen mod p
}

// Konverton një polinom në një mesazh të tipit string
string poly_to_msg(polynomial d) {
    string msg; // Stringu që përmban mesazhin
    for (int i = 0; i < d.size(); i++) {
        // Rregullon koeficientët e polinomit nëse janë jashtë rangut
        if (d[i] < -p / 2)
            d[i] += p;
        if (d[i] > p / 2)
            d[i] -= p;
        msg += int_to_char[d[i]]; // Shton çdo koeficient si karakter në string
    }
    return msg; // Kthen mesazhin si string
}

// Funksioni kryesor
int main() {
    // Inicializon tabelat për konvertimin nga karakter në int dhe anasjelltas
    for (int i = 0; i < 128; i++) {
        char_to_int[char(i)] = -128 + i;
        int_to_char[-128 + i] = char(i);
    }
    for (int i = 128; i < 256; i++) {
        char_to_int[char(i)] = i - 127;
        int_to_char[i - 127] = char(i);
    }

    // Inicializon polinomin bazë
    base.coef.resize(N + 1);
    base.coef[N] = 1;
    base.coef[0] = -1;
    cout << "Base = ";
    print(base); // Printon polinomin bazë

    // Gjeneron çelësat publik dhe privat
    key k = generate_keys();
    cout << "N = " << N << ", p = " << p << ", q = " << q << endl;
    // Printon pjesët e çelësave
    cout << "f = ";
    print(k.f);
    cout << "fp = ";
    print(k.fp);
    cout << "fq = ";
    print(k.fq);
    cout << "g = ";
    print(k.g);
    cout << "h = ";
    print(k.h);

    // Merr input nga përdoruesi
    cout << "\nEnter the message to be encrypted: ";
    string user_input;
    getline(cin, user_input);

    cout << "\nOriginal message:\n" << user_input << endl;
    string decrypt_message;
    // Proceson mesazhin në pjesë të veçanta për enkriptim dhe dekriptim
    for (int i = 0; i < user_input.size(); i += N) {
        string message = user_input.substr(i, min((int)N, (int)user_input.size() - i));
        cout << "\nPart #" << (i + N) / N << ":\n" << message << "\n";

        // Enkripton mesazhin
        polynomial e = encrypt(message, k.h);
        cout << "e = ";
        print(e); // Printon mesazhin e enkriptuar

        // Dekripton mesazhin
        polynomial d = decrypt(e, k);
        cout << "d = ";
        print(d); // Printon mesazhin e dekriptuar

        // Konverton polinomin e dekriptuar në string dhe e shton në mesazhin përfundimtar
        decrypt_message += poly_to_msg(d);
    }

    cout << "\nDecrypted message:\n" << decrypt_message << endl;
    return 0; // Përfundon ekzekutimin e programit
}