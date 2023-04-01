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
*/

#include <stdio.h>
#include <graphics.h>
#include "tools.h"

#define WIN_WIDTH 900
#define WIN_HEIGHT 600

enum {PEASHOOTER, SUNFLOWER, PLANTS_COUNT};

IMAGE imgBg;
IMAGE imgBar;
IMAGE imgCards[PLANTS_COUNT];
IMAGE *imgPlant[PLANTS_COUNT][20];   // 20 means that there are at most 20 frames of movement per plant

int curX, curY; // The coordination of the plant currently selected
int curPlant = -1; // -1: no selection, 0 = first plant(peashooter), 1 = second plant ......

struct plant {
	int type; // -1: no plant, 0: peashooter, 1: sunflower ...
	int frame_index; // The index of the sequence of frame currently displaying
};

struct plant map[3][9];

bool fileExist(const char* name) {
	FILE* fp = fopen(name, "r");
	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

void gameInit() {
	// Load the background image
	// Changed character set setting to "Multi-Byte"
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	memset(imgPlant, 0, sizeof(imgPlant));
	memset(map, -1, sizeof(map));

	// Initialize the plant cards
	char name[64];
	for (int i = 0; i < PLANTS_COUNT; i++) {
		// Generate the name of the card file (image)
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i+1);
		loadimage(&imgCards[i], name);

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

	// Create the game window
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);
}

void startUI() {

}

void updateWindow() {
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
				int index = map[i][j].frame_index;
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

	EndBatchDraw(); // End the buffer
}

void userClick() {

	ExMessage msg;
	static int status = 0; // Stores if a plant is selected now
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 338 + 65 * PLANTS_COUNT &&
				msg.y < 96 && msg.y > 5) {
				int index = (msg.x - 338) / 65;
				printf("%d\n", index);
				status = 1;
				curPlant = index;
				curX = msg.x;
				curY = msg.y;
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

void updateGame() {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type >= 0) {
				map[i][j].frame_index++;
				int plant_type = map[i][j].type;
				int frame_index = map[i][j].frame_index;
				if (imgPlant[plant_type][frame_index] == NULL) {
					map[i][j].frame_index = 1;
				}
			}
		}
	}
}

int main(void) {
	gameInit();

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
