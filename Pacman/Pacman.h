#pragma once

// If Windows and not in Debug, this will run without a console window
// You can use this to output information when debugging using cout or cerr
#ifdef WIN32 
#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif
#define MUNCHIECOUNT 40
#define GHOSTCOUNT 4
#define WALLCOUNT 46
#define SEQUENCECOUNT 12
#define PATROLSEQUENCECOUNT 20
// Just need to include main header file
#include "S2D/S2D.h"

// Reduces the amount of typing by including all classes in S2D namespace
using namespace S2D;

// Declares the Pacman class which inherits from the Game class.
// This allows us to overload the Game class methods to help us
// load content, draw and update our game.
class Pacman : public Game
{
public:
	/// <summary> Constructs the Pacman class. </summary>
	Pacman(int argc, char* argv[]);

	/// <summary> Destroys any data associated with Pacman class. </summary>
	virtual ~Pacman();

	/// <summary> All content should be loaded in this method. </summary>
	void virtual LoadContent();

	/// <summary> Called every frame - update game logic here. </summary>
	void virtual Update(int elapsedTime);

	/// <summary> Called every frame - draw game here. </summary>
	void virtual Draw(int elapsedTime);
	void virtual DrawGame(int elapsedTime);
	void virtual DrawMenu(int elapsedTime);
	void virtual DrawScores(int elapsedTime);

	enum Direction {
		UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3
	};

	float getRotationFromDirection(Direction dir) // TODO Messes up sprite drawn position, make spritesheet instead
	{
		switch (dir)
		{
		case UP:
		case DOWN:
			return 90.0f;
			break;
		case LEFT:
		case RIGHT:
			return 0.0f;
			break;
		}
	}

	float getYPosFromDirection(Direction dir)
	{
		switch (dir)
		{
		case RIGHT:
			return 0.0f;
		case DOWN:
			return 32.0f;
		case LEFT:
			return 64.0f;
		case UP:
			return 96.0f;
		}
	}
private:
	//Input methods
	void Input(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void InputGame(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void InputMenu(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);
	void InputScoresMenu(int elapsedTime, Input::KeyboardState* state, Input::MouseState* mouseState);

	//Check methods
	//void CheckViewportCollision();
	bool CollisionCheck(int x1, int y1, int width1, int height1, int x2, int y2, int width2, int height2);

	//Update methods
	void UpdatePacman(int elapsedTime);
	void ResetMap();
	//void UpdateMunchie(int elapsedTime);

	// Menu
	Texture2D* _menuBackground;
	Rect* _menuSpriteBounds;
	Vector2* _menuStringPosition;
	Vector2* _pauseMenuStringPosition;
	Vector2* _scoresMenuStringPosition;
	bool _paused, _pKeyDown;

	void UpdateMenuPacman();
	struct MovementSequence
	{
		MovementSequence* next;
		Direction dir;
		Rect* posAndSize;

		MovementSequence(Direction dir, Rect* posAndSize)
		{
			this->dir = dir;
			this->posAndSize = posAndSize;
		}

		~MovementSequence()
		{
			delete posAndSize;
		}
	};
	MovementSequence* _sequence[SEQUENCECOUNT];
	bool Pacman::HasReachedSequence(Vector2* pos, Rect* source, MovementSequence* seq);

	// Data to represent Pacman
	struct PacCharacter {
		float _speed = 4.0f;
		float _speedMulti = 1.0f;
		Direction _direction = RIGHT;
		int frame = 0;
		Rect* _sourceRect;
		Texture2D* _texture;
		Vector2* _position;
		bool dead, super;
	};

	PacCharacter* _ourPac;
	bool MovePac(int x, int y);

	int _frameCount;

	// Data to represent Munchie
	struct Enemy {
		int _frameCount;
		Rect* _position;
		Rect* _sourceRect;
		Texture2D* _texture;

		~Enemy() {
			delete _position;
			delete _sourceRect;
		}
	};

	Enemy* _munchies[MUNCHIECOUNT];
	Enemy* _cherry;
	bool cherryEaten;
	int superFrames;

	struct MovingEnemy {
		int _id;
		Vector2* _position;
		Rect* _sourceRect;
		Texture2D* _texture;
		Direction _direction;
		double _speed = 4.0f;
		int _extraData = 0;

		~MovingEnemy() {
			delete _position;
			delete _sourceRect;
			delete _texture;
		}
	};

	MovingEnemy* _ghosts[GHOSTCOUNT];
	MovementSequence* _patrolSequence[PATROLSEQUENCECOUNT];
	Texture2D* _sadTex;
	void CheckGhostCollisions();
	void UpdateGhost(MovingEnemy* ghost, int elapsedTime);
	bool CanMoveGhost(MovingEnemy* ghost);

	// Position for String
	Vector2* _debugStringPosition;

	Texture2D* _gameOverTexture;
	Texture2D* _gameWonTexture;
	Vector2* _gameOverPos;
	Vector2* _gameOverStringPos;
	Vector2* _gameOverNamePos;
	Rect* _gameOverSource;

	SoundEffect* _pog;
	SoundEffect* _nom;
	SoundEffect* _death;

	struct Button {
		string text;
		Vector2* textPos;
		Vector2* buttonPos;
		Rect* sourceRect;
		const S2D::Color* col;

		Button(string display, int x, int y, int width, int height, const S2D::Color* colour)
		{
			text = display;
			sourceRect = new Rect(0, 0, width, height);
			buttonPos = new Vector2(x, y-height + (height/6));
			textPos = new Vector2(x, y);
			col = colour;
		}

		~Button() {
			delete textPos;
			delete buttonPos;
			delete sourceRect;
		}
	};
	Texture2D* _buttonTexture;
	void DrawButton(Button* button);

	Button* _startGame;
	Button* _scoreScreen;
	Button* _toMenu;

	struct Wall {
		Vector2* _position;
		Rect* _sourceRect;

		Wall(int x, int y, int width, int height)
		{
			_sourceRect = new Rect(0, 0, width, height);
			_position = new Vector2(x, y);
		}

		~Wall() {
			delete _position;
			delete _sourceRect;
		}
	};
	Texture2D* _wallTexture;
	Wall* _walls[WALLCOUNT];

	struct Score {
		int score;
		string name;
	};
	Score scores[10];
	int scoresInFile = 0;

	int IsScoreInTop10(int score);
	void SaveScores();
};