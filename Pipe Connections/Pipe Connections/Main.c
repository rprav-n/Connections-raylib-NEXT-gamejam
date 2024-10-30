#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

int main()
{

	const int GAME_WIDTH = 128;
	const int GAME_HEIGHT = 128;

	const int CELL_SIZE = 16;
	const int SCALE_FACTOR = 4;

	const int WINDOW_WIDTH = GAME_WIDTH * SCALE_FACTOR;
	const int WINDOW_HEIGHT = GAME_HEIGHT * SCALE_FACTOR;

	const int TOTAL_COUNT = GAME_WIDTH / CELL_SIZE;
	const int SPACING = 2;
	const int REMAINING = TOTAL_COUNT - (SPACING * 2);

	const int START = SPACING * CELL_SIZE;
	const int END = REMAINING * CELL_SIZE;


	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pipe Connections");

	Texture2D atlasTexture = LoadTexture("..\\..\\pipe_test_tile.png");

	RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
	SetTextureFilter(renderTexture.texture, TEXTURE_FILTER_POINT);

	SetTargetFPS(60);

	// For testing
	Vector2 playerPos = (Vector2){ (float)START, (float)START };
	Rectangle source = (Rectangle){
		7.f * CELL_SIZE, 0.f,
		CELL_SIZE,
		CELL_SIZE
	};

	Rectangle dest = (Rectangle){
		(float)START + (CELL_SIZE / 2.f), (float)START + (CELL_SIZE / 2.f),
		CELL_SIZE,
		CELL_SIZE
	};

	float rotation = 0.f;

	while (!WindowShouldClose())
	{

		Vector2 mousePosition = GetMousePosition();
		Vector2 gridPosition = (Vector2){ floorf(mousePosition.x / (CELL_SIZE * SCALE_FACTOR)),
				floorf(mousePosition.y / (CELL_SIZE * SCALE_FACTOR)) };

		gridPosition.x = Clamp(gridPosition.x, SPACING, TOTAL_COUNT - SPACING - 1);
		gridPosition.y = Clamp(gridPosition.y, SPACING, TOTAL_COUNT - SPACING - 1);

		playerPos.x = gridPosition.x * CELL_SIZE;
		playerPos.y = gridPosition.y * CELL_SIZE;


		BeginTextureMode(renderTexture);
		ClearBackground(RAYWHITE);

		// Draw rectangle backgrounds
		for (int x = SPACING; x < TOTAL_COUNT - SPACING; x++)
		{
			for (int y = SPACING; y < TOTAL_COUNT - SPACING; y++)
			{
				if (y % 2 == 0)
				{
					if (x % 2 == 0)
					{
						DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, LIGHTGRAY);
					}
					else
					{
						DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, GRAY);
					}
				}
				else
				{
					if (x % 2 == 0)
					{
						DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, GRAY);
					}
					else
					{
						DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, LIGHTGRAY);
					}
				}
			}
		}

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			rotation += 90.f;
			if (rotation >= 360.f)
			{
				rotation = 0.f;
			}
		}

		DrawRectangleLines(START, START, END, END, GRAY);

		// Drawing one tile here for testing
		DrawTexturePro(atlasTexture, source, dest, (Vector2) { dest.width / 2.f, dest.height / 2.f }, rotation, WHITE);

		DrawRectangleLines(playerPos.x, playerPos.y, CELL_SIZE, CELL_SIZE, RED);

		EndTextureMode();


		BeginDrawing();
		ClearBackground(RAYWHITE);
		Rectangle source = { 0.0f, 0.0f, (float)renderTexture.texture.width, (float)-renderTexture.texture.height };
		Rectangle dest = { 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };

		DrawTexturePro(renderTexture.texture, source, dest, (Vector2) { 0, 0 }, 0.f, WHITE);

		EndDrawing();
	}

	CloseWindow();

	return 0;
}