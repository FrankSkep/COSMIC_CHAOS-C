#include "src/game.h"
#include "src/resources.h"

int main()
{
    /*********** CONSTANTES ************/
    const int playRadius = 45;        // Tamaño del jugador
    const float playerSpeed = 15.0f;  // Velocidad del jugador
    const float spawnInterval = 0.3f; // Intervalo de tiempo entre la aparición de meteoros

    /************ VARIABLES *************/

    // ---- ESTADOS DEL JUEGO----
    bool isPlaying = false; // Determina si esta en juego
    bool gameOver = false;  // Controla gameover
    // --------------------------------------------------------
    float elapsedTime = 0.0f;
    float rotation = 0.0f; // Rotacion de meteoros
    int score = 0;         // Inicio del puntaje
    int lives = 5;         // Vidas Iniciales
    int level = 1;         // Nivel inicial
    int seconds = 1.00;    // espera entre niveles
    int totalseconds = 0;  // -------
    int minutesT = 0;      // CLOCK
    int secondsT = 0;      // -------
    double timeseconds = 0;

    InitWindow(SCR_WIDTH, SCR_HEIGHT, "BETA 0.12");
    SetTargetFPS(75);

    /************** Carga de texturas **************/
    Texture2D menu, game, gameoverT;
    Texture2D shipTextures[6], coinsTx[6], heartsTx[6];
    loadTextures(&menu, &game, &gameoverT, shipTextures, coinsTx, heartsTx);

    /************** Carga de sonidos **************/
    InitAudioDevice();
    Music gameMusic, gameover;
    Sound soundcoin;
    loadSounds(&gameMusic, &gameover, &soundcoin);

    /***** Ajustes texturas cambiantes *****/
    int currentFrame = 0; // indice de la textura actual
    float frameTimeCounter = 0.0f;
    float frameSpeed = 1.0f / 8.0f; // velocidad de cambio de imagen (cada 1/4 de segundo)

    // Posicion jugador
    Vector2 playPosition = {(float)SCR_WIDTH / 2, (float)SCR_HEIGHT / 1.1f};
    // Posicion nave
    Vector2 shipCenter;

    /*************** BUCLE DEL JUEGO ***************/
    while (!WindowShouldClose())
    {
        if (!isPlaying) // Menu principal
        {
            StopMusicStream(gameover); // Detiene musica gameover
            drawMainMenu(&menu);       // Dibuja menu principal

            logicaMenu(&secondsT, &isPlaying);
        }
        else
        {
            /*-------------------- PARTIDA --------------------*/
            if (!gameOver)
            {
                StopMusicStream(gameover); // Detiene musica de gameover
                UpdateMusicStream(gameMusic);
                PlayMusicStream(gameMusic); // Reproduce musica de la partida

                // Calcula y actualiza la posición del centro de la nave
                shipCenter = {playPosition.x - shipTextures[currentFrame].width / 2, playPosition.y - shipTextures[currentFrame].height / 2};

                /***** SPRITES *****/
                frameTimeCounter += GetFrameTime();
                // pasado el tiempo, cambia la imagen de la nave
                if (frameTimeCounter >= frameSpeed)
                {
                    currentFrame = (currentFrame + 1) % 6; // Cambiar al siguiente marco (0, 1, 2, 0, 1, 2, ...)
                    frameTimeCounter = 0.0f;               // Reiniciar el contador de tiempo
                }

                elapsedTime += GetFrameTime(); // Actualizar temporizador

                //------- Controlar movimiento del jugador -------
                if (IsKeyDown(KEY_RIGHT) && playPosition.x + playRadius < SCR_WIDTH)
                {
                    playPosition.x += playerSpeed;
                }
                if (IsKeyDown(KEY_D) && playPosition.x + playRadius < SCR_WIDTH)
                {
                    playPosition.x += playerSpeed;
                }
                if (IsKeyDown(KEY_LEFT) && playPosition.x - playRadius > 0)
                {
                    playPosition.x -= playerSpeed;
                }
                if (IsKeyDown(KEY_A) && playPosition.x - playRadius > 0)
                {
                    playPosition.x -= playerSpeed;
                }
                if (IsKeyDown(KEY_UP) && playPosition.y - playRadius > 0) // Ajuste para la parte superior
                {
                    playPosition.y -= playerSpeed;
                }
                if (IsKeyDown(KEY_W) && playPosition.y - playRadius > 0) // Ajuste para la parte superior
                {
                    playPosition.y -= playerSpeed;
                }
                if (IsKeyDown(KEY_DOWN) && playPosition.y + playRadius < SCR_HEIGHT) // Ajuste para la parte inferior
                {
                    playPosition.y += playerSpeed;
                }
                if (IsKeyDown(KEY_S) && playPosition.y + playRadius < SCR_HEIGHT) // Ajuste para la parte inferior
                {
                    playPosition.y += playerSpeed;
                }

                // ---------- GENERACION OBJETOS ----------
                if (elapsedTime >= spawnInterval)
                {
                    for (int i = 0; i < MAX_GRAY_METEORS; i++)
                    {
                        if (!grayMeteors[i].active)
                        {
                            InitGrayMeteor(&grayMeteors[i]);
                            break;
                        }
                    }
                    for (int i = 0; i < MAX_BROWN_METEORS; i++)
                    {
                        if (!brownMeteors[i].active)
                        {
                            InitBrownMeteor(&brownMeteors[i]);
                            break;
                        }
                    }
                    for (int i = 0; i < MAX_COINS; i++)
                    {
                        if (!coins[i].active)
                        {
                            InitCoin(&coins[i]);
                            break;
                        }
                    }
                    for (int i = 0; i < MAX_HEARTS; i++)
                    {
                        if (!hearts[i].active)
                        {
                            InitHearts(&hearts[i]);
                            break;
                        }
                    }
                    elapsedTime = 0.0f; // Reiniciar el temporizador
                }

                /*------------------ FISICAS Y COLISIONES ------------------*/
                // Velocidad de rotacion meteoros
                rotation += 2.5f;

                // Meteoro gris
                for (int i = 0; i < MAX_GRAY_METEORS; i++)
                {
                    if (grayMeteors[i].active)
                    {
                        grayMeteors[i].position.y += GRAY_METEOR_SPEED;
                        if (grayMeteors[i].position.y > SCR_HEIGHT + GRAY_METEOR_RADIUS * 2)
                        {
                            grayMeteors[i].active = false; // Eliminar meteoro al salir de la pantalla
                        }

                        // Detectar colisión con jugador
                        if (CheckCollision(playPosition, playRadius, grayMeteors[i].position, GRAY_METEOR_RADIUS))
                        {
                            grayMeteors[i].active = false; // Elimina meteoro tocado
                            lives--;                       // Pierde una vida
                            if (lives <= 0)
                            {
                                gameOver = true;
                            }
                        }
                    }
                }
                // Meteoro cafe
                for (int i = 0; i < MAX_BROWN_METEORS; i++)
                {
                    if (brownMeteors[i].active)
                    {
                        brownMeteors[i].position.y += BROWN_METEOR_SPEED;
                        if (brownMeteors[i].position.y > SCR_HEIGHT + BROWN_METEOR_RADIUS * 2)
                        {
                            brownMeteors[i].active = false;
                        }

                        // Detectar colisión con jugador
                        if (CheckCollision(playPosition, playRadius, brownMeteors[i].position, BROWN_METEOR_RADIUS))
                        {
                            brownMeteors[i].active = false; // Elimina meteoro tocado
                            lives--;                        // Pierde una vida
                            if (lives <= 0)
                            {
                                gameOver = true;
                            }
                        }
                    }
                }

                /*------------------ OBJETOS ------------------*/
                // Moneda (Incrementador de puntos)
                for (int i = 0; i < MAX_COINS; i++)
                {
                    if (coins[i].active)
                    {
                        coins[i].position.y += COINS_SPEED;
                        if (coins[i].position.y > SCR_HEIGHT + COINS_RADIUS * 2)
                        {
                            coins[i].active = false;
                        }

                        // Detectar colisión con jugador y aumentar el contador de puntos
                        if (CheckCollision(playPosition, playRadius, coins[i].position, COINS_RADIUS))
                        {
                            coins[i].active = false;
                            score += 10; // Aumentar el puntaje
                            PlaySound(soundcoin);
                        }
                    }
                }
                // Corazon (Vida adicional)
                for (int i = 0; i < MAX_HEARTS; i++)
                {
                    if (hearts[i].active)
                    {
                        hearts[i].position.y += HEARTS_SPEED;
                        if (hearts[i].position.y > SCR_HEIGHT + HEARTS_RADIUS * 2)
                        {
                            hearts[i].active = false;
                        }

                        // Detectar colisión con jugador y aumentar vidas
                        if (CheckCollision(playPosition, playRadius, hearts[i].position, HEARTS_RADIUS))
                        {
                            hearts[i].active = false; // Eliminar la esfera tocada
                            lives++;                  // Gana una vida
                        }
                    }
                }
                /*------------- DIBUJA PARTIDA EN CURSO -------------*/
                BeginDrawing();
                gameInterface(&game, &shipTextures[currentFrame], &shipCenter, &coinsTx[currentFrame], &heartsTx[currentFrame], &lives, &score, &rotation);

                /*------------------- NIVELES -------------------*/
                timeseconds = GetTime(); // Obtener el tiempo transcurrido en segundos

                totalseconds = (int)timeseconds;
                minutesT = totalseconds / 60;
                secondsT = totalseconds % 60;

                // Dibujar el tiempo transcurrido en pantalla con formato de reloj (00:00)
                DrawText(TextFormat("%02d:%02d", minutesT, secondsT), 20, 20, 100, WHITE);

                EndDrawing();

                Levels(&score, &level, &elapsedTime, &playPosition, &seconds, &lives);
                /*------------------- ------ -------------------*/

                if (gameOver)
                {
                    minutesT = 0, secondsT = 0, totalseconds = 0, timeseconds = 0;

                    rotation = 0;              // Reiniciar rotacion
                    currentFrame = 0;          // Reiniciar currentFrame
                    resetItems(&playPosition); // Reinicia posicion y desactiva objetos

                    StopMusicStream(gameMusic); // Detener musica partida

                    PlayMusicStream(gameover); // Reproducir musica gameover
                }
            }
            /*-------------------- FIN DE PARTIDA --------------------*/

            /*------------------- GAMEOVER -------------------*/
            if (gameOver)
            {
                BeginDrawing();
                // Reproducir musica gameover
                UpdateMusicStream(gameover);

                // Dibuja interfaz
                gameOverInterface(&gameoverT, &score, &level);

                // Vuelve a jugar al presionar enter
                if (IsKeyDown(KEY_ENTER))
                {
                    resetStats(&lives, &score, &level, &timeseconds);
                    gameOver = false;
                }
                // Vuelve al menu al presionar Q
                if (IsKeyDown(KEY_Q))
                {
                    resetStats(&lives, &score, &level, &timeseconds);
                    isPlaying = false;
                    gameOver = false;
                }
                EndDrawing();
            }
            /*---------------------------------------------------*/

            DrawFPS(20, SCR_HEIGHT - 40);
        }
    }

    // Descarga de recursos
    unloadTextures(&menu, &game, &gameoverT, shipTextures, coinsTx, heartsTx);
    unloadSounds(&gameMusic, &gameover, &soundcoin);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}