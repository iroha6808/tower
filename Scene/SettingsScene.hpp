#ifndef SettingsScene_HPP
#define SettingsScene_HPP
#include <memory>

#include "Engine/IScene.hpp"
#include <allegro5/allegro_audio.h>

class SettingsScene final : public Engine::IScene {
private:
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    static bool canUseCheatCode;
    Engine::Label *text;
public:
    explicit SettingsScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void ClickChangeMode(int stage);
    void BGMSlideOnValueChanged(float value);
    void SFXSlideOnValueChanged(float value);
    static bool isEasyMode();
};

#endif   // SettingsScene_HPP
