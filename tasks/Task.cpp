/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include <ptu_directedperception/Driver.h>

#include "Task.hpp"

using namespace ptu_directedperception;

namespace ptu_directedperception {
class DriverImpl {
    public:
        ptu::Driver mDriver;
};
}

Task::Task(std::string const& name)
    : TaskBase(name), mpImpl(new DriverImpl) 
{
}

Task::Task(std::string const& name, RTT::ExecutionEngine* engine)
    : TaskBase(name, engine), mpImpl(new DriverImpl)
{
}

Task::~Task()
{
}

::base::Vector2d Task::ptFromRBS(::base::samples::RigidBodyState const & rbs_in)
{
    return rbs_in.orientation.matrix().eulerAngles(0,1,2).segment<2>(1).reverse();
}

::base::samples::RigidBodyState Task::rbsFromPT(::base::Vector2d const & pt_in)
{
    base::samples::RigidBodyState rbs;
    rbs.orientation = Eigen::AngleAxisd(pt_in[1],base::Vector3d::UnitY()) *
        Eigen::AngleAxisd(pt_in[0],base::Vector3d::UnitZ());
    return rbs;
}

::base::samples::RigidBodyState Task::rbsFromPanTilt(double pan, double tilt)
{
    base::Vector2d pt_vector;
    pt_vector << pan,tilt;
    return rbsFromPT(pt_vector);
}

bool Task::configureHook()
{

    mpImpl->mDriver.openURI(_io_port.get());

    mpImpl->mDriver.initialize();
    
    setDriver(&(mpImpl->mDriver));
    
    if (! TaskBase::configureHook())
        return false;

    return true;
}

bool Task::startHook()
{
    if (! TaskBase::startHook())
        return false;
        
    if ( _pan_speed.get() > 0 )
        mpImpl->mDriver.setSpeedRad(ptu::PAN, _pan_speed);
        
    if ( _tilt_speed.get() > 0 )
        mpImpl->mDriver.setSpeedRad(ptu::TILT, _tilt_speed);

    return true;
}

void Task::updateHook()
{
    TaskBase::updateHook();

    base::samples::RigidBodyState lrbs;
    base::Vector2d pt;
    bool new_pan, new_tilt;
    new_pan = new_tilt = false;

    if ( _set_orientation.readNewest(lrbs) == RTT::NewData ) {

        pt = ptFromRBS(lrbs);
        new_pan = new_tilt = true;

    } else {
     
        new_pan = _pan_set.readNewest(pt[0]) == RTT::NewData;
        new_tilt = _tilt_set.readNewest(pt[1]) == RTT::NewData;
    }

    if (new_pan) mpImpl->mDriver.setPosRad(ptu::PAN, false, pt[0]);
    if (new_tilt) mpImpl->mDriver.setPosRad(ptu::TILT, false, pt[1]);

    pt << mpImpl->mDriver.getPosRad(ptu::PAN, false), 
       - mpImpl->mDriver.getPosRad(ptu::TILT, false);

    _pan_angle.write(pt[0]);
    _tilt_angle.write(pt[1]);

    if ( _orientation_samples.connected() ) {
        base::samples::RigidBodyState lrbs_out = rbsFromPT(pt);
        lrbs_out.time = base::Time::now();
        lrbs_out.sourceFrame = _head_frame.get();
        lrbs_out.targetFrame = _base_frame.get();
        lrbs_out.position = Eigen::Vector3d::Zero();
        _orientation_samples.write(lrbs_out);
    }
    
}

// void Task::errorHook()
// {
//     TaskBase::errorHook();
// }

void Task::stopHook()
{
    TaskBase::stopHook();
    mpImpl->mDriver.setHalt();
}

//void Task::cleanupHook()
//{
//    TaskBase::cleanupHook();
//}

