#include "raylib.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAZE_WIDTH 19
#define MAZE_HEIGHT 21
#define CELL_SIZE 30
#define SCREEN_WIDTH (MAZE_WIDTH * CELL_SIZE)
#define SCREEN_HEIGHT (MAZE_HEIGHT * CELL_SIZE + 100)
#define MAX_GHOSTS 4

typedef enum {
    WALL = 0,
    DOT = 1,
    EMPTY = 2,
    POWER_DOT = 3,
    PACMAN = 4,
    GHOST = 5
} CellType;

typedef struct {
    float x, y;
    int direction; // 0=up, 1=right, 2=down, 3=left
    Color color;
    bool frightened;
    float frightenedTimer;
} Ghost;

typedef struct {
    float x, y;
    int direction;
    int nextDirection;
    bool alive;
} Player;

// Labirent haritası
int maze[MAZE_HEIGHT][MAZE_WIDTH] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0},
    {0,3,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,3,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,1,0},
    {0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0},
    {0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0},
    {2,2,2,0,1,0,1,1,1,1,1,1,1,0,1,0,2,2,2},
    {0,0,0,0,1,0,1,0,0,2,0,0,1,0,1,0,0,0,0},
    {1,1,1,1,1,1,1,0,2,2,2,0,1,1,1,1,1,1,1},
    {0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,0,0},
    {2,2,2,0,1,0,1,1,1,1,1,1,1,0,1,0,2,2,2},
    {0,0,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,0},
    {0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,1,0},
    {0,1,0,0,1,0,1,0,0,0,0,0,1,0,1,0,0,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,3,0,0,1,0,0,0,1,0,1,0,0,0,1,0,0,3,0},
    {0,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

Player player;
Ghost ghosts[MAX_GHOSTS];
int score = 0;
int dotsRemaining = 0;
bool gameWon = false;
bool gameOver = false;
float powerUpTimer = 0;

void InitGame(void) {
    // Pacman başlangıç pozisyonu
    player.x = 9.0f;
    player.y = 15.0f;
    player.direction = 1; // sağa doğru
    player.nextDirection = 1;
    player.alive = true;

    // Hayaletleri başlat
    ghosts[0].x = 9.0f;
    ghosts[0].y = 9.0f;
    ghosts[0].direction = 0;
    ghosts[0].color = RED;
    ghosts[0].frightened = false;
    ghosts[0].frightenedTimer = 0;

    ghosts[1].x = 9.0f;
    ghosts[1].y = 10.0f;
    ghosts[1].direction = 1;
    ghosts[1].color = PINK;
    ghosts[1].frightened = false;
    ghosts[1].frightenedTimer = 0;

    ghosts[2].x = 8.0f;
    ghosts[2].y = 10.0f;
    ghosts[2].direction = 2;
    ghosts[2].color = ORANGE;
    ghosts[2].frightened = false;
    ghosts[2].frightenedTimer = 0;

    ghosts[3].x = 10.0f;
    ghosts[3].y = 10.0f;
    ghosts[3].direction = 3;
    ghosts[3].color = SKYBLUE;
    ghosts[3].frightened = false;
    ghosts[3].frightenedTimer = 0;

    // Noktaları say
    dotsRemaining = 0;
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (maze[y][x] == DOT || maze[y][x] == POWER_DOT) {
                dotsRemaining++;
            }
        }
    }

    score = 0;
    gameWon = false;
    gameOver = false;
    powerUpTimer = 0;
}

bool IsValidMove(float x, float y) {
    int gridX = (int)(x + 0.5f);
    int gridY = (int)(y + 0.5f);

    if (gridX < 0 || gridX >= MAZE_WIDTH || gridY < 0 || gridY >= MAZE_HEIGHT) {
        return false;
    }

    return maze[gridY][gridX] != WALL;
}

void UpdatePlayer(void) {
    if (!player.alive) return;

    // Yön değişikliği kontrolü
    float nextX = player.x;
    float nextY = player.y;

    switch (player.nextDirection) {
    case 0: nextY -= 0.1f; break; // yukarı
    case 1: nextX += 0.1f; break; // sağ
    case 2: nextY += 0.1f; break; // aşağı
    case 3: nextX -= 0.1f; break; // sol
    }

    if (IsValidMove(nextX, nextY)) {
        player.direction = player.nextDirection;
    }

    // Hareket
    switch (player.direction) {
    case 0: player.y -= 0.1f; break;
    case 1: player.x += 0.1f; break;
    case 2: player.y += 0.1f; break;
    case 3: player.x -= 0.1f; break;
    }

    // Sınırları kontrol et
    if (!IsValidMove(player.x, player.y)) {
        switch (player.direction) {
        case 0: player.y += 0.1f; break;
        case 1: player.x -= 0.1f; break;
        case 2: player.y -= 0.1f; break;
        case 3: player.x += 0.1f; break;
        }
    }

    // Tunnel geçişi (yanlarda)
    if (player.x < 0) player.x = MAZE_WIDTH - 1;
    if (player.x >= MAZE_WIDTH) player.x = 0;

    // Nokta toplama
    int gridX = (int)(player.x + 0.5f);
    int gridY = (int)(player.y + 0.5f);

    if (gridX >= 0 && gridX < MAZE_WIDTH && gridY >= 0 && gridY < MAZE_HEIGHT) {
        if (maze[gridY][gridX] == DOT) {
            maze[gridY][gridX] = EMPTY;
            score += 10;
            dotsRemaining--;

            if (dotsRemaining == 0) {
                gameWon = true;
            }
        }
        else if (maze[gridY][gridX] == POWER_DOT) {
            maze[gridY][gridX] = EMPTY;
            score += 50;
            dotsRemaining--;
            powerUpTimer = 10.0f; // 10 saniye güçlü mod

            // Tüm hayaletleri korkut
            for (int i = 0; i < MAX_GHOSTS; i++) {
                ghosts[i].frightened = true;
                ghosts[i].frightenedTimer = 10.0f;
            }

            if (dotsRemaining == 0) {
                gameWon = true;
            }
        }
    }
}

void UpdateGhosts(void) {
    for (int i = 0; i < MAX_GHOSTS; i++) {
        Ghost* ghost = &ghosts[i];

        // Korkmuş durumu güncelle
        if (ghost->frightened) {
            ghost->frightenedTimer -= GetFrameTime();
            if (ghost->frightenedTimer <= 0) {
                ghost->frightened = false;
            }
        }

        // Basit AI - rastgele yön değiştirme
        if (GetRandomValue(0, 100) < 5) {
            ghost->direction = GetRandomValue(0, 3);
        }

        // Hareket
        float speed = ghost->frightened ? 0.05f : 0.08f;
        float nextX = ghost->x;
        float nextY = ghost->y;

        switch (ghost->direction) {
        case 0: nextY -= speed; break;
        case 1: nextX += speed; break;
        case 2: nextY += speed; break;
        case 3: nextX -= speed; break;
        }

        if (IsValidMove(nextX, nextY)) {
            ghost->x = nextX;
            ghost->y = nextY;
        }
        else {
            // Duvarla karşılaştığında yön değiştir
            ghost->direction = GetRandomValue(0, 3);
        }

        // Tunnel geçişi
        if (ghost->x < 0) ghost->x = MAZE_WIDTH - 1;
        if (ghost->x >= MAZE_WIDTH) ghost->x = 0;

        // Pacman ile çarpışma kontrolü
        if (player.alive &&
            fabs(ghost->x - player.x) < 0.5f &&
            fabs(ghost->y - player.y) < 0.5f) {

            if (ghost->frightened) {
                // Hayalet yenildi
                score += 200;
                ghost->x = 9.0f;
                ghost->y = 9.0f;
                ghost->frightened = false;
            }
            else {
                // Pacman öldü
                player.alive = false;
                gameOver = true;
            }
        }
    }
}

void DrawMaze(void) {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            Rectangle rect = { x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE };

            switch (maze[y][x]) {
            case WALL:
                DrawRectangleRec(rect, BLUE);
                break;
            case DOT:
                DrawCircle(x * CELL_SIZE + CELL_SIZE / 2,
                    y * CELL_SIZE + CELL_SIZE / 2, 2, YELLOW);
                break;
            case POWER_DOT:
                DrawCircle(x * CELL_SIZE + CELL_SIZE / 2,
                    y * CELL_SIZE + CELL_SIZE / 2, 8, YELLOW);
                break;
            }
        }
    }
}

void DrawPlayer(void) {
    if (!player.alive) return;

    Color playerColor = powerUpTimer > 0 ? GOLD : YELLOW;
    DrawCircle((int)(player.x * CELL_SIZE + CELL_SIZE / 2),
        (int)(player.y * CELL_SIZE + CELL_SIZE / 2),
        CELL_SIZE / 2 - 2, playerColor);

    // Ağız
    Vector2 center = { player.x * CELL_SIZE + CELL_SIZE / 2, player.y * CELL_SIZE + CELL_SIZE / 2 };
    float angle = player.direction * 90.0f;

    DrawCircleSector(center, CELL_SIZE / 2 - 2, angle - 30, angle + 30, 10, BLACK);
}

void DrawGhosts(void) {
    for (int i = 0; i < MAX_GHOSTS; i++) {
        Ghost* ghost = &ghosts[i];
        Color color = ghost->frightened ? DARKBLUE : ghost->color;

        DrawCircle((int)(ghost->x * CELL_SIZE + CELL_SIZE / 2),
            (int)(ghost->y * CELL_SIZE + CELL_SIZE / 2),
            CELL_SIZE / 2 - 2, color);

        // Gözler
        DrawCircle((int)(ghost->x * CELL_SIZE + CELL_SIZE / 2 - 6),
            (int)(ghost->y * CELL_SIZE + CELL_SIZE / 2 - 4),
            3, WHITE);
        DrawCircle((int)(ghost->x * CELL_SIZE + CELL_SIZE / 2 + 6),
            (int)(ghost->y * CELL_SIZE + CELL_SIZE / 2 - 4),
            3, WHITE);
        DrawCircle((int)(ghost->x * CELL_SIZE + CELL_SIZE / 2 - 6),
            (int)(ghost->y * CELL_SIZE + CELL_SIZE / 2 - 4),
            1, BLACK);
        DrawCircle((int)(ghost->x * CELL_SIZE + CELL_SIZE / 2 + 6),
            (int)(ghost->y * CELL_SIZE + CELL_SIZE / 2 - 4),
            1, BLACK);
    }
}

void DrawUI(void) {
    DrawText(TextFormat("Score: %d", score), 10, MAZE_HEIGHT * CELL_SIZE + 10, 20, WHITE);
    DrawText(TextFormat("Dots: %d", dotsRemaining), 10, MAZE_HEIGHT * CELL_SIZE + 40, 20, WHITE);

    if (powerUpTimer > 0) {
        DrawText(TextFormat("Power: %.1f", powerUpTimer), 10, MAZE_HEIGHT * CELL_SIZE + 70, 20, GOLD);
        powerUpTimer -= GetFrameTime();
        if (powerUpTimer < 0) powerUpTimer = 0;
    }

    if (gameWon) {
        DrawText("YOU WIN! Press R to restart", SCREEN_WIDTH / 2 - 140, SCREEN_HEIGHT / 2, 20, GREEN);
    }
    else if (gameOver) {
        DrawText("GAME OVER! Press R to restart", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 20, RED);
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pacman - Raylib");
    SetTargetFPS(60);

    InitGame();

    while (!WindowShouldClose()) {
        // Güncelleme
        if (!gameOver && !gameWon) {
            // Kontroller
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) player.nextDirection = 0;
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) player.nextDirection = 1;
            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) player.nextDirection = 2;
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) player.nextDirection = 3;

            UpdatePlayer();
            UpdateGhosts();
        }

        // Yeniden başlatma
        if (IsKeyPressed(KEY_R) && (gameOver || gameWon)) {
            InitGame();
        }

        // Çizim
        BeginDrawing();
        ClearBackground(BLACK);

        DrawMaze();
        DrawPlayer();
        DrawGhosts();
        DrawUI();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

//Lütfen compound literal ifadeleri ((Vector2){0,0} gibi) kullanma. 
