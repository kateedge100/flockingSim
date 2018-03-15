#include "prey.h"
#include "Flock.h"

#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Transformation.h>



Prey::Prey(Flock *_Flock, int _ID) : Boid(_Flock, _ID)
{

}

Prey::~Prey()
{

}


void Prey::update()
{

    avoidBoundaries();

    flock();

    m_pos+=m_vel;

    updateRotation();
}

void Prey::draw()
{
    // get the VBO instance and draw the built in teapot
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    ngl::Transformation trans;
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use(m_Flock->getShaderName());
      trans.setPosition(m_pos);
      trans.setRotation(m_rotation);

    ngl::Mat4 MV;
    ngl::Mat4 MVP;
    ngl::Mat3 normalMatrix;
    ngl::Mat4 M;
    M=trans.getMatrix();
    MV=m_Flock->getCam()->getViewMatrix()*M;
    MVP=m_Flock->getCam()->getProjectionMatrix()*MV;
    normalMatrix=MV;
    normalMatrix.inverse().transpose();
    shader->setUniform("MV",MV);
    shader->setUniform("MVP",MVP);
    shader->setUniform("normalMatrix",normalMatrix);
    shader->setUniform("M",M);

    prim->draw("cone");


}

void Prey::avoidBoundaries()
{



    if(m_pos.m_z > 5)
    {
        ngl::Vec3 desiredVel = {m_vel[0],0,-m_vel[2]};

        desiredVel.normalize();

        m_vel.operator +=(steerBoid(desiredVel));

        //std::cout<<" out of z bounds\n";
    }
    else if(m_pos.m_z < -5)
    {
        ngl::Vec3 desiredVel = {m_vel[0],0,-m_vel[2]};

        desiredVel.normalize();

        m_vel.operator +=(steerBoid(desiredVel));

        //std::cout<<" out of z bounds\n";
    }
    else if(m_pos.m_x > 3)
    {
        ngl::Vec3 desiredVel = {-m_vel[0],0,m_vel[2]};

        desiredVel.normalize();

        m_vel.operator +=(steerBoid(desiredVel));

       // std::cout<<" out of x bounds\n";
    }
    else if(m_pos.m_x < -3)
    {
        ngl::Vec3 desiredVel = {-m_vel[0],0,m_vel[2]};

        desiredVel.normalize();

        m_vel.operator +=(steerBoid(desiredVel));

       // std::cout<<" out of x bounds\n";
    }

}


void Prey::updateRotation()
{

    //rotation 0 when facing in z axis
        ngl::Vec3 facing = {0,0,1};

             //only update if moving
             if(m_vel.operator !=({0,0,0}))
             {


                 float mag1 = facing.length();
                 float mag2 = m_vel.length() ;

                 //find angle between z axis and boids velocity vector
                 float steer = std::acos(facing.dot(m_vel)/(mag1*mag2));
                 //math.acos(dotProduct(facing, self.velocity)/(mag1*mag2));

                 //convert from radians to degrees
                 steer = steer*(180/M_PI);

                 //if rotation past 180 degrees must take away from 360, then update boid rotation
                 if(m_vel[0]>0)
                 {
                     m_rotation[1] = steer;
                 }
                 else
                 {
                     m_rotation[1]= 360-steer;
                 }
             }

}

void Prey::flock()
{
        ngl::Vec3 steer = {0,0,0};

        //compute the flocking component vectors
        ngl::Vec3 alignment = {0,0,0};
        ngl::Vec3 cohesion = {0,0,0};
        ngl::Vec3 separation = {0,0,0};

        alignment.operator =(alignBoid());
        cohesion.operator =(cohesionBoid());
        separation.operator =(seperateBoid());


        //flocking component weights
        float alignmentWeight = 1;
        float cohesionWeight = 1;
        float separationWeight = 1.2;

        //find resulting flocking vector
        steer[0] += (cohesion[0] * cohesionWeight) + (alignment[0] * alignmentWeight) + (separation[0] * separationWeight);
        steer[2] += (cohesion[2] * cohesionWeight) + (alignment[2] * alignmentWeight) + (separation[2] * separationWeight);


        if(steer.operator !=(ngl::Vec3{0,0,0}))
        {

            steer.normalize();

        }




        //steer towards flocking vector
        m_vel[0] += steer[0];//steerBoid(steer)[0];
        m_vel[2] += steer[2];//steerBoid(steer)[2];

        if(m_vel.operator !=(ngl::Vec3{0,0,0}))
        {
            m_vel.normalize();

            // limit velocity
            limitVel(0.004);
        }

}

ngl::Vec3 Prey::alignBoid()
{
    int numberOfNeighbours = 0;
    ngl::Vec3 alignmentVector {0,0,0};

    std::vector <Prey> boidsVector = m_Flock->getBoidsVector();




    for(int i = 0; i< m_Flock->getNoBoids(); i++)
    {
        //only flock with other flocking boids
        if(boidsVector[i].getID() != getID())
        {
            if(boidsVector[i].m_flockFlag == true)
            {
                if( distanceToBoid(boidsVector[i]) < 0.8)
                {

                    alignmentVector[0] += boidsVector[i].m_vel[0];
                    alignmentVector[2] += boidsVector[i].m_vel[2];

                    numberOfNeighbours += 1;
                }
            }
        }
    }

    // avoid dividing by zero
    if(numberOfNeighbours != 0 && alignmentVector.operator !=(ngl::Vec3{0,0,0}))
    {



        //find average velocity of boids in the current boids neighborhood
        alignmentVector[0] /= numberOfNeighbours;
        alignmentVector[2] /= numberOfNeighbours;




        alignmentVector.normalize();
    }

    return alignmentVector;

}

ngl::Vec3 Prey::seperateBoid()
{
    int numberOfNeighbours = 0;
    ngl::Vec3 seperationVector {0,0,0};
    std::vector <Prey> boidsVector = m_Flock->getBoidsVector();

    ngl::Vec3 diff {0,0,0};



    for(int i = 0; i <m_Flock->getNoBoids(); i++)
    {
        if(boidsVector[i].getID() != getID())
        {
            if(boidsVector[i].m_flockFlag == true)
            {
                if(distanceToBoid(boidsVector[i]) <0.6)
                {

                    //vector from current boid to neighbor
                    diff[0] = boidsVector[i].m_pos[0]-m_pos[0];
                    diff[2] = boidsVector[i].m_pos[2]-m_pos[2];

                    diff.normalize();

                    //the closer to its neighbors the greater the seperation vector
                    seperationVector[0] += diff[0] / (distanceToBoid(boidsVector[i]));
                    seperationVector[2] += diff[2] / (distanceToBoid(boidsVector[i]));


                    numberOfNeighbours += 1;
                }
            }
        }
    }

    //avoid dividing by zero
    if(numberOfNeighbours != 0)
    {

        seperationVector[0] /= numberOfNeighbours;
        seperationVector[2] /= numberOfNeighbours;

        //run in opposite direction to average neighbor position
        seperationVector[0] *= -1;
        seperationVector[2] *= -1;

        seperationVector.normalize();
    }



    return seperationVector;

}

ngl::Vec3 Prey::cohesionBoid()
{
    int numberOfNeighbours = 0;
    ngl::Vec3 cohesionVector {0,0,0};

    std::vector <Prey> boidsVector = m_Flock->getBoidsVector();

    for(int i = 0; i < m_Flock->getNoBoids(); i++)
    {
        if(boidsVector[i].getID() != getID())
        {
            if( boidsVector[i].m_flockFlag = true)
            {
                if(distanceToBoid(boidsVector[i]) < 1.0)
                {




                    cohesionVector[0] += boidsVector[i].m_pos[0];
                    cohesionVector[2] += boidsVector[i].m_pos[2];


                    numberOfNeighbours += 1;
                }
            }
        }
    }

    //avoid dividing by zero
    if(numberOfNeighbours != 0)
    {

        //find average position
        cohesionVector[0] /= numberOfNeighbours;
        cohesionVector[2] /= numberOfNeighbours;

        //find vector from agent to average position
        cohesionVector[0] = (cohesionVector[0] - m_pos[0]);
        cohesionVector[2] = (cohesionVector[2] - m_pos[2]);

        cohesionVector.normalize();
    }

    return cohesionVector;


}

ngl::Vec3 Prey::steerBoid(ngl::Vec3 _target)
{
    ngl::Vec3 steer = {0,0,0};
    steer[0] = _target[0] - m_vel[0];
    steer[2] = _target[2] - m_vel[2];

    //std::cout<<"steer "<<steer[0]<<"\n";

    //steer.operator =( (steer/steer.length())*0.001);

    //std::cout<<steer[0]<<"\n";

    return steer;

}

float Prey::distanceToBoid(const Prey _boid)
{
    float distance = std::sqrt((m_pos[0]-_boid.m_pos[0])*(m_pos[0]-_boid.m_pos[0]) + (m_pos[2]-_boid.m_pos[2])*(m_pos[2]-_boid.m_pos[2]));

    return distance;

}

void Prey::limitVel(float _limit)
{

    if( m_vel.length() > _limit)
    {
        m_vel[0] = (m_vel[0]/m_vel.length())*_limit;
        m_vel[2] = (m_vel[2]/m_vel.length())*_limit;

    }
}



