#include "Pacman.h"
#include <time.h>
#include <sstream>

int score = 0;

Pacman::Pacman(int argc, char* argv[]) : Game(argc, argv)
{
	srand(time(NULL));
	_ourPac = new PacCharacter();
	_ourPac->dead = false;
	_frameCount = 0;
	_paused = false;
	_atMenu = true;

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
}

void Pacman::LoadContent()
{
	// Load Menu
	_menuBackground = new Texture2D();
	_menuBackground->Load("Textures/PacChamp.png", false);
	_menuSpriteBounds = new Rect(0.0f, 0.0f, 32.0f, 32.0f);
	_menuStringPosition = new Vector2(Graphics::GetViewportWidth() / 2.0f - 88, Graphics::GetViewportHeight() / 2.0f);

	// Load Game Over Stuff
	_gameOverTexture = new Texture2D();
	_gameOverTexture->Load("Textures/GameOver.png", false);
	_gameOverSource = new Rect(0, 0, 64, 64);
	_gameOverPos = new Vector2(Graphics::GetViewportWidth() / 2 - 32, Graphics::GetViewportHeight() / 2 - 100);
	_gameOverStringPos = new Vector2(Graphics::GetViewportWidth() / 2 - 39, Graphics::GetViewportHeight() / 2);

	// Load Pacman
	_ourPac->_pacmanTexture = new Texture2D();
	_ourPac->_pacmanTexture->Load("Textures/Pacman.tga", false);
	_ourPac->_pacmanPosition = new Vector2(350.0f, 350.0f);
	_ourPac->_pacmanSourceRect = new Rect(32.0f, 0.0f, 32, 32);

	// Load Munchies
	Texture2D* munchieTex = new Texture2D();
	munchieTex->Load("Textures/Kappachie.png", false);
	for (int i = 0; i < size(_munchies); i++)
	{
		_munchies[i] = new Enemy();
		_munchies[i]->_texture = munchieTex;
		_munchies[i]->_position = new Rect(rand() % (Graphics::GetViewportWidth() - 32), rand() % (Graphics::GetViewportHeight() - 32), 32, 32);
		_munchies[i]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);
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
	_ghosts[0]->_position = new Rect((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()), 32, 32);
	_ghosts[0]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	_ghosts[1]->_texture = new Texture2D();
	_ghosts[1]->_texture->Load("Textures/GhostGrayscale.png", false);
	_ghosts[1]->_position = new Rect((rand() % Graphics::GetViewportWidth()), (rand() % Graphics::GetViewportHeight()), 32, 32);
	_ghosts[1]->_sourceRect = new Rect(0.0f, 0.0f, 32, 32);

	// Set string position
	_debugStringPosition = new Vector2(10.0f, 25.0f);

	// Load Sound Effects
	_pog = new SoundEffect();
	_pog->Load("Sounds/pog.wav");
	_nom = new SoundEffect();
	_nom->Load("Sounds/nom.wav");
}

bool wDown, aDown, sDown, dDown;
int vecX, vecY;

void Pacman::Update(int elapsedTime)
{
	// Gets the current state of the keyboard
	Input::KeyboardState* keyboardState = Input::Keyboard::GetState();
	//Gets the current state of the mouse
	Input::MouseState* mouseState = Input::Mouse::GetState();

	if (_atMenu)
	{
		if (keyboardState->IsKeyDown(Input::Keys::SPACE))
		{
			_atMenu = false;
			_pog->SetLooping(true);
			Audio::Play(_pog);
		}
		return;
	}
	else {
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
	}

	if (_paused) return;

	_frameCount++;

	Input(elapsedTime, keyboardState, mouseState);
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
}

void Pacman::Input(int elapsedTime, Input::KeyboardState* keyboardState, Input::MouseState* mouseState)
{
	// Handle Mouse Input – Reposition Cherry 
	if (!_ourPac->dead)
		if (mouseState->LeftButton == Input::ButtonState::PRESSED)
		{
			_cherry->_position->X = mouseState->X;
			_cherry->_position->Y = mouseState->Y;
		}

	if (keyboardState->IsKeyDown(Input::Keys::LEFTSHIFT))
		_ourPac->_speedMulti = 2.0;
	else
		_ourPac->_speedMulti = 1.0;

	if (keyboardState->IsKeyDown(Input::Keys::D)) // Checks if D key is pressed
	{
		if (!dDown)
		{
			dDown = true;
			_ourPac->dir = RIGHT;
			vecX = 1;
			vecY = 0;
		}
	}
	else
		dDown = false;

	if (keyboardState->IsKeyDown(Input::Keys::A))
	{
		if (!aDown)
		{
			aDown = true;
			_ourPac->dir = LEFT;
			vecX = -1;
			vecY = 0;
		}
	}
	else
		aDown = false;

	if (keyboardState->IsKeyDown(Input::Keys::W))
	{
		if (!wDown)
		{
			wDown = true;
			_ourPac->dir = UP;
			vecX = 0;
			vecY = -1;
		}
	}
	else
		wDown = false;

	if (keyboardState->IsKeyDown(Input::Keys::S))
	{
		if (!sDown)
		{
			sDown = true;
			_ourPac->dir = DOWN;
			vecX = 0;
			vecY = 1;
		}
	}
	else
		sDown = false;
}

void Pacman::UpdatePacman(int elapsedTime)
{
	if (vecX == 1)
		_ourPac->_pacmanPosition->X += _ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime;
	if (vecX == -1)
		_ourPac->_pacmanPosition->X -= _ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime;
	if (vecY == 1)
		_ourPac->_pacmanPosition->Y += _ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime;
	if (vecY == -1)
		_ourPac->_pacmanPosition->Y -= _ourPac->_speedMulti * _ourPac->_pacmanSpeed * elapsedTime;

	if (_ourPac->_pacmanPosition->X < -22)
		_ourPac->_pacmanPosition->X = 1017;
	if (_ourPac->_pacmanPosition->X > 1019)
		_ourPac->_pacmanPosition->X = -20;


	if (_ourPac->_pacmanPosition->Y < -22)
		_ourPac->_pacmanPosition->Y = 756;
	if (_ourPac->_pacmanPosition->Y > 758)
		_ourPac->_pacmanPosition->Y = -20;
}

void Pacman::Draw(int elapsedTime)
{
	// Allows us to easily create a string
	std::stringstream stream;
	stream << "Pacman X: " << _ourPac->_pacmanPosition->X << " Y: " << _ourPac->_pacmanPosition->Y << endl << "Score: " << score;

	SpriteBatch::BeginDraw(); // Starts Drawing

	if (_atMenu)
	{
		std::stringstream menuStream;
		menuStream << "Press SPACE to start";
		//SpriteBatch::Draw(_menuBackground, _menuSpriteBounds, 10.0f, 0.0f);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
		SpriteBatch::EndDraw(); // Ends Drawing
		return;
	}

	for (int i = 0; i < size(_munchies); i++)
	{
		_munchies[i]->_sourceRect->X = (_frameCount % 10 >= 5 ? 32.0f : 0.0f); // Animates Munchie
		SpriteBatch::Draw(_munchies[i]->_texture, _munchies[i]->_position, _munchies[i]->_sourceRect, Vector2::Zero, 0.75f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Munchie
	}

	SpriteBatch::Draw(_cherry->_texture, _cherry->_position, _cherry->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, SpriteEffect::NONE); // Draws Cherry

	for (int i = 0; i < GHOSTCOUNT; i++)
		SpriteBatch::Draw(_ghosts[i]->_texture, _ghosts[i]->_position, _ghosts[i]->_sourceRect, Vector2::Zero, 1.0f, 0.0f, Color::White, (_ghosts[i]->direction == 0 ? SpriteEffect::NONE : SpriteEffect::FLIPHORIZONTAL)); // Draws Ghost

	if (!_ourPac->dead)
	{
		_ourPac->_pacmanSourceRect->Y = getYPosFromDirection(_ourPac->dir);
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
		//SpriteBatch::Draw(_menuBackground, _menuSpriteBounds, 10.0f, 0.0f);
		SpriteBatch::DrawString(menuStream.str().c_str(), _menuStringPosition, Color::Red);
	}

	// Draws String
	SpriteBatch::DrawString(stream.str().c_str(), _debugStringPosition, Color::Green);
	SpriteBatch::EndDraw(); // Ends Drawing
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
		ghost->direction = 1;
	}
	else if (ghost->_position->X <= 0) // Hits Right Edge
	{
		ghost->direction = 0;
	}
}