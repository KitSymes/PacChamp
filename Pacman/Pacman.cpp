#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>

enum State {
	Menu, InGame, ScoresMenu
};

int deathStage = -1;
int menuPacSequence = 0;
int score = 0;
int scoresStringY;
State stage = Menu;
bool keysPressed[26], backspace;
string name;

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv)
{
	srand(time(NULL));
	_ourPac = new PacCharacter();
	_ourPac->dead = false;
	_frameCount = 0;
	_paused = false;

	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->_id = i;
	}

	//Initialise important Game aspects
	Audio::Initialise();
	Graphics::Initialise(argc, argv, this, 1024, 768, false, 25, 25, "Pacman", 60);
	Input::Initialise();

	// Start the Game Loop - This calls Update and Draw in game loop
	Graphics::StartGameLoop();
}

Pacman::~Pacman()
{
	// Delete Menu Stuff
	delete _menuBackground;
	delete _menuSpriteBounds;
	delete _menuStringPosition;
	delete _scoresMenuStringPosition;
	delete _pauseMenuStringPosition;

	for (int i = 0; i < SEQUENCECOUNT; i++)
		delete _sequence[i];
	delete[] _sequence;

	for (int i = 0; i < PATROLSEQUENCECOUNT; i++)
		delete _patrolSequence[i];
	delete[] _patrolSequence;

	// Delete PacChamp
	delete _ourPac->_texture;
	delete _ourPac->_sourceRect;
	delete _ourPac->_position;
	delete _ourPac;

	delete _wallTexture;

	// Delete Munchies
	delete _munchies[0]->_texture;
	for (int i = 0; i < size(_munchies); i++)
		delete _munchies[i];
	delete[] _munchies;

	// Delete Ghosts
	for (int i = 0; i < GHOSTCOUNT; i++)
		delete _ghosts[i];
	delete[] _ghosts;

	// Delete Cherry
	delete _cherry;

	// Delete Game Over Stuff
	delete _gameOverTexture;
	delete _gameOverSource;
	delete _gameOverPos;
	delete _gameOverStringPos;
	delete _gameOverNamePos;

	// Delete Sound Effects
	delete _pog;
	delete _nom;
	delete _death;

	// Delete Buttons
	delete _buttonTexture;
	delete _startGame;
	delete _scoreScreen;
	delete _toMenu;

	// Delete Walls
	delete _wallTexture;
	for (int i = 0; i < WALLCOUNT; i++)
		delete _walls[i];
	delete[] _walls;
}

void Pacman::LoadContent()
{
	// Load Menu
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/PacChamp.png", false);
	_menuSpriteBounds = new Rect(0.0f, 0.0f, 32.0f, 32.0f);
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f - 144, Graphics::GetViewportHeight() / 2.0f - 40);
	scoresStringY = Graphics::GetViewportHeight() / 2.0f - 10.0f;
	_scoresMenuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f - 88, scoresStringY);
	_pauseMenuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f - 88, Graphics::GetViewportHeight() / 2.0f);
	{
		_sequence[0] = new MovementSequence(Direction::DOWN, new Rect(32 * 7.9, 32 * 11, 32, 32));
		_sequence[1] = new MovementSequence(Direction::RIGHT, new Rect(32 * 7, 32 * 16.9, 32, 32));
		_sequence[2] = new MovementSequence(Direction::DOWN, new Rect(32 * 14.9, 32 * 16, 32, 32));
		_sequence[3] = new MovementSequence(Direction::RIGHT, new Rect(32 * 14, 32 * 18.9, 32, 32));
		_sequence[4] = new MovementSequence(Direction::DOWN, new Rect(32 * 22, 32 * 18, 32, 32));
		_sequence[5] = new MovementSequence(Direction::LEFT, new Rect(32 * 21, 32 * 20.9, 32, 32));
		_sequence[6] = new MovementSequence(Direction::DOWN, new Rect(32 * 16.1, 32 * 20, 32, 32));
		_sequence[7] = new MovementSequence(Direction::RIGHT, new Rect(32 * 17, 32 * 22.9, 32, 32));
		_sequence[8] = new MovementSequence(Direction::UP, new Rect(32 * 31, 32 * 22, 32, 32));
		_sequence[9] = new MovementSequence(Direction::LEFT, new Rect(32 * 30, 32 * 19, 32, 32));
		_sequence[10] = new MovementSequence(Direction::UP, new Rect(32 * 23.1, 32 * 20, 32, 32));
		_sequence[11] = new MovementSequence(Direction::RIGHT, new Rect(32 * 24, 32 * 10, 32, 32));
		for (int i = 0; i < SEQUENCECOUNT - 1; i++)
			_sequence[i]->next = _sequence[i + 1];
		_sequence[SEQUENCECOUNT - 1]->next = _sequence[0];
	}

	// Load Game Over Stuff
	_gameOverTexture = new Texture2D();
	_gameOverTexture->Load("Textures/GameOver.png", false);
	_gameOverSource = new Rect(0, 0, 64, 64);
	_gameOverPos = new Vector2(Graphics::GetViewportWidth() / 2 - 32, Graphics::GetViewportHeight() / 2 - 110);
	_gameOverStringPos = new Vector2(Graphics::GetViewportWidth() / 2 - 39, Graphics::GetViewportHeight() / 2 - 25);
	_gameOverNamePos = new Vector2(Graphics::GetViewportWidth() / 2 - 39, Graphics::GetViewportHeight() / 2 - 5);

	// Load Pacman
	_ourPac->_texture = new Texture2D();
	_ourPac->_texture->Load("Textures/Pacman.tga", false);
	_ourPac->_sourceRect = new Rect(0.0f, 0.0f, 32.0f, 32.0f);

	// Load Walls
	_wallTexture = new Texture2D();
	_wallTexture->Load("Textures/wall.png", false);
	{
		_walls[0] = new Wall(0, 0, Graphics::GetViewportWidth(), 32); // Top Bounds

		_walls[1] = new Wall(0, 32, 32, 32 * 6); // Left Side Bounds
		_walls[2] = new Wall(0, 32 * 7, 32 * 7, 32 * 4);
		_walls[3] = new Wall(0, 32 * 12, 32 * 7, 32 * 4);
		_walls[4] = new Wall(0, 32 * 15, 32, 32 * 8);

		_walls[5] = new Wall(0, Graphics::GetViewportHeight() - 32, Graphics::GetViewportWidth(), 32); // Bottom Bounds

		_walls[6] = new Wall(32 * 2, 32 * 2, 32 * 5, 32 * 2); // 1st Column Top Box

		_walls[7] = new Wall(32 * 2, 32 * 5, 32 * 5, 32); // 1st Column Slim Box

		_walls[8] = new Wall(32 * 1, 32 * 19, 32 * 2, 32); // 1st Column Small - at Bottom

		_walls[9] = new Wall(32 * 2, 32 * 17, 32 * 5, 32); // 1st Column Pointing Left Shape
		_walls[10] = new Wall(32 * 4, 32 * 18, 32 * 3, 32 * 2);

		_walls[11] = new Wall(32 * 8, 32 * 2, 32 * 6, 32 * 2); // 2nd Column Top Block

		_walls[12] = new Wall(32 * 8, 32 * 5, 32, 32 * 6); // 2nd Column Sideways T
		_walls[13] = new Wall(32 * 9, 32 * 7, 32 * 4, 32);

		_walls[14] = new Wall(32 * 8, 32 * 12, 32, 32 * 4); // 2nd Column |

		_walls[15] = new Wall(32 * 8, 32 * 17, 32 * 5, 32); // 2nd Column Above Inverted T

		_walls[16] = new Wall(32 * 2, 32 * 21, 32 * 12, 32); // 2nd Column Inverted T
		_walls[17] = new Wall(32 * 8, 32 * 19, 32 * 2, 32 * 2);

		_walls[18] = new Wall(32 * 15, 32, 32 * 2, 32 * 3); // Top Divide |

		_walls[19] = new Wall(32 * 11, 32 * 5, 32 * 10, 32); // Top T
		_walls[20] = new Wall(32 * 15, 32 * 6, 32 * 2, 32 * 2);

		_walls[21] = new Wall(32 * 11, 32 * 15, 32 * 10, 32); // Middle T
		_walls[22] = new Wall(32 * 15, 32 * 16, 32 * 2, 32 * 2);

		_walls[23] = new Wall(32 * 11, 32 * 19, 32 * 10, 32); // Bottom T
		_walls[24] = new Wall(32 * 15, 32 * 20, 32 * 2, 32 * 2);

		// Right side of Walls
		_walls[25] = new Wall(32 * 18, 32 * 2, 32 * 6, 32 * 2); // 2nd Column Top Block

		_walls[26] = new Wall(32 * 23, 32 * 5, 32, 32 * 6); // 2nd Column Sideways T
		_walls[27] = new Wall(32 * 19, 32 * 7, 32 * 4, 32);

		_walls[28] = new Wall(32 * 23, 32 * 12, 32, 32 * 4); // 2nd Column |

		_walls[29] = new Wall(32 * 19, 32 * 17, 32 * 5, 32); // 2nd Column -- Above Inverted T

		_walls[30] = new Wall(32 * 18, 32 * 21, 32 * 12, 32); // 2nd Column Inverted T
		_walls[31] = new Wall(32 * 22, 32 * 19, 32 * 2, 32 * 2);

		_walls[32] = new Wall(Graphics::GetViewportWidth() - 32, 32, 32, 32 * 6); // Right Side Bounds
		_walls[33] = new Wall(Graphics::GetViewportWidth() - (32 * 7), 32 * 7, 32 * 7, 32 * 4);
		_walls[34] = new Wall(Graphics::GetViewportWidth() - (32 * 7), 32 * 12, 32 * 7, 32 * 4);
		_walls[35] = new Wall(Graphics::GetViewportWidth() - 32, 32 * 15, 32, 32 * 8);

		_walls[36] = new Wall(32 * 25, 32 * 2, 32 * 5, 32 * 2); // 1st Column Top Box

		_walls[37] = new Wall(32 * 25, 32 * 5, 32 * 5, 32); // 1st Column Slim Box

		_walls[38] = new Wall(32 * 29, 32 * 19, 32 * 2, 32); // 1st Column Small - at Bottom

		_walls[39] = new Wall(32 * 25, 32 * 17, 32 * 5, 32); // 1st Column Pointing Left Shape
		_walls[40] = new Wall(32 * 25, 32 * 18, 32 * 3, 32 * 2);

		_walls[41] = new Wall(32 * 11, 32 * 9, 32, 32 * 4); // Ghost Cage
		_walls[42] = new Wall(32 * 11, 32 * 13, 32 * 10, 32);
		_walls[43] = new Wall(32 * 20, 32 * 9, 32, 32 * 4);
		_walls[44] = new Wall(32 * 12, 32 * 9, 32 * 3, 32);
		_walls[45] = new Wall(32 * 17, 32 * 9, 32 * 3, 32);
	}

	// Load Munchies
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Kappachie.png", false);
	for (int i = 0; i < size(_munchies); i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->_frameCount = rand() % 20;
		_munchies[i]->_texture = munchieTex;
		_munchies[i]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
		/*_munchies[i]->_position = new Rect(32 * (rand() % 32), 32 * (rand() % 24), 32, 32);
		bool ok = false;
		while (!ok)
		{
			ok = true;
			for (int w = 0; w < WALLCOUNT; w++)
			{
				if (_walls[w] == nullptr) continue;
				ok = !CollisionCheck(_munchies[i]->_position->X + 8, _munchies[i]->_position->Y + 8, _munchies[i]->_sourceRect->Width - 16, _munchies[i]->_sourceRect->Height - 16,
					_walls[w]->_position->X, _walls[w]->_position->Y, _walls[w]->_sourceRect->Width, _walls[w]->_sourceRect->Height);

				if (!ok) break;

				for (int m = 0; m < i; m++)
				{
					ok = !CollisionCheck(_munchies[i]->_position->X, _munchies[i]->_position->Y, _munchies[i]->_sourceRect->Width, _munchies[i]->_sourceRect->Height,
						_munchies[m]->_position->X, _munchies[m]->_position->Y, _munchies[m]->_sourceRect->Width, _munchies[m]->_sourceRect->Height);

					if (!ok) break;
				}

				if (!ok) break;
			}

			if (!ok)
			{
				_munchies[i]->_position->X = 32 * (rand() % 32);
				_munchies[i]->_position->Y = 32 * (rand() % 24);
			}
		}*/
	}

	// Load Cherry
	_cherry = new Enemy();
	_cherry->_texture = new Texture2D();
	_cherry->_texture->Load("Textures/Cherry.png", false);
	_cherry->_position = new Rect(-10, -10, 32, 32);
	_cherry->_sourceRect = new Rect(0, 0, 64, 64);

	// Load Ghosts
	_ghosts[0]->_texture = new Texture2D();
	_ghosts[0]->_texture->Load("Textures/GhostBlue.png", false);
	_ghosts[0]->_position = new Vector2();
	_ghosts[0]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	/*_ghosts[0]->_direction = LEFT;
	{
		bool ok = false;
		while (!ok)
		{
			ok = true;
			for (int w = 0; w < WALLCOUNT; w++)
			{
				if (_walls[w] == nullptr) continue;
				ok = !CollisionCheck(_ghosts[0]->_position->X, _ghosts[0]->_position->Y, _ghosts[0]->_sourceRect->Width, _ghosts[0]->_sourceRect->Height,
					_walls[w]->_position->X, _walls[w]->_position->Y, _walls[w]->_sourceRect->Width, _walls[w]->_sourceRect->Height);
				if (!ok) break;
			}

			if (!ok)
			{
				_ghosts[0]->_position->X = rand() % (Graphics::GetViewportWidth());
				_ghosts[0]->_position->Y = rand() % (Graphics::GetViewportHeight());
			}
		}
	}*/

	_ghosts[1]->_texture = new Texture2D();
	_ghosts[1]->_texture->Load("Textures/GhostGreen.png", false);
	_ghosts[1]->_position = new Vector2();
	_ghosts[1]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	_ghosts[2]->_texture = new Texture2D();
	_ghosts[2]->_texture->Load("Textures/GhostYellow.png", false);
	_ghosts[2]->_position = new Vector2();
	_ghosts[2]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	{
		_patrolSequence[0] = new MovementSequence(Direction::DOWN, new Rect(32 * 23, 32 * 8, 32, 32));
		_patrolSequence[1] = new MovementSequence(Direction::LEFT, new Rect(32 * 22, 32 * 17, 32, 32));
		_patrolSequence[2] = new MovementSequence(Direction::DOWN, new Rect(32 * 16, 32 * 16, 32, 32));
		_patrolSequence[3] = new MovementSequence(Direction::LEFT, new Rect(32 * 17, 32 * 19, 32, 32));
		_patrolSequence[4] = new MovementSequence(Direction::UP, new Rect(32 * 6, 32 * 18, 32, 32));
		_patrolSequence[5] = new MovementSequence(Direction::RIGHT, new Rect(32 * 7, 32 * 3, 32, 32));
		_patrolSequence[6] = new MovementSequence(Direction::DOWN, new Rect(32 * 23, 32 * 4, 32, 32));
		_patrolSequence[7] = new MovementSequence(Direction::LEFT, new Rect(32 * 22, 32 * 7, 32, 32));
		_patrolSequence[8] = new MovementSequence(Direction::DOWN, new Rect(32 * 16, 32 * 6, 32, 32));
		_patrolSequence[9] = new MovementSequence(Direction::LEFT, new Rect(32 * 17, 32 * 9, 32, 32));
		_patrolSequence[10] = new MovementSequence(Direction::DOWN, new Rect(32 * 8, 32 * 8, 32, 32));
		_patrolSequence[11] = new MovementSequence(Direction::RIGHT, new Rect(32 * 9, 32 * 17, 32, 32));
		_patrolSequence[12] = new MovementSequence(Direction::DOWN, new Rect(32 * 15, 32 * 16, 32, 32));
		_patrolSequence[13] = new MovementSequence(Direction::RIGHT, new Rect(32 * 14, 32 * 19, 32, 32));
		_patrolSequence[14] = new MovementSequence(Direction::UP, new Rect(32 * 25, 32 * 18, 32, 32));
		_patrolSequence[15] = new MovementSequence(Direction::LEFT, new Rect(32 * 24, 32 * 3, 32, 32));
		_patrolSequence[16] = new MovementSequence(Direction::DOWN, new Rect(32 * 8, 32 * 4, 32, 32));
		_patrolSequence[17] = new MovementSequence(Direction::RIGHT, new Rect(32 * 9, 32 * 7, 32, 32));
		_patrolSequence[18] = new MovementSequence(Direction::DOWN, new Rect(32 * 15, 32 * 6, 32, 32));
		_patrolSequence[19] = new MovementSequence(Direction::RIGHT, new Rect(32 * 14, 32 * 9, 32, 32));
		for (int i = 0; i < PATROLSEQUENCECOUNT - 1; i++)
			_patrolSequence[i]->next = _patrolSequence[i + 1];
		_patrolSequence[PATROLSEQUENCECOUNT - 1]->next = _patrolSequence[0];
	}

	_ghosts[3]->_texture = new Texture2D();
	_ghosts[3]->_texture->Load("Textures/GhostRed.png", false);
	_ghosts[3]->_position = new Vector2();
	_ghosts[3]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Set string position
	_debugStringPosition = new Vector2(10.0f, 25.0f);

	// Load Sound Effects
	_pog = new SoundEffect();
	_pog->Load("Sounds/pog.wav");
	_nom = new SoundEffect();
	_nom->Load("Sounds/nom.wav");
	_death = new SoundEffect();
	_death->Load("Sounds/death.wav");

	// Load Buttons
	_buttonTexture = new Texture2D();
	_buttonTexture->Load("Textures/Button.png", false);

	_startGame = new Button("Start", Graphics::GetViewportWidth() / 2 - 20, Graphics::GetViewportHeight() / 2 - 5, 41, 20, Color::Yellow);
	_scoreScreen = new Button("Scores", Graphics::GetViewportWidth() / 2 - 29, Graphics::GetViewportHeight() / 2 + 25, 59, 20, Color::Yellow);
	_toMenu = new Button("Back", Graphics::GetViewportWidth() / 2 - 21, Graphics::GetViewportHeight() / 2 + 20, 42, 20, Color::Yellow);

	// Load Scores
	ifstream inFile("scores.txt");
	if (inFile.is_open())
	{
		for (int i = 0; i < 10; i++)
		{
			scores[i].name = "";
			scores[i].score = -1;
		}
		while (!inFile.eof())
		{
			inFile >> scores[scoresInFile].name;
			inFile >> scores[scoresInFile].score;
			scoresInFile++;
		}

		inFile.close();
	}

	ResetMap();
}

void Pacman::ResetMap()
{
	menuPacSequence = 0;
	score = 0;

	_ourPac->_position = new Vector2(0.0f, 350.0f);
	_ourPac->_direction = RIGHT;
	_ourPac->_speedMulti = 1.0;
	_ourPac->dead = false;
	_ourPac->super = false;
	deathStage = -1;
	superFrames = -1;
	cherryEaten = false;

	_ghosts[0]->_position->X = rand() % (Graphics::GetViewportWidth() - 128) + 64;
	_ghosts[0]->_position->Y = rand() % (Graphics::GetViewportHeight() - 128) + 64;
	_ghosts[0]->_direction = LEFT;
	/*{
		bool ok = false;
		while (!ok)
		{
			ok = true;
			for (int w = 0; w < WALLCOUNT; w++)
			{
				if (_walls[w] == nullptr) continue;
				ok = !CollisionCheck(_ghosts[0]->_position->X, _ghosts[0]->_position->Y, _ghosts[0]->_sourceRect->Width, _ghosts[0]->_sourceRect->Height,
					_walls[w]->_position->X, _walls[w]->_position->Y, _walls[w]->_sourceRect->Width, _walls[w]->_sourceRect->Height);
				if (!ok) break;
			}

			if (!ok)
			{
				_ghosts[0]->_position->X = rand() % (Graphics::GetViewportWidth());
				_ghosts[0]->_position->Y = rand() % (Graphics::GetViewportHeight());
			}
		}
	}*/


	_ghosts[1]->_position->X = rand() % (Graphics::GetViewportWidth() - 128) + 64;
	_ghosts[1]->_position->Y = rand() % (Graphics::GetViewportHeight() - 128) + 64;
	_ghosts[1]->_direction = DOWN;

	_ghosts[2]->_position->X = 32 * 16;
	_ghosts[2]->_position->Y = 32 * 11;
	_ghosts[2]->_direction = UP;
	_ghosts[2]->_extraData = -1;

	_ghosts[3]->_position->X = 32 * 15;
	_ghosts[3]->_position->Y = 32 * 11;
	_ghosts[3]->_direction = UP;
	_ghosts[3]->_extraData = true;

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->_position = new Rect(32 * (rand() % 32), 32 * (rand() % 24), 32, 32);
		bool ok = false;
		while (!ok)
		{
			ok = true;
			for (int w = 0; w < WALLCOUNT; w++)
			{
				if (_walls[w] == nullptr) continue;
				ok = !CollisionCheck(_munchies[i]->_position->X + 8, _munchies[i]->_position->Y + 8, _munchies[i]->_sourceRect->Width - 16, _munchies[i]->_sourceRect->Height - 16,
					_walls[w]->_position->X, _walls[w]->_position->Y, _walls[w]->_sourceRect->Width, _walls[w]->_sourceRect->Height);

				if (!ok) break;

				for (int m = 0; m < i; m++)
				{
					ok = !CollisionCheck(_munchies[i]->_position->X, _munchies[i]->_position->Y, _munchies[i]->_sourceRect->Width, _munchies[i]->_sourceRect->Height,
						_munchies[m]->_position->X, _munchies[m]->_position->Y, _munchies[m]->_sourceRect->Width, _munchies[m]->_sourceRect->Height);

					if (!ok) break;
				}

				if (!ok) break;
			}

			if (!ok)
			{
				_munchies[i]->_position->X = 32 * (rand() % 32);
				_munchies[i]->_position->Y = 32 * (rand() % 24);
			}
		}
	}

	name = "";
	for (int i = 0; i < 26; i++)
		keysPressed[i] = false;
	backspace = false;
}

bool wDown, aDown, sDown, dDown, moving, animate = false, trymove = false, clicked = false;

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	switch (stage)
	{
	case Menu:
		InputMenu(elapsedTime, keyboardState, mouseState);
		UpdateMenuPacman();
		break;
	case InGame:
		if (!_ourPac->dead)
			if (keyboardState->IsKeyDown(Input::Keys::ESCAPE) && !_pKeyDown)
			{
				_pKeyDown = true;
				_paused = !_paused;

				if (_paused) {
					if (_pog->IsLoaded())
						Audio::Pause(_pog);
				}
				else {
					if (_pog->IsLoaded() && !_ourPac->dead)
						Audio::Resume(_pog);
				}
			}

		if (keyboardState->IsKeyUp(Input::Keys::ESCAPE))
			_pKeyDown = false;

		if (_paused) return;

		_frameCount++;

		InputGame(elapsedTime, keyboardState, mouseState);

		if (!_ourPac->dead) {
			_ourPac->_sourceRect->Y = getYPosFromDirection(_ourPac->_direction);
			if (!_ourPac->super && animate)
				_ourPac->_sourceRect->X = (_frameCount % 20 >= 10 ? 32.0f : 0.0f); // Animates Packman
			UpdatePacman(elapsedTime);
		}
		else {
			if (deathStage == -1)
			{
				deathStage++;
				_ourPac->_sourceRect->X = 3 * 32;
				_ourPac->_sourceRect->Y = 0;
			}
			else if (deathStage < 4 && _frameCount % 5 == 0) // 1st death frame may play for less than 1/5th of a frame
			{
				deathStage++;
				_ourPac->_sourceRect->Y = deathStage * 32;
			}
		}

		for (int i = 0; i < GHOSTCOUNT; i++)
			UpdateGhost(_ghosts[i], elapsedTime);
		if (!_ourPac->dead && !_ourPac->super)
			CheckGhostCollisions();

		for (int i = 0; i < MUNCHIECOUNT; i++)
		{
			if (_munchies[i]->_position->X != -100 && CollisionCheck(_ourPac->_position->X, _ourPac->_position->Y, _ourPac->_sourceRect->Width, _ourPac->_sourceRect->Height,
				_munchies[i]->_position->X, _munchies[i]->_position->Y, _munchies[i]->_sourceRect->Width, _munchies[i]->_sourceRect->Height)) {
				_munchies[i]->_position->X = -100;
				score += 100;
				Audio::Play(_nom);
			}
		}

		if (!cherryEaten && CollisionCheck(_ourPac->_position->X, _ourPac->_position->Y, _ourPac->_sourceRect->Width, _ourPac->_sourceRect->Height,
			_cherry->_position->X, _cherry->_position->Y, _cherry->_sourceRect->Width, _cherry->_sourceRect->Height))
		{
			_ourPac->super = true;
			cherryEaten = true;
			_ourPac->_sourceRect->X = 64.0f; // Animates Packman
			superFrames = 20 * 20;
		}
		else if (superFrames > 0)
			superFrames--;
		else if (superFrames == 0)
		{
			superFrames = -1;
			_ourPac->super = false;
		}


		break;
	case ScoresMenu:
		InputScoresMenu(elapsedTime, keyboardState, mouseState);
		break;
	}
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState)
{
	switch (stage)
	{
	case Menu:
		InputMenu(elapsedTime, keyboardState, mouseState);
		break;
	case InGame:
		InputGame(elapsedTime, keyboardState, mouseState);
		break;
	case ScoresMenu:
		InputScoresMenu(elapsedTime, keyboardState, mouseState);
		break;
	}
}

void Pacman::InputGame(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState) // Handle Mouse Input – Reposition Cherry 
{
	if (_ourPac->dead)
	{
		if (deathStage != 4)
			return;
		if (!clicked) {
			if (mouseState->LeftButton == Input::ButtonState::PRESSED)
			{
				clicked = true;
				if (CollisionCheck(mouseState->X, mouseState->Y, 1, 1,
					_toMenu->buttonPos->X, _toMenu->buttonPos->Y, _toMenu->sourceRect->Width, _toMenu->sourceRect->Height)) {
					stage = Menu;
					if (!name.empty())
					{
						int pos = IsScoreInTop10(score);
						if (pos >= 0)
						{
							for (int i = scoresInFile; i >= 0; i--)
							{
								if (i > pos)
								{
									scores[i].name = scores[i - 1].name;
									scores[i].score = scores[i - 1].score;
								}
								else if (i == pos)
								{
									scores[i].name = name;
									scores[i].score = score;
									scoresInFile++;
									break;
								}
							}
							SaveScores();
						}

					}
					ResetMap();
				}
			}
		}
		else if (mouseState->LeftButton == Input::ButtonState::RELEASED)
			clicked = false;

		for (int i = 0; i < 26; i++)
		{
			if (!keysPressed[i] && keyboardState->IsKeyDown(Input::Keys(i)) && name.length() < 16)
			{
				name += char(65 + i);
				keysPressed[i] = true;
			}
			else if (keyboardState->IsKeyUp(Input::Keys(i)))
				keysPressed[i] = false;
		}

		if (!backspace && keyboardState->IsKeyDown(Input::Keys::BACKSPACE) && !name.empty())
		{
			name.erase(name.length() - 1);
			backspace = true;
		}
		else if (keyboardState->IsKeyUp(Input::Keys::BACKSPACE))
			backspace = false;

		return;
	}

	if (mouseState->LeftButton == Input::ButtonState::PRESSED) // Moves Cherry
	{
		_cherry->_position->X = mouseState->X;
		_cherry->_position->Y = mouseState->Y;
	}

	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT)) // Speed Boost
		_ourPac->_speedMulti = 2.0;
	else
		_ourPac->_speedMulti = 1.0;

	if (keyboardState->IsKeyDown(Input::Keys::D)) // Checks if D key is pressed
	{
		if (!dDown)
		{
			dDown = true;
			_ourPac->_direction = RIGHT;
			trymove = true;
		}
	}
	else
		dDown = false;

	if (keyboardState->IsKeyDown(Input::Keys::A)) // Checks if A key is pressed
	{
		if (!aDown)
		{
			aDown = true;
			_ourPac->_direction = LEFT;
			trymove = true;
		}
	}
	else
		aDown = false;

	if (keyboardState->IsKeyDown(Input::Keys::W)) // Checks if W key is pressed
	{
		if (!wDown)
		{
			wDown = true;
			_ourPac->_direction = UP;
			trymove = true;
		}
	}
	else
		wDown = false;

	if (keyboardState->IsKeyDown(Input::Keys::S)) // Checks if S key is pressed
	{
		if (!sDown)
		{
			sDown = true;
			_ourPac->_direction = DOWN;
			trymove = true;
		}
	}
	else
		sDown = false;
};

void Pacman::InputMenu(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState) {

	if (!clicked) {
		if (mouseState->LeftButton == Input::ButtonState::PRESSED)
		{
			clicked = true;
			if (CollisionCheck(mouseState->X, mouseState->Y, 1, 1,
				_startGame->buttonPos->X, _startGame->buttonPos->Y, _startGame->sourceRect->Width, _startGame->sourceRect->Height)) {
				stage = InGame;
				_pog->SetLooping(true);
				Audio::Play(_pog);
			}
			else if (CollisionCheck(mouseState->X, mouseState->Y, 1, 1,
				_scoreScreen->buttonPos->X, _scoreScreen->buttonPos->Y, _scoreScreen->sourceRect->Width, _scoreScreen->sourceRect->Height)) {
				stage = ScoresMenu;
			}
		}
	}
	else if (mouseState->LeftButton == Input::ButtonState::RELEASED)
		clicked = false;

	if (keyboardState->IsKeyDown(Input::Keys::SPACE))
	{
		stage = InGame;
		_pog->SetLooping(true);
		Audio::Play(_pog);
		_ourPac->dead = false;
	}

};

void Pacman::InputScoresMenu(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState)
{
	if (!clicked) {
		if (mouseState->LeftButton == Input::ButtonState::PRESSED)
		{
			clicked = true;
			if (CollisionCheck(mouseState->X, mouseState->Y, 1, 1,
				_toMenu->buttonPos->X, _toMenu->buttonPos->Y, _toMenu->sourceRect->Width, _toMenu->sourceRect->Height))
				stage = Menu;
		}
	}
	else if (mouseState->LeftButton == Input::ButtonState::RELEASED)
		clicked = false;
}

void Pacman::UpdatePacman(int elapsedTime)
{
	bool moved = false;
	if (trymove)
		switch (_ourPac->_direction)
		{
		case UP:
			moved = MovePac(0, -(_ourPac->_speedMulti * _ourPac->_speed));
			break;
		case DOWN:
			moved = MovePac(0, _ourPac->_speedMulti * _ourPac->_speed);
			break;
		case LEFT:
			moved = MovePac(-(_ourPac->_speedMulti * _ourPac->_speed), 0);
			break;
		case RIGHT:
			moved = MovePac(_ourPac->_speedMulti * _ourPac->_speed, 0);
			break;
		default:
			break;
		}

	if (moved)
	{
		if (!moving)
		{
			moving = true;
			animate = true;
			Audio::Play(_pog);
		}
	}
	else
	{
		if (moving)
		{
			moving = false;
			animate = false;
			if (!_ourPac->super)
				_ourPac->_sourceRect->X = 0.0f;
			Audio::Stop(_pog);
			trymove = false;
		}
	}

	if (_ourPac->_position->X < -22)
		_ourPac->_position->X = 1017;
	if (_ourPac->_position->X > 1019)
		_ourPac->_position->X = -20;


	if (_ourPac->_position->Y < -22)
		_ourPac->_position->Y = 756;
	if (_ourPac->_position->Y > 758)
		_ourPac->_position->Y = -20;
}

void Pacman::UpdateMenuPacman()
{
	switch (_ourPac->_direction)
	{
	case UP:
		MovePac(0, -(_ourPac->_speedMulti * _ourPac->_speed));
		break;
	case DOWN:
		MovePac(0, _ourPac->_speedMulti * _ourPac->_speed);
		break;
	case LEFT:
		MovePac(-(_ourPac->_speedMulti * _ourPac->_speed), 0);
		break;
	case RIGHT:
		MovePac(_ourPac->_speedMulti * _ourPac->_speed, 0);
		break;
	default:
		break;
	}

	if (HasReachedSequence(_ourPac->_position, _ourPac->_sourceRect, _sequence[menuPacSequence]))
	{
		_ourPac->_direction = _sequence[menuPacSequence]->dir;
		menuPacSequence = (menuPacSequence + 1) % SEQUENCECOUNT;
	}

	if (_ourPac->_position->X < -22)
		_ourPac->_position->X = 1017;
	if (_ourPac->_position->X > 1019)
		_ourPac->_position->X = -20;

	if (_ourPac->_position->Y < -22)
		_ourPac->_position->Y = 756;
	if (_ourPac->_position->Y > 758)
		_ourPac->_position->Y = -20;
}

bool Pacman::MovePac(int x, int y)
{
	int left1 = _ourPac->_position->X + x + 2;
	int right1 = _ourPac->_position->X + _ourPac->_sourceRect->Width + x - 4;
	int top1 = _ourPac->_position->Y + y + 4;
	int bottom1 = _ourPac->_position->Y + _ourPac->_sourceRect->Height + y - 2;
	int top2 = 0, bottom2 = 0, left2 = 0, right2 = 0;

	if (stage == InGame)
		for (int i = 0; i < WALLCOUNT; i++)
		{
			bottom2 = _walls[i]->_position->Y + _walls[i]->_sourceRect->Height;
			left2 = _walls[i]->_position->X;
			right2 = _walls[i]->_position->X + _walls[i]->_sourceRect->Width;
			top2 = _walls[i]->_position->Y;

			if ((bottom1 > top2 && (top1 < bottom2) && (right1 > left2) && (left1 < right2)))
				return false;
		}

	_ourPac->_position->X += x;
	_ourPac->_position->Y += y;

	return true;
}

void Pacman::Draw(int elapsedTime)
{
	switch (stage)
	{
	case Menu:
		DrawMenu(elapsedTime);
		break;
	case InGame:
		DrawGame(elapsedTime);
		break;
	case ScoresMenu:
		DrawScores(elapsedTime);
		break;
	}
}

void Pacman::DrawGame(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << /*"Pacman X: " << _ourPac->_position->X << " Y: " << _ourPac->_position->Y << endl <<*/ "Score: " << score;

	SpriteBatch::BeginDraw(); // Starts Drawing

	for (int i = 0; i < WALLCOUNT; i++)
		SpriteBatch::Draw(_wallTexture, _walls[i]->_position, _walls[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Wall

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		if (_munchies[i]->_position->X == -100) continue;
		_munchies[i]->_frameCount++;
		_munchies[i]->_sourceRect->X = (_munchies[i]->_frameCount % 10 >= 5 ? 32.0f : 0.0f); // Animates Munchie
		SpriteBatch::Draw(_munchies[i]->_texture, _munchies[i]->_position, _munchies[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Munchie
	}

	if (!cherryEaten)
	{
		_cherry->_sourceRect->Y = ((_frameCount % 20) / 4) * 64.0f; // Animates Cherry
		SpriteBatch::Draw(_cherry->_texture, _cherry->_position, _cherry->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Cherry
	}

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		_ghosts[i]->_sourceRect->X = (_frameCount % 5 >= 2 ? 32.0f : 0.0f);
		_ghosts[i]->_sourceRect->Y = (_ghosts[i]->_direction == UP || _ghosts[i]->_direction == DOWN ? 32.0f : 0.0f);
		SpriteBatch::Draw(_ghosts[i]->_texture, _ghosts[i]->_position, _ghosts[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, (_ghosts[i]->_direction == LEFT ? SpriteEffect::FLIPHORIZONTAL : (_ghosts[i]->_direction == UP ? SpriteEffect::FLIPVERTICAL : SpriteEffect::NONE))); // Draws Ghost
	}

	if (deathStage != 4)
		SpriteBatch::Draw(_ourPac->_texture, _ourPac->_position, _ourPac->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Pacman over Munchies
	else
	{
		std::stringstream nameStream;
		nameStream << "Enter Name: " << name << endl;
		DrawButton(_toMenu);
		SpriteBatch::Draw(_gameOverTexture, _gameOverPos, _gameOverSource, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);
		SpriteBatch::DrawString("You Died", _gameOverStringPos, Color::Red);
		_gameOverNamePos->X = Graphics::GetViewportWidth() / 2 - 52 - name.length() * 6;
		SpriteBatch::DrawString(nameStream.str().c_str(), _gameOverNamePos, Color::Red);
	}

	if (_paused)
	{
		std::stringstream menuStream;
		menuStream << "PAUSED";
		SpriteBatch::DrawString(menuStream.str().c_str(), _pauseMenuStringPosition, Color::Red);
	}

	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _debugStringPosition, Color::Green);
	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::DrawMenu(int elapsedTime) {
	SpriteBatch::BeginDraw(); // Starts Drawing

	for (int i = 0; i < WALLCOUNT; i++)
		if (_walls[i] != nullptr)
			SpriteBatch::Draw(_wallTexture, _walls[i]->_position, _walls[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Wall

	_frameCount++;
	_ourPac->_sourceRect->Y = getYPosFromDirection(_ourPac->_direction);
	_ourPac->_sourceRect->X = (_frameCount % 20 >= 10 ? 32.0f : 0.0f); // Animates Packman
	SpriteBatch::Draw(_ourPac->_texture, _ourPac->_position, _ourPac->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Pacman over Munchies

	//for (int i = 0; i < PATROLSEQUENCECOUNT; i++)
		//SpriteBatch::Draw(_buttonTexture, _patrolSequence[i]->posAndSize, _patrolSequence[i]->posAndSize, Vector2::Zero, 1.0f, 0.0f, Color::Yellow, SpriteEffect::NONE); // Draws Wall

	std::stringstream menuStream;
	menuStream << "Click Start or press SPACE to start";
	//SpriteBatch::Draw(_menuBackground, _menuSpriteBounds, 10.0f, 0.0f);
	DrawButton(_startGame);
	DrawButton(_scoreScreen);
	SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::DrawScores(int elapsedTime) {
	SpriteBatch::BeginDraw(); // Starts Drawing
	std::stringstream menuStream;
	int i;
	for (i = 0; i < 10; i++) {
		if (scores[i].score == -1)
			if (i == 0)
			{
				menuStream << "No scores recorded!" << endl;
				break;
			}
			else {
				i--;
				break;
			}
		menuStream << i + 1 << ". " << scores[i].score << " - " << scores[i].name << endl;
	}
	_scoresMenuStringPosition->Y = scoresStringY - i * 30.0f;
	DrawButton(_toMenu);
	SpriteBatch::DrawString(menuStream.str().c_str(), _scoresMenuStringPosition, Color::Red);
	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::DrawButton(Button* button)
{
	SpriteBatch::Draw(_buttonTexture, button->buttonPos, button->sourceRect, Vector2::Zero, 1.0f, 0.0f, button->col, SpriteEffect::NONE);
	SpriteBatch::DrawString(button->text.c_str(), button->textPos, Color::Red);
}

bool Pacman::CollisionCheck(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2)
{
	int left1 = x1;
	int left2 = x2;
	int right1 = x1 + width1;
	int right2 = x2 + width2;
	int top1 = y1;
	int top2 = y2;
	int bottom1 = y1 + height1;
	int bottom2 = y2 + height2;
	if (bottom1 < top2) return false;
	if (top1 > bottom2) return false;
	if (right1 < left2) return false;
	if (left1 > right2) return false;
	return true;
}

void Pacman::CheckGhostCollisions()
{
	int bottom1 = _ourPac->_position->Y + _ourPac->_sourceRect->Height;
	int left1 = _ourPac->_position->X;
	int right1 = _ourPac->_position->X + _ourPac->_sourceRect->Width;
	int top1 = _ourPac->_position->Y;
	int bottom2 = 0, left2 = 0, right2 = 0, top2 = 0;

	for (int i = 0; i < GHOSTCOUNT; i++)
	{
		bottom2 = _ghosts[i]->_position->Y + _ghosts[i]->_sourceRect->Height;
		left2 = _ghosts[i]->_position->X;
		right2 = _ghosts[i]->_position->X + _ghosts[i]->_sourceRect->Width;
		top2 = _ghosts[i]->_position->Y;

		if ((bottom1 > top2 && (top1 < bottom2) && (right1 > left2) && (left1 < right2)))
		{
			_ourPac->dead = true;
			Audio::Stop(_pog);
			Audio::Play(_death);
			break;
		}
	}
}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	bool canMove = true; // Defaults to True

	switch (ghost->_id) // 0-> left-right 1-> up-down 2-> patrol 3-> random
	{
	case 0:
	case 1:
		if (ghost->_position->X + ghost->_sourceRect->Width >= Graphics::GetViewportWidth()) // Hits Right Edge
		{
			if (ghost->_position->X > Graphics::GetViewportWidth())
				ghost->_position->X = Graphics::GetViewportWidth();
			ghost->_direction = LEFT;
		}
		else if (ghost->_position->X <= 0) // Hits Left Edge
		{
			if (ghost->_position->X < 0)
				ghost->_position->X = 0;
			ghost->_direction = RIGHT;
		}

		if (ghost->_position->Y + ghost->_sourceRect->Height >= Graphics::GetViewportHeight()) // Hits Bottom Edge
		{
			if (ghost->_position->Y > Graphics::GetViewportHeight())
				ghost->_position->Y = Graphics::GetViewportHeight();
			ghost->_direction = UP;
		}
		else if (ghost->_position->Y <= 0) // Hits Top Edge
		{
			if (ghost->_position->Y < 0)
				ghost->_position->Y = 0;
			ghost->_direction = DOWN;
		}
		break;
	case 2:
		if (ghost->_extraData == -1)
		{
			canMove = CanMoveGhost(ghost);
			if (!canMove) {
				ghost->_direction = RIGHT;
				ghost->_extraData++;
			}
		}
		else
			if (HasReachedSequence(ghost->_position, ghost->_sourceRect, _patrolSequence[ghost->_extraData]))
			{
				ghost->_direction = _patrolSequence[ghost->_extraData]->dir;
				ghost->_extraData = (ghost->_extraData + 1) % PATROLSEQUENCECOUNT;
			}
		break;
	case 3:
		canMove = CanMoveGhost(ghost);
		if (!canMove)
			if (ghost->_extraData)
			{
				ghost->_direction = LEFT;
				ghost->_extraData = false;
			}
			else
				ghost->_direction = Direction(rand() % 4);
		break;
	default:
		break;
	}

	if (canMove)
	{
		if (ghost->_direction == LEFT)
			ghost->_position->X -= ghost->_speed;
		else if (ghost->_direction == RIGHT)
			ghost->_position->X += ghost->_speed;
		else if (ghost->_direction == UP)
			ghost->_position->Y -= ghost->_speed;
		else if (ghost->_direction == DOWN)
			ghost->_position->Y += ghost->_speed;
	}
}

bool Pacman::CanMoveGhost(MovingEnemy* ghost)
{
	int x = 0, y = 0;
	switch (ghost->_direction)
	{
	case UP:
		y = -(ghost->_speed);
		break;
	case DOWN:
		y = ghost->_speed;
		break;
	case LEFT:
		x = -(ghost->_speed);
		break;
	case RIGHT:
		x = ghost->_speed;
		break;
	default:
		break;
	}

	int left1 = ghost->_position->X + x;
	int right1 = ghost->_position->X + ghost->_sourceRect->Width + x;
	int top1 = ghost->_position->Y + y;
	int bottom1 = ghost->_position->Y + ghost->_sourceRect->Height + y;
	int top2 = 0, bottom2 = 0, left2 = 0, right2 = 0;

	for (int i = 0; i < WALLCOUNT; i++)
	{
		bottom2 = _walls[i]->_position->Y + _walls[i]->_sourceRect->Height;
		left2 = _walls[i]->_position->X;
		right2 = _walls[i]->_position->X + _walls[i]->_sourceRect->Width;
		top2 = _walls[i]->_position->Y;

		if ((bottom1 > top2 && (top1 < bottom2) && (right1 > left2) && (left1 < right2)))
			return false;
	}

	return true;
}

int Pacman::IsScoreInTop10(int scoreToCheck)
{
	for (int i = 0; i < 10; i++)
	{
		if (scoreToCheck > scores[i].score)
			return i;
	}

	if (scoresInFile < 10)
		return scoresInFile;

	return -1;
}

void Pacman::SaveScores()
{
	ofstream ofStream("scores.txt", std::ios::out | std::ios::trunc);
	for (int i = 0; i < scoresInFile; i++)
	{
		ofStream << scores[i].name << endl;
		ofStream << scores[i].score;
		if (i != scoresInFile - 1)
			ofStream << endl;
	}
	ofStream.close();
}

bool Pacman::HasReachedSequence(Vector2* pos, Rect* source, MovementSequence* seq)
{
	int left1 = pos->X;
	int right1 = pos->X + source->Width;
	int top1 = pos->Y;
	int bottom1 = pos->Y + source->Height;
	int top2 = 0, bottom2 = 0, left2 = 0, right2 = 0;

	bottom2 = seq->posAndSize->Y + seq->posAndSize->Height;
	left2 = seq->posAndSize->X;
	right2 = seq->posAndSize->X + seq->posAndSize->Width;
	top2 = seq->posAndSize->Y;

	if ((bottom1 > top2 && (top1 < bottom2) && (right1 > left2) && (left1 < right2)))
	{
		return true;
	}
	return false;
}