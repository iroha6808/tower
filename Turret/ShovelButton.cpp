#include <allegro5/color.h>
#include <allegro5/allegro_primitives.h>


#include "Engine/GameEngine.hpp"
#include "Engine/IScene.hpp"
#include "Scene/PlayScene.hpp"
#include "ShovelButton.hpp"

bool ShovelButton::shovel_mode = false;

PlayScene *ShovelButton::getPlayScene() {
    return dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetActiveScene());
}
ShovelButton::ShovelButton(std::string img, std::string imgIn, Engine::Sprite Base, Engine::Sprite Shovel, float x, float y, int money) : ImageButton(img, imgIn, x, y), money(money), Base(Base), Shovel(Shovel) {
}
void ShovelButton::Update(float deltaTime) {
    ImageButton::Update(deltaTime);
    if (getPlayScene()->GetMoney() >= money) {
        Enabled = true;
        Base.Tint = Shovel.Tint = al_map_rgba(255, 255, 255, 255);
    } 
    else {
        Enabled = false;
        Base.Tint = Shovel.Tint = al_map_rgba(0, 0, 0, 160);
    }
}
void ShovelButton::Draw() const {
    ImageButton::Draw();
    Base.Draw();
    Shovel.Draw();

    if (shovel_mode) {
        al_draw_rectangle(Position.x, Position.y,
                          Position.x + Size.x, Position.y + Size.y,
                          al_map_rgb(255, 0, 0), 3);

        al_draw_filled_rectangle(Position.x, Position.y,
                                 Position.x + Size.x, Position.y + Size.y,
                                 al_map_rgba(255, 0, 0, 50));
    }
}


void ShovelButton::ClickShovel() {
    if (!Enabled) return;
    shovel_mode = !shovel_mode;
}
