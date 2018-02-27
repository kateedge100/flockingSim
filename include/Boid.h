#ifndef Boid_H_
#define Boid_H_

#include <ngl/Vec3.h>
#include <ngl/Colour.h>

// pre-declare Flock class
class Flock;

class Boid
{
public :

	/// @brief ctor
    /// @param _pos the start position of the Boid
    Boid(Flock *_Flock	);

    /// @brief a method to update the Boid position
  void update();
    /// @brief a method to draw the Boid
  void draw() const;

    /// @brief mutator for the Boid life
	inline void setLifeTime(int _l){m_lifetime=_l;}
private :
    /// @brief the curent Boid position
	ngl::Vec3 m_pos;
    /// @brief the velocity vector of the Boid
    ngl::Vec3 m_vel;
    /// @brief the rotation vector of the Boid
    ngl::Vec3 m_rotation;
    /// @brief bool to determine whether the boid should flock
    bool m_flockFlag;
    /// @brief the current life value of the Boid
	int m_currentLife;
    /// @brief the total lifetime of the current Boid
	int m_lifetime;
    /// @brief colour of the Boid
	ngl::Colour m_colour;
    const Flock *m_Flock;
    void updateRotation();
    void flock();
    ngl::Vec3 alignBoid();
    ngl::Vec3 seperateBoid();
    ngl::Vec3 cohesionBoid();
    ngl::Vec3 steerBoid(ngl::Vec3);
};


#endif
