#include <iostream>
#include <raylib.h>
#include <deque>
#include "raymath.h"


Color Green = { 190, 235, 70, 255 };
Color DarkGreen = { 58, 73, 17, 255 };

int cellSize = 30;
int cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;


bool ElementInDeque(Vector2 element, std::deque<Vector2> deque)
{
	for (unsigned int i = 0; i < deque.size(); i++)
	{
		if (Vector2Equals(deque[i], element))
		{
			return true;
		}
	}
	return false;
}


bool eventTriggered(double interval)
{
	double currentTime = GetTime();
	if (currentTime - lastUpdateTime >= interval)
	{
		lastUpdateTime = currentTime;
		return true;
	}
	return false;
}


class Snake 
{
public:
	std::deque <Vector2> body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
	Vector2 direction = { 1, 0 };

	bool addSegment = false;

	void Draw()
	{
		for (unsigned int i = 0; i < body.size(); i++)
		{
			float x = body[i].x;
			float y = body[i].y;
			Rectangle segments = { offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize };
			DrawRectangleRounded(segments, 0.5, 6, DarkGreen);
		}
	}

	void Update()
	{
		body.push_front(Vector2Add(body[0], direction));

		if (addSegment == true)
		{
			addSegment = false;
		}
		else
		{
			body.pop_back();
		}
	}

	void Reset()
	{
		body = { Vector2{6,9}, Vector2{5,9}, Vector2{4,9} };
		direction = { 1, 0 };
	}
};


class Food
{
public:
	Vector2 position;
	Texture2D texture;

	Food(std::deque<Vector2> snakeBody)
	{
		Image image = LoadImage("png/food.png");
		texture = LoadTextureFromImage(image);
		UnloadImage(image);
		position = GenerateRandomPos(snakeBody);
	}

	~Food()
	{
		UnloadTexture(texture);
	}

	void Draw()
	{
		DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
	}

	Vector2 GenerateRandomCell()
	{
		float x = GetRandomValue(0, cellCount - 1);
		float y = GetRandomValue(0, cellCount - 1);
		return Vector2{ x,y };
	}

	Vector2 GenerateRandomPos(std::deque<Vector2> snakeBody)
	{
		Vector2 position = GenerateRandomCell();
		while (ElementInDeque(position, snakeBody))
		{
			position = GenerateRandomCell();
		}
		return position;
	}
};


class Game
{
public:
	Snake snake = Snake();
	Food food = Food(snake.body);

	bool running = true;
	int score = 0;

	Sound eatSound;
	Sound wallHitSound;

	Game()
	{
		InitAudioDevice();
		eatSound = LoadSound("Sounds/food.mp3");
		wallHitSound = LoadSound("Sounds/hurt.mp3");
	}

	~Game()
	{
		UnloadSound(eatSound);
		UnloadSound(wallHitSound);
	}

	void Draw()
	{
		snake.Draw();
		food.Draw();
	}

	void Update()
	{
		if (running == true)
		{
			snake.Update();
			CheckCollisionWithFood();
			CheckCollisionWithEdges();
			CheckCollisionWithTail();
		}	
	}

	void CheckCollisionWithFood()
	{
		if (Vector2Equals(snake.body[0], food.position))
		{
			food.position = food.GenerateRandomPos(snake.body);
			snake.addSegment = true;
			PlaySound(eatSound);
			score++;
		}
	}

	void CheckCollisionWithEdges()
	{
		if (snake.body[0].x == cellCount || snake.body[0].x == -1)
		{
			GameOver();
		}
		if (snake.body[0].y == cellCount || snake.body[0].y == -1)
		{
			GameOver();
		}
	}

	void CheckCollisionWithTail()
	{
		std::deque<Vector2> headlessBody = snake.body;
		headlessBody.pop_front();

		if (ElementInDeque(snake.body[0], headlessBody))
		{
			GameOver();
		}
	}

	void GameOver()
	{
		snake.Reset();
		food.position = food.GenerateRandomPos(snake.body);
		running = false;
		PlaySound(wallHitSound);
		score = 0;
	}
};



int main()
{
	InitWindow(2 * offset + cellSize*cellCount, 2 * offset + cellSize * cellCount, "Retro Snake Game");
	SetTargetFPS(60);

	Game game = Game();

	while (WindowShouldClose() == false)
	{
		BeginDrawing();


		if (eventTriggered(0.2))
		{
			game.Update();
		}
		
		if ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && game.snake.direction.y != 1)
		{
			game.snake.direction = { 0, -1 };
			game.running = true;
		}
		if ((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) && game.snake.direction.y != -1)
		{
			game.snake.direction = { 0, 1 };
			game.running = true;
		}
		if ((IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) && game.snake.direction.x != 1)
		{
			game.snake.direction = { -1, 0 };
			game.running = true;
		}
		if ((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) && game.snake.direction.x != -1)
		{
			game.snake.direction = { 1, 0 };
			game.running = true;
		}


		//Drawing
		ClearBackground(Green);
		DrawRectangleLinesEx(Rectangle{ (float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10 }, 5, DarkGreen);
		DrawText("Retro Snake", offset - 5, 20, 40, DarkGreen);
		DrawText(TextFormat("%i", game.score), offset - 5, offset + cellSize * cellCount + 10, 40, DarkGreen);
		game.Draw();

		EndDrawing();
	}

	CloseWindow();
	return 0;
}