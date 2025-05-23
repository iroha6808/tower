#include <functional>
#include <string>
#include <fstream>
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "PlayScene.hpp"
#include "UI/Component/Image.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "Scene/PlayScene.hpp"
#include "Scene/ScoreScene1.hpp"
#include "SaveScene.hpp"

void SaveScene::Initialize() {
    ticks = 0;
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    
    AddNewObject(new Engine::Image("win/benjamin-sad.png", halfW, halfH + 50, 0, 0, 0.5, 0.5));
    AddNewObject(new Engine::Label("Save Your Name", "pirulen.ttf", 48, halfW, halfH / 4 - 10, 255, 255, 255, 255, 0.5, 0.5));
    AddNewObject(new Engine::Label("Press Enter To Save", "pirulen.ttf", 24, halfW, halfH / 4 + 50, 255, 255, 255, 255, 0.5, 0.5));

    input = "";
    show = new Engine::Label("_", "pirulen.ttf", 32, halfW, halfH / 2, 255, 255, 255, 255, 0.5, 0.5);
    AddNewObject(show);

    /*
    Engine::ImageButton *btn;
    btn = new Engine::ImageButton("win/dirt.png", "win/floor.png", halfW - 200, halfH * 7 / 4 - 50, 400, 100);
    btn->SetOnClickCallback(std::bind(&SaveScene::SaveOnClick, this, 2));
    AddNewControlObject(btn);
    AddNewObject(new Engine::Label("Save", "pirulen.ttf", 48, halfW, halfH * 7 / 4, 0, 0, 0, 255, 0.5, 0.5));
    */
}


void SaveScene::Terminate() {
    IScene::Terminate();
    AudioHelper::StopBGM(bgmId);
}

void SaveScene::Update(float deltaTime) {
    ticks += deltaTime;
    if (ticks > 4 && ticks < 100 &&
        dynamic_cast<PlayScene *>(Engine::GameEngine::GetInstance().GetScene("play"))->MapId == 2) {
        ticks = 100;
        bgmId = AudioHelper::PlayBGM("happy.ogg");
    }
}

void SaveScene::OnKeyDown(int keyCode) {
    if (keyCode == ALLEGRO_KEY_ENTER) {
        SaveOnClick(0);
        return;
    }
    else if (keyCode == ALLEGRO_KEY_BACKSPACE) {
        if (!input.empty()) input.pop_back();
    } 
    else {
        if (input.length() < 15) {  
            if ((keyCode >= ALLEGRO_KEY_A && keyCode <= ALLEGRO_KEY_Z)) {
                char c = 'A' + (keyCode - ALLEGRO_KEY_A);
                input += c;
            }
        }
    }
    show->Text = input + "_";
}

void SaveScene::SaveOnClick(int stage) {
    // how to get score
    auto* playScene = dynamic_cast<PlayScene*>(Engine::GameEngine::GetInstance().GetScene("play"));
    int score = 0;
    if (playScene) {
        score = playScene->GetScore();
    }

    std::ofstream file("C:/2025_I2P2_TowerDefense-main/Resource/scoreboard.txt", std::ios::app);
    if (file.is_open()) {
        file << input << " " << score << " " << ScoreScene::SystemTimeString() << "\n";
        file.close();
    }

    Engine::GameEngine::GetInstance().ChangeScene("start");
}