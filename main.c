#include "raylib.h"
#include "src/elements.h"  // Struct, Enum y Constantes
#include "src/resources.h" // Carga texturas y sonidos
#include "src/game.h"      // Funciones logica del juego
#include "src/drawing.h"   // Funciones de dibujo

int main()
{
    srand(time(NULL));
    selecNpreguntas();
    /*------------- CONSTANTES -------------*/
    const int playRadius = 45;       // Tamaño del jugador
    const float playerSpeed = 15.0f; // Velocidad del jugador
    const float rotationSpeed = 2.0;
    const float maxRotation = 20.0f;                // Máxima rotación hacia la derecha
    const float minRotation = -20.0f;               // Máxima rotación hacia la izquierda
    const float rotationInterpolationSpeed = 50.0f; // Definir la velocidad de interpolación para volver a la posición original
    const float spawnInterval = 0.2f;               // Intervalo de tiempo entre la aparición de objetos
    const float spawnIntervalPU = 1.2f;

    // Variables del juego
    bool gameOver = false;
    short i, shieldActive = 0, object;
    float elapsedTime1 = 0.0f, elapsedTime2 = 0.0f, rotationMeteor = 0.0f;
    float playerRotation = 0.0f;
    float currentRotation = 0.0f;
    float targetRotation = 0.0f;

    // Variables del cronómetro
    int totalseconds = 0, minutesT = 0, secondsT = 0;
    float timeseconds = 0;
    int tuto = 0, tutob = 1, tutostate = 1;

    // Configuración de la ventana
    InitWindow(SCR_WIDTH, SCR_HEIGHT, "BETA 0.9.4");
    SetTargetFPS(75);

    // Carga de texturas y sonidos
    loadTextures();
    InitAudioDevice();
    loadSounds();

    // Jugador
    Tdata data = getDataPlayer();          // Entrada de datos
    GameStats stats = {5, 0, 0, 0, 10, 0}; // Estadísticas iniciales

    // Variables de sprites
    short currentFrame = 0;    // Índice de textura actual (0, 5)
    short currentFrameExp = 0; // Índice de textura actual (0, 2)
    float frameTimeCounter = 0.0f;
    float frameSpeed = 1.0f / 8.0f; // Velocidad de cambio de imagen

    // Posición centrada del jugador
    Vector2 playerPosition = {
        (float)SCR_WIDTH / 2 - shipTx[currentFrame].width / 2,
        (float)SCR_HEIGHT / 1.1f - shipTx[currentFrame].height / 2};

    // Centro de textura de meteoros
    Vector2 grayCenter, brownCenter;

    // Variables de estado
    bool saveProgress = false; // Guardar estadísticas del jugador
    bool showQuestion = false; // Mostrar pregunta
    bool continuar = false;    // Animación después de pregunta
    int contin = 0;
    bool muteMusic = false;

    // Estado inicial del juego
    GameState gameState = MAIN_MENU;
    int keyOption;

    /*------------------------ BUCLE DEL JUEGO ------------------------*/
    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }

        // Actualizar estado del juego
        updateGameState(&gameState, keyOption, &stats, &muteMusic);
        keyOption = -1; // Reiniciar valor de tecla

        // Actualizar musica en cada modo
        updateMusic(gameState, muteMusic);

        // ESTADOS DEL JUEGO
        switch (gameState)
        {
        case MAIN_MENU:
            drawMainMenu();
            keyOption = GetKeyPressed();
            break;

        case HOW_TO_PLAY:
            drawHowToPlay();
            keyOption = GetKeyPressed();
            break;

        case ABOUT_GAME:
            aboutTheGame();
            keyOption = GetKeyPressed();
            break;

        case HISTORY_SCORE:
            DrawScoresTable("record.dat", muteMusic);
            keyOption = GetKeyPressed();
            break;

        case IN_GAME:
            gameOver = false;

            /***** SPRITES *****/
            frameTimeCounter += GetFrameTime();
            // Pasado el tiempo, cambia imagen
            if (frameTimeCounter >= frameSpeed)
            {
                currentFrame = (currentFrame + 1) % 6;       // Cambiar entre 0, 1, 2, 3, 4, 5
                currentFrameExp = (currentFrameExp + 1) % 3; // Cambiar entre 0, 1, 2
                frameTimeCounter = 0.0f;                     // Reiniciar el contador de tiempo
            }

            /*------------------ CONTROLES ------------------*/
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) // Mover hacia la derecha
            {
                if (playerPosition.x + playRadius < SCR_WIDTH)
                {
                    playerPosition.x += playerSpeed;
                    // Rotacion hacia la derecha
                    if (currentRotation < maxRotation)
                    {
                        currentRotation += rotationSpeed;
                    }
                }
            }
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) // Mover hacia la izquierda
            {
                if (playerPosition.x - playRadius > 0)
                {
                    playerPosition.x -= playerSpeed;

                    // Rotacion hacia la izquierda
                    if (currentRotation > minRotation)
                    {
                        currentRotation -= rotationSpeed;
                    }
                }
            }

            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) // Mover hacia arriba
            {
                if (playerPosition.y - playRadius > 0)
                {
                    playerPosition.y -= playerSpeed;
                }
            }
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) // Mover hacia abajo
            {
                if (playerPosition.y + playRadius < SCR_HEIGHT)
                {
                    playerPosition.y += playerSpeed;
                }
            }

            // Interpolar la rotación de vuelta a la posición original cuando se suelta la tecla
            if (!IsKeyDown(KEY_RIGHT) && !IsKeyDown(KEY_D) && !IsKeyDown(KEY_LEFT) && !IsKeyDown(KEY_A))
            {
                // Si la rotación actual no es igual a la rotación objetivo, interpola hacia la rotación objetivo
                if (currentRotation != targetRotation)
                {
                    if (currentRotation < targetRotation)
                    {
                        currentRotation += rotationInterpolationSpeed * GetFrameTime();
                        if (currentRotation > targetRotation)
                        {
                            currentRotation = targetRotation;
                        }
                    }
                    else
                    {
                        if (currentRotation > targetRotation)
                        {
                            currentRotation -= rotationInterpolationSpeed * GetFrameTime();
                            if (currentRotation < targetRotation)
                            {
                                currentRotation = targetRotation;
                            }
                        }
                    }
                }
            }
            // Actualizar la rotación del jugador
            playerRotation = currentRotation;

            if (IsKeyPressed(KEY_SPACE)) // Disparar misil
            {
                for (i = 0; i < MAX_SHOTS; i++)
                {
                    if (!shots[i].active)
                    {
                        if (stats.totalMunicion > 0)
                        {
                            shots[i].active = true;
                            shots[i].position = playerPosition; // Posición inicial del disparo
                            PlaySound(shotSound);
                            stats.totalMunicion--;
                            break;
                        }
                    }
                }
            }

            /*--------------------- GENERACION OBJETOS ---------------------*/
            elapsedTime1 += GetFrameTime();
            elapsedTime2 += GetFrameTime();
            if (elapsedTime1 >= spawnInterval)
            {
                generateObjects(grayMeteors, MAX_GRAY, GRAY_METEOR_RADIUS);

                generateObjects(brownMeteors, MAX_BROWN, BROWN_METEOR_RADIUS);

                generateObjects(coinGold, MAX_COINS, COINS_RADIUS);

                generateObjects(hearts, MAX_HEART, HEARTS_RADIUS);

                elapsedTime1 = 0.0f; // Reiniciar el temporizador
            }
            if (elapsedTime2 >= spawnIntervalPU)
            {
                generateObjects(shieldB, MAX_OBJECT, COINS_RADIUS);

                generateObjects(municiones, MAX_OBJECT, COINS_RADIUS);
                elapsedTime2 = 0.0f; // Reiniciar el temporizador
            }

            /*--------------------- FISICAS Y COLISIONES ---------------------*/
            if (objectColision(grayMeteors, MAX_GRAY, GRAY_METEOR_SPEED, GRAY_METEOR_RADIUS, &playerPosition, playRadius, &grayMeteor, true))
            {
                if (shieldActive <= 0)
                {
                    stats.lives--; // Pierde una vida
                    if (stats.lives <= 0)
                    {
                        gameOver = true;
                    }
                }
                else
                {
                    shieldActive--;
                }
            }
            if (objectColision(brownMeteors, MAX_BROWN, BROWN_METEOR_SPEED, BROWN_METEOR_RADIUS, &playerPosition, playRadius, &brownMeteor, true))
            {
                if (shieldActive <= 0)
                {
                    stats.lives--; // Pierde una vida
                    if (stats.lives <= 0)
                    {
                        gameOver = true;
                    }
                }
                else
                {
                    shieldActive--;
                }
            }
            /*----- Moneda (Incrementador de puntos) -----*/
            if (objectColision(coinGold, MAX_COINS, COINS_SPEED, COINS_RADIUS, &playerPosition, playRadius, &coinsTx[currentFrame], false))
            {
                stats.score += 10; // Aumentar el puntaje
                PlaySound(soundcoin);
            }
            // Caja de municion
            if (objectColision(shieldB, MAX_OBJECT, SHIELD_SPEED, COINS_RADIUS, &playerPosition, playRadius, &shield, false))
            {
                object = 1;
                showQuestion = true;
                PlaySound(soundcoin);
            }
            if (objectColision(municiones, MAX_OBJECT, AMMO_SPEED, COINS_RADIUS, &playerPosition, playRadius, &ammoTx, false))
            {
                object = 2;
                showQuestion = true;
                PlaySound(soundcoin);
            }
            /*----- Corazon (Vida adicional) -----*/
            if (objectColision(hearts, MAX_HEART, HEARTS_SPEED, HEARTS_RADIUS, &playerPosition, playRadius, &heartsTx[currentFrameExp], false))
            {
                if (stats.lives < 5)
                {
                    stats.lives++; // Gana una vida
                }
            }

            /*----- Disparos -----*/
            for (i = 0; i < MAX_SHOTS; i++)
            {
                if (shots[i].active)
                {
                    if (shots[i].collided)
                    {
                        // Disminuir el temporizador de la explosión
                        shots[i].explosionTimer -= GetFrameTime();

                        if (shots[i].explosionTimer <= 0)
                        {
                            // Desactivar el disparo después de la animación de explosión
                            shots[i].active = false;
                            shots[i].collided = false;
                        }
                    }
                    else
                    {
                        // Mover el disparo hacia arriba
                        shots[i].position.y -= SHOT_SPEED * GetFrameTime();

                        // Comprobar si el disparo está fuera de la pantalla y desactivarlo
                        if (shots[i].position.y < 0)
                        {
                            shots[i].active = false;
                        }

                        for (int j = 0; j < MAX_GRAY; j++)
                        {
                            if (grayMeteors[j].active)
                            {
                                // Calcula punto de collision
                                grayCenter.x = grayMeteors[j].position.x - grayMeteor.width / 2;
                                grayCenter.y = grayMeteors[j].position.y - grayMeteor.height / 2;
                                //  Colision con meteoro gris
                                if (CheckCollision(&shots[i].position, SHOT_RADIUS, &grayCenter, GRAY_METEOR_RADIUS))
                                {
                                    PlaySound(burstMisil);
                                    StopSound(shotSound);
                                    stats.score += 5;
                                    grayMeteors[j].active = false;
                                    shots[i].collided = true;
                                    shots[i].explosionTimer = 0.4f; // Duracion animacion de explosión (0.4 segundos)
                                }
                            }
                        }
                        for (int j = 0; j < MAX_BROWN; j++)
                        {
                            if (brownMeteors[j].active)
                            {
                                // Calcula punto de collision
                                brownCenter.x = brownMeteors[j].position.x - brownMeteor.width / 2;
                                brownCenter.y = brownMeteors[j].position.y - brownMeteor.height / 2;
                                //  Colisión con meteoro café
                                if (CheckCollision(&shots[i].position, SHOT_RADIUS, &brownCenter, BROWN_METEOR_RADIUS))
                                {
                                    PlaySound(burstMisil);
                                    StopSound(shotSound);
                                    stats.score += 5;
                                    brownMeteors[j].active = false;
                                    shots[i].collided = true;
                                    shots[i].explosionTimer = 0.4f; // Duracion animacion de explosión (0.4 segundos)
                                }
                            }
                        }
                    }
                }
            }
            rotationMeteor += 2.5f; // Velocidad de rotacion meteoros
            pausa();                // Verifica si se pulso 'P', para pausar el juego

            /*---------------- DIBUJO PARTIDA EN CURSO ---------------*/
            BeginDrawing();

            // Dibujar interfaz de la partida
            drawGameInterface(hearthF[currentFrameExp], hearthE[currentFrame], shield, &stats, data.name, shieldActive, minutesT, secondsT); // Dibujar objetos de la partida

            // Dibuja jugador (nave)
            drawPlayer(shipTx[currentFrameExp], forceF[currentFrame], &playerPosition, &playerRotation, shieldActive);

            // Dibuja meteoros
            drawMeteor(grayMeteors, MAX_GRAY, grayMeteor, rotationMeteor);
            drawMeteor(brownMeteors, MAX_GRAY, brownMeteor, rotationMeteor);

            // Dibuja monedas, corazones y powerUps
            drawObject(coinsTx[currentFrame], coinGold, MAX_COINS);
            drawObject(heartsTx[currentFrameExp], hearts, MAX_HEART);
            drawObject(ballE[currentFrameExp], shieldB, MAX_OBJECT);
            drawObject(ammoTx, municiones, MAX_OBJECT);

            // Dibuja misiles
            drawShots(misil, &explosionTx[currentFrameExp]);

            // Animacion despues de responder pregunta
            if (continuar)
            {
                if (contin == 5)
                {
                    PlaySound(soundcoin);
                    PlaySound(soundcoin);
                    screenMessage("¡GO!", 0.5, BLANK);
                    contin = 0;
                    continuar = false;
                }
                if (contin == 4)
                {
                    PlaySound(soundcoin);
                    screenMessage("1", 0.7, BLANK);
                    contin = 5;
                }
                if (contin == 3)
                {
                    PlaySound(soundcoin);
                    screenMessage("2", 0.7, BLANK);
                    contin = 4;
                }
                if (contin == 2)
                {
                    if (tutostate)
                    {
                        textQuestion("ACABAS DE CHOCAR CON UNO DE LOS POWER UPS ", 40, SCR_HEIGHT - 300, 3, 1, &tutotx1);
                        textQuestion("imagen central de los power-ups ", 100, SCR_HEIGHT - 500, 3, 1, &tutotx1);
                        textQuestion("SI RESPONDES BIEN PODRAS USARLO ", 40, SCR_HEIGHT - 250, 3, 5, &tutotx1);
                        textQuestion("continuemos ", 60, SCR_HEIGHT - 500, 3, 3, &tutotx1);
                        tutostate = 0;
                    }
                    PlaySound(soundcoin);
                    screenMessage("3", 0.7, BLANK);
                    contin = 3;
                }
                if (contin == 1)
                {
                    contin = 2;
                }
            }

            if (showQuestion) // Si tomo moneda de pregunta
            {
                drawQuestion(&showQuestion, &stats.rachaAciertos, &shieldActive, &stats.totalMunicion, &stats.lives, object);
                continuar = true;
                contin = 1;
            }

            tuto++;
            if (tutob) // ESTO IRA EN FUNCIO TRANQUI CORNEJO
            {
                switch (tuto)
                {
                case 60:
                    textQuestion("hola gran viajero bienvenido a 'COSMIC-CAOS'", 40, SCR_HEIGHT - 300, 3, 0, &tutotx1);
                    esperarTecla();
                    break;
                case 70:
                    textQuestion("Utiliza las flacha de tu teclado para moverte en la direccion de desees Y ATRAPAR ESAS MONEDAS", 40, SCR_HEIGHT - 300, 3, 0, &tutotx1);
                    esperarTecla();
                    break;
                case 90:
                    textQuestion("PERO NO TAN RAPIDO ", 40, SCR_HEIGHT - 300, 3, 0, &tutotx1);
                    esperarTecla();
                    textQuestion("TIENES QUE SABER QUE ESOS METEORITOS PODRIAN CHOCAR CONTIGO, ESQUIVALOS!! ", 40, SCR_HEIGHT - 250, 3, 0, &tutotx1);
                    esperarTecla();
                    break;
                case 140:
                    textQuestion("MUY BIEN, PERO CUIDA TUS CORAZONES  ", 40, SCR_HEIGHT - 300, 3, 0, &tutotx1);
                    esperarTecla();
                    textQuestion("PUEDES TOMAR UNA DE LAS VIDAS QUE CAEN PARA PODER SOBREVIVIR MAS TIEMPO ", 40, SCR_HEIGHT - 250, 3, 0, &tutotx1);
                    esperarTecla();
                    break;
                case 280:
                    textQuestion("VAS BIEN YA TE ESTAS ADAPTANDO ", 40, SCR_HEIGHT - 300, 3, 0, &tutotx1);
                    esperarTecla();
                    textQuestion("VEAMOS QUE PASA SI CAMTURAS UNA DE ESAS ESFERAS AZULES ", 40, SCR_HEIGHT - 250, 3, 0, &tutotx1);
                    esperarTecla();
                    break;
                case 500:
                    textQuestion("ULTIMO (SIGUIRNTES) ", 40, SCR_HEIGHT - 300, 3, 0, &tutotx1);
                    esperarTecla();
                    break;
                case 501:
                    break;
                case 1010:
                    tutob = 0;
                    UnloadTexture(tutotx);
                    UnloadTexture(tutotx1);
                    break;
                default:
                    break;
                }
            }

            /*-------- TIEMPO TRANSCURRIDO --------*/
            timeseconds += GetFrameTime(); // Obtener el tiempo transcurrido en segundos
            totalseconds = timeseconds;
            minutesT = totalseconds / 60;
            secondsT = totalseconds % 60;
            Levels(&stats, &elapsedTime1, &playerPosition, &totalseconds);

            if (gameOver)
            {
                minutesT = 0, secondsT = 0, totalseconds = 0;
                rotationMeteor = 0;          // Reiniciar rotacion
                resetItems(&playerPosition); // Reinicia posicion y desactiva objetos

                saveProgress = true; // Al terminar almenos un juego, ya puede guardar progreso

                // Actualizar mejor puntaje
                if (stats.score > data.score)
                {
                    data.score = stats.score;
                }
                // Actualizar maximo nivel alcanzado
                if (stats.level > data.maxLevel)
                {
                    data.maxLevel = stats.level;
                }
                // Actualizar maxima racha de respuestas correctas
                if (stats.rachaAciertos > data.rachaAciertos)
                {
                    data.rachaAciertos = stats.rachaAciertos;
                }

                if (!muteMusic)
                {
                    StopMusicStream(gameMusic); // Detener musica partida
                    PlayMusicStream(gameover);  // Reproducir musica gameover
                }
                gameState = GAME_OVER;
            }
            EndDrawing();
            break;

        case GAME_OVER:
            if (!muteMusic)
            {
                UpdateMusicStream(gameover);
            }
            // Dibuja interfaz
            BeginDrawing();
            gameOverInterface(stats.score, stats.level);
            EndDrawing();

            keyOption = GetKeyPressed();
            break;
        }
    }

    // Guardar estadisticas en archivo record.dat
    if (saveProgress)
    {
        appendScoresToFile("record.dat", data);
    }
    // Descarga de recursos
    unloadTextures();
    unloadSounds();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
