/*
* Development log
* 1. Create a new empty project
* 2. Import resources
* 3. Add the initial game scene
* 4. Add the plants bar
* 5. Add the plants' cards
* 6. Add the rendering of the plant being dragged
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
IMAGE *imgPlant[PLANTS_COUNT][20];   

int curX, curY; // The coordination of the plant currently selected
int curPlant = -1; // -1: no selection, 0 = first plant(peashooter), 1 = second plant ......

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

void updateWindow() {
	BeginBatchDraw(); // Start the buffer

	putimage(0, 0, &imgBg);
	putimagePNG(250, 0, &imgBar);

	for (int i = 0; i < PLANTS_COUNT; i++) {
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);

	}

	// Render the plants that are being dragged
	if (curPlant >= 0) {
		IMAGE* img = imgPlant[curPlant][0];
		int img_height = img->getheight();
		int img_width = img->getwidth();
		putimagePNG(curX - img_width/2, curY - img_height/2, img);
	}

	EndBatchDraw(); // End the buffer
}

void userClick() {

	ExMessage msg;
	static int status = 0;
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 338 && msg.x < 338 + 65 * PLANTS_COUNT &&
				msg.y < 96 && msg.y > 5) {
				int index = (msg.x - 338) / 65;
				printf("%d\n", index);
				status = 1;
				curPlant = index;
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1) {
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP) {

		}
	}

}

int main(void) {
	gameInit();

	while (1) {
		userClick();

		updateWindow();
	}

	system("pause");
	return 0;
}
