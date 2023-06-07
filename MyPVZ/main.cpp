/*
* Development log
* 1. Create a new empty project
* 2. Import resources
* 3. Add the initial game scene
* 4. Add the plants bar
* 5. Add the plants' cards
* 6. Add the rendering of the plant being dragged
* 7. Add the planting of the plants
* 8. Add the rendering of movement of plants on the map
* 9. Added the game UI
* 10. Added the logic of randomly generated sunshine
*/

#include <stdio.h>
#include <stdbool.h>
#include <graphics.h>
#include <time.h>
#include "tools.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// Width and height of the game window
#define WIN_WIDTH 900
#define WIN_HEIGHT 600

// Plants list (selected)
enum {PEASHOOTER, SUNFLOWER, PLANTS_COUNT};

// imgBg is the game background, imgBar is the plants bar, imgCards are the 
// cards of the selected plants, and imgPlant stores the frames of the plants
IMAGE imgBg;
IMAGE imgBar;
IMAGE imgCards[PLANTS_COUNT];
IMAGE *imgPlant[PLANTS_COUNT][20];  
// 20 means that there are at most 20 frames of movement per plant

int curX, curY; // The coordination of the plant currently selected
int curPlant = -1; // -1: no selection, 0 = first plant(peashooter), 1 = second plant ...

struct plant {
	int type; // -1: no plant, 0: peashooter, 1: sunflower ...
	int frame_index; // The index of the sequence of frame currently displaying
};

// The map for the plants
struct plant map[3][9];

struct sunshineBall {
	int x;
	int y; // x and y stores the position that the sunshineBall locates at
	int frameIndex; // The index of frame that should be rendered now
	int dest_y; // The destination that the sunshine should be at
	bool in_use; // Stores whether the sunshineBall is in use
	int timer; // Stores the time after the sunshine have been generated
};

// All the sunshine balls. Will be recycled after the sunshine is collected
// Thus, there is a maximum of 50 sunshine balls that can exist simultaneously
struct sunshineBall all_balls[50]; 
// The frames of SunshineBalls
IMAGE imagSunshineBall[29];

// Sunshine
int sunshine = -1;

// Helper function that help determines whether a file exists
bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r"); // Would return null if open failed
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

// Function to initialize the game
void gameInit(void) {

	// Assign the seed to generate the random number between 0 and 640 (900 - 260 = 640)
	srand(time(NULL));

	// Load the background image
	// Changed character set setting to "Multi-Byte"
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	// Set the memory in the arrays appropriately
	memset(imgPlant, 0, sizeof(imgPlant));
	memset(map, -1, sizeof(map));
	memset(all_balls, 0, sizeof(all_balls));

	// Initialize the plant cards
	char name[64];
	for (int i = 0; i < PLANTS_COUNT; i++) {
		// Generate the name of the card file (image) and load that image
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i+1);
		loadimage(&imgCards[i], name);

		// Also load the images of the frames. Because different plants have
		//   different number of frames, we need to determine when the last
		//   frame have been loaded
		for (int j = 0; j < 20; j++) {
			sprintf_s(name, sizeof(name), "res/plants_moving/%d/%d.png", i, j+1);
			if (fileExist(name)) {
				imgPlant[i][j] = new IMAGE;
				if (imgPlant[i][j]) {
					loadimage(imgPlant[i][j], name);
				}
			}
			else {
				break;
			}
		}
	}

	curPlant = -1;
	sunshine = 50;

	// Initialize all the sunshineBalls stored
	for (int i = 0; i < 29; i++) {
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imagSunshineBall[i], name);
	}

	// Create the game window
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);

	// Set the font
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY; // Antialiase
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);
}

// Function to start the UI of the game
void startUI(void) {
	
	// see resources for the usage of those images
	IMAGE img_menu, img_start1, img_start2;
	loadimage(&img_menu, "res/menu.png");
	loadimage(&img_start1, "res/menu1_eng.png");
	loadimage(&img_start2, "res/menu2_eng.png");

	// selected = 0 means that the start button is not selected
	// selected = 1 means that the start button is selected
	int selected = 0;
	while (1) {
		BeginBatchDraw();
		// render the background image
		putimage(0, 0, &img_menu);
		// render the start button
		putimagePNG(460, 60, selected ? &img_start2 : &img_start1);

		// If the user clicks the button, then start the game
		ExMessage msg;
		if (peekmessage(&msg)) {
			if (msg.message == WM_LBUTTONDOWN &&
				msg.x > 460 && msg.x < 460+364 &&
				msg.y > 60 && msg.y < 60+169) {
				selected = 1;

			}
			else if (msg.message == WM_LBUTTONUP) {
				return;
			}
		}

		EndBatchDraw();
	}
}

// Function to update the window that the player sees
void updateWindow(void) {
	BeginBatchDraw(); // Start the buffer

	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar);

	for (int i = 0; i < PLANTS_COUNT; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);

	}

	// Render the map
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type != -1) {
				int x = 256 + j * 81;
				int y = 190 + i * 102;
				int plant_type = map[i][j].type; // Debug: Change the index 
				int index = map[i][j].frame_index / 3;
				putimagePNG(x, y, imgPlant[plant_type][index]);
			}
		}
	}

	// Render the plants that are being dragged
	if (curPlant >= 0) {
		IMAGE* img = imgPlant[curPlant][0];
		int img_height = img->getheight();
		int img_width = img->getwidth();
		putimagePNG(curX - img_width / 2, curY - img_height / 2, img);
	}

	// Render the sunshine balls

	int max_balls = sizeof(all_balls) / sizeof(all_balls[0]);
	for (int i = 0; i < max_balls; i++) {
		if (all_balls[i].in_use) {
			IMAGE* img = &imagSunshineBall[all_balls[i].frameIndex];
			putimagePNG(all_balls[i].x, all_balls[i].y, img);

		}
	}

	// Show the available sunshine
	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(276, 67, scoreText);

	EndBatchDraw(); // End the buffer
}

void collect_sunshine(ExMessage* msg) {
	int count = sizeof(all_balls) / sizeof(all_balls[0]);
	int width = imagSunshineBall[0].getwidth();
	int height = imagSunshineBall[0].getheight();
	for (int i = 0; i < count; i++) {
		if (all_balls[i].in_use) {
			int x = all_balls[i].x;
			int y = all_balls[i].y;
			if (msg->x > x && msg->x < x + width &&
				msg->y > y && msg->y < y + height) {
				all_balls[i].in_use = false;
				sunshine += 25;
				mciSendString("play res/sunshine.mp3", 0, 0, 0);
			}
		}
	}
}

// Detect Click from user in game
void userClick(void) {

	ExMessage msg;
	static int status = 0; // Stores if a plant is selected now
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			// Selecting plants
			if (msg.x > 338 && msg.x < 338 + 65 * PLANTS_COUNT &&
				msg.y < 96 && msg.y > 5) {
				int index = (msg.x - 338) / 65;
				printf("%d\n", index);
				status = 1;
				curPlant = index;
				curX = msg.x;
				curY = msg.y;
			}
			// Collect sunshine
			else {
				collect_sunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {
			if (msg.x > 256 && msg.y > 179 && msg.y < 489) {
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 256) / 81;
				printf("%d, %d\n", row, col);

				if (map[row][col].type == -1) {
					map[row][col].type = curPlant;
					map[row][col].frame_index = 1;
				}
			}
			

			curPlant = -1;
			status = 0;
		}
	}

}

void createSunshine(void) {

	// Limit the frequency of the sunshine generated
	static int count = 0;
	static int frequency = 400;
	count++;
	if (count >= frequency) {
		frequency = 200 + rand() % 200;
		count = 0;

		// Take a usable ball from all the balls in the array
		int max_balls = sizeof(all_balls);

		int i = 0;
		for (i = 0; i < max_balls && all_balls[i].in_use; i++);
		// If i = max_balls, then all the array of sunshine balls have been
		//   searched, but no available sunshine ball was find. Thus, this function
		//   simply ends.
		if (i >= max_balls) return;

		all_balls[i].in_use = 1;
		all_balls[i].frameIndex = 0;
		// Assign random numbers
		all_balls[i].x = 260 + rand() % (900 - 260);
		all_balls[i].y = 60;
		all_balls[i].dest_y = 200 + (rand() % 4) * 90;
	}
}

void updateSunshine(void) {
	int max_balls = sizeof(all_balls) / sizeof(all_balls[0]);
	for (int i = 0; i < max_balls; i++) {
		if (all_balls[i].in_use) {
			all_balls[i].frameIndex = (all_balls[i].frameIndex + 1) % 29;

			// If the sunshineball has not reached the ground, keep going downward
			if (all_balls[i].timer == 0) {
				all_balls[i].y += 2;
			}

			if (all_balls[i].y >= all_balls[i].dest_y) {
				all_balls[i].timer++;

				// the sunshineball disappears after 100 gameticks
				if (all_balls[i].timer > 100) {
					all_balls[i].in_use = false;
				}
				
			}
		}
	}
}

// Update the map during the game
void updateGame(void) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type >= 0) {
				map[i][j].frame_index++;
				int plant_type = map[i][j].type;
				int frame_index = map[i][j].frame_index / 3;
				if (imgPlant[plant_type][frame_index] == NULL) {
					map[i][j].frame_index = 1;
				}
			}
		}
	}

	createSunshine(); // Create the sunshine
	updateSunshine(); // Update the status of the sunshine
}

int main(void) {
	gameInit();

	startUI();
	int timer = 0;
	bool flag = false;
	while (1) {
		userClick();

		timer += getDelay();
		if (timer > 20) {
			flag = true;
			timer = 0;
		}

		if (flag) {
			flag = false;

			updateWindow();
			updateGame();
		}
		
	}

	system("pause");
	return 0;
}
