#ifndef ENEMY_HPP
#define ENEMY_HPP
#include <list>
#include <string>
#include <vector>

#include "Engine/Point.hpp"
#include "Engine/Sprite.hpp"

class Bullet;
class PlayScene;
class Turret;

class Enemy : public Engine::Sprite {
protected:
    std::vector<Engine::Point> path;
    float speed;
    float maxspeed;
    float hp;
    float maxhp;
    int money;
    bool isSlow;
    PlayScene *getPlayScene();
    virtual void OnExplode();
    
public:
    float reachEndTime;
    std::list<Turret *> lockedTurrets;
    std::list<Bullet *> lockedBullets;
    Enemy(std::string img, float x, float y, float radius, float speed, float hp, int money);
    virtual void Hit(float damage);
    void UpdatePath(const std::vector<std::vector<int>> &mapDistance);
    void Update(float deltaTime) override;
    void Draw() const override;
    float GetSpeed() {return speed;}
    float GetMaxSpeed() {return maxspeed;}
    void UpdateSpeed(float a) {speed *= a;} 
    void BackToSpeed() {speed = maxspeed;}
    void makeSlow() {isSlow = true;}
    bool checkSlow() {return isSlow;}
};
#endif   // ENEMY_HPP
