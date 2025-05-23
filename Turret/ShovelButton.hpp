#ifndef SHOVELBUTTON_HPP
#define SHOVELBUTTON_HPP
#include <string>

#include "Engine/Sprite.hpp"
#include "UI/Component/ImageButton.hpp"

class PlayScene;

class ShovelButton : public Engine::ImageButton {
protected:
    PlayScene *getPlayScene();

public:
    int money;
    static bool shovel_mode;
    Engine::Sprite Base;
    Engine::Sprite Shovel;
    ShovelButton(std::string img, std::string imgIn, Engine::Sprite Base, Engine::Sprite Turret, float x, float y, int money);
    void Update(float deltaTime) override;
    void Draw() const override;
    void ClickShovel();
};
#endif   // TURRETBUTTON_HPP
