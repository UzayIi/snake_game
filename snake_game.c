#include <stdio.h>
#include <stdlib.h>
#include <windows.h> // Sleep(), SetConsoleCursorPosition(), CONSOLE_CURSOR_INFO
#include <conio.h>   // _kbhit(), _getch()
#include <time.h>    // time()

// --- Oyun Alanı Ayarları ---
#define WIDTH 50
#define HEIGHT 20

// --- Global Değişkenler ---
int game_over;
int score;

// Konumları saklamak için bir yapı (struct)
typedef struct {
    int x;
    int y;
} Point;

Point head, fruit;
Point tail[100]; // Yılanın kuyruğu için bir dizi (maksimum 100 parça)
int tail_len;

// Yönler için bir enum (numaralandırma)
enum Direction {
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};
enum Direction dir;

// --- Yardımcı Fonksiyonlar (Windows Konsol API) ---

// İmleci konsolda belirli bir (x, y) koordinatına taşır
void GoToXY(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Konsol imlecini gizler (oyun sırasında yanıp sönmesin diye)
void HideCursor() {
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
}

// --- Oyun Fonksiyonları ---

// Oyunu başlatan kurulum fonksiyonu
void Setup() {
    HideCursor();
    game_over = 0;
    dir = STOP; // Başlangıçta yılan duruyor
    score = 0;
    tail_len = 0;

    // Yılanın başını ekranın ortasına yerleştir
    head.x = WIDTH / 2;
    head.y = HEIGHT / 2;

    // Rastgele sayı üretecini başlat
    srand(time(NULL));

    // İlk yemi rastgele bir konuma yerleştir
    fruit.x = rand() % WIDTH;
    fruit.y = rand() % HEIGHT;
}

// Ekranı çizen fonksiyon
void Draw() {
    // İmleci (0, 0)'a (sol üst köşe) taşıyarak ekranı temizlemeden üzerine yaz
    // Bu, system("cls") kullanmaktan daha hızlıdır ve titreşimi engeller.
    GoToXY(0, 0);

    // 1. Üst duvarı çiz
    for (int i = 0; i < WIDTH + 2; i++)
        printf("#");
    printf("\n");

    // 2. Oyun alanını ve içindekileri çiz
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Sol duvar
            if (x == 0)
                printf("#");

            // Yılanın başı
            if (x == head.x && y == head.y) {
                printf("O");
            }
            // Yem
            else if (x == fruit.x && y == fruit.y) {
                printf("F"); // "Fruit" (Yem)
            }
            // Diğer
            else {
                int is_tail = 0;
                // Yılanın kuyruğunu kontrol et
                for (int k = 0; k < tail_len; k++) {
                    if (tail[k].x == x && tail[k].y == y) {
                        printf("o");
                        is_tail = 1;
                        break;
                    }
                }
                // Kuyruk değilse boşluk bas
                if (!is_tail)
                    printf(" ");
            }

            // Sağ duvar
            if (x == WIDTH - 1)
                printf("#");
        }
        printf("\n");
    }

    // 3. Alt duvarı çiz
    for (int i = 0; i < WIDTH + 2; i++)
        printf("#");
    printf("\n");

    // 4. Skoru yaz
    printf("Skor: %d\n", score);
    printf("Kapatmak icin 'X'e basin.\n");
}

// Kullanıcı girdisini (klavye) alan fonksiyon
void Input() {
    // _kbhit() : Klavyede bir tuşa basılıp basılmadığını kontrol eder
    if (_kbhit()) {
        // _getch() : Basılan tuşun karakterini alır (Enter'a basmayı beklemez)
        switch (_getch()) {
        case 'a':
        case 'A':
            if (dir != RIGHT) // Yılanın kendi üzerine dönmesini engelle
                dir = LEFT;
            break;
        case 'd':
        case 'D':
            if (dir != LEFT)
                dir = RIGHT;
            break;
        case 'w':
        case 'W':
            if (dir != DOWN)
                dir = UP;
            break;
        case 's':
        case 'S':
            if (dir != UP)
                dir = DOWN;
            break;
        case 'x':
        case 'X':
            game_over = 1; // 'X' tuşu oyunu bitirir
            break;
        }
    }
}

// Oyunun mantığını (hareket, çarpışma vb.) işleten fonksiyon
void Logic() {
    // 1. Kuyruğu güncelle (Her parça bir öncekinin yerine geçer)
    Point prev_pos = head; // Başın şu anki konumu
    Point temp;

    for (int i = 0; i < tail_len; i++) {
        temp = tail[i];
        tail[i] = prev_pos;
        prev_pos = temp;
    }

    // 2. Başı yeni yöne göre hareket ettir
    switch (dir) {
    case LEFT:
        head.x--;
        break;
    case RIGHT:
        head.x++;
        break;
    case UP:
        head.y--;
        break;
    case DOWN:
        head.y++;
        break;
    default:
        break; // Duruyorsa (STOP) hareket etme
    }

    // 3. Çarpışma Kontrolü (Duvarlar)
    if (head.x >= WIDTH || head.x < 0 || head.y >= HEIGHT || head.y < 0) {
        game_over = 1;
        return; // Fonksiyondan çık, oyun bitti
    }

    // 4. Çarpışma Kontrolü (Kendi kuyruğu)
    for (int i = 0; i < tail_len; i++) {
        if (tail[i].x == head.x && tail[i].y == head.y) {
            game_over = 1;
            return;
        }
    }

    // 5. Yem Yeme Kontrolü
    if (head.x == fruit.x && head.y == fruit.y) {
        score += 10;
        tail_len++; // Kuyruğu bir parça uzat

        // Yeni yem oluştur
        fruit.x = rand() % WIDTH;
        fruit.y = rand() % HEIGHT;

        // (İsteğe bağlı) Yeni yemin yılanın üzerinde oluşmasını engelle
        int on_snake;
        do {
            on_snake = 0;
            for(int i = 0; i < tail_len; i++) {
                if(fruit.x == tail[i].x && fruit.y == tail[i].y) {
                    on_snake = 1;
                    fruit.x = rand() % WIDTH;
                    fruit.y = rand() % HEIGHT;
                    break;
                }
            }
        } while (on_snake);

    }
}

// --- Ana Fonksiyon ---
int main() {
    Setup(); // Oyunu kur

    // Ana Oyun Döngüsü
    while (!game_over) {
        Draw();  // Ekranı çiz
        Input(); // Girdiyi al
        Logic(); // Oyun mantığını işlet
        Sleep(100); // Oyun hızı (100 milisaniye bekle)
    }

    // Oyun bitti ekranı
    GoToXY(WIDTH / 2 - 5, HEIGHT / 2); // Ekranın ortasına git
    printf("OYUN BITTI!");
    GoToXY(WIDTH / 2 - 7, HEIGHT / 2 + 1);
    printf("Skorunuz: %d\n", score);
    
    // İmleci tekrar görünür yap ve programdan çık
    GoToXY(0, HEIGHT + 4); // İmleci en alta al
    
    return 0;
}