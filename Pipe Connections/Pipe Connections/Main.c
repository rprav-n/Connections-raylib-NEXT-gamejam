//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define GAME_WIDTH 128.f
#define GAME_HEIGHT 128.f
#define SCALE_FACTOR 6.f
#define CELL_SIZE 16.f
#define WINDOW_WIDTH (SCALE_FACTOR * GAME_WIDTH)
#define WINDOW_HEIGHT (SCALE_FACTOR * GAME_HEIGHT)
#define FPS 60
#define TOTAL_COUNT 8
#define SPACING 2
#define ROWS 4
#define COLS 4
#define BOX_COUNT 16
#define START (CELL_SIZE * SPACING)
#define END (BOX_COUNT * ROWS)
#define TOTAL_PUZZLES 1
#define FIRE_MOVE_SPEED 8.f

enum Direction
{
	TOP,
	LEFT,
	RIGHT,
	BOTTOM
};

struct Player
{
	Vector2 pos;
};

struct Box
{
	int id;
	int index;
	float x;
	float y;
	bool isMain;
	bool isWaterConnected;
	int rotation;

	bool isLeftOpen;
	bool isTopOpen;
	bool isRightOpen;
	bool isBottomOpen;

	Vector2 gridPos;
	Vector2 size;
	Rectangle source;
	Rectangle dest;
	Vector2 origin;

	enum Direction direction;

};

struct Puzzle
{
	int puzzleGrid[ROWS][ROWS];
	int answerGrid[ROWS][ROWS];
	bool isCorrect;
	bool isLost;
};


void RotateBox(struct Box* box);
int GetBoxIndexByPos(struct Box boxes[BOX_COUNT], Vector2 pos);
void CheckForAdjacentBox(struct Box boxes[BOX_COUNT], struct Box* box, Vector2 pos, Vector2 visted[BOX_COUNT]);


int main()
{

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pipe Connections");
	InitAudioDevice();

	Texture2D atlasTexture = LoadTexture("..\\..\\atlas.png");
	Texture2D bricksTexture = LoadTexture("..\\..\\bricks.png");

	RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
	SetTextureFilter(renderTexture.texture, TEXTURE_FILTER_POINT);

	Font mxFont = LoadFont("..\\..\\m6x11plus.ttf");

	Music fireMusic = LoadMusicStream("..\\..\\flame.mp3");
	PlayMusicStream(fireMusic);

	Shader fireShader = LoadShader(NULL, "fire.fs");
	Shader crtShader = LoadShader(NULL, "crt.fs");
	Texture2D noiseTexture = LoadTexture("..\\..\\noise.png");

	// { 1.0f, 0.25f, 0.25f } valve red color = #ff4242
	// { 1.0f, 0.5f, 0.0f } = orange colr = #ff8000
	float orangeColor[3] = { 1.0f, 0.5f, 0.0f };

	SetShaderValue(fireShader, GetShaderLocation(fireShader, "flameColor"), orangeColor, SHADER_UNIFORM_VEC3);
	SetShaderValue(fireShader, GetShaderLocation(fireShader, "animationSpeed"), (float[1]) { 0.5f }, SHADER_UNIFORM_FLOAT);

	Rectangle fireSource = (Rectangle){
		.x = 0.f, 
		.y = 32.f,
		.width = GAME_WIDTH,
		.height = GAME_HEIGHT,
	};

	Rectangle fireDest = (Rectangle){
		.x = 0.f,
		.y = GAME_HEIGHT,
		.width = GAME_WIDTH,
		.height = GAME_HEIGHT,
	};

	struct Player player = {
		.pos = (Vector2){ (float)START, (float)START }
	};
	
	struct Puzzle puzzles[TOTAL_PUZZLES];

	int currentPuzzleIndex = 0;
	puzzles[0] = (struct Puzzle){
		.puzzleGrid = {
			{3, 1, 2, 1},
			{7, 12, 5, 5},
			{12, 8, 10, 7},
			{3, 12, 4, 11},
		},
		.answerGrid = {
			{0, 3, 0, 1},
			{3, 1, 2, 1},
			{2, 3, 3, 3},
			{3, 3, 0, 3}
		},
		.isCorrect = false,
	};

	struct Box boxes[BOX_COUNT];

	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			int id = puzzles[currentPuzzleIndex].puzzleGrid[i][j];
			int index = (i * ROWS) + j;

			struct Box box = {
				.isLeftOpen = (id == 3) || (id == 5) || (id == 7) || (id == 8) || (id == 10) || (id == 11) || (id == 13),
				.isRightOpen = (id == 1) || (id == 5) || (id == 7) || (id == 8) || (id == 9) || (id == 12) || (id == 14),
				.isTopOpen = (id == 4) || (id == 6) || (id == 7) || (id == 9) || (id == 10) || (id == 13) || (id == 14),
				.isBottomOpen = (id == 2) || (id == 6) || (id == 8) || (id == 9) || (id == 10) || (id == 11) || (id == 12),
				.index = index,
				.x = j,
				.y = i,
				.isMain = id == 8,
				.isWaterConnected = id == 8,
				.rotation = 0,
				.size = (Vector2){CELL_SIZE, CELL_SIZE},
				.source = (Rectangle){
						(id - 1) * CELL_SIZE, 0.f,
						CELL_SIZE,
						CELL_SIZE
					},
				.direction = RIGHT,
			};

			boxes[index] = box;
		}
	}

	float time = 0.0f;
	float fireYoffset = 1.2f;
	char str[5];
	

	SetTargetFPS(FPS);
	while (!WindowShouldClose())
	{

		float dt = GetFrameTime();
		time += dt;

		SetShaderValue(crtShader, GetShaderLocation(crtShader, "texture0"), &renderTexture, SHADER_UNIFORM_FLOAT);
		SetShaderValue(crtShader, GetShaderLocation(crtShader, "scanline_count"), (float[1]){ 0.f }, SHADER_UNIFORM_FLOAT);

		if (!puzzles[currentPuzzleIndex].isCorrect && !puzzles[currentPuzzleIndex].isLost)
		{
			UpdateMusicStream(fireMusic);

			fireYoffset = Vector2MoveTowards((Vector2) { 0.f, fireYoffset }, (Vector2) { 0.f, -0.0f }, dt * 0.05f).y;
			
			SetShaderValue(fireShader, GetShaderLocation(fireShader, "time"), &time, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fireShader, GetShaderLocation(fireShader, "yOffset"), (float[1]) { fireYoffset }, SHADER_UNIFORM_FLOAT);


			if (fireYoffset <= 0.0f)
			{
				// TODO Lost
				puzzles[currentPuzzleIndex].isLost = true;
			}
		}


		// Set visited to -1
		Vector2 visited[BOX_COUNT];
		for (int i = 0; i < BOX_COUNT; i++)
		{
			visited[i] = (Vector2){ -1, -1 };
		}

		// Update mouse + player movement
		Vector2 mousePosition = GetMousePosition();
		Vector2 gridPosition = (Vector2){ floorf(mousePosition.x / (CELL_SIZE * SCALE_FACTOR)),
				floorf(mousePosition.y / (CELL_SIZE * SCALE_FACTOR)) };

		gridPosition.x = Clamp(gridPosition.x, SPACING, TOTAL_COUNT - SPACING - 1);
		gridPosition.y = Clamp(gridPosition.y, SPACING, TOTAL_COUNT - SPACING - 1);

		player.pos.x = gridPosition.x * CELL_SIZE;
		player.pos.y = gridPosition.y * CELL_SIZE;

		// Update box
		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < ROWS; j++)
			{
				int index = GetBoxIndexByPos(boxes, (Vector2) { i, j });

				boxes[index].dest = (Rectangle){
					((i + SPACING) * CELL_SIZE) + (CELL_SIZE / 2.f), ((j + SPACING) * CELL_SIZE) + (CELL_SIZE / 2.f),
					CELL_SIZE,
					CELL_SIZE
				};

				boxes[index].origin = (Vector2){ boxes[index].dest.width / 2.f, boxes[index].dest.height / 2.f };

				if (boxes[index].x == gridPosition.x - 2 && boxes[index].y == gridPosition.y - 2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) 
					&& !puzzles[currentPuzzleIndex].isCorrect)
				{
					RotateBox(&boxes[index]);
				}
			}
		}

		// Set all water connection to false
		for (int i = 0; i < BOX_COUNT; i++)
		{
			if (!boxes[i].isMain)
			{
				boxes[i].isWaterConnected = false;
			}
		}

		// Update water present in the pipes
		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < ROWS; j++)
			{
				int index = GetBoxIndexByPos(boxes, (Vector2) { i, j });
				if (boxes[index].isMain)
				{
					CheckForAdjacentBox(boxes, &boxes[index], (Vector2) { i, j }, visited);
					break;
				}
			}
		}

		// Validate answer
		puzzles[currentPuzzleIndex].isCorrect = true;
		for (int i = 0; i < BOX_COUNT; i++)
		{
			if (!boxes[i].isWaterConnected)
			{
				puzzles[currentPuzzleIndex].isCorrect = false;
			}
		}

		BeginTextureMode(renderTexture);
		ClearBackground(RAYWHITE);

		// Draw brickS
		DrawTexture(bricksTexture, 0, 0, WHITE);

		// Draw fire
		BeginShaderMode(fireShader);
		DrawTexture(noiseTexture, 0, 0, WHITE);
		EndShaderMode();

		// Draw sprites
		for (int i = 0; i < ROWS; i++)
		{
			for (int j = 0; j < ROWS; j++)
			{
				int index = (i * ROWS) + j;

				if (boxes[index].isMain || boxes[index].isWaterConnected)
				{
					boxes[index].source.y = 16.f;
				}
				else
				{
					boxes[index].source.y = 0.f;
				}
				DrawTexturePro(atlasTexture, boxes[index].source, boxes[index].dest, boxes[index].origin, boxes[index].rotation, WHITE);
			}
		}

		// Draw player
		DrawRectangleLines(player.pos.x, player.pos.y, CELL_SIZE, CELL_SIZE, WHITE);

		EndTextureMode();

		
		BeginDrawing();
		ClearBackground(RAYWHITE);
		//BeginShaderMode(crtShader);
		Rectangle source = { 0.0f, 0.0f, (float)renderTexture.texture.width, (float)-renderTexture.texture.height };
		Rectangle dest = { 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };

		DrawTexturePro(renderTexture.texture, source, dest, (Vector2) { 0, 0 }, 0.f, WHITE);

		// Draw player won
		if (puzzles[currentPuzzleIndex].isCorrect)
		{
			//DrawRectangleLines(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GREEN);
			DrawRectangle(0, 80, WINDOW_WIDTH, 100, BLACK);
			DrawTextEx(mxFont, "Fire's put out!", (Vector2) { WINDOW_WIDTH / 2.f - 180.f, 90 }, 72.f, 1.f, WHITE);
		}
		
		if (puzzles[currentPuzzleIndex].isLost)
		{
			DrawTextEx(mxFont, "You let fire burn the place!", (Vector2) { 50.f, WINDOW_HEIGHT / 8.f }, 72.f, 1.f, RED);
		}

		
		sprintf_s(str, sizeof(str), "%1.1f", fireYoffset);

		//EndShaderMode();
		EndDrawing();
	}

	UnloadShader(fireShader);
	UnloadTexture(noiseTexture);
	UnloadTexture(atlasTexture);
	UnloadMusicStream(fireMusic);
	UnloadRenderTexture(renderTexture);
	CloseAudioDevice();
	CloseWindow();

	return 0;
}


void RotateBox(struct Box* box)
{
	box->rotation += 90.f;
	if (box->rotation >= 360.f)
	{
		box->rotation = 0.f;
	}

	switch (box->rotation)
	{
	case 0:
		box->direction = RIGHT;
		break;
	case 90:
		box->direction = BOTTOM;
		break;
	case 180:
		box->direction = LEFT;
		break;
	case 270:
		box->direction = TOP;
		break;
	default:
		break;
	}

	// valve open update
	bool wasLeftOpen = box->isLeftOpen;
	bool wasRightOpen = box->isRightOpen;
	bool wasTopOpen = box->isTopOpen;
	bool wasBottomOpen = box->isBottomOpen;

	if (box->isLeftOpen) box->isLeftOpen = false;
	if (box->isRightOpen) box->isRightOpen = false;
	if (box->isTopOpen) box->isTopOpen = false;
	if (box->isBottomOpen) box->isBottomOpen = false;

	if (wasLeftOpen) box->isTopOpen = true;
	if (wasRightOpen) box->isBottomOpen = true;
	if (wasTopOpen) box->isRightOpen = true;
	if (wasBottomOpen) box->isLeftOpen = true;

}


int GetBoxIndexByPos(struct Box boxes[BOX_COUNT], Vector2 pos)
{
	for (int i = 0; i < BOX_COUNT; i++)
	{
		if (boxes[i].x == pos.x && boxes[i].y == pos.y)
		{
			return boxes[i].index;
		}
	}
}


void CheckForAdjacentBox(struct Box boxes[BOX_COUNT], struct Box* box, Vector2 pos, Vector2 visited[BOX_COUNT])
{

	// check if already visited
	for (int i = 0; i < BOX_COUNT; i++)
	{
		if (visited[i].x == pos.x && visited[i].y == pos.y)
		{
			return;
		}
	}

	visited[box->index] = pos;

	// top
	if (pos.y > 0)
	{
		Vector2 newBoxPos = (Vector2){ pos.x, pos.y - 1 };
		int newBoxIndex = GetBoxIndexByPos(boxes, newBoxPos);

		if (box->isTopOpen && boxes[newBoxIndex].isBottomOpen)
		{
			box->isWaterConnected = true;
			boxes[newBoxIndex].isWaterConnected = true;
			CheckForAdjacentBox(boxes, &boxes[newBoxIndex], newBoxPos, visited);
		}

	}

	// right
	if (pos.x < 3)
	{
		Vector2 newBoxPos = (Vector2){ pos.x + 1, pos.y };
		int newBoxIndex = GetBoxIndexByPos(boxes, newBoxPos);

		if (box->isRightOpen && boxes[newBoxIndex].isLeftOpen)
		{
			box->isWaterConnected = true;
			boxes[newBoxIndex].isWaterConnected = true;
			CheckForAdjacentBox(boxes, &boxes[newBoxIndex], newBoxPos, visited);
		}

	}

	// bottom
	if (pos.y < 3)
	{
		Vector2 newBoxPos = (Vector2){ pos.x, pos.y + 1 };
		int newBoxIndex = GetBoxIndexByPos(boxes, newBoxPos);

		if (box->isBottomOpen && boxes[newBoxIndex].isTopOpen)
		{
			box->isWaterConnected = true;
			boxes[newBoxIndex].isWaterConnected = true;
			CheckForAdjacentBox(boxes, &boxes[newBoxIndex], newBoxPos, visited);
		}

	}

	// left
	if (pos.x > 0)
	{
		Vector2 newBoxPos = (Vector2){ pos.x - 1, pos.y, };
		int newBoxIndex = GetBoxIndexByPos(boxes, newBoxPos);

		if (box->isLeftOpen && boxes[newBoxIndex].isRightOpen)
		{
			box->isWaterConnected = true;
			boxes[newBoxIndex].isWaterConnected = true;
			CheckForAdjacentBox(boxes, &boxes[newBoxIndex], newBoxPos, visited);
		}

	}

}
