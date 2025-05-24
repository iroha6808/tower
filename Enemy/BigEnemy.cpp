#include <string>

#include "BigEnemy.hpp"
#include "Scene/PlayScene.hpp"
#include <allegro5/allegro_primitives.h>
#include <allegro5/color.h>
#include "Engine/AudioHelper.hpp"
#include "Turret/Turret.hpp"
#include "Bullet/Bullet.hpp"

// TODO HACKATHON-3 (1/3): You can imitate the 2 files: 'SoldierEnemy.hpp', 'SoldierEnemy.cpp' to create a new enemy.
BigEnemy::BigEnemy(int x, int y) : Enemy("play/enemy-4.png", x, y, 10, 50, 5, 15) {
    haveshield = true; // BigEnemy has shield at the beginning.
}

void BigEnemy::Hit(float damage) {
    if (!haveshield) hp -= damage;
    if (haveshield) haveshield = false; // BigEnemy lose shield after first hit.
    if (hp <= 0) {
        OnExplode();
        // Remove all turret's reference to target.
        for (auto &it : lockedTurrets)
            it->Target = nullptr;
        for (auto &it : lockedBullets)
            it->Target = nullptr;
        getPlayScene()->EarnMoney(money);
        getPlayScene()->EnemyGroup->RemoveObject(objectIterator);
        AudioHelper::PlayAudio("explosion.wav");
    }
}

void BigEnemy::Update(float deltaTime) {
    // Pre-calculate the velocity.
    if (haveshield) printf("BigEnemy has shield\n");
    else printf("BigEnemy has no shield\n");
    float remainSpeed = speed * deltaTime;
    while (remainSpeed != 0) {
        if (path.empty()) {
            // Reach end point.
            Hit(hp);
            getPlayScene()->Hit();
            reachEndTime = 0;
            return;
        }
        Engine::Point target = path.back() * PlayScene::BlockSize + Engine::Point(PlayScene::BlockSize / 2, PlayScene::BlockSize / 2);
        Engine::Point vec = target - Position;
        // Add up the distances:
        // 1. to path.back()
        // 2. path.back() to border
        // 3. All intermediate block size
        // 4. to end point
        reachEndTime = (vec.Magnitude() + (path.size() - 1) * PlayScene::BlockSize - remainSpeed) / speed;
        Engine::Point normalized = vec.Normalize();
        if (remainSpeed - vec.Magnitude() > 0) {
            Position = target;
            path.pop_back();
            remainSpeed -= vec.Magnitude();
        } else {
            Velocity = normalized * remainSpeed / deltaTime;
            remainSpeed = 0;
        }
    }
    Rotation = atan2(Velocity.y, Velocity.x);
    Sprite::Update(deltaTime);
}
void BigEnemy::Draw() const {
    Sprite::Draw();

    if (PlayScene::DebugMode) {
        al_draw_circle(Position.x, Position.y, CollisionRadius, al_map_rgb(255, 0, 0), 2);
    }

    if (!haveshield) {
        float length = hp / maxhp;

        al_draw_filled_rectangle(Position.x - 23, Position.y - 28, Position.x + 23, Position.y - 23, al_map_rgb(0, 0, 0));
        al_draw_filled_rectangle(Position.x - 20, Position.y - 27, Position.x + 20, Position.y - 24, al_map_rgb(255, 255, 255));
        ALLEGRO_COLOR hpColor = al_map_rgb(255 * (1 - length), 255 * length, 0);
        al_draw_filled_rectangle(Position.x - 20, Position.y - 27, Position.x - 20 + 40 * length, Position.y - 24, hpColor);
    }

    else if (haveshield) {
        al_draw_filled_circle(Position.x, Position.y, 30, al_map_rgb(0, 100, 255));
        float length = hp / maxhp;

        al_draw_filled_rectangle(Position.x - 23, Position.y - 28, Position.x + 23, Position.y - 23, al_map_rgb(0, 0, 0));
        al_draw_filled_rectangle(Position.x - 20, Position.y - 27, Position.x + 20, Position.y - 24, al_map_rgb(255, 255, 255));
        ALLEGRO_COLOR hpColor = al_map_rgb(255 * (1 - length), 255 * length, 0);
        al_draw_filled_rectangle(Position.x - 20, Position.y - 27, Position.x - 20 + 40 * length, Position.y - 24, hpColor);
    }

    
}
