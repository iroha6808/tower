#include <allegro5/base.h>
#include <random>
#include <string>

#include "Engine/Point.hpp"
#include "Scene/PlayScene.hpp"
#include "Health.hpp"
#include "Engine/AudioHelper.hpp"


Health::Health(int x, int y)
    : Enemy("play/health.png", x, y, 20, 30, 1, 0),
      head("play/health.png", x, y), targetRotation(0) {
}
void Health::Draw() const {
    Enemy::Draw();
    // head.Draw();
}
void Health::Update(float deltaTime) {
    Enemy::Update(deltaTime);
    head.Position = Position;
    // Choose arbitrary one.
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<> dist(0.0f, 4.0f);
    float rnd = dist(rng);
    if (rnd < deltaTime) {
        // Head arbitrary rotation.
        std::uniform_real_distribution<> distRadian(-ALLEGRO_PI, ALLEGRO_PI);
        targetRotation = distRadian(rng);
    }
    head.Rotation = (head.Rotation + deltaTime * targetRotation) / (1 + deltaTime);
}
void Health::Hit(float damage) {
    PlayScene* scene = getPlayScene();
    if (!scene) return;
    Enemy::Hit(damage);
    if (hp <= 0) {
        scene->AddLife(5);
    }
}
