#ifndef ScoreScene_HPP
#define ScoreScene_HPP

#include <memory>
#include <string>
#include <vector>
#include <allegro5/allegro_audio.h>
#include "Engine/IScene.hpp"

class ScoreScene final : public Engine::IScene {
private:
    
    std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE> bgmInstance;
    std::vector<std::tuple<std::string, int, std::string>> people;
    
public:
    explicit ScoreScene() = default;
    void Initialize() override;
    void Terminate() override;
    void BackOnClick(int stage);
    void NextOnClick(int stage);
    void PrevOnClick(int stage);
    void Readtxt(const std::string& filename);
    void ShowRank();
    static std::string SystemTimeString();
};

#endif