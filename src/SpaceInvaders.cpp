#include "SpaceInvaders.h"
#include "sprites.hpp"
#include <ByteBoi.h>
#include "Highscore.h"
#include <ByteBoiLED.h>
#include <SD.h>

uint drawTime = 0;
SpaceInvaders::SpaceInvaders* SpaceInvaders::SpaceInvaders::instance = nullptr;
SpaceInvaders::SpaceInvaders::SpaceInvaders(Display* display) :
		Context(*display), baseSprite(screen.getSprite()),
		buttons(Input::getInstance()), display(display)
{
	instance = this;
	randomSeed(millis()*micros());
	starsSetup();
	gamestatus = "title";

	SD.begin(SD_CS, SPI);

	Samples.gameOver = new Sample(SD.open(ByteBoi.getSDPath() + "/Music/GameOver.aac"), true);
	Samples.menu = new Sample(SD.open(ByteBoi.getSDPath() + "/Music/Menu.aac"));
	Samples.game = new Sample(SD.open(ByteBoi.getSDPath() + "/Music/Game.aac"));
	Samples.menu->setLooping(true);
	Samples.gameOver->setLooping(false);
	Samples.game->setLooping(true);

}
void SpaceInvaders::SpaceInvaders::start()
{
	Highscore.begin();
	prevGamestatus = "";
	draw();
	LoopManager::addListener(this);

	if(gamestatus == "title"){
		Playback.play(Samples.menu);
	}else if(gamestatus == "running"){
		Playback.play(Samples.game);
	}
}
void SpaceInvaders::SpaceInvaders::stop()
{
	clearButtonCallbacks();
	// jb.clear();
	// delete[] highscoresPath;
	LoopManager::removeListener(this);
	Playback.stop();
}
void SpaceInvaders::SpaceInvaders::pack(){
}

void SpaceInvaders::SpaceInvaders::draw(){
	if (gamestatus == "title") {
		showtitle();
	}
	if (gamestatus == "running") { // game running loop
		baseSprite->clear(TFT_BLACK);
		for(int i = 0; i < STAR_COUNT; i++)
		{
			// Remove the star from the screen by changing its pixel to the background color.
			baseSprite->drawPixel(stars[i].x, stars[i].y, BACKGROUND_COLOR);
			// Draw the star at its new coordinate.
			baseSprite->fillRect(stars[i].x, stars[i].y, 2, 2, STAR_COLOR);

		}
		drawplayership(); // draw player ship
		drawplayershot(); // move + draw player shoot
		drawinvaders(); // draw invaders
		drawInvaderShot(); // invaders shoot
		drawbunkers(); // draw bunkers & check collission with player shot
		drawsaucer(); // draw saucer & remove if hit
		showscore(); // show lives, score, level
	}
	if (gamestatus == "gameover") { // game over
		baseSprite->clear(TFT_BLACK);
		drawplayership(); // draw player ship
		drawplayershot(); // move + draw player shoot
		drawinvaders(); // draw invaders
		drawInvaderShot(); // invaders shoot
		drawbunkers(); // draw bunkers & check collission with player shot
		drawsaucer(); // draw saucer & remove if hit
		baseSprite->setTextColor(TFT_RED);
		baseSprite->setTextSize(2);
		baseSprite->setFont(&fonts::Font0);
		baseSprite->drawRect(14, 41, 134, 38, TFT_WHITE);
		baseSprite->drawRect(13, 40, 136, 40, TFT_WHITE);
		baseSprite->fillRect(15, 42, 132, 36, TFT_BLACK);
		baseSprite->setTextDatum(textdatum_t::middle_center);
		baseSprite->drawString("GAME OVER", screen.getWidth() / 2, screen.getHeight() / 2);
		baseSprite->setTextDatum(textdatum_t::bottom_center);

	}
	if(gamestatus == "paused")
	{
		baseSprite->clear(TFT_BLACK);
		baseSprite->setTextColor(TFT_RED);
		baseSprite->setFont(&fonts::Font2);
		baseSprite->setTextSize(2);
		baseSprite->drawString("Paused", screen.getWidth()/2 + 5, baseSprite->height()/2 - 10);
		baseSprite->setFreeFont(TT1);
		baseSprite->drawString("A:RESUME      B:QUIT", screen.getWidth()/2 + 5, baseSprite->height() - 15);
	}
	if(gamestatus == "eraseData")
	{
		eraseDataDraw();
	}
	if(gamestatus == "dataDisplay")
	{
		dataDisplay();
	}
	if(gamestatus == "enterInitials")
	{
		enterInitialsDraw();
	}
}
void SpaceInvaders::SpaceInvaders::loop(uint)
{
	if(gamestatus != prevGamestatus)
	{
		screenChange = 1;
		prevGamestatus = gamestatus;
	}
	else
	{
		screenChange = 0;
	}
	
	if (gamestatus == "title") {
		if(screenChange)
		{
			titleSetup();
		}
		if(millis() - blinkMillis >= 250)
		{
			blinkMillis = millis();
			blinkState = !blinkState;
		}
		for(int i = 0; i < STAR_COUNT; i++)
		{
			// Update the position of the star.
			stars[i].update();

			// If the star's Y position is off the screen after the update.
			if(stars[i].y >= baseSprite->height())
			{
				// Randomize its position.
				stars[i].randomize(0, baseSprite->width(), 0, baseSprite->height(), STAR_SPEED_MIN, STAR_SPEED_MAX);
				// Set its Y position back to the top.
				stars[i].y = 0;
			}
		}
	}
	if (gamestatus == "newgame") { newgame(); } // new game

	if (gamestatus == "newlevel") { newlevel(); } // new level

	if (gamestatus == "running") { // game running loop
		if(screenChange)
		{
			setButtonsCallbacks();
		}
		for(int i = 0; i < STAR_COUNT; i++)
		{
			// Update the position of the star.
			stars[i].update();

			// If the star's Y position is off the screen after the update.
			if(stars[i].y >= baseSprite->height())
			{
				// Randomize its position.
				stars[i].randomize(0, baseSprite->width(), 0, baseSprite->height(), STAR_SPEED_MIN, STAR_SPEED_MAX);
				// Set its Y position back to the top.
				stars[i].y = 0;
			}
		}
		invaderlogic(); // invader logic
		updateInvaderShot();
		updatePlayerShot();
		nextlevelcheck(); // next level?
		saucerappears(); // saucer appears?
		movesaucer(); // move saucer
	}
	if (gamestatus == "gameover") { // game over
		if(screenChange){
			Playback.stop();
			delay(100);
			Playback.play(Samples.gameOver);
			clearButtonCallbacks();
			buttons->setBtnPressCallback(BTN_A, [](){
				instance->gamestatus = "enterInitials";
				Playback.stop();
			});
			buttons->setBtnPressCallback(BTN_B, [](){
				instance->gamestatus = "enterInitials";
				Playback.stop();
			});
		}
	}
	if(gamestatus == "paused")
	{
		if(screenChange){
			clearButtonCallbacks();
			buttons->setBtnPressCallback(BTN_A, [](){
				Playback.play(instance->Samples.game);
				instance->gamestatus = "running";
				instance->setButtonsCallbacks();
			});
			buttons->setBtnPressCallback(BTN_B, [](){
				instance->gamestatus = "title";
			});
		}
	}
	if(gamestatus == "eraseData")
	{
		if(screenChange){
			eraseDataSetup();
		}
		eraseDataUpdate();
	}
	if(gamestatus == "dataDisplay")
	{
		if(screenChange){
			dataDisplaySetup();
		}
	}
	if(gamestatus == "enterInitials")
	{
		if(screenChange){
			enterInitialsSetup();
		}
		enterInitialsUpdate();
	}
	draw();
	screen.commit();
}
void SpaceInvaders::SpaceInvaders::starsSetup()
{
	// Loop through each star.
	for(int i = 0; i < STAR_COUNT; i++)
	{
		// Randomize its position && speed.
		stars[i].randomize(0, baseSprite->width(), 0, baseSprite->height(), STAR_SPEED_MIN, STAR_SPEED_MAX);
	}
} 
void SpaceInvaders::SpaceInvaders::drawBitmap(int16_t x, int16_t y, const byte *bitmap, uint16_t color, uint8_t scale) {
	uint16_t w = *(bitmap++);
	uint16_t h = *(bitmap++);
	baseSprite->drawBitmap(x, y, bitmap, w, h, color, scale);
}

//ported nonstandard
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::newgame() {
	score = 0;
	lives = 3;
	gamelevel = 0;
	shipx = screen.getWidth() / 2 - 4;
	shipy = screen.getHeight() - 14;
	shotx = -1;
	shoty = -1;
	deadcounter = -1;
	saucers = -1;
	invadershots = 0;

	starsSetup();
	for (int i = 0; i < 4; i++) {
		invadershotx[i] = -1;
		invadershoty[i] = -1;
	}
	gamestatus = "newlevel";

	Playback.play(Samples.game);
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::newlevel() {
	invaderanz = invadersRows*invadersColumns;
	invaderctr = invadersRows*invadersColumns - 1;
	invaderxr = 1;
	invaderyr = 1;
	checkdir = 0;
	nextxdir = 4;
	nextydir = 0;
	yeahtimer = 0;
	invadershottimer = 120;
	saucertimer = 480;
	int down = gamelevel * 2;
	if (gamelevel > 8) { down = 20; }
	for (int i = 0; i < invadersColumns*invadersRows; i++) {
		invaderx[i] = 10 + i%invadersColumns * 16;
		invadery[i] = 14 + 12*int(i/invadersColumns) + down;
		// invadery[i] = 14 + down;
		// invadery[i + 6] = 13*2 + down;
		// invadery[i + 12] = 19*2 + down;
		// invadery[i + 18] = 25*2 + down;
		// invadery[i + 24] = 31*2 + down;
		switch (int(i/invadersColumns))
		{
		case 0:
			invaders[i] = 4;
			break;
		case 1:
			invaders[i] = 2;
			break;
		case 2:
			invaders[i] = 2;
			break;
		case 3:
			invaders[i] = 0;
			break;
		case 4:
			invaders[i] = 0;
			break;

		default:
			break;
		}
		invaderframe[i] = 0;
	}
	for (int & bunker : bunkers) {
		bunker = 0;
		if (gamelevel > 5) { bunker = -1; }
	}
	gamestatus = "running";
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::showscore() {
	if (infoshow == 1 && saucers == -1) {
		if (lives > 1) { drawBitmap(0, 0, invaderz_playership[0], TFT_WHITE, 2); }
		if (lives > 2) { drawBitmap(18, 0, invaderz_playership[0], TFT_WHITE, 2); }
		baseSprite->setTextColor(TFT_WHITE);
		baseSprite->setFreeFont(TT1);
		baseSprite->setTextSize(2);
		baseSprite->setCursor(117 - 4 * (score > 9) - 4 * (score > 99) - 4 * (score > 999), 12);
		baseSprite->print(score);
		baseSprite->setCursor(144, 12);
		baseSprite->print(gamelevel + 1);
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::nextlevelcheck() {
	// increment timer after all invaders killed
	if (invaderanz == 0) {
		yeahtimer++;
		if (yeahtimer >= 90) {
			gamelevel++;
			gamestatus = "newlevel";
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::handledeath() {
	deadcounter--;
	if (deadcounter == 0) {
		deadcounter = -1;
		lives--;
		shipx = screen.getWidth() / 2 - 4;
		if (lives == 0) { gamestatus = "gameover"; }
	}
}


//ported specific
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::clearButtonCallbacks()
{
	for(uint8_t i = 0; i < 6; i++)
	{
		buttons->setBtnReleaseCallback(i, nullptr);
		buttons->setBtnPressCallback(i, nullptr);
		buttons->setButtonHeldRepeatCallback(i, 0, nullptr);
		buttons->setButtonHeldCallback(i, 0, nullptr);
	}
}
void SpaceInvaders::SpaceInvaders::setButtonsCallbacks() {
	clearButtonCallbacks();
	buttons->setButtonHeldRepeatCallback(BTN_LEFT, 10, [](uint){
		if (instance->shipx > 0 && instance->deadcounter == -1) {
			instance->shipx-=5;
		}
	});
	buttons->setButtonHeldRepeatCallback(BTN_RIGHT, 10, [](uint){
		if (instance->shipx < 143 && instance->deadcounter == -1) {
			instance->shipx+=5;
		}
	});
	buttons->setBtnPressCallback(BTN_A, [](){
		if(instance->shotx == -1 && instance->deadcounter == -1){
			LED.setRGB(static_cast<LEDColor>(LEDColor::YELLOW));
			instance->shotx = instance->shipx + 6;
			instance->shoty = instance->shipy - 2;
			Playback.tone(400, 50);
		}
		LED.setRGB(OFF);
	});
	buttons->setBtnPressCallback(BTN_B, [](){
		Serial.println("paused");
		Playback.stop();
		instance->gamestatus = "paused";
	});
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::drawplayership() {
	if (deadcounter == -1) {
		drawBitmap(shipx, shipy, invaderz_playership[0], TFT_WHITE, 2);
	}
	else {
		drawBitmap(shipx, shipy, invaderz_playership[1 + invadershotframe], TFT_YELLOW, 2);
		handledeath();
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::drawplayershot() {
	if (shotx != -1) {
		baseSprite->drawLine(shotx, shoty, shotx, shoty + 6, TFT_YELLOW);
		baseSprite->drawLine(shotx+1, shoty, shotx+1, shoty + 6, TFT_YELLOW);
	}
}
void SpaceInvaders::SpaceInvaders::updatePlayerShot()
{
	if (shotx != -1) {
		shoty = shoty - 8;
		if (shoty < 0) {
			shotx = -1;
			shoty = -1;
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::invaderlogic() {
	// increment invader counter
	if (invaderanz > 0) {
		checkdir = 0;
		if(invaders[invaderctr] == -1)
		{
			while (invaders[invaderctr] == -1){
				invaderctr++;
				if (invaderctr >= invadersColumns*invadersRows) {
					invaderctr = 0;
					checkdir = 1;
				}

			}
		}
		else
		{
			invaderctr++;
			if (invaderctr >= invadersColumns*invadersRows) {
				invaderctr = 0;
				checkdir = 1;
			}
		}
		

		// change direction?
		if (checkdir == 1) {
			if (nextydir != 0) {
				invaderxr = 0;
				invaderyr = 2;
			}
			else {
				invaderxr = nextxdir;
				invaderyr = 0;
			}
			checkdir = 0;
		}

		// change invader position
		invaderx[invaderctr] = invaderx[invaderctr] + invaderxr*3;
		invadery[invaderctr] = invadery[invaderctr] + invaderyr*3;

		// determine bunker removal if invaders are too low
		if (invadery[invaderctr] > 80) {
			for (int i = 0; i < 4; i++) {
				bunkers[i] = -1;
			}
		}

		// determine game over if invaders reach bottom
		if (invadery[invaderctr] >= shipy - 14) {
			gamestatus = "gameover";
		}

		// determine screen border hit -> go down, then change direction
		if (invaderx[invaderctr] > screen.getWidth() - 20 && invaderxr > 0) {
			nextxdir = -4;
			nextydir = 4;
		}
		if (invaderx[invaderctr] < 6 && invaderxr < 0) {
			nextxdir = 4;
			nextydir = 4;
		}
		if (invaderyr != 0) { nextydir = 0; }

		//change invader shape
		invaderframe[invaderctr] = ++invaderframe[invaderctr] % 2;

		// remove killed invader
		if (invaders[invaderctr] == 6) {
			invaders[invaderctr] = -1;
			invaderanz--;
		}

		// release invadershoot
		if (invadershottimer <= 0 && invadershots < gamelevel + 1 && invadershots < 4 && invadery[invaderctr] < 80) {
			invadershottimer = 120 - gamelevel * 10;
			invadershots++;
			int flag = 0;
			for (int u = 0; u < 4; u++) {
				if (flag == 0 && invadershotx[u] == -1) {
					uint8_t x = random(0, invaderanz);
					for(int8_t i = x; i >= 0; i--)
					{
						if(invaders[i] == -1)
						{
							x++;
						}
					}
					invadershotx[u] = invaderx[x] + 2;
					invadershoty[u] = invadery[x];
					flag = 1;
				}
			}
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::drawinvaders() {
	infoshow = 1;
	for (int i = 0; i < 40; i++) {
		if (invaders[i] != -1) {
			if (invaders[i] == 0) drawBitmap(invaderx[i], invadery[i], invaderz_invader[invaders[i] + invaderframe[i]], TFT_ORANGE, 2);
			if (invaders[i] == 2) drawBitmap(invaderx[i], invadery[i], invaderz_invader[invaders[i] + invaderframe[i]], TFT_PINK, 2);
			if (invaders[i] == 4) drawBitmap(invaderx[i], invadery[i], invaderz_invader[invaders[i] + invaderframe[i]], TFT_BLUE, 2);
			if (invaders[i] == 6) drawBitmap(invaderx[i], invadery[i], invaderz_invader[invaders[i] + invaderframe[i]], TFT_YELLOW, 2);
			
			if (invadery[i] < 5) {
				infoshow = 0;
			}
		}

		// determine collission: invader & player shoot
		checkl = invaderx[i];
		checkr = invaderx[i] + 12;
		checkt = invadery[i];
		checkb = invadery[i] + 8;
		if (invaders[i] == 4) {
			checkl++;
			checkr--;
		}
		if (invaders[i] != -1 && invaders[i] != 6 && shotx >= checkl && shotx <= checkr && shoty + 2 >= checkt && shoty <= checkb) {
			score = score + invaders[i] * 10 + 10;
			invaders[i] = 6;
			shotx = -1;
			shoty = -1;
			// destroyed->note(10, 0.05);
			Playback.tone(50, 50);

			// invaderDestroyed->play();
		}

	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::drawInvaderShot()
{
	for (int i = 0; i < 4; i++) {
		if (invadershotx[i] != -1) {
			drawBitmap(invadershotx[i], invadershoty[i], invaderz_bomb[invadershotframe], TFT_RED, 2);
		}
	}
}

void SpaceInvaders::SpaceInvaders::updateInvaderShot() {
	// handle invadershoot timer & framecounter
	invadershottimer--;
	if(invadershotframe == pastInvaderShotFrame)
		invadershotframe = !invadershotframe;
	else
		pastInvaderShotFrame = !pastInvaderShotFrame;
	// move invadershots, draw & check collission
	for (int i = 0; i < 4; i++) {
		if (invadershotx[i] != -1) {
			invadershoty[i] = invadershoty[i] + 3;

			// check collission: invadershot & bunker
			for (int u = 0; u < 4; u++) {
				checkl = 22 + u * 36;
				checkr = 22 + u * 36 + 14;
				checkt = 90;
				checkb = 100;
				if (bunkers[u] != -1 && invadershotx[i] + 1 >= checkl && invadershotx[i] <= checkr && invadershoty[i] + 3 >= checkt && invadershoty[i] <= checkb) {
					bunkers[u]++;
					if (bunkers[u] > 4) { bunkers[u] = -1; }
					invadershotx[i] = -1;
					invadershoty[i] = -1;
					invadershots--;
				}

			}

			// check collission: invadershot & player
			checkl = shipx;
			checkr = shipx + 12;
			checkt = shipy;
			checkb = shipy + 6;
			if (deadcounter == -1 && invadershotx[i] + 1 >= checkl && invadershotx[i] <= checkr && invadershoty[i] + 3 >= checkt && invadershoty[i] <= checkb) {
				deadcounter = 60;
				// destroyed->note(10, 0.05);
				Playback.tone(50, 50);
				// playerDestroyed->play();
			}

			//check collission: invadershot & playershoot
			checkl = invadershotx[i];
			checkr = invadershotx[i] + 1;
			checkt = invadershoty[i];
			checkb = invadershoty[i] + 3;
			if (shotx >= checkl && shotx <= checkr && shoty + 2 >= checkt && shoty <= checkb) {
				shotx = -1;
				shoty = -1;
				invadershotx[i] = -1;
				invadershoty[i] = -1;
				invadershots--;
			}

			// invadershoot on bottom off screen?
			if (invadershoty[i] > screen.getHeight()) {
				invadershotx[i] = -1;
				invadershoty[i] = -1;
				invadershots--;
			}
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::drawbunkers() {
	for (int i = 0; i < 4; i++) {
		checkl = 22 + i * 36;
		checkr = 22 + i * 36 + 14;
		checkt = 90;
		checkb = 100;
		if (bunkers[i] != -1 && shotx >= checkl && shotx <= checkr && shoty + 2 >= checkt && shoty <= checkb) {
			bunkers[i]++;
			shotx = -1;
			shoty = -1;
			if (bunkers[i] > 4) { bunkers[i] = -1; }
		}

		if (bunkers[i] != -1) {
			drawBitmap(22 + i * 36, 90, invaderz_bunker[bunkers[i]], TFT_GREEN, 2);
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::saucerappears() {
	saucertimer--;
	if (saucertimer <= 0) {
		saucertimer = 480;
		if (infoshow == 1 && saucers == -1) {
			// mainMusic->pause();
			// removeTrack(mainMusic);
			// addTrack(ufoSound);
			// ufoSound->setSpeed(2);
			// ufoSound->setRepeat(1);
			// ufoSound->play();
			saucers = 0;
			int i = random(2);
			if (i == 0) {
				saucerx = 0;
				saucerdir = 1;
			}
			else {
				saucerx = screen.getWidth() + 18;
				saucerdir = -1;
			}
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::movesaucer() {
	if (saucers == 0) {
		saucerx = saucerx + saucerdir;
		if (saucerx <= 0 || saucerx >= screen.getWidth() + 18) {
	// removeTrack(ufoSound);
	// mainMusic->resume();
	// addTrack(mainMusic);
			saucers = -1;
		}

		// check collission: player shot & saucer  
		checkl = saucerx;
		checkr = saucerx + 20;
		checkt = 0;
		checkb = 6;
		if (shotx >= checkl && shotx <= checkr && shoty + 2 >= checkt && shoty <= checkb) {
			score += 100;
			saucers = 1;
			shotx = -1;
			shoty = -1;
			saucerwait = 30;
		}
	}
}
//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::drawsaucer() {
	if (saucers != -1) {
		drawBitmap(saucerx, 0, invaderz_saucer[saucers], TFT_RED, 2);
		if (saucers == 1) {
			saucerwait--;
			if (saucerwait <= 0) {
		// removeTrack(ufoSound);
		// addTrack(mainMusic);
		// mainMusic->play();
		// mainMusic->setRepeat(1);
				saucers = -1;
			}
		}
	}
}


//----------------------------------------------------------------------------
void SpaceInvaders::SpaceInvaders::eraseDataSetup()
{
	elapsedMillis = millis();
	blinkState = 1;
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_B, [](){
		instance->gamestatus = "dataDisplay";

	});
	buttons->setBtnPressCallback(BTN_A, [](){
		Highscore.clear();
		instance->gamestatus = "title";

	});
}
void SpaceInvaders::SpaceInvaders::eraseDataDraw()
{
	baseSprite->clear(TFT_BLACK);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setTextColor(TFT_WHITE);
	baseSprite->drawString("ARE YOU SURE?", screen.getWidth() / 2, 17);
	baseSprite->drawString("This cannot be reverted!", screen.getWidth() / 2, 37);
//	baseSprite->drawString("", screen.getWidth() / 2, 53);

	baseSprite->drawString("B: Cancel", screen.getWidth() / 2, 105);

	baseSprite->setCursor(35, 81);
	baseSprite->print("A:");

	if (blinkState){
		baseSprite->drawRect(55, 64, 30*2, 9*2, TFT_RED);
		baseSprite->drawRect(55, 64, 30*2, 9*2, TFT_RED);
		baseSprite->setTextColor(TFT_RED);
		baseSprite->setCursor(62, 81);
		baseSprite->print("DELETE");
	}
	else {
		baseSprite->fillRect(55, 64, 30*2, 9*2, TFT_RED);
		baseSprite->setTextColor(TFT_WHITE);
		baseSprite->setCursor(62, 81);
		baseSprite->print("DELETE");
	}
}
void SpaceInvaders::SpaceInvaders::eraseDataUpdate()
{
	if (millis() - elapsedMillis >= 350) {
		elapsedMillis = millis();
		blinkState = !blinkState;
	}
}

void SpaceInvaders::SpaceInvaders::dataDisplaySetup()
{
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_UP, [](){
		instance->gamestatus = "eraseData";
	});
	buttons->setBtnPressCallback(BTN_A, [](){
		instance->gamestatus = "title";
	});
	buttons->setBtnPressCallback(BTN_B, [](){
		instance->gamestatus = "title";
	});
}
void SpaceInvaders::SpaceInvaders::dataDisplay()
{
	baseSprite->clear(TFT_BLACK);
	baseSprite->setTextSize(1);
	baseSprite->setFont(&fonts::Font2);
	baseSprite->setTextColor(TFT_RED);
	baseSprite->drawString("HIGHSCORES", screen.getWidth() / 2, 15);
	baseSprite->setCursor(3, 110);
	for (int i = 1; i < 6;i++)
	{
		baseSprite->setCursor(22, 16 + i * 16);
		if(i <= Highscore.count())
		{
			baseSprite->printf("%d.   %.3s    %04d", i, Highscore.get(i - 1).name, Highscore.get(i - 1).score);
		}
		else
		{
			baseSprite->printf("%d.    ---   ----", i);
		}
	}
	baseSprite->drawString("Press UP to erase", screen.getWidth() / 2, 117);
}

void SpaceInvaders::SpaceInvaders::showtitle() {
	baseSprite->clear(TFT_BLACK);
	for(int i = 0; i < STAR_COUNT; i++)
	{
		// Remove the star from the screen by changing its pixel to the background color.
		baseSprite->drawPixel(stars[i].x, stars[i].y, BACKGROUND_COLOR);
		// Draw the star at its new coordinate.
		baseSprite->fillRect(stars[i].x, stars[i].y, 2, 2, STAR_COLOR);
	}
	baseSprite->setTextColor(TFT_WHITE);
	baseSprite->drawIcon(titleLogo, (screen.getWidth() - 73*2) / 2, 7, 73, 21, 2, TFT_WHITE);
	baseSprite->setTextColor(TFT_RED);
	baseSprite->setFreeFont(TT1);
	baseSprite->setTextSize(2);
	baseSprite->setTextDatum(textdatum_t::bottom_center);
	baseSprite->drawString("START", screen.getWidth()/2 + 5, 75);
	baseSprite->drawString("HIGHSCORES", screen.getWidth()/2 + 5, 95);
	baseSprite->drawString("QUIT", screen.getWidth()/2 + 5, 115);
	if(blinkState)
	{
		baseSprite->drawRect((screen.getWidth() - 98) / 2, 60 + cursor * 20, 98, 16, TFT_RED);
		baseSprite->drawRect((screen.getWidth() - 100) / 2, 59 + cursor * 20, 100, 18, TFT_RED);
	}
}
void SpaceInvaders::SpaceInvaders::titleSetup()
{
	cursor = 0;
	blinkMillis = millis();
	blinkState = 0;
	clearButtonCallbacks();

	buttons->setBtnPressCallback(BTN_UP, [](){
		if(instance->cursor > 0)
		{
			instance->cursor--;
			instance->blinkMillis = millis();
			instance->blinkState = 1;
			Playback.tone(500, 50);
		}
	});
	buttons->setBtnPressCallback(BTN_DOWN, [](){
		if(instance->cursor < 2)
		{
			instance->cursor++;
			instance->blinkMillis = millis();
			instance->blinkState = 1;
			Playback.tone(500, 50);
		}
	});
	
	buttons->setBtnPressCallback(BTN_A, [](){
		
		switch (instance->cursor)
		{
			case 0:
				instance->gamestatus = "newgame";
				break;
			case 1:
				instance->gamestatus = "dataDisplay";
				break;
			case 2:
				ByteBoi.backToLauncher();
		}
	});

	Playback.play(Samples.menu);
}

void SpaceInvaders::SpaceInvaders::enterInitialsSetup()
{
	tempScore = Highscore.get(0).score;
	name = "AAA";
	charCursor = 0;
	previous = "";
	elapsedMillis = millis();
	hiscoreMillis = millis();
	blinkState = 1;
	hiscoreBlink = 0;
	clearButtonCallbacks();
	buttons->setBtnPressCallback(BTN_UP,[](){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]++;
		// A-Z 0-9 :-? !-/ ' '
		if (instance->name[instance->charCursor] == '0') instance->name[instance->charCursor] = ' ';
		if (instance->name[instance->charCursor] == '!') instance->name[instance->charCursor] = 'A';
		if (instance->name[instance->charCursor] == '[') instance->name[instance->charCursor] = '0';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = '!';
	});
	buttons->setButtonHeldRepeatCallback(BTN_UP, 200, [](uint){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]++;
		// A-Z 0-9 :-? !-/ ' '
		if (instance->name[instance->charCursor] == '0') instance->name[instance->charCursor] = ' ';
		if (instance->name[instance->charCursor] == '!') instance->name[instance->charCursor] = 'A';
		if (instance->name[instance->charCursor] == '[') instance->name[instance->charCursor] = '0';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = '!';
	});
	buttons->setButtonHeldRepeatCallback(BTN_DOWN, 200, [](uint){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]--;
		if (instance->name[instance->charCursor] == ' ') instance->name[instance->charCursor] = '?';
		if (instance->name[instance->charCursor] == '/') instance->name[instance->charCursor] = 'Z';
		if (instance->name[instance->charCursor] == 31)  instance->name[instance->charCursor] = '/';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = ' ';
	});
	buttons->setBtnPressCallback(BTN_DOWN, [](){
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
		instance->name[instance->charCursor]--;
		if (instance->name[instance->charCursor] == ' ') instance->name[instance->charCursor] = '?';
		if (instance->name[instance->charCursor] == '/') instance->name[instance->charCursor] = 'Z';
		if (instance->name[instance->charCursor] == 31)  instance->name[instance->charCursor] = '/';
		if (instance->name[instance->charCursor] == '@') instance->name[instance->charCursor] = ' ';
	});
	buttons->setBtnPressCallback(BTN_LEFT, [](){
		if(instance->charCursor > 0){
			instance->charCursor--;
			instance->blinkState = 1;
			instance->elapsedMillis = millis();
		}
	});
	buttons->setBtnPressCallback(BTN_RIGHT, [](){
		if(instance->charCursor < 2){
			instance->charCursor++;
			instance->blinkState = 1;
			instance->elapsedMillis = millis();
		}
	});
	buttons->setBtnPressCallback(BTN_A, [](){
		instance->charCursor++;
		instance->blinkState = 1;
		instance->elapsedMillis = millis();
	});

	Playback.stop();
}
void SpaceInvaders::SpaceInvaders::enterInitialsUpdate() {
  
    if (millis() - elapsedMillis >= 350) //cursor blinking routine
	{
		elapsedMillis = millis();
		blinkState = !blinkState;
	}
    if(millis()-hiscoreMillis >= 1000)
    {
      hiscoreMillis = millis();
      hiscoreBlink = !hiscoreBlink;
    }
    previous = name;

    if (previous != name)
    {
      blinkState = 1;
      elapsedMillis = millis();
    }

	if(charCursor >= 3)
	{
		Score newScore;
		strcpy(newScore.name, name.c_str());
		newScore.score = score;
		Highscore.add(newScore);
		gamestatus = "dataDisplay";
	}
}
void SpaceInvaders::SpaceInvaders::enterInitialsDraw() {
	baseSprite->clear(TFT_BLACK);
    baseSprite->setFont(&fonts::Font2);
    baseSprite->setTextColor(TFT_WHITE);
    baseSprite->setTextSize(1);
    baseSprite->drawString("ENTER NAME", screen.getWidth() / 2, 16);

    if(hiscoreBlink && score > tempScore)
    	baseSprite->drawString("NEW HIGH!", screen.getWidth() / 2, 80);
    else{
    	baseSprite->setCursor(39, 80);
    	baseSprite->printf("SCORE: %04d", score);
    }

    baseSprite->setCursor(66, 55);
    baseSprite->print(name[0]);
	baseSprite->setCursor(81, 55);
    baseSprite->print(name[1]);
	baseSprite->setCursor(96, 55);
    baseSprite->print(name[2]);
    // baseSprite->drawRect(30, 38, 100, 20, TFT_WHITE);
	if(blinkState){
		baseSprite->drawFastHLine(63 + 15*charCursor, 54, 12, TFT_WHITE);
	}
}
