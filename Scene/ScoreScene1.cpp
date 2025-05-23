#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro.h>
#include <functional>
#include <memory>
#include <string>
#include <ctime>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cstdio>

#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Point.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "Scene/ScoreScene1.hpp"
#include "UI/Component/ImageButton.hpp"
#include "UI/Component/Label.hpp"
#include "UI/Component/Slider.hpp"

using namespace std;
int curpage = -1;

bool cmp(const tuple<string, int, string>& a, const tuple<string, int, string>& b) {
    return get<1>(a) > get<1>(b);
}


void ScoreScene::Readtxt(const string& filename) {
    people.clear();

    ifstream file(filename); 
    if (!file.is_open()) {
        printf("cannot open score.txt\n");
        return;
    }
    else printf("open score.txt succeessfully\n");

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string name, sys_date, sys_time;
        int score;
        iss >> name >> score >> sys_date >> sys_time;
        people.push_back({name, score, sys_date + " " + sys_time}); 
    }

    sort(people.begin(), people.end(), cmp);
}

void ScoreScene::ShowRank() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;

    Readtxt("C:/2025_I2P2_TowerDefense-main/Resource/scoreboard.txt");
    AddNewObject(new Engine::Label("Local Ranking", "pirulen.ttf", 120, halfW, halfH / 3, 10, 255, 255, 255, 0.5, 0.5));

    if (curpage == 2) {
        for (size_t i = 6; i < 12; i++) {
            AddNewObject(new Engine::Label("#" + to_string(i + 1) + "  " + get<0>(people[i]) + " - " + to_string(get<1>(people[i])) + " at " + get<2>(people[i]), "pirulen.ttf", 40, halfW, halfH / 3 + 100 + (i - 6) * 70, 10, 255, 255, 255, 0.5, 0.5));
        }
        return;
    }

    for (size_t i = 0; i < 6; i++) {
        AddNewObject(new Engine::Label("#" + to_string(i + 1) + "  " + get<0>(people[i]) + " - " + to_string(get<1>(people[i])) + " at " + get<2>(people[i]), "pirulen.ttf", 40, halfW, halfH / 3 + 100 + i * 70, 10, 255, 255, 255, 0.5, 0.5));
    }
}

void ScoreScene::Initialize() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    int halfW = w / 2;
    int halfH = h / 2;
    int btnY = halfH * 3 / 2;
    int btnW = 200;
    int btnH = 100;
    int spacing = 220;

    ShowRank();

    Engine::ImageButton* btnBack = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - btnW / 2, btnY - btnH / 2 + 100, btnW, btnH);
    btnBack->SetOnClickCallback(bind(&ScoreScene::BackOnClick, this, 1));
    AddNewControlObject(btnBack);
    AddNewObject(new Engine::Label("Back", "pirulen.ttf", 48, halfW, btnY + 100, 0, 0, 0, 255, 0.5, 0.5));

    Engine::ImageButton* btnNext = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW + spacing - btnW / 2, btnY - btnH / 2 + 100, btnW, btnH);
    btnNext->SetOnClickCallback(bind(&ScoreScene::NextOnClick, this, 1));
    AddNewControlObject(btnNext);
    AddNewObject(new Engine::Label("pg dn", "pirulen.ttf", 48, halfW + spacing, btnY + 100, 0, 0, 0, 255, 0.5, 0.5));

    Engine::ImageButton* btnPrev = new Engine::ImageButton("stage-select/dirt.png", "stage-select/floor.png", halfW - spacing - btnW / 2, btnY - btnH / 2 + 100, btnW, btnH);
    btnPrev->SetOnClickCallback(bind(&ScoreScene::PrevOnClick, this, 1));
    AddNewControlObject(btnPrev);
    AddNewObject(new Engine::Label("pg up", "pirulen.ttf", 48, halfW - spacing, btnY + 100, 0, 0, 0, 255, 0.5, 0.5));

    bgmInstance = AudioHelper::PlaySample("select.ogg", true, AudioHelper::BGMVolume);
}
void ScoreScene::Terminate() {
    AudioHelper::StopSample(bgmInstance);
    bgmInstance = shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void ScoreScene::BackOnClick(int stage) {
    Engine::GameEngine::GetInstance().ChangeScene("start");
}
void ScoreScene::NextOnClick(int stage) {
    curpage = 2;
    Engine::GameEngine::GetInstance().ChangeScene("score2");
}
void ScoreScene::PrevOnClick(int stage) {
    curpage = 1;
    Engine::GameEngine::GetInstance().ChangeScene("score1");
}
std::string ScoreScene::SystemTimeString() {
    std::time_t system_time = std::time(nullptr);
    std::tm* t = std::localtime(&system_time);
    char time_string[50];
    std::strftime(time_string, sizeof(time_string), "%Y/%m/%d %H:%M:%S", t);
    return std::string(time_string);
}