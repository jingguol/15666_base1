#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <list>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, lshift;

	PPU466 ppu;

	struct Player {
		glm::vec2 pos = glm::vec2(PPU466::ScreenWidth / 2.0f - 1.0f, 5.0f);
		const float velocity = 60.0f;
		float timer = 0.0f;
		const float threshold = 0.5f;
	} player;

	struct Enemy {
		const glm::vec2 pos = glm::vec2(PPU466::ScreenWidth / 2.0f - 1.0f, 
			PPU466::ScreenHeight - 16.0f); 
		int hp = 100;
		float timer1 = 0.0f;
		float timer2 = 0.0f;
		const float threshold1 = 5.0f;
		float threshold2 = 0.5f;
	} enemy;

	std::list<int> availableSprites;

	struct Bullet {
		enum Owner {
			player = 0,
			enemy = 1
		} owner;
		glm::vec2 pos;
		glm::vec2 velocity;
		uint32_t spriteIndex;
		Bullet(Owner o, glm::vec2 p, glm::vec2 v, uint32_t i)
			:owner(o), pos(p), velocity(v), spriteIndex(i) { }
	};

	std::vector<Bullet> bullets;

	bool collidePlayer(Bullet& b) const;
	bool collideEnemy(Bullet& b) const;

	struct Teleport {
		glm::vec2 pos;
		const float velocity = 100.0f;
		bool active = false;
	} tele;
};
