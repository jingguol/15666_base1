#include "PlayMode.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include "Load.hpp"
#include "data_path.hpp"

#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <cstdlib>

PlayMode::PlayMode() {
	srand(static_cast<uint32_t>(time(NULL)));

	/** Load assets.
	 * This might not be the best place or the best way to do it,
	 * but I failed to utilize the Load<T> function provided.
	 * The compiler just won't stop complaining. */
	std::ifstream ifs;

	ifs.open(data_path("palette.dat"));
	// Palette 0 is for player
	ifs.read(reinterpret_cast<char*>(this->ppu.palette_table[0].data()), sizeof(uint8_t) * 16);
	// Palette 1 is for enemy
	ifs.read(reinterpret_cast<char*>(this->ppu.palette_table[1].data()), sizeof(uint8_t) * 16);
	// Palette 2 is for background
	ifs.read(reinterpret_cast<char*>(this->ppu.palette_table[2].data()), sizeof(uint8_t) * 16);
	ifs.close();

	ifs.open(data_path("tile.dat"));
	// Tile 0 is player
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[0].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[0].bit1.data()), sizeof(uint8_t) * 8);
	// Tile 1 is player bullet
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[1].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[1].bit1.data()), sizeof(uint8_t) * 8);
	// Tile 2-5 are enemy
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[2].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[2].bit1.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[3].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[3].bit1.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[4].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[4].bit1.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[5].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[5].bit1.data()), sizeof(uint8_t) * 8);
	// Tile 6 is enemy bullet
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[6].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[6].bit1.data()), sizeof(uint8_t) * 8);
	ifs.close();
	// Tile 7 is teleport marker
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[7].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[7].bit1.data()), sizeof(uint8_t) * 8);
	// Tile 8 is background
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[8].bit0.data()), sizeof(uint8_t) * 8);
	ifs.read(reinterpret_cast<char*>(this->ppu.tile_table[8].bit1.data()), sizeof(uint8_t) * 8);


	// Initialize background
	for(auto it = this->ppu.background.begin(); it != this->ppu.background.end(); ++it) {
		*it = (2 << 8) | 8;
	}

	// Sprite 0-5 are reserved for player, enemy, tele
	for (uint32_t i = 6; i < 64; ++i) {
		availableSprites.push_back(i);
	}
}

PlayMode::~PlayMode() { }

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LSHIFT) {
			lshift.downs += 1;
			lshift.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LSHIFT) {
			lshift.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {
	
	if (lshift.downs > 0) {
		tele.pos = player.pos;
		tele.active = true;
	}

	if (tele.active && lshift.pressed) {
		if (left.pressed) tele.pos.x -= tele.velocity * elapsed;
		if (right.pressed) tele.pos.x += tele.velocity * elapsed;
		if (down.pressed) tele.pos.y -= tele.velocity * elapsed;
		if (up.pressed) tele.pos.y += tele.velocity * elapsed;
		tele.pos.x = std::max(4.0f, tele.pos.x);
		tele.pos.x = std::min(PPU466::ScreenWidth - 5.0f, tele.pos.x);
		tele.pos.y = std::max(4.0f, tele.pos.y);
		tele.pos.y = std::min(PPU466::ScreenHeight - 5.0f, tele.pos.y);
	} else if (tele.active && !lshift.pressed) {
		player.pos = tele.pos;
		tele.active = false;
	} else {
		if (left.pressed) player.pos.x -= player.velocity * elapsed;
		if (right.pressed) player.pos.x += player.velocity * elapsed;
		if (down.pressed) player.pos.y -= player.velocity * elapsed;
		if (up.pressed) player.pos.y += player.velocity * elapsed;
		player.pos.x = std::max(4.0f, player.pos.x);
		player.pos.x = std::min(PPU466::ScreenWidth - 5.0f, player.pos.x);
		player.pos.y = std::max(3.0f, player.pos.y);
		player.pos.y = std::min(PPU466::ScreenHeight - 6.0f, player.pos.y);

		player.timer += elapsed;
		if (player.timer >= player.threshold) {
			if (!availableSprites.empty()) {
				int index = availableSprites.back();
				availableSprites.pop_back();
				bullets.push_back(Bullet(Bullet::Owner::player, player.pos, 
					glm::vec2(0.0f, 120.0f), index));
			}
			player.timer -= player.threshold;	
		}

		enemy.timer1 += elapsed;
		enemy.timer2 += elapsed;
		if (enemy.timer1 >= enemy.threshold1) {
			const int numBullets = 20;
			for (int i = 0; i < numBullets; ++i) {
				if (!availableSprites.empty()) {
					int index = availableSprites.back();
					availableSprites.pop_back();
					float angle = (45.0f + 90.0f / numBullets * i) / 180.0f * 3.14159f;
					bullets.push_back(Bullet(Bullet::Owner::enemy, enemy.pos,
						glm::vec2(-100.0f * cosf(angle), -100.0f * sinf(angle)), index));
				}
			}
			enemy.timer1 -= enemy.threshold1;
		}
		if (enemy.timer2 >= enemy.threshold2) {
			if (!availableSprites.empty()) {
				int index = availableSprites.back();
				availableSprites.pop_back();
				// float angle = (rand() % 181) / 180.0f * 3.14159f;
				float angle = atan2f(enemy.pos.y - player.pos.y, enemy.pos.x - player.pos.x);
				angle += (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.25f;
				bullets.push_back(Bullet(Bullet::Owner::enemy, enemy.pos,
						glm::vec2(-150.0f * cosf(angle), -150.0f * sinf(angle)), index));
			}
			enemy.timer2 = 0;
			enemy.threshold2 = static_cast<float>(rand()) / RAND_MAX / 3.0f + 0.1f;
		}

		auto it = bullets.begin();
		while (it != bullets.end()) {
			it->pos += elapsed * it->velocity;
			if (it->owner == Bullet::Owner::player) {
				if (collideEnemy(*it)) {
					enemy.hp--;
					int index = it->spriteIndex;
					availableSprites.push_back(index);
					it = bullets.erase(it);
					continue;
				}
			} else {
				if (collidePlayer(*it)) {
					set_current(nullptr);
					int index = it->spriteIndex;
					availableSprites.push_back(index);
					it = bullets.erase(it);
					continue;
				}
			}
			if (it->pos.x < 0.0f || it->pos.x > PPU466::ScreenWidth ||
				it->pos.y < 0.0f || it->pos.y > PPU466::ScreenHeight) {
				int index = it->spriteIndex;
				availableSprites.push_back(index);
				it = bullets.erase(it);
			} else {
				++it;
			}
		}
	}

	if (enemy.hp <= 0) {
		set_current(nullptr);
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
	lshift.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---

	// Player sprite:
	ppu.sprites[0].x = int8_t(player.pos.x - 3.0f);
	ppu.sprites[0].y = int8_t(player.pos.y - 2.0f);
	ppu.sprites[0].index = 0;
	ppu.sprites[0].attributes = 0;

	// Enemy sprite
	ppu.sprites[1].x = int8_t(enemy.pos.x + 1.0f);
	ppu.sprites[1].y = int8_t(enemy.pos.y);
	ppu.sprites[1].index = 2;
	ppu.sprites[1].attributes = 1;
	ppu.sprites[2].x = int8_t(enemy.pos.x - 7.0f);
	ppu.sprites[2].y = int8_t(enemy.pos.y);
	ppu.sprites[2].index = 3;
	ppu.sprites[2].attributes = 1;
	ppu.sprites[3].x = int8_t(enemy.pos.x + 1.0f);
	ppu.sprites[3].y = int8_t(enemy.pos.y + 8.0f);
	ppu.sprites[3].index = 4;
	ppu.sprites[3].attributes = 1;
	ppu.sprites[4].x = int8_t(enemy.pos.x - 7.0f);
	ppu.sprites[4].y = int8_t(enemy.pos.y + 8.0f);
	ppu.sprites[4].index = 5;
	ppu.sprites[4].attributes = 1;

	for (auto it = bullets.cbegin(); it != bullets.cend(); ++it) {
		if (it->owner == Bullet::Owner::player) {
			int index = it->spriteIndex;
			ppu.sprites[index].x = int8_t(it->pos.x - 3.0f);
			ppu.sprites[index].y = int8_t(it->pos.y - 3.0f);
			ppu.sprites[index].index = 1;
			ppu.sprites[index].attributes = 0;
		} else {
			int index = it->spriteIndex;
			ppu.sprites[index].x = int8_t(it->pos.x - 3.0f);
			ppu.sprites[index].y = int8_t(it->pos.y - 3.0f);
			ppu.sprites[index].index = 6;
			ppu.sprites[index].attributes = 1;
		}
	}

	if (tele.active) {
		ppu.sprites[5].x = int8_t(tele.pos.x - 3.0f);
		ppu.sprites[5].y = int8_t(tele.pos.y - 3.0f);
		ppu.sprites[5].index = 7;
		ppu.sprites[5].attributes = 0;
	} else {
		ppu.sprites[5].x = 255;
		ppu.sprites[5].y = 255;
		ppu.sprites[5].index = 7;
		ppu.sprites[5].attributes = 0;
	}

	//--- actually draw ---
	ppu.draw(drawable_size);
}

bool PlayMode::collideEnemy(Bullet& b) const {
	return (b.pos.x >= enemy.pos.x - 7.0f && b.pos.x <= enemy.pos.x + 7.0f &&
		b.pos.y >= enemy.pos.y);
}

bool PlayMode::collidePlayer(Bullet& b) const {
	return (b.pos.x >= player.pos.x - 2.0f && b.pos.x <= player.pos.x + 2.0f &&
		b.pos.y >= player.pos.y - 2.0f && b.pos.y <= player.pos.y + 2.0f);
}