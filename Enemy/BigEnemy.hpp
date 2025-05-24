#ifndef BIGENEMY_HPP
#define BIGENEMY_HPP
#include "Enemy.hpp"

class BigEnemy : public Enemy {
    bool haveshield = true; // BigEnemy has shield at the beginning.
public:
    BigEnemy(int x, int y);
    void Update(float deltaTime) override;
    void Draw() const override;
    void Hit(float dmg);
};
#endif  
