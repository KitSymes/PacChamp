#include "Pacman.h"
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>

enum State {
	Menu, InGame, Scores
};

int score = 0;
State state = Menu;

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv)
{
	srand(time(NULL));
	_ourPac = new PacCharacter();
	_ourPac->dead = false;
	_frameCount = 0;
	_paused = false;

	for (int i = 0; i < GHOSTCOUNT; i++) {
		_ghosts[i] = new MovingEnemy();
		_ghosts[i]->direction = 0;
		_ghosts[i]->speed = 0.2f;
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
	delete _pauseMenuStringPosition;

	// Delete PacChamp
	delete _ourPac->_pacmanTexture;
	delete _ourPac->_pacmanSourceRect;
	delete _ourPac->_pacmanPosition;
	delete _ourPac;

	delete _wallTexture;

	// Delete Munchies
	delete _munchies[0]->_texture;
	for (int i = 0; i < size(_munchies); i++)
	{
		//delete _munchies[i]->_sourceRect; Moved to deconstructor
		//delete _munchies[i]->_rect;
		delete _munchies[i];
	}
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

	// Delete Sound Effects
	delete _pog;
	delete _nom;
	delete _death;

	// Delete Buttons
	delete _buttonTexture;
	delete _startGame;
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
	_pauseMenuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f - 88, Graphics::GetViewportHeight() / 2.0f);

	// Load Game Over Stuff
	_gameOverTexture = new Texture2D();
	_gameOverTexture->Load("Textures/GameOver.png", false);
	_gameOverSource = new Rect(0, 0, 64, 64);
	_gameOverPos = new Vector2(Graphics::GetViewportWidth() / 2 - 32, Graphics::GetViewportHeight() / 2 - 100);
	_gameOverStringPos = new Vector2(Graphics::GetViewportWidth() / 2 - 39, Graphics::GetViewportHeight() / 2);

	// Load Pacman
	_ourPac->_pacmanTexture = new Texture2D();
	_ourPac->_pacmanTexture->Load("Textures/Pacman.tga", false);
	_ourPac->_pacmanPosition = new Vector2(320.0f, 350.0f);
	_ourPac->_pacmanSourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Load Walls
	_wallTexture = new Texture2D();
	_wallTexture->Load("Textures/wall.png", false);
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

	// Load Munchies
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Kappachie.png", false);
	for (int i = 0; i < size(_munchies); i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->_frameCount = rand() % 20;
		_munchies[i]->_texture = munchieTex;
		_munchies[i]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
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

	// Load Cherry
	_cherry = new Enemy();
	_cherry->_texture = new Texture2D();
	_cherry->_texture->Load("Textures/Cherry.png", false);
	_cherry->_position = new Rect(-10, -10, 32, 32);
	_cherry->_sourceRect = new Rect(0, 0, 64, 64);

	// Load Ghosts
	_ghosts[0]->_texture = new Texture2D();
	_ghosts[0]->_texture->Load("Textures/GhostBlue.png", false);
	_ghosts[0]->_position = new Rect(rand() % Graphics::GetViewportWidth(), rand() % Graphics::GetViewportHeight(), 32, 32);
	_ghosts[0]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
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
	}

	_ghosts[1]->_texture = new Texture2D();
	_ghosts[1]->_texture->Load("Textures/GhostGrayscale.png", false);
	_ghosts[1]->_position = new Rect((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()), 32, 32);
	_ghosts[1]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
	{
		bool ok = false;
		while (!ok)
		{
			ok = true;
			for (int w = 0; w < WALLCOUNT; w++)
			{
				if (_walls[w] == nullptr) continue;
				ok = !CollisionCheck(_ghosts[1]->_position->X, _ghosts[1]->_position->Y, _ghosts[1]->_sourceRect->Width, _ghosts[1]->_sourceRect->Height,
					_walls[w]->_position->X, _walls[w]->_position->Y, _walls[w]->_sourceRect->Width, _walls[w]->_sourceRect->Height);
				if (!ok) break;
			}

			if (!ok)
			{
				_ghosts[1]->_position->X = rand() % (Graphics::GetViewportWidth());
				_ghosts[1]->_position->Y = rand() % (Graphics::GetViewportHeight());
			}
		}
	}

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
	_toMenu = new Button("Back", Graphics::GetViewportWidth() / 2 - 21, Graphics::GetViewportHeight() / 2 + 20, 42, 20, Color::Yellow);
}

bool wDown, aDown, sDown, dDown, moving, animate = false, trymove = false;

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();
	switch (state)
	{
	case Menu:
		InputMenu(elapsedTime, keyboardState, mouseState);
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
		if (!_ourPac->dead)
			UpdatePacman(elapsedTime);
		for (int i = 0; i < GHOSTCOUNT; i++)
			UpdateGhost(_ghosts[i], elapsedTime);
		CheckGhostCollisions();

		for (int i = 0; i < MUNCHIECOUNT; i++)
		{
			if (CollisionCheck(_ourPac->_pacmanPosition->X, _ourPac->_pacmanPosition->Y, _ourPac->_pacmanSourceRect->Width, _ourPac->_pacmanSourceRect->Height,
				_munchies[i]->_position->X, _munchies[i]->_position->Y, _munchies[i]->_sourceRect->Width, _munchies[i]->_sourceRect->Height)) {
				_munchies[i]->_position->X = -100;
				score += 100;
				Audio::Play(_nom);
			}
		}

		break;
	case Scores:
		break;
	}
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState)
{
	switch (state)
	{
	case Menu:
		InputMenu(elapsedTime, keyboardState, mouseState);
		break;
	case InGame:
		InputGame(elapsedTime, keyboardState, mouseState);
		break;
	case Scores:
		// TODO InputScores(elapsedTime, keyboardState, mouseState);
		break;
	}
}

void Pacman::InputGame(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState) {// Handle Mouse Input – Reposition Cherry 

	if (_ourPac->dead)
		return;

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
			_ourPac->dir = RIGHT;
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
			_ourPac->dir = LEFT;
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
			_ourPac->dir = UP;
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
			_ourPac->dir = DOWN;
			trymove = true;
		}
	}
	else
		sDown = false;
};

void Pacman::InputMenu(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState) {

	if (mouseState->LeftButton == Input::ButtonState::PRESSED) // Moves Cherry
	{
		if (CollisionCheck(mouseState->X, mouseState->Y, 1, 1,
			_startGame->buttonPos->X, _startGame->buttonPos->Y, _startGame->sourceRect->Width, _startGame->sourceRect->Height)) {
			state = InGame;
			_pog->SetLooping(true);
			_ourPac->dead = false;
		}
	}

	if (keyboardState->IsKeyDown(Input::Keys::SPACE))
	{
		state = InGame;
		_pog->SetLooping(true);
		_ourPac->dead = false;
	}

};

void Pacman::UpdatePacman(int elapsedTime)
{
	bool moved = false;
	if (trymove)
		switch (_ourPac->dir)
		{
		case UP:
			moved = MovePac(0, -(_ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime));
			break;
		case DOWN:
			moved = MovePac(0, _ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime);
			break;
		case LEFT:
			moved = MovePac(-(_ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime), 0);
			break;
		case RIGHT:
			moved = MovePac(_ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime, 0);
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
			_ourPac->_pacmanSourceRect->X = 0.0f;
			Audio::Stop(_pog);
			trymove = false;
		}
	}

	if (_ourPac->_pacmanPosition->X < -22)
		_ourPac->_pacmanPosition->X = 1017;
	if (_ourPac->_pacmanPosition->X > 1019)
		_ourPac->_pacmanPosition->X = -20;


	if (_ourPac->_pacmanPosition->Y < -22)
		_ourPac->_pacmanPosition->Y = 756;
	if (_ourPac->_pacmanPosition->Y > 758)
		_ourPac->_pacmanPosition->Y = -20;
}

bool Pacman::MovePac(int x, int y)
{
	int left1 = _ourPac->_pacmanPosition->X + x + 2;
	int right1 = _ourPac->_pacmanPosition->X + _ourPac->_pacmanSourceRect->Width + x - 4;
	int top1 = _ourPac->_pacmanPosition->Y + y + 4;
	int bottom1 = _ourPac->_pacmanPosition->Y + _ourPac->_pacmanSourceRect->Height + y - 2;
	int top2 = 0, bottom2 = 0, left2 = 0, right2 = 0;

	for (int i = 0; i < WALLCOUNT; i++)
	{
		bottom2 = _walls[i]->_position->Y + _walls[i]->_sourceRect->Height;
		left2 = _walls[i]->_position->X;
		right2 = _walls[i]->_position->X + _walls[i]->_sourceRect->Width;
		top2 = _walls[i]->_position->Y;

		if ((bottom1 > top2 && (top1 < bottom2) && (right1 > left2) && (left1 < right2)))
		{
			cout << "Stopped by" << endl;
			return false;
		}
	}

	_ourPac->_pacmanPosition->X += x;
	_ourPac->_pacmanPosition->Y += y;

	return true;
}

void Pacman::Draw(int elapsedTime)
{
	switch (state)
	{
	case Menu:
		DrawMenu(elapsedTime);
		break;
	case InGame:
		DrawGame(elapsedTime);
		break;
	case Scores:
		DrawScores(elapsedTime);
		break;
	}
}

void Pacman::DrawGame(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _ourPac->_pacmanPosition->X << " Y: " << _ourPac->_pacmanPosition->Y << endl << "Score: " << score;

	SpriteBatch::BeginDraw(); // Starts Drawing

	for (int i = 0; i < WALLCOUNT; i++)
		SpriteBatch::Draw(_wallTexture, _walls[i]->_position, _walls[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Wall

	for (int i = 0; i < MUNCHIECOUNT; i++)
	{
		_munchies[i]->_frameCount++;
		_munchies[i]->_sourceRect->X = (_munchies[i]->_frameCount % 10 >= 5 ? 32.0f : 0.0f); // Animates Munchie
		SpriteBatch::Draw(_munchies[i]->_texture, _munchies[i]->_position, _munchies[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Munchie
	}

	SpriteBatch::Draw(_cherry->_texture, _cherry->_position, _cherry->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Cherry

	for (int i = 0; i < GHOSTCOUNT; i++)
		SpriteBatch::Draw(_ghosts[i]->_texture, _ghosts[i]->_position, _ghosts[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, (_ghosts[i]->direction == 0 ? SpriteEffect::NONE : SpriteEffect::FLIPHORIZONTAL)); // Draws Ghost

	if (!_ourPac->dead)
	{
		_ourPac->_pacmanSourceRect->Y = getYPosFromDirection(_ourPac->dir);
		if (animate)
			_ourPac->_pacmanSourceRect->X = (_frameCount % 20 >= 10 ? 32.0f : 0.0f); // Animates Packman
		SpriteBatch::Draw(_ourPac->_pacmanTexture, _ourPac->_pacmanPosition, _ourPac->_pacmanSourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Pacman over Munchies
	}
	else
	{
		SpriteBatch::Draw(_gameOverTexture, _gameOverPos, _gameOverSource, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE);
		SpriteBatch::DrawString("You Died", _gameOverStringPos, Color::Red);
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

	std::stringstream menuStream;
	menuStream << "Click Start or press SPACE to start";
	//SpriteBatch::Draw(_menuBackground, _menuSpriteBounds, 10.0f, 0.0f);
	DrawButton(_startGame);
	SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	SpriteBatch::EndDraw(); // Ends Drawing
}

void Pacman::DrawScores(int elapsedTime) {
	SpriteBatch::BeginDraw(); // Starts Drawing
	std::stringstream menuStream;
	menuStream << "TODO";
	DrawButton(_toMenu);
	SpriteBatch::DrawString(menuStream.str().c_str(), _pauseMenuStringPosition, Color::Red);
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
	int bottom1 = _ourPac->_pacmanPosition->Y + _ourPac->_pacmanSourceRect->Height;
	int left1 = _ourPac->_pacmanPosition->X;
	int right1 = _ourPac->_pacmanPosition->X + _ourPac->_pacmanSourceRect->Width;
	int top1 = _ourPac->_pacmanPosition->Y;
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
			_ourPac->_pacmanPosition->X = -200;
			Audio::Stop(_pog);
			Audio::Play(_death);
			break;
		}
	}
}

void Pacman::UpdateGhost(MovingEnemy* ghost, int elapsedTime)
{
	if (ghost->direction == 0) // Moves Right
	{
		ghost->_position->X += ghost->speed * elapsedTime;
	}
	else if (ghost->direction == 1) // Moves Left
	{
		ghost->_position->X -= ghost->speed * elapsedTime;
	}

	if (ghost->_position->X + ghost->_sourceRect->Width >= Graphics::GetViewportWidth()) // Hits Right Edge
	{
		if (ghost->_position->X > Graphics::GetViewportWidth())
			ghost->_position->X = Graphics::GetViewportWidth();
		ghost->direction = 1;
	}
	else if (ghost->_position->X <= 0) // Hits Left Edge
	{
		if (ghost->_position->X < 0)
			ghost->_position->X = 0;
		ghost->direction = 0;
	}
}