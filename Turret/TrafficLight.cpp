#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/LaserBullet.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "TrafficLight.hpp"
#include "Scene/PlayScene.hpp"
#include "Enemy/Enemy.hpp"

const int TrafficLight::Price = 200;
TrafficLight::TrafficLight(float x, float y) : Turret("play/tower-base.png", "play/traffic-removebg-preview.png", x, y, 300, Price, 0.0) {
    // Move center downward, since we the turret head is slightly biased upward.
    Anchor.y += 8.0f / GetBitmapHeight();
}
void TrafficLight::CreateBullet() {
    // no bullet will be created
}
void TrafficLight::Update(float deltaTime) {
    Turret::Update(deltaTime);

    for (auto& obj : getPlayScene()->EnemyGroup->GetObjects()) {
        Enemy* enemy = dynamic_cast<Enemy*>(obj);
        if (!enemy) continue;

        float dx = Position.x - enemy->Position.x;
        float dy = Position.y - enemy->Position.y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= 300) {
            if (enemy->GetSpeed() == enemy->GetMaxSpeed()) {
                enemy->UpdateSpeed(0.5f);
            }
        } 
        else {
            if (enemy->GetSpeed() < enemy->GetMaxSpeed()) {
                enemy->BackToSpeed();
            }
        }
    }
}