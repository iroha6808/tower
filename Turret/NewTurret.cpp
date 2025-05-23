#include <allegro5/base.h>
#include <cmath>
#include <string>

#include "Bullet/Einstein.hpp"
#include "Engine/AudioHelper.hpp"
#include "Engine/Group.hpp"
#include "Engine/Point.hpp"
#include "NewTurret.hpp"
#include "Scene/PlayScene.hpp"

const int NewTurret::Price = 200;
NewTurret::NewTurret(float x, float y)
    : Turret("play/tower-base.png", "play/turret-4.png", x, y, 200, Price, 0.5) {
    // Move center downward, since we the turret head is slightly biased upward.
    Anchor.y += 8.0f / GetBitmapHeight();
}
void NewTurret::CreateBullet() {
    Engine::Point diff = Engine::Point(cos(Rotation - ALLEGRO_PI / 2), sin(Rotation - ALLEGRO_PI / 2));
    float rotation = atan2(diff.y, diff.x);
    Engine::Point normalized = diff.Normalize();
    // Change bullet position to the front of the gun barrel.
    getPlayScene()->BulletGroup->AddNewObject(new EinsteinBullet(Position + normalized * 36, diff, rotation, this));
    AudioHelper::PlayAudio("gun.wav");
}
