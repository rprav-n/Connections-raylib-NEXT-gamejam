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
#define START_POS (CELL_SIZE * SPACING)
#define END_POS (BOX_COUNT * ROWS)
#define TOTAL_PUZZLES 4
#define FIRE_MOVE_SPEED 0.05f
#define BUTTON_WIDTH 64
#define BUTTON_HEIGHT 32

/* 
	int blackColor = 0x000000ff;
	int whiteColor = 0xffffffff;
	int blueColor = 0x29adffff;
	int redColor = 0xff4242ff;
	int greenColor = 0x45e082ff;
	int orangeColor = 0xff8000ff; // color used for flames
*/

enum Direction
{
	TOP,
	LEFT,
	RIGHT,
	BOTTOM
};

enum State
{
	START,
	PLAYING,
	END,
	WON,
	LOST,
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
	bool isCorrect;
	bool isLost;
};

struct Text 
{
	char text[50];
	float fontSize;
	float spacing;
	int speed;
	Color color;
	Vector2 pos;
	Vector2 startPos;
	Vector2 origin;
	Vector2 size;
};


void InitBoxes(struct Box boxes[BOX_COUNT], struct Puzzle puzzle);
void DrawBoxes(struct Box boxes[BOX_COUNT], Texture2D atlasTexture);
void RotateBox(struct Box* box);
int GetBoxIndexByPos(struct Box boxes[BOX_COUNT], Vector2 pos);
void CheckForAdjacentBox(struct Box boxes[BOX_COUNT], struct Box* box, Vector2 pos, Vector2 visted[BOX_COUNT]);
Vector2 GetFontOrigin(struct Text textData);
Vector2 GetFontSize(Font font, struct Text textData);
void DrawCustomText(Font font, struct Text textData);

int main()
{

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pipe Connections");
	InitAudioDevice();

	RenderTexture2D renderTexture = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
	SetTextureFilter(renderTexture.texture, TEXTURE_FILTER_POINT);

	Texture2D atlasTexture = LoadTexture("assets/atlas.png");
	Texture2D bricksTexture = LoadTexture("assets/bricks.png");
	Texture2D noiseTexture = LoadTexture("assets/noise.png");
	Texture2D startPageTexture = LoadTexture("assets/start_page.png");


	Font mx16Font = LoadFont("assets/m6x11.ttf");

	Sound wrenchSnd = LoadSound("assets/wrench.wav");
	SetSoundVolume(wrenchSnd, 2.f);

	Music fireMusic = LoadMusicStream("assets/flame.mp3");
	SetMusicVolume(fireMusic, 0.2f);
	PlayMusicStream(fireMusic);

	Music bgMusic = LoadMusicStream("assets/bg_music.ogg");
	SetMusicVolume(bgMusic, 0.4f);
	PlayMusicStream(bgMusic);

	Shader fireShader = LoadShader(NULL, "assets/shaders/fire.fs");

	// { 1.0f, 0.25f, 0.25f } valve red color = #ff4242
	// { 1.0f, 0.5f, 0.0f } = orange colr = #ff8000
	float orangeColorFloat[3] = { 1.0f, 0.5f, 0.0f };
	Color blackColor = GetColor(0x000000ff);
	Color whiteColor = GetColor(0xffffffff);
	Color blueColor = GetColor(0x29adffff);
	Color redColor = GetColor(0xff4242ff);
	Color greenColor = GetColor(0x45e082ff);
	Color orangeColor = GetColor(0xff8000ff);
	Color darkBrownColor = GetColor(0x4d2b32ff);
	Color lightBrownColor = GetColor(0x7a4841ff);

	SetShaderValue(fireShader, GetShaderLocation(fireShader, "flameColor"), orangeColorFloat, SHADER_UNIFORM_VEC3);
	SetShaderValue(fireShader, GetShaderLocation(fireShader, "animationSpeed"), (float[1]) { 0.5f }, SHADER_UNIFORM_FLOAT);
	SetShaderValueTexture(fireShader, GetShaderLocation(fireShader, "texture0"), noiseTexture);

	Camera2D camera = {};
	camera.target = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
	camera.offset = (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	bool shouldCameraShake = false;

	struct Player player = {
		.pos = (Vector2){ (float)START_POS, (float)START_POS }
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
		.isCorrect = false,
	};
	puzzles[1] = (struct Puzzle){
		.puzzleGrid = {
			{13, 14, 2, 3},
			{2, 11, 7, 5},
			{3, 12, 8, 9},
			{2, 9, 14, 3}
		},
		.isCorrect = false,
	};
	puzzles[2] = (struct Puzzle){
		.puzzleGrid = {
			{13, 6, 5, 12},
			{10, 8, 6, 10},
			{10, 9, 1, 5},
			{4, 3, 11, 11}
		},
		.isCorrect = false,
	};
	puzzles[3] = (struct Puzzle){
		.puzzleGrid = {
			{11, 8, 6, 12},
			{5, 10, 8, 9},
			{11, 7, 12, 6},
			{2, 5, 5, 11}
		},
		.isCorrect = false,
	};

	struct Box boxes[BOX_COUNT];

	InitBoxes(boxes, puzzles[currentPuzzleIndex]);

	float time = 0.0f;
	float fireYoffset = 1.f;
	float wrenchRotation = 0.f;


	// Set all texts
	struct Text playText = 
	{
		.text = "Press 'P' to play",
		.fontSize = 32.f,
		.spacing = 1.f,
		.color = whiteColor,
		.speed = 20,
	};
	playText.size = GetFontSize(mx16Font, playText);
	playText.origin = GetFontOrigin(playText);
	playText.pos = (Vector2){WINDOW_WIDTH/2.f, WINDOW_HEIGHT - 200.f};
	playText.startPos = (Vector2){WINDOW_WIDTH/2.f, WINDOW_HEIGHT - 200.f};

	struct Text levelText = 
	{
		.text = "Level: 1",
		.fontSize = 32.f,
		.spacing = 1.f,
		.color = whiteColor,
	};
	levelText.size = GetFontSize(mx16Font, levelText);
	levelText.origin = (Vector2){0.f, 0.f};
	levelText.pos = (Vector2){10.f, 10.f};

	struct Text timeText = 
	{
		.text = "Time: 1.0000",
		.fontSize = 32.f,
		.spacing = 1.f,
		.color = whiteColor,
	};
	timeText.size = GetFontSize(mx16Font, timeText);
	timeText.origin = (Vector2){0.f, 0.f};
	timeText.pos = (Vector2){10.f, levelText.pos.y + levelText.size.y + 10.f};

	struct Text burnedText = 
	{
		.text = "BURNNNN'd",
		.fontSize = 64.f,
		.spacing = 2.f,
		.color = whiteColor,
	};
	burnedText.size = GetFontSize(mx16Font, burnedText);
	burnedText.origin = GetFontOrigin(burnedText);
	burnedText.pos = (Vector2){WINDOW_WIDTH/2.f, WINDOW_HEIGHT/4.f};

	struct Text restartText = 
	{
		.text = "Press 'R' to restart",
		.fontSize = 32.f,
		.spacing = 1.f,
		.color = whiteColor,
	};
	restartText.size = GetFontSize(mx16Font, restartText);
	restartText.origin = GetFontOrigin(restartText);
	restartText.pos = (Vector2){WINDOW_WIDTH/2.f, burnedText.pos.y + burnedText.size.y + 10.f};

	struct Text wonText = 
	{
		.text = "Doused!",
		.fontSize = 64.f,
		.spacing = 2.f,
		.color = greenColor,
	};
	wonText.size = GetFontSize(mx16Font, wonText);
	wonText.origin = GetFontOrigin(wonText);
	wonText.pos = (Vector2){WINDOW_WIDTH/2.f, WINDOW_HEIGHT/8.f};

	struct Text nextText = 
	{
		.text = "Press 'N' for next level",
		.fontSize = 32.f,
		.spacing = 1.f,
		.color = whiteColor,
	};
	nextText.size = GetFontSize(mx16Font, nextText);
	nextText.origin = GetFontOrigin(nextText);
	nextText.pos = (Vector2){WINDOW_WIDTH/2.f, wonText.pos.y + wonText.size.y + 10.f};

	struct Text endText = 
	{
		.text = "Thank you for playing my game!",
		.fontSize = 48.f,
		.spacing = 2.f,
		.color = whiteColor,
	};
	endText.size = GetFontSize(mx16Font, endText);
	endText.origin = GetFontOrigin(endText);
	endText.pos = (Vector2){WINDOW_WIDTH/2.f, WINDOW_HEIGHT/6.f};

	enum State gameState = START;

	//DisableCursor();

	SetTargetFPS(FPS);
	
	float shakeDuration = 0.0f;
    float shakeIntensity = 4.0f;

	while (!WindowShouldClose())
	{
		// Common state
		float dt = GetFrameTime();
		time += dt;

		if (IsKeyPressed(KEY_G)) {
			shouldCameraShake = true;
		}

		if (shouldCameraShake)
        {
            shakeDuration = 0.05f;
            shakeIntensity = 1.f;
        }

		if (shakeDuration > 0.0f)
        {
            float offsetX = GetRandomValue(-shakeIntensity, shakeIntensity);
            float offsetY = GetRandomValue(-shakeIntensity, shakeIntensity);

            // Apply shake effect to camera target
            camera.target.x += offsetX;
            camera.target.y += offsetY;

            shakeDuration -= dt;
            shouldCameraShake = false;
        }
        else
        {
            camera.target = Vector2MoveTowards(camera.target, (Vector2){WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f}, 200 * dt);
        }


		UpdateMusicStream(bgMusic);

		// Update Code
		switch (gameState)
		{
		case START:

			playText.pos.y += playText.speed * dt;
			if (playText.pos.y > playText.startPos.y + 30.f || playText.pos.y < playText.startPos.y) {
				playText.speed *= -1;
			}

			UpdateMusicStream(fireMusic);

			SetShaderValue(fireShader, GetShaderLocation(fireShader, "time"), &time, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fireShader, GetShaderLocation(fireShader, "yOffset"), (float[1]) { 0.65f }, SHADER_UNIFORM_FLOAT);
			if (IsKeyPressed(KEY_P))
			{
				gameState = PLAYING;
			}
			break;
		case PLAYING:
			
			if (!puzzles[currentPuzzleIndex].isCorrect && !puzzles[currentPuzzleIndex].isLost)
			{
				UpdateMusicStream(fireMusic);

				fireYoffset = Vector2MoveTowards((Vector2) { 0.f, fireYoffset }, (Vector2) { 0.f, -0.0f }, dt * FIRE_MOVE_SPEED).y;
				SetShaderValue(fireShader, GetShaderLocation(fireShader, "time"), &time, SHADER_UNIFORM_FLOAT);
				SetShaderValue(fireShader, GetShaderLocation(fireShader, "yOffset"), (float[1]) { fireYoffset }, SHADER_UNIFORM_FLOAT);

				if (fireYoffset <= 0.0f)
				{
					gameState = LOST;
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
						&& !puzzles[currentPuzzleIndex].isCorrect && !puzzles[currentPuzzleIndex].isLost)
					{
						RotateBox(&boxes[index]);
						shouldCameraShake = true;
						PlaySound(wrenchSnd);

						wrenchRotation += 90.f;
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

			if (puzzles[currentPuzzleIndex].isCorrect)
			{
				currentPuzzleIndex += 1;
				if (currentPuzzleIndex < TOTAL_PUZZLES)
				{
					gameState = WON;
				}
				else
				{
					gameState = END;
				}
			}

			snprintf(levelText.text, sizeof levelText.text, "Level: %d", currentPuzzleIndex);
			snprintf(timeText.text, sizeof timeText.text, "Time: %1.4f", fireYoffset);
			levelText.size = GetFontSize(mx16Font, levelText);
			timeText.size = GetFontSize(mx16Font, timeText);

			break;
		case END:
			break;
		case WON:
			if (IsKeyPressed(KEY_N))
			{
				InitBoxes(boxes, puzzles[currentPuzzleIndex]);
				fireYoffset = 1.f;
				gameState = PLAYING;
			}
			break;
		case LOST:
			UpdateMusicStream(fireMusic);

			SetShaderValue(fireShader, GetShaderLocation(fireShader, "time"), &time, SHADER_UNIFORM_FLOAT);
			SetShaderValue(fireShader, GetShaderLocation(fireShader, "yOffset"), (float[1]) { 0.5f }, SHADER_UNIFORM_FLOAT);
			if (IsKeyPressed(KEY_R))
			{
				InitBoxes(boxes, puzzles[currentPuzzleIndex]);
				fireYoffset = 1.f;
				gameState = PLAYING;
			}
			break;
		default:
			break;
		}

		BeginTextureMode(renderTexture);
		ClearBackground(whiteColor);

		// Draw brickS
		DrawTexture(bricksTexture, 0, 0, whiteColor);

		switch (gameState)
		{
		case START:
			// Draw fire
			BeginShaderMode(fireShader);
			DrawTexture(noiseTexture, 0, 0, whiteColor);
			EndShaderMode();
			break;
		case PLAYING:
			// Draw fire
			BeginShaderMode(fireShader);
			DrawTexture(noiseTexture, 0, 0, whiteColor);
			EndShaderMode();

			// Draw boxes
			DrawBoxes(boxes, atlasTexture);

			// Draw player
			DrawRectangleLines(player.pos.x, player.pos.y, CELL_SIZE, CELL_SIZE, whiteColor);			
			break;
		case END:
			DrawBoxes(boxes, atlasTexture);
			break;
		case WON:
			DrawBoxes(boxes, atlasTexture);
			break;
		case LOST:
			DrawBoxes(boxes, atlasTexture);
			BeginShaderMode(fireShader);
			DrawTexture(noiseTexture, 0, 0, whiteColor);
			EndShaderMode();
			break;
		default:
			break;
		}

		EndTextureMode();

		
		BeginDrawing();
		ClearBackground(darkBrownColor);

		BeginMode2D(camera);
		
		
		Rectangle source = { 0.0f, 0.0f, (float)renderTexture.texture.width, (float)-renderTexture.texture.height };
		Rectangle dest = { 0, 0, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };

		DrawTexturePro(renderTexture.texture, source, dest, (Vector2) { 0, 0 }, 0.f, whiteColor);

		// Draw custom cursor
		/*Vector2 mousePosition = GetMousePosition();
		DrawTextureEx(wrenchTexture, mousePosition, 0.f, 4.f, whiteColor);*/

		switch (gameState)
		{
		case START:
			
			DrawCustomText(mx16Font, playText);
			DrawTexture(startPageTexture, 0, 0, whiteColor);
			break;
		
		case PLAYING:
			DrawCustomText(mx16Font, levelText);
			DrawCustomText(mx16Font, timeText);
			break;
		case WON:
			DrawCustomText(mx16Font, levelText);
			DrawCustomText(mx16Font, timeText);
			DrawCustomText(mx16Font, wonText);
			DrawCustomText(mx16Font, nextText);
			break;
		case LOST:
			DrawCustomText(mx16Font, burnedText);
			DrawCustomText(mx16Font, restartText);
			break;
		case END:
			DrawCustomText(mx16Font, endText);
			break;
		default:
			break;
		}
		EndMode2D();
		EndDrawing();
	}

	UnloadShader(fireShader);
	UnloadMusicStream(bgMusic);
	UnloadMusicStream(fireMusic);
	UnloadSound(wrenchSnd);
	UnloadFont(mx16Font);
	UnloadTexture(noiseTexture);
	UnloadTexture(bricksTexture);
	UnloadTexture(atlasTexture);
	UnloadRenderTexture(renderTexture);
	CloseAudioDevice();
	CloseWindow();

	return 0;
}

void InitBoxes(struct Box boxes[BOX_COUNT], struct Puzzle puzzle)
{
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < ROWS; j++)
		{
			int id = puzzle.puzzleGrid[i][j];
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
}


void DrawBoxes(struct Box boxes[BOX_COUNT], Texture2D atlasTexture)
{
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
	return -1;
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


Vector2 GetFontOrigin(struct Text textData) 
{
	return (Vector2){textData.size.x/2.f, textData.size.y/2.f};
}

Vector2 GetFontSize(Font font, struct Text textData) 
{
	return MeasureTextEx(font, textData.text, textData.fontSize, textData.spacing);
}

void DrawCustomText(Font font, struct Text textData) 
{
	DrawTextPro(font, textData.text, textData.pos, textData.origin, 0.f, textData.fontSize, textData.spacing, textData.color);
}