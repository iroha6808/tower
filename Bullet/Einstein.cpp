#include <allegro5/base.h>
#include <random>
#include <string>

#include "Enemy/Enemy.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "Einstein.hpp"
#include "Scene/PlayScene.hpp"
#include "UI/Animation/DirtyEffect.hpp"

class Turret;

EinsteinBullet::EinsteinBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent) : Bullet("play/Einstein.png", 600, 1, position, forwardDirection, rotation - ALLEGRO_PI / 2, parent, remain_cnt) {
}
void EinsteinBullet::OnExplode(Enemy* enemy) {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(2, 5);
    getPlayScene()->GroundEffectGroup->AddNewObject(new DirtyEffect("play/emc2.png", dist(rng), enemy->Position.x, enemy->Position.y));

    Enemy* nearest = nullptr;
    float seq = std::numeric_limits<float>::max();
    float radius = 100; 

    for (auto& obj : getPlayScene()->EnemyGroup->GetObjects()) {
        Enemy* e = dynamic_cast<Enemy*>(obj);
        if (e && e != enemy) {
            float distSq = (e->Position - enemy->Position).MagnitudeSquared();
            if (distSq < radius * radius && distSq < seq) {
                seq = distSq;
                nearest = e;
                break;
            }
        }
    }

    if (!nearest) return;

    Engine::Point direction = (nearest->Position - enemy->Position).Normalize();
    float rotation = atan2(direction.y, direction.x);
    --remain_cnt;
    getPlayScene()->BulletGroup->AddNewObject(new EinsteinBullet(enemy->Position, direction, rotation, parent));
}
