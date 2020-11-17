// Zombie Arena.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include<sstream>
#include<fstream>
#include <SFML/Graphics.hpp>
#include<SFML/Audio.hpp>
#include "Player.h"
#include"ZombieArena.h"
#include"TextureHolder.h"
#include"Bullet.h"
#include"Pickup.h"
using namespace sf;

int main()
{
    TextureHolder holder;
    //The game will always be in one of four statea
    enum class State { PAUSED, LEVELING_UP, GAME_OVER, PLAYING };
    State state = State::GAME_OVER;
    //Get the screen resolution and create an SFML window
    Vector2f resolution;
    resolution.x = VideoMode::getDesktopMode().width;
    resolution.y = VideoMode::getDesktopMode().height;
    RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);
    //Create an SFML view for the main action
    View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));
    //Clock for timing everything
    Clock clock;
    //How long has the PLAYING state been active
    Time gameTimeTotal;
    //Where is the mouse in relation to world coordinates
    Vector2f mouseWorldPosition;
    //Where is the mouse in relation to screencoordinates
    Vector2i mouseScreenPosition;
    //Create an instance of Player
    Player player;
    //The boundaries of the arena
    IntRect arena;
    //Create the background
    VertexArray background;
    Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");
    //insted of: textureBackground.loadFromFile("graphics/background_sheet.png");
    //Prepare for a hord of zombies
    int numZombies;
    int numZombiesAlive;
    Zombie* zombies = nullptr;
    //Add bullets
    Bullet bullets[100];
    int currentBullet = 0;
    int bulletsSpare = 24;
    int bulletsInClip = 6;
    int clipSize = 6;
    float fireRate = 1;
    //When was the fire button last pressed
    Time lastPressed;
    //Hide mouse pointer and replace with crosshair
    window.setMouseCursorVisible(true);
    Sprite spriteCrosshair;
    Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");
    spriteCrosshair.setTexture(textureCrosshair);
    spriteCrosshair.setOrigin(25, 25);
    //Create a console of pickups
    Pickup healthPickup(1);
    Pickup ammoPickup(2);

    int score = 0;
    int hiScore = 0;

    //For the home/ game over screen
    Sprite spriteGameOver;
    Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
    spriteGameOver.setTexture(textureGameOver);
    spriteGameOver.setPosition(0, 0);
    //Create a view for the HUD
    View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));
    //Create a sprite for ammo icon
    Sprite spriteAmmoIcon;
    Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
    spriteAmmoIcon.setTexture(textureAmmoIcon);
    spriteAmmoIcon.setPosition(20, 680);
    Font font;
    font.loadFromFile("fonts/zombiecontrol.ttf");
    //Paused
    Text pausedText;
    pausedText.setFont(font);
    pausedText.setCharacterSize(150);
    pausedText.setFillColor(Color::White);
    pausedText.setPosition(280, 280);
    pausedText.setString("Press Enter \nto continue");
    //Game Over
    Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(120);
    gameOverText.setFillColor(Color::White);
    gameOverText.setPosition(170, 600);
    gameOverText.setString("Press Enter to play");
    //Leveling up
    Text levelUpText;
    levelUpText.setFont(font);
    levelUpText.setCharacterSize(60);
    levelUpText.setFillColor(Color::White);
    levelUpText.setPosition(105, 170);
    std::stringstream levelUpStream;
    levelUpStream << "Choose your advantage: " 
        << "\n1- Increased rate of fire" 
        << "\n2 - Increased clip size (next reload)" 
        << "\n3 - Increased max health" 
        << "\n4 - Increased run speed" 
        << "\n5 - More and better health pickups" 
        << "\n6 - More and better ammo pickups";
    levelUpText.setString(levelUpStream.str());
    //Ammo
    Text ammoText;
    ammoText.setFont(font);
    ammoText.setCharacterSize(50);
    ammoText.setFillColor(Color::White);
    ammoText.setPosition(140, 690);
    //Score
    Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(50);
    scoreText.setFillColor(Color::White);
    scoreText.setPosition(20, 0);
    //Load high score from text file
    std::ifstream inputFile("gamedata/scores.txt");
    if (inputFile.is_open())
    {
        inputFile >> hiScore;
        inputFile.close();
    }
    //Hi Score
    Text hiScoreText;
    hiScoreText.setFont(font);
    hiScoreText.setCharacterSize(50);
    hiScoreText.setFillColor(Color::White);
    hiScoreText.setPosition(990, 0);
    std::stringstream s;
    s << "Hi Score:" << hiScore;
    hiScoreText.setString(s.str());
    //Zombies remaining
    Text zombiesRemainingText;
    zombiesRemainingText.setFont(font);
    zombiesRemainingText.setCharacterSize(50);
    zombiesRemainingText.setFillColor(Color::White);
    zombiesRemainingText.setPosition(1060, 690);
    zombiesRemainingText.setString("Zombies: 100");
    //Wave number
    int wave = 0;
    Text waveNumberText;
    waveNumberText.setFont(font);
    waveNumberText.setCharacterSize(50);
    waveNumberText.setFillColor(Color::White);
    waveNumberText.setPosition(850, 690);
    waveNumberText.setString("Wave: 0");
    //Health bar
    RectangleShape healthBar;
    healthBar.setFillColor(Color::Red);
    healthBar.setPosition(330, 690);
    //Debug HUD
    Text debugText;
    debugText.setFont(font);
    debugText.setCharacterSize(25);
    debugText.setFillColor(Color::White);
    debugText.setPosition(20, 0);
    std::ostringstream ss;
    //When did we last update the HUD?
    int framesSinceLastHUDUpdate = 0;
    
    Time timeSinceLastUpdate;
    //How often we should update the HUD
    int fpsMeasurementFrameInterval = 1000;

    //Sounds
    SoundBuffer hitBuffer;
    hitBuffer.loadFromFile("sound/hit.wav");
    Sound hit;
    hit.setBuffer(hitBuffer);
    SoundBuffer splatBuffer;
    splatBuffer.loadFromFile("sound/splat.wav");
    Sound splat;
    splat.setBuffer(splatBuffer);
    SoundBuffer shootBuffer;
    shootBuffer.loadFromFile("sound/shoot.wav");
    Sound shoot;
    shoot.setBuffer(shootBuffer);
    SoundBuffer reloadBuffer;
    reloadBuffer.loadFromFile("sound/reload.wav");
    Sound reload;
    reload.setBuffer(reloadBuffer);
    SoundBuffer reloadFailedBuffer;
    reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
    Sound reloadFailed;
    reloadFailed.setBuffer(reloadFailedBuffer);
    SoundBuffer powerupBuffer;
    powerupBuffer.loadFromFile("sound/powerup.wav");
    Sound powerup;
    powerup.setBuffer(powerupBuffer);
    SoundBuffer pickupBuffer;
    pickupBuffer.loadFromFile("sound/pickup.wav");
    Sound pickup;
    pickup.setBuffer(pickupBuffer);



    //main game loop
    while (window.isOpen())
    {
        /***    Handle input    ***/
        //Handle events by polling
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::KeyPressed)
            {
                //Pause a game while playing
                if (event.key.code == Keyboard::Return && state == State::PLAYING)
                {
                    state = State::PAUSED;
                }
                //Restart while paused
                else if (event.key.code == Keyboard::Return && state == State::PAUSED)
                {
                    state = State::PLAYING;
                    //Reset the clock so there isn;t a frame jump
                    clock.restart();
                }
                else if (event.key.code == Keyboard::Return && state == State::GAME_OVER)
                {
                    state = State::LEVELING_UP;
                    wave = 0;
                    score = 0;
                    currentBullet = 0;
                    bulletsSpare = 24;
                    bulletsInClip = 6;
                    clipSize = 6;
                    fireRate = 1;
                    player.resetPlayerStats();
                }
                if (state == State::PLAYING)
                {
                    //Reloading
                    if (event.key.code == Keyboard::R)
                    {
                        if (bulletsSpare >= clipSize)
                        {
                            bulletsInClip = clipSize;
                            bulletsSpare -= clipSize;
                            reload.play();
                        }
                        else if (bulletsSpare > 0)
                        {
                            bulletsInClip = bulletsSpare;
                            bulletsSpare = 0;
                            reload.play();
                        }
                        else
                        {
                            reloadFailed.play();
                        }
                    }
                }
            }
        }// End event polling
        //Handle the player quitting
        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
           window.close();
        }
        //Handle WASD while playing    
        if (state == State::PLAYING)
        {
            if (Keyboard::isKeyPressed(Keyboard::W))
            {
                player.moveUp();
            }
            else
            {
                player.stopUp();
            }
            if (Keyboard::isKeyPressed(Keyboard::S))
            {
                player.moveDown();
            }
            else
            {
                player.stopDown();
            }
            if (Keyboard::isKeyPressed(Keyboard::A))
            {
                player.moveLeft();
            }
            else
            {
                player.stopLeft();
            }
            if (Keyboard::isKeyPressed(Keyboard::D))
            {
                player.moveRight();
            }
            else
            {
                player.stopRight();
            }
            //Fire a bullet
            if (Mouse::isButtonPressed(sf::Mouse::Left))
            {
                if (gameTimeTotal.asMilliseconds() - lastPressed.asMilliseconds() > 1000 / fireRate && bulletsInClip > 0)
                {
                    bullets[currentBullet].shoot(player.getCenter().x, player.getCenter().y, mouseWorldPosition.x, mouseWorldPosition.y);
                    currentBullet++;
                    if (currentBullet > 99)
                    {
                        currentBullet = 0;
                    }
                    lastPressed = gameTimeTotal;
                    shoot.play();
                    bulletsInClip--;
                }
            }
        }
        //Handle LEVELING UP state
        if (state == State::LEVELING_UP)
        {
            if (event.key.code == Keyboard::Num1)
            {
                fireRate++;
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num2)
            {
                clipSize += clipSize;
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num3)
            {
                player.upgradeHealth();
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num4)
            {
                player.upgradeSpeed();
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num5)
            {
                healthPickup.upgrade();
                state = State::PLAYING;
            }
            if (event.key.code == Keyboard::Num6)
            {
                ammoPickup.upgrade();
                state = State::PLAYING;
            }
            if (state == State::PLAYING)
            {
                //Increase wave number
                wave++;
                //Prepare the level
                arena.width = 500 * wave;
                arena.height = 500 * wave;
                arena.left = 0;
                arena.top = 0;
                //Pass vertex array by reference to the createBackground function
                int tileSize = createBackground(background, arena);
                //Spawn the player in the middle of the arena
                player.spawn(arena, resolution, tileSize);
                //Configure the pickups
                healthPickup.setArena(arena);
                ammoPickup.setArena(arena);
                //Create a horde of zombies
                numZombies = 5 * wave;
                //Delete previously allocated memory if it exists
                delete[] zombies;
                zombies = createHorde(numZombies, arena);
                numZombiesAlive = numZombies;
                powerup.play();
                //Reset clock so no frame jump
                clock.restart();
            }
        }//End leveling up    
        /***    Update the frame    ***/
        if (state == State::PLAYING)
        {
            //Update delta time
            Time dt = clock.restart();
            //Update total game time
            gameTimeTotal += dt;
            //Make a decimal fraction of 1 from the delta time
            float dtAsSeconds = dt.asSeconds();
            //Where is the mouse?
            mouseScreenPosition = Mouse::getPosition();
            //Convert mouse position to world coordinates of maineView
            mouseWorldPosition = window.mapPixelToCoords(Mouse::getPosition(), mainView);
            //Set the crosshair to the mouse world location
            spriteCrosshair.setPosition(mouseWorldPosition);
            player.update(dtAsSeconds, Mouse::getPosition());
            //Make a note of the players new position
            Vector2f playerPosition(player.getCenter());
            //Make the view centre around the player
            mainView.setCenter(player.getCenter());
            //Loop through each Zombie and update them
            for (int i = 0; i < numZombies; i++)
            {
                if (zombies[i].isAlive())
                {
                    zombies[i].update(dt.asSeconds(), playerPosition);
                }
            }
            //Update bullets inflight
            for (int i = 0; i < 100; i++)
            {
                if (bullets[i].isInFlight())
                {
                    bullets[i].update(dtAsSeconds);
                }
            }
            //Update pickups
            healthPickup.update(dtAsSeconds);
            ammoPickup.update(dtAsSeconds);
            //Collision detection
            //Zombies
            for (int i = 0; i < 100; i++)
            {
                for (int j = 0; j < numZombies; j++)
                {
                    if (bullets[i].isInFlight() && zombies[j].isAlive())
                    {
                        if (bullets[i].getPosition().intersects(zombies[j].getPosition()))
                        {
                            bullets[i].stop();
                            if (zombies[j].hit())
                            {
                                score += 10;
                                if (score >= hiScore)
                                {
                                    hiScore = score;
                                }
                                numZombiesAlive--;
                                if (numZombiesAlive == 0)
                                {
                                    state = State::LEVELING_UP;
                                }
                                splat.play();
                            }
                        }
                    }
                }
            }
            //Have any zombies touched the player
            for (int i = 0; i < numZombies; i++)
            {
                if (player.getPosition().intersects(zombies[i].getPosition()) && zombies[i].isAlive())
                {
                    if (player.hit(gameTimeTotal))
                    {
                        hit.play();
                    }
                    if (player.getHealth() <= 0)
                    {
                        state = State::GAME_OVER;
                        std::ofstream outputFile("gamedata/scores.txt");
                        outputFile << hiScore;
                        outputFile.close();
                    }
                }
            }
            //Has the player touched health pickup
            if (player.getPosition().intersects(healthPickup.getPosition()) && healthPickup.isSpawned())
            {
                player.increaseHealthLevel(healthPickup.gotIt());
                pickup.play();
            }
            if (player.getPosition().intersects(ammoPickup.getPosition()) && ammoPickup.isSpawned())
            {
                bulletsSpare += ammoPickup.gotIt();
                pickup.play();
            }
            //size up health bar
            healthBar.setSize(Vector2f(player.getHealth() * 3, 50));
            //Increment number of frames since last update
            framesSinceLastHUDUpdate++;
            //recalculate 
            if (framesSinceLastHUDUpdate > fpsMeasurementFrameInterval)
            {
                //Update HUD game text
                std::stringstream ssAmmo;
                std::stringstream ssScore;
                std::stringstream ssHiScore;
                std::stringstream ssWave;
                std::stringstream ssZombiesAlive;
                //Update ammo text;
                ssAmmo << bulletsInClip << "/" << bulletsSpare;
                ammoText.setString(ssAmmo.str());
                ssScore << "Score: " << score;
                scoreText.setString(ssScore.str());
                ssHiScore << "Hi Score: " << hiScore;
                hiScoreText.setString(ssHiScore.str());
                ssWave << "Wave: " << wave;
                waveNumberText.setString(ssWave.str());
                ssZombiesAlive << "Zombies: " << numZombiesAlive;
                zombiesRemainingText.setString(ssZombiesAlive.str());
                framesSinceLastHUDUpdate = 0;
            }
        }//End updating the scene
        /***    Draw the scene  ***/
        if (state == State::PLAYING)
        {
            window.clear();
            //set mainView to be displayed in the window and draw everything related to it
            window.setView(mainView);
            //DRaw background
            window.draw(background, &textureBackground);
            //Draw zombies
            for (int i = 0; i < numZombies; i++)
            {
                window.draw(zombies[i].getSprite());
            }
            //Draw bullets
            for (int i = 0; i < 100; i++)
            {
                if (bullets[i].isInFlight())
                {
                    window.draw(bullets[i].getShape());
                }
            }
            //Draw player
            window.draw(player.getSprite());
            //Draw pickups
            if (ammoPickup.isSpawned())
            {
                window.draw(ammoPickup.getSprite());
            }
            if (healthPickup.isSpawned())
            {
                window.draw(healthPickup.getSprite());
            }
            //Draw crosshair
            window.draw(spriteCrosshair);
            //Switch to HUD view
            window.setView(hudView);
            window.draw(spriteAmmoIcon);
            window.draw(ammoText);
            window.draw(scoreText);
            window.draw(hiScoreText);
            window.draw(healthBar);
            window.draw(waveNumberText);
            window.draw(zombiesRemainingText);
        }
        if (state == State::LEVELING_UP)
        {
            window.draw(spriteGameOver);
            window.draw(levelUpText);
        }
        if (state == State::PAUSED)
        {
            window.draw(pausedText);
        }
        if (state == State::GAME_OVER)
        {
            window.draw(spriteGameOver);
            window.draw(gameOverText);
            window.draw(scoreText);
            window.draw(hiScoreText);
        }
        window.display();
    }//End game loop
    delete[] zombies;
    return 0;

}
