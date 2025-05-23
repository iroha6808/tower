#include <algorithm>
#include <allegro5/allegro.h>
#include <cmath>
#include <fstream>
#include <functional>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include "Enemy/Enemy.hpp"
#include "Enemy/BigEnemy.hpp"
#include "Enemy/SoldierEnemy.hpp"
#include "Enemy/PlaneEnemy.hpp"
#include "Enemy/TankEnemy.hpp"
#include "Enemy/Health.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/GameEngine.hpp"
#include "Engine/Group.hpp"
#include "Engine/LOG.hpp"
#include "Engine/Resources.hpp"
#include "PlayScene.hpp"
#include "SettingsScene.hpp"
#include "ScoreScene1.hpp"
#include "Turret/LaserTurret.hpp"
#include "Turret/MachineGunTurret.hpp"
#include "Turret/TrafficLight.hpp"
#include "Turret/NewTurret.hpp"
#include "Turret/TurretButton.hpp"
#include "Turret/ShovelButton.hpp"
#include "UI/Animation/DirtyEffect.hpp"
#include "UI/Animation/Plane.hpp"
#include "UI/Component/Label.hpp"

// TODO HACKATHON-4 (1/3): Trace how the game handles keyboard input.
// TODO HACKATHON-4 (2/3): Find the cheat code sequence in this file.
// TODO HACKATHON-4 (3/3): When the cheat code is entered, a plane should be spawned and added to the scene.
// TODO HACKATHON-5 (1/4): There's a bug in this file, which crashes the game when you win. Try to find it.
// TODO HACKATHON-5 (2/4): The "LIFE" label are not updated when you lose a life. Try to fix it.

bool PlayScene::DebugMode = false;
const std::vector<Engine::Point> PlayScene::directions = { Engine::Point(-1, 0), Engine::Point(0, -1), Engine::Point(1, 0), Engine::Point(0, 1) };
const int PlayScene::MapWidth = 20, PlayScene::MapHeight = 13;
const int PlayScene::BlockSize = 64;
const float PlayScene::DangerTime = 7.61;
const Engine::Point PlayScene::SpawnGridPoint = Engine::Point(-1, 0);
const Engine::Point PlayScene::EndGridPoint = Engine::Point(MapWidth, MapHeight - 1);
// here is the cheat code
const std::vector<int> PlayScene::code = {
    ALLEGRO_KEY_UP, ALLEGRO_KEY_UP, ALLEGRO_KEY_DOWN, ALLEGRO_KEY_DOWN,
    ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT, ALLEGRO_KEY_LEFT, ALLEGRO_KEY_RIGHT,
    ALLEGRO_KEY_B, ALLEGRO_KEY_A/*, ALLEGRO_KEYMOD_SHIFT*/ ,ALLEGRO_KEY_ENTER
};
Engine::Point PlayScene::GetClientSize() {
    return Engine::Point(MapWidth * BlockSize, MapHeight * BlockSize);
}
void PlayScene::Initialize() {
    mapState.clear();
    keyStrokes.clear();
    ticks = 0;
    deathCountDown = -1;
    lives = 15;
    money = 150;
    cur_score = 100;
    SpeedMult = 1;
    // Add groups from bottom to top.
    AddNewObject(TileMapGroup = new Group());
    AddNewObject(GroundEffectGroup = new Group());
    AddNewObject(DebugIndicatorGroup = new Group());
    AddNewObject(TowerGroup = new Group());
    AddNewObject(EnemyGroup = new Group());
    AddNewObject(BulletGroup = new Group());
    AddNewObject(EffectGroup = new Group());
    // Should support buttons.
    AddNewControlObject(UIGroup = new Group());
    ReadMap();
    ReadEnemyWave();
    mapDistance = CalculateBFSDistance();
    ConstructUI();
    imgTarget = new Engine::Image("play/target.png", 0, 0);
    imgTarget->Visible = false;
    preview = nullptr;
    UIGroup->AddNewObject(imgTarget);
    // Preload Lose Scene
    deathBGMInstance = Engine::Resources::GetInstance().GetSampleInstance("astronomia.ogg");
    Engine::Resources::GetInstance().GetBitmap("lose/benjamin-happy.png");
    // Start BGM.
    bgmId = AudioHelper::PlayBGM("play.ogg");

    
}
void PlayScene::Terminate() {
    AudioHelper::StopBGM(bgmId);
    AudioHelper::StopSample(deathBGMInstance);
    deathBGMInstance = std::shared_ptr<ALLEGRO_SAMPLE_INSTANCE>();
    IScene::Terminate();
}
void PlayScene::Update(float deltaTime) {
    
    // If we use deltaTime directly, then we might have Bullet-through-paper problem.
    // Reference: Bullet-Through-Paper
    if (SpeedMult == 0)
        deathCountDown = -1;
    else if (deathCountDown != -1)
        SpeedMult = 1;
    // Calculate danger zone.
    std::vector<float> reachEndTimes;
    for (auto &it : EnemyGroup->GetObjects()) {
        reachEndTimes.push_back(dynamic_cast<Enemy *>(it)->reachEndTime);
    }
    // Can use Heap / Priority-Queue instead. But since we won't have too many enemies, sorting is fast enough.
    std::sort(reachEndTimes.begin(), reachEndTimes.end());
    float newDeathCountDown = -1;
    int danger = lives;
    for (auto &it : reachEndTimes) {
        if (it <= DangerTime) {
            danger--;
            if (danger <= 0) {
                // Death Countdown
                float pos = DangerTime - it;
                if (it > deathCountDown) {
                    // Restart Death Count Down BGM.
                    AudioHelper::StopSample(deathBGMInstance);
                    if (SpeedMult != 0)
                        deathBGMInstance = AudioHelper::PlaySample("astronomia.ogg", false, AudioHelper::BGMVolume, pos);
                }
                float alpha = pos / DangerTime;
                alpha = std::max(0, std::min(255, static_cast<int>(alpha * alpha * 255)));
                dangerIndicator->Tint = al_map_rgba(255, 255, 255, alpha);
                newDeathCountDown = it;
                break;
            }
        }
    }
    deathCountDown = newDeathCountDown;
    if (SpeedMult == 0)
        AudioHelper::StopSample(deathBGMInstance);
    if (deathCountDown == -1 && lives > 0) {
        AudioHelper::StopSample(deathBGMInstance);
        dangerIndicator->Tint.a = 0;
    }
    if (SpeedMult == 0)
        deathCountDown = -1;
    for (int i = 0; i < SpeedMult; i++) {
        IScene::Update(deltaTime);
        // Check if we should create new enemy.
        ticks += deltaTime;
        if (enemyWaveData.empty()) {
            if (EnemyGroup->GetObjects().empty()) {
                std::cout<<"win\n";
                Engine::GameEngine::GetInstance().ChangeScene("win");
            }
            continue;
        }
        auto current = enemyWaveData.front();
        if (ticks < current.second)
            continue;
        ticks -= current.second;
        enemyWaveData.pop_front();
        const Engine::Point SpawnCoordinate = Engine::Point(SpawnGridPoint.x * BlockSize + BlockSize / 2, SpawnGridPoint.y * BlockSize + BlockSize / 2);
        Enemy *enemy;
        switch (current.first) {
            case 1:
                EnemyGroup->AddNewObject(enemy = new SoldierEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            // TODO HACKATHON-3 (2/3): Add your new enemy here.
            case 2:
                EnemyGroup->AddNewObject(enemy = new PlaneEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 3:
                EnemyGroup->AddNewObject(enemy = new TankEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 4:
                EnemyGroup->AddNewObject(enemy = new BigEnemy(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            case 5:
                EnemyGroup->AddNewObject(enemy = new Health(SpawnCoordinate.x, SpawnCoordinate.y));
                break;
            default:
                continue;
        }
        enemy->UpdatePath(mapDistance);
        // Compensate the time lost.
        enemy->Update(ticks);
    }
    if (preview) {
        preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
        // To keep responding when paused.
        preview->Update(deltaTime);
    }
}
void PlayScene::Draw() const {
    IScene::Draw();
    if (DebugMode) {
        // Draw reverse BFS distance on all reachable blocks.
        for (int i = 0; i < MapHeight; i++) {
            for (int j = 0; j < MapWidth; j++) {
                if (mapDistance[i][j] != -1) {
                    // Not elegant nor efficient, but it's quite enough for debugging.
                    Engine::Label label(std::to_string(mapDistance[i][j]), "pirulen.ttf", 32, (j + 0.5) * BlockSize, (i + 0.5) * BlockSize);
                    label.Anchor = Engine::Point(0.5, 0.5);
                    label.Draw();
                }
            }
        }
    }
}
void PlayScene::OnMouseDown(int button, int mx, int my) {
    if ((button & 1) && !imgTarget->Visible && preview) {
        // Cancel turret construct.
        UIGroup->RemoveObject(preview->GetObjectIterator());
        preview = nullptr;
    }
    IScene::OnMouseDown(button, mx, my);
}
void PlayScene::OnMouseMove(int mx, int my) {
    IScene::OnMouseMove(mx, my);
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if (!preview || x < 0 || x >= MapWidth || y < 0 || y >= MapHeight) {
        imgTarget->Visible = false;
        return;
    }
    imgTarget->Visible = true;
    imgTarget->Position.x = x * BlockSize;
    imgTarget->Position.y = y * BlockSize;
}
void PlayScene::OnMouseUp(int button, int mx, int my) {
    IScene::OnMouseUp(button, mx, my);
    if (!imgTarget->Visible)
        return;
    const int x = mx / BlockSize;
    const int y = my / BlockSize;
    if (button & 1) {
        if (mapState[y][x] != TILE_OCCUPIED) {
            if (!preview)
                return;
            // Check if valid.
            if (!CheckSpaceValid(x, y)) {
                Engine::Sprite *sprite;
                GroundEffectGroup->AddNewObject(sprite = new DirtyEffect("play/target-invalid.png", 1, x * BlockSize + BlockSize / 2, y * BlockSize + BlockSize / 2));
                sprite->Rotation = 0;
                return;
            }
            // Purchase.
            EarnMoney(-preview->GetPrice());
            // Remove Preview.
            preview->GetObjectIterator()->first = false;
            UIGroup->RemoveObject(preview->GetObjectIterator());
            // Construct real turret.
            preview->Position.x = x * BlockSize + BlockSize / 2;
            preview->Position.y = y * BlockSize + BlockSize / 2;
            preview->Enabled = true;
            preview->Preview = false;
            preview->Tint = al_map_rgba(255, 255, 255, 255);
            TowerGroup->AddNewObject(preview);
            // To keep responding when paused.
            preview->Update(0);
            // Remove Preview.
            preview = nullptr;

            mapState[y][x] = TILE_OCCUPIED;
            OnMouseMove(mx, my);
        }
    }
}
void PlayScene::OnKeyDown(int keyCode) {
    IScene::OnKeyDown(keyCode);
    if (keyCode == ALLEGRO_KEY_TAB) {
        DebugMode = !DebugMode;
    } else {
        keyStrokes.push_back(keyCode);
        if (keyStrokes.size() > code.size())
            keyStrokes.pop_front();
    }
    if (keyCode == ALLEGRO_KEY_Q) {
        // Hotkey for MachineGunTurret.
        UIBtnClicked(0);
    } 
    else if (keyCode == ALLEGRO_KEY_W) {
        // Hotkey for LaserTurret.
        UIBtnClicked(1);
    }
    /*else if (keyCode == ALLEGRO_KEY_S) {
        UIBtnClicked(2);
    }*/
    else if (keyCode >= ALLEGRO_KEY_0 && keyCode <= ALLEGRO_KEY_9) {
        // Hotkey for Speed up.
        SpeedMult = keyCode - ALLEGRO_KEY_0;
    }
    // cheat code checking
    if (keyStrokes.size() == code.size()) {
        auto cur_in = keyStrokes.begin();
        auto cheatcode = code.begin();
        bool same = true;
        while (cur_in != keyStrokes.end() && cheatcode != code.end()) {
            if (*cur_in != *cheatcode) {
                same = false;
                break;
            }
            cur_in++;
            cheatcode++;
        }
        if (same && SettingsScene::isEasyMode()) {
            // do something
            EffectGroup->AddNewObject(new Plane());
            keyStrokes.clear();
            money += 10000;
            cur_score += 10000;
            UIScore->Text = "SCORE " + std::to_string(cur_score);
        }
        if (!SettingsScene::isEasyMode()) {
            UIGroup->AddNewObject(UIMode = new Engine::Label("This mode doesn't suppot code", "pirulen.ttf", 12, 1440, Engine::GameEngine::GetInstance().GetScreenSize().y - 90, 75, 0, 150, 255, 0.5, 0.5));
        }
    }

}
void PlayScene::Hit() {
    lives--;
    cur_score--;
    if (cur_score < 0) cur_score = 0;
    UILives->Text = "LIFE " + std::to_string(lives);
    UIScore->Text = "SCORE " + std::to_string(cur_score);
    if (lives <= 0) {
        Engine::GameEngine::GetInstance().ChangeScene("lose");
    }
    
}
int PlayScene::GetMoney() const {
    return money;
}
void PlayScene::EarnMoney(int money) {
    this->money += money;
    UIMoney->Text = std::string("$") + std::to_string(this->money);
    this->cur_score += money;
    UIScore->Text = std::string("SCORES " + std::to_string(this->cur_score));
}
void PlayScene::ReadMap() {
    std::string filename = std::string("Resource/map") + std::to_string(MapId) + ".txt";
    // Read map file.
    char c;
    std::vector<bool> mapData;
    std::ifstream fin(filename);
    while (fin >> c) {
        switch (c) {
            case '0': mapData.push_back(false); break;
            case '1': mapData.push_back(true); break;
            case '\n':
            case '\r':
                if (static_cast<int>(mapData.size()) / MapWidth != 0)
                    throw std::ios_base::failure("Map data is corrupted.");
                break;
            default: throw std::ios_base::failure("Map data is corrupted.");
        }
    }
    fin.close();
    // Validate map data.
    if (static_cast<int>(mapData.size()) != MapWidth * MapHeight)
        throw std::ios_base::failure("Map data is corrupted.");
    // Store map in 2d array.
    mapState = std::vector<std::vector<TileType>>(MapHeight, std::vector<TileType>(MapWidth));
    for (int i = 0; i < MapHeight; i++) {
        for (int j = 0; j < MapWidth; j++) {
            const int num = mapData[i * MapWidth + j];
            mapState[i][j] = num ? TILE_FLOOR : TILE_DIRT;
            if (num)
                TileMapGroup->AddNewObject(new Engine::Image("play/floor.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
            else
                TileMapGroup->AddNewObject(new Engine::Image("play/dirt.png", j * BlockSize, i * BlockSize, BlockSize, BlockSize));
        }
    }
}
void PlayScene::ReadEnemyWave() {
    std::string filename = std::string("C:/2025_I2P2_TowerDefense-main/Resource/enemy") + std::to_string(MapId) + ".txt";
    // Read enemy file.
    float type, wait, repeat;
    enemyWaveData.clear();
    std::ifstream fin(filename);
    while (fin >> type && fin >> wait && fin >> repeat) {
        for (int i = 0; i < repeat; i++)
            enemyWaveData.emplace_back(type, wait);
    }
    fin.close();
}
void PlayScene::ConstructUI() {
    int w = Engine::GameEngine::GetInstance().GetScreenSize().x;
    int h = Engine::GameEngine::GetInstance().GetScreenSize().y;
    // Background
    UIGroup->AddNewObject(new Engine::Image("play/sand.png", 1280, 0, 320, 832));
    // Text
    UIGroup->AddNewObject(new Engine::Label(std::string("Stage ") + std::to_string(MapId), "pirulen.ttf", 32, 1294, 0));
    /*UIGroup->AddNewObject(new Engine::Label(std::string("Shovel Mode : S"), "pirulen.ttf", 16, 1374, 208));*/
    UIGroup->AddNewObject(UIMoney = new Engine::Label(std::string("$") + std::to_string(money), "pirulen.ttf", 24, 1294, 48));
    UIGroup->AddNewObject(UILives = new Engine::Label(std::string("Life ") + std::to_string(lives), "pirulen.ttf", 24, 1294, 88));
    UIGroup->AddNewObject(UIScore = new Engine::Label("SCORES  " + std::to_string(cur_score), "pirulen.ttf", 24, 1400, h - 30, 75, 0, 150, 255, 0.5, 0.5));
    TurretButton *btn;
    // Button 1
    btn = new TurretButton("play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1294, 136, 0, 0, 0, 0),
        Engine::Sprite("play/turret-1.png", 1294, 128, 0, 0, 0, 0), 1294, 136, MachineGunTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 0));
    UIGroup->AddNewControlObject(btn);

    // Button 1.5
    btn = new TurretButton("play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1294, 216, 0, 0, 0, 0),
        Engine::Sprite("play/turret-4.png", 1294, 208, 0, 0, 0, 0), 1294, 216, 0);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 2));
    UIGroup->AddNewControlObject(btn);

    // Button 2
    btn = new TurretButton("play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1294, 296, 0, 0, 0, 0),
        Engine::Sprite("play/turret-2.png", 1294, 288, 0, 0, 0, 0), 1294, 296, LaserTurret::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 1));
    UIGroup->AddNewControlObject(btn);

    // Button 2.5
    btn = new TurretButton("play/floor.png", "play/dirt.png",
        Engine::Sprite("play/tower-base.png", 1294, 376, 0, 0, 0, 0),
        Engine::Sprite("play/traffic-removebg-preview.png", 1294, 368, 0, 0, 0, 0), 1294, 376, TrafficLight::Price);
    btn->SetOnClickCallback(std::bind(&PlayScene::UIBtnClicked, this, 4));
    UIGroup->AddNewControlObject(btn);
    
    int shift = 135 + 25;
    dangerIndicator = new Engine::Sprite("play/benjamin.png", w - shift, h - shift);
    dangerIndicator->Tint.a = 0;
    UIGroup->AddNewObject(dangerIndicator);
}

void PlayScene::UIBtnClicked(int id) {
    if (preview)
        UIGroup->RemoveObject(preview->GetObjectIterator());
    if (id == 0 && money >= MachineGunTurret::Price)
        preview = new MachineGunTurret(0, 0);
    else if (id == 1 && money >= LaserTurret::Price)
        preview = new LaserTurret(0, 0);
    else if (id == 2 && money >= NewTurret::Price)
        preview = new NewTurret(0, 0);
    else if (id == 4 && money >= TrafficLight::Price)
        preview = new TrafficLight(0, 0);
    if (!preview)
        return;
    preview->Position = Engine::GameEngine::GetInstance().GetMousePosition();
    preview->Tint = al_map_rgba(255, 255, 255, 200);
    preview->Enabled = false;
    preview->Preview = true;
    UIGroup->AddNewObject(preview);
    OnMouseMove(Engine::GameEngine::GetInstance().GetMousePosition().x, Engine::GameEngine::GetInstance().GetMousePosition().y);
}

bool PlayScene::CheckSpaceValid(int x, int y) {
    if (x < 0 || x >= MapWidth || y < 0 || y >= MapHeight)
        return false;
    auto map00 = mapState[y][x];
    mapState[y][x] = TILE_OCCUPIED;
    std::vector<std::vector<int>> map = CalculateBFSDistance();
    mapState[y][x] = map00;
    if (map[0][0] == -1)
        return false;
    for (auto &it : EnemyGroup->GetObjects()) {
        Engine::Point pnt;
        pnt.x = floor(it->Position.x / BlockSize);
        pnt.y = floor(it->Position.y / BlockSize);
        if (pnt.x < 0) pnt.x = 0;
        if (pnt.x >= MapWidth) pnt.x = MapWidth - 1;
        if (pnt.y < 0) pnt.y = 0;
        if (pnt.y >= MapHeight) pnt.y = MapHeight - 1;
        if (map[pnt.y][pnt.x] == -1)
            return false;
    }
    // All enemy have path to exit.
    mapState[y][x] = TILE_OCCUPIED;
    mapDistance = map;
    for (auto &it : EnemyGroup->GetObjects())
        dynamic_cast<Enemy *>(it)->UpdatePath(mapDistance);
    return true;
}
std::vector<std::vector<int>> PlayScene::CalculateBFSDistance() {
    // Reverse BFS to find path.
    std::vector<std::vector<int>> map(MapHeight, std::vector<int>(std::vector<int>(MapWidth, -1)));
    std::queue<Engine::Point> que;
    // Push end point.
    // BFS from end point.
    if (mapState[MapHeight - 1][MapWidth - 1] != TILE_DIRT)
        return map;
    que.push(Engine::Point(MapWidth - 1, MapHeight - 1));
    map[MapHeight - 1][MapWidth - 1] = 0;
    int now_dist = 1;
    while (!que.empty()) {
        Engine::Point p = que.front();
        que.pop();
        // TODO PROJECT-1 (1/1): Implement a BFS starting from the most right-bottom block in the map.
        //               For each step you should assign the corresponding distance to the most right-bottom block.
        //               mapState[y][x] is TILE_DIRT if it is empty.
        int x = p.x, y = p.y;
        if (y - 1 >= 0 && y - 1 < MapHeight && x >= 0 && x < MapWidth) {
            if (map[y - 1][x] == -1 && mapState[y - 1][x] == TILE_DIRT) {
                que.push(Engine::Point(x, y - 1));
                map[y - 1][x] = now_dist;
            }
        }
        if (y + 1 >= 0 && y + 1 < MapHeight && x >= 0 && x < MapWidth) {
            if (map[y + 1][x] == -1 && mapState[y + 1][x] == TILE_DIRT) {
                que.push(Engine::Point(x, y + 1));
                map[y + 1][x] = now_dist;
            }
        }
        if (y >= 0 && y < MapHeight && x - 1 >= 0 && x - 1 < MapWidth) {
            if (map[y][x - 1] == -1 && mapState[y][x - 1] == TILE_DIRT) {
                que.push(Engine::Point(x - 1, y));
                map[y][x - 1] = now_dist;
            }
        }
        if (y >= 0 && y < MapHeight && x + 1 >= 0 && x + 1 < MapWidth) {
            if (map[y][x + 1] == -1 && mapState[y][x + 1] == TILE_DIRT) {
                que.push(Engine::Point(x + 1, y));
                map[y][x + 1] = now_dist;
            }
        }
        now_dist++;
    }
    return map;
}

int PlayScene::GetScore() { return cur_score; }
void PlayScene::AddScore(int val) { cur_score += val; }
void PlayScene::AddLife(int val) { 
    lives += val;
    std::cout << "[PlayScene::AddLife] Lives now: " << lives << std::endl;
    UILives->Text = "LIFE " + std::to_string(lives);
}