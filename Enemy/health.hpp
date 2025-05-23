#ifndef HEALTH_HPP
#define HEALTH_HPP
#include "Enemy.hpp"
#include "Engine/Sprite.hpp"

class Health : public Enemy {
private:
    Sprite head;
    float targetRotation;

public:
    Health(int x, int y);
    void Draw() const override;
    void Update(float deltaTime) override;
    void Hit(float damage) override;
};
#endif
