#ifndef SAVESCENE_HPP
#define SAVESCENE_HPP

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>
#include <string>

class SaveScene final : public Engine::IScene {
private:
    float ticks;
    ALLEGRO_SAMPLE_ID bgmId;
    std::string input;
    Engine::Label* show; 

public:
    explicit SaveScene() = default;
    void Initialize() override;
    void Terminate() override;
    void Update(float deltaTime) override;
    void SaveOnClick(int stage);
    void OnKeyDown(int keyCode) override;
};
#endif   // SAVESCENE_HPP
