#include <iostream>
#include <windows.h>  
#include <cstdlib>
#include <ctime>
#include <cmath>

#define HEIGHT 90
#define WIDTH  160

struct vec2{
    int x, y;
};

// up / upl / upr / mid / lowl / lowr / low
char constexpr segs[10] {
    0b1110111, // 0
    0b0010010, // 1
    0b1011101, // 2
    0b1011011, // 3
    0b0111010, // 4
    0b1101011, // 5
    0b1101111, // 6
    0b1010010, // 7
    0b1111111, // 8
    0b1111011, // 9
};

class ConsoleScreen {
    private:

    char screenBuffer[HEIGHT*WIDTH] = {' '};

    public:

    ConsoleScreen() {
        SetFontSize(8, 8);
    }

    void SetFontSize(int x, int y) {
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        cfi.nFont = 0;
        cfi.dwFontSize.X = x;
        cfi.dwFontSize.Y = y;
        cfi.FontFamily = FF_DONTCARE;
        cfi.FontWeight = FW_NORMAL;
        wcscpy(cfi.FaceName, L"Consolas");

        SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    }

    void ShowBuffer() {
        std::string OutputBuffer = "";

        OutputBuffer += (char)218;
        for (int i = 0; i < WIDTH; i++) {
            OutputBuffer += (char)196;
        }
        OutputBuffer += (char)191;
        OutputBuffer += '\n';
        
        for (int i = 0; i < HEIGHT; i++) {
            OutputBuffer += (char)179;
            for (int j = 0; j < WIDTH; j++) {
                OutputBuffer += screenBuffer[i*WIDTH + j];
                screenBuffer[i*WIDTH + j] = ' ';
            }
            OutputBuffer += (char)179;
            OutputBuffer += '\n';
        }

        OutputBuffer += (char)192;
        for (int i = 0; i < WIDTH; i++) {
            OutputBuffer += (char)196;
        }
        OutputBuffer += (char)217;
        
        system("cls");
        std::cout << OutputBuffer;
    }
/*
    up / upl / upr / mid / lowl / lowr / low

    0b1110111, // 0
    0b0010010, // 1
    0b1011101, // 2
    0b1011011, // 3
    0b0111010, // 4
    0b1101011, // 5
    0b1111101, // 6
    0b1010010, // 7
    0b1111111, // 8
    0b1111011, // 9
*/

    void DrawNumber3x5(vec2 p, int n, char c) {
        char seg7 = segs[n];
        if ((seg7 & (1ULL << 6)) != 0) { // up  
            DrawLine(
                {p.x + 0, p.y + 0}, 
                {p.x + 2, p.y + 0}, c);
        }
        if ((seg7 & (1ULL << 5)) != 0) { // upl 
            DrawLine(
                {p.x + 0, p.y + 0},
                {p.x + 0, p.y + 2}, c);
        }
        if ((seg7 & (1ULL << 4)) != 0) { // upr 
            DrawLine(
                {p.x + 2, p.y + 0}, 
                {p.x + 2, p.y + 2}, c);
        }
        if ((seg7 & (1ULL << 3)) != 0) { // mid 
            DrawLine(
                {p.x + 0, p.y + 2}, 
                {p.x + 2, p.y + 2}, c);
        }
        if ((seg7 & (1ULL << 2)) != 0) { // lowl
            DrawLine(
                {p.x + 0, p.y + 2}, 
                {p.x + 0, p.y + 4}, c);
        }
        if ((seg7 & (1ULL << 1)) != 0) { // lowr
            DrawLine(
                {p.x + 2, p.y + 2}, 
                {p.x + 2, p.y + 4}, c);
        }
        if ((seg7 & (1ULL << 0)) != 0) { // low 
            DrawLine(
                {p.x + 0, p.y + 4}, 
                {p.x + 2, p.y + 4}, c);
        }
    }

    void DrawRect(vec2 p, vec2 size, char c) {
        for (int i = 0; i < size.y; i++) {
            for (int j = 0; j < size.x; j++) {
                screenBuffer[(p.y + i)*WIDTH + (p.x + j)] = c;
            }
        }
    }

    void DrawLine(vec2 p1, vec2 p2, char c) {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;

        if (abs(dx) > abs(dy)) {
            float m = dy/dx;
            
            for (int i = p1.x; i != p2.x; p1.x < p2.x ? i++ : i--) {
                int y = p1.y + (i - p1.x)*m;
                screenBuffer[y*WIDTH + i] = c;
            }

            int y = p1.y + (p2.x - p1.x)*m;
            screenBuffer[y*WIDTH + p2.x] = c;

        } else {
            float n = dx/dy;
            
            for (int i = p1.y; i != p2.y; p1.y < p2.y ? i++ : i--) {
                int x = p1.x + (i - p1.y)*n;
                screenBuffer[i*WIDTH + x] = c;
            }

            int x = p1.x + (p2.y - p1.y)*n;
            screenBuffer[p2.y*WIDTH + x] = c;
        }
    }
};

#define PONG_HEIGHT 12
#define PONG_WIDTH 2
#define PONG_SPEED 2.5f
#define PI 3.141592653

#define RAD_TO_DEG (180.0 / PI)
#define DEG_TO_RAD (PI / 180.0)

struct Rect {
    float x, y, w, h;
};

struct Pong {
    Rect rect;
    int point;
};

struct Ball {
    Rect rect;
    float d, v;
    bool alive;
};

float Clamp(float n, float lower, float upper){
    if (n < lower) return lower;
    if (n > upper) return upper;
    return n;
}

bool RectInter(Rect a, Rect b) {
    return (a.x < b.x + b.w &&
            a.x + a.w > b.x &&
            a.y < b.y + b.h &&
            a.y + a.h > b.y );
}

int GetPongTarget(Ball ball, int limit) {
    float vx = ball.v * cos(ball.d);
    float vy = ball.v * sin(ball.d);

    float y_movement = ((limit - ball.rect.x) / vx) * vy;
    if (y_movement == 0) {
        return ball.rect.y;
    } 
    else if (y_movement > 0) {  // bajando
        y_movement -= HEIGHT - ball.rect.y;
        int rest = abs(fmod(y_movement, HEIGHT));
        int n_bounce = abs(y_movement / HEIGHT);

        if (n_bounce % 2 == 0) {  // arriba
            return HEIGHT - rest;
        } else {  // abajo
            return rest;
        }
    } 
    else if (y_movement < 0) {  // subiendo
        y_movement += ball.rect.y;
        int rest = abs(fmod(y_movement, HEIGHT));
        int n_bounce = abs(y_movement / HEIGHT);

        if (n_bounce % 2 == 0) {  // arriba
            return rest;
        } else {  // abajo
            return HEIGHT - rest;
        }
    }

    return 0;
}


int main() {
    ConsoleScreen Screen;

    Pong Player = {{4, (HEIGHT - PONG_HEIGHT) / 2, PONG_WIDTH, PONG_HEIGHT}, 0};
    Pong Bot = {{WIDTH - 4 - PONG_WIDTH, (HEIGHT - PONG_HEIGHT) / 2, PONG_WIDTH, PONG_HEIGHT}, 0};

    Ball Ball = {{(WIDTH-1)/2, (HEIGHT-1)/2, 3, 3}, 0.0f, 3.0f, false};

    srand(time(NULL));

    while (Player.point < 10 && Bot.point < 10) {
        if (!Ball.alive) {
            Ball = {{(WIDTH-1)/2, (HEIGHT-1)/2, 3, 3}, 0.0f, 3.0f, true};

            int dir = rand() % 4;
            Ball.d = (dir*90 + 45) * DEG_TO_RAD;

            // if (rand() % 2 == 0) {
            //     Ball.d = (rand() % 1200 - 600)*DEG_TO_RAD;
            // } else {
            //     Ball.d = (rand() % 1200 + 1200)*DEG_TO_RAD;
            // }
        } else {
            Ball.rect.x += Ball.v * cos(Ball.d);
            Ball.rect.y += Ball.v * sin(Ball.d);

            // rebote contra los pongs
            if (RectInter(Ball.rect, Player.rect)) {
                Ball.d = fmod(PI - Ball.d, 2*PI);
                Ball.rect.x = Player.rect.x + Player.rect.w;
                Ball.v *= 1.05f;
            }
            if (RectInter(Ball.rect, Bot.rect)) {
                Ball.d = fmod(PI - Ball.d, 2*PI);
                Ball.rect.x = Bot.rect.x - Ball.rect.w;
                Ball.v *= 1.05f;
            }

            // rebote arriba y abajo
            if ((Ball.rect.y + Ball.rect.h) >= HEIGHT) {
                Ball.rect.y = HEIGHT - Ball.rect.h - 1;
                Ball.d = fmod(abs(Ball.d - 2.0f*PI), 2*PI);
            }
            if (Ball.rect.y < 0) {
                Ball.rect.y = 0;
                Ball.d = fmod(abs(Ball.d - 2.0f*PI), 2*PI);
            }

            // pasa el limite en x, suma puntos
            if ((Ball.rect.x + Ball.rect.w) < 0) {
                Ball.alive = false;
                Bot.point++;
            }
            if (Ball.rect.x >= WIDTH) {
                Ball.alive = false;
                Player.point++;
            }

            if (Player.point >= 10 || Bot.point >= 10) {
                break;
            }

            int target = cos(Ball.d) > 0 ? GetPongTarget(Ball, Bot.rect.x) : HEIGHT/2;

            if (Bot.rect.y + Bot.rect.h/2 < target - 1.5) {
                Bot.rect.y += PONG_SPEED;
                Bot.rect.y = Clamp(Bot.rect.y, 0, HEIGHT - PONG_HEIGHT - 1);
            }
            if (Bot.rect.y + Bot.rect.h/2 > target + 1.5) {
                Bot.rect.y -= PONG_SPEED;
                Bot.rect.y = Clamp(Bot.rect.y, 0, HEIGHT - PONG_HEIGHT - 1);
            }
        }

        // tomar input de las flechas
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            Player.rect.y -= PONG_SPEED;
            Player.rect.y = Clamp(Player.rect.y, 0, HEIGHT - PONG_HEIGHT - 1);
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            Player.rect.y += PONG_SPEED; 
            Player.rect.y = Clamp(Player.rect.y, 0, HEIGHT - PONG_HEIGHT - 1);
        }
        
        // draw middle line
        Screen.DrawLine({WIDTH/2, 0}, {WIDTH/2, HEIGHT-1}, 254);

        // draw points
        Screen.DrawNumber3x5({WIDTH*1/4 - 1, HEIGHT*1/4}, Player.point, 219);
        Screen.DrawNumber3x5({WIDTH*3/4 - 1, HEIGHT*1/4},    Bot.point, 219);

        //draw pongs
        Screen.DrawRect({(int)Player.rect.x, (int)Player.rect.y}, 
                        {(int)Player.rect.w, (int)Player.rect.h}, 219);

        Screen.DrawRect({(int)   Bot.rect.x, (int)   Bot.rect.y}, 
                        {(int)   Bot.rect.w, (int)   Bot.rect.h}, 219);
                        
        // draw Ball
        if (Ball.alive && Ball.rect.x >= 0 && Ball.rect.x + Ball.rect.w < WIDTH) {
            Screen.DrawRect({(int)Ball.rect.x, (int)Ball.rect.y}, {(int)Ball.rect.w, (int)Ball.rect.h}, 219);
        }

        Screen.ShowBuffer();
        Sleep(10);
    }

    system("cls");
    Screen.SetFontSize(50,80);

    if (Player.point >= 10) {
        std::cout << "YOU WIN" << std::endl;
    }

    if (Bot.point >= 10) {
        std::cout << "YOU LOSE" << std::endl;
    }

    getchar();
    return 0;
}