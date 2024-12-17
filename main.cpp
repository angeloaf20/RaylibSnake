#include <raylib.h>
#include <raymath.h>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>

enum GameState
{
	START,
	PLAY,
	END,
};

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
constexpr int CELL_SIZE = 50;

double lastTime = GetTime();

static bool LimitTime(double interval)
{
	double currentTime = GetTime();
	double delta = currentTime - lastTime;

	if (delta >= interval)
	{
		lastTime = currentTime;
		return true;
	}
	return false;
}

static void HandleInput(Vector2& movementVector)
{
	if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) && movementVector.y != 1)
		movementVector = { 0, -1 };
	if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN) && movementVector.y != -1)
		movementVector = { 0, 1 };
	if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) && movementVector.x != 1)
		movementVector = { -1, 0 };
	if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) && movementVector.x != -1)
		movementVector = { 1, 0 };
}

static bool SnakeContainsPosition(Vector2& position, std::deque<Vector2> snake)
{
	for (Vector2 pos : snake)
	{
		if (Vector2Equals(pos, position))
		{
			return true;
		}
	}

	return false;
}

static void AssignFoodPosition(Vector2& position, std::deque<Vector2> snake)
{
	do
	{
		position.x = GetRandomValue(0, 9);
		position.y = GetRandomValue(0, 9);
	} while (SnakeContainsPosition(position, snake));
}

static void DrawFood(Vector2 foodPosition, Rectangle background)
{
	Rectangle food{};
	food.x = background.x + foodPosition.x * CELL_SIZE;
	food.y = background.y + foodPosition.y * CELL_SIZE;
	food.width = CELL_SIZE;
	food.height = CELL_SIZE;
	DrawRectangleRounded(food, 0.5, 5, DARKGREEN);
}

static void DrawSnake(std::deque<Vector2> snake, Rectangle background, Color snakeGreen)
{
	for (Vector2 position : snake)
	{
		float x = position.x;
		float y = position.y;
		float offsetX = background.x;
		float offsetY = background.y;

		Rectangle snakePart{
			offsetX + x * CELL_SIZE,
			offsetY + y * CELL_SIZE,
			(float)CELL_SIZE,
			(float)CELL_SIZE
		};

		DrawRectangleRounded(snakePart, 0.5f, 5, snakeGreen);
	}
}

static void UpdateSnake(std::deque<Vector2>& snake, Vector2 movementVector, bool addToBody)
{
	Vector2 newPos = Vector2Add(snake[0], movementVector);
	snake.push_front(newPos);
	if (!addToBody)
	{
		snake.pop_back();
	}
}

static void CheckSnakeHitSelf(std::deque<Vector2> snake, bool& gameRunning)
{
	for (size_t index = 1; index < snake.size(); index++)
	{
		if (Vector2Equals(snake[0], snake[index]))
			gameRunning = false;
	}
}

static void CheckSnakeHitWalls(std::deque<Vector2> snake, bool& gameRunning)
{
	// using "non-converted" positions (as in, no offset or cell size taken into account)
	if (snake[0].x < 0 || snake[0].x > 9 || snake[0].y < 0 || snake[0].y > 9)
		gameRunning = false;
}

static void CheckSnakeAteFood(std::deque<Vector2> snake, Vector2& foodPosition, int& score, bool& addToBody)
{
	if (Vector2Equals(snake[0], foodPosition))
	{
		addToBody = true;
		score++;
		AssignFoodPosition(foodPosition, snake);
	}
}

static void ResetSnake(std::deque<Vector2>& snake, Vector2& movementVector)
{
	snake.clear();
	movementVector = { 1, 0 };
	snake = { Vector2{6, 2}, Vector2{5, 2}, Vector2{4, 2} };
}

static void SaveScore(int& highScore)
{
	std::ofstream saveFile("save.txt");

	if (saveFile.is_open())
	{

		saveFile << highScore;
	}

	saveFile.close();
}

static void GetHighScore(int& highScore)
{
	std::string num{};
	std::ifstream saveFile("save.txt");

	while (getline(saveFile, num))
	{
		highScore = atoi(num.c_str());
	}

	saveFile.close();
}

int main()
{
	SetTargetFPS(50);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World");

	GameState state = START;
	bool gameRunning = true;
	bool checkedSaveFile = false;
	int highScore = 0;
	int score = 0;

	// BACKGROUND
	Rectangle background{};
	background.width = 500;
	background.height = 500;
	background.x = (SCREEN_WIDTH - background.width) / 2;
	background.y = (SCREEN_HEIGHT - background.height) / 2;
	Color backgroundGreen{ 217, 255, 102 , 255 };
	// BACKGROUND

	// SNAKE
	Color snakeGreen{ 68, 102, 0, 255 };
	std::deque<Vector2> snake = { Vector2{6, 2}, Vector2{5, 2}, Vector2{4, 2} };
	Vector2 movementVector = { 1, 0 };
	// SNAKE

	// FOOD
	Color foodColor{ 102, 153, 0, 0 };
	Vector2 foodPosition{};
	AssignFoodPosition(foodPosition, snake);
	// FOOD

	while (!WindowShouldClose())
	{
		switch (state)
		{
			case START:
			{
				if (IsKeyPressed(KEY_SPACE))
				{
					state = PLAY;
					score = 0;
					gameRunning = true;
					checkedSaveFile = false;
				}

				if (!checkedSaveFile)
				{
					checkedSaveFile = true;
					GetHighScore(highScore);
				}

				BeginDrawing();
				ClearBackground(backgroundGreen);
				DrawText("Press SPACE to start!", 50, (SCREEN_HEIGHT - 100) / 2, 100, snakeGreen);
				EndDrawing();
				break;
			}
			case PLAY:
			{
				if (!gameRunning)
				{
					state = END;
				}

				bool addToBody = false;

				HandleInput(movementVector);

				if (LimitTime(0.2) && gameRunning)
				{
					CheckSnakeHitSelf(snake, gameRunning);
					CheckSnakeHitWalls(snake, gameRunning);
					CheckSnakeAteFood(snake, foodPosition, score, addToBody);
					UpdateSnake(snake, movementVector, addToBody);
				}

				BeginDrawing();

				ClearBackground(backgroundGreen);
				DrawRectangleLinesEx(background, 4.5f, snakeGreen);
				DrawFood(foodPosition, background);
				DrawSnake(snake, background, snakeGreen);

				DrawText("Snake in Raylib", background.x, 50, 25, snakeGreen);
				DrawText(TextFormat("Score: %i", score), background.x + 300, 50, 25, snakeGreen);

				EndDrawing();
				break;
			}
			case END:
			{
				if (IsKeyPressed(KEY_Y))
				{
					SaveScore(score);
					state = START;
				}

				if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_N))
				{
					state = START;
				}

				ResetSnake(snake, movementVector);

				BeginDrawing();
				ClearBackground(backgroundGreen);

				if (score < 80)
				{
					DrawText("You lost!", 420, (SCREEN_HEIGHT - 150) / 2, 100, snakeGreen);
				}
				else
				{
					DrawText("Wow, you won!", 420, (SCREEN_HEIGHT - 150) / 2, 100, snakeGreen);
				}

				DrawText("Press SPACE to go back!", 120, (SCREEN_HEIGHT - 350) / 2, 80, snakeGreen);
				
				DrawText(TextFormat("Highest score: %d", highScore), 400, (SCREEN_HEIGHT + 80) / 2, 65, snakeGreen);
				DrawText("Save game? Press Y for yes, N for no", 150, (SCREEN_HEIGHT + 300) / 2, 50, snakeGreen);

				EndDrawing();
				break;
			}
		}
	}
}