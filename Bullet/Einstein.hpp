#ifndef EINSTEIN_HPP
#define EINSTEIN_HPP
#include "Bullet.hpp"

class Enemy;
class Turret;
namespace Engine {
    struct Point;
}   // namespace Engine

class EinsteinBullet : public Bullet {
    Enemy *lock_target;
public:
    explicit EinsteinBullet(Engine::Point position, Engine::Point forwardDirection, float rotation, Turret *parent);
    void OnExplode(Enemy *enemy) override;
};
#endif   // FIREBULLET_HPP
