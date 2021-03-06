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
    //rbs.orientation = Eigen::AngleAxisd(pt_in[1]/4.0,base::Vector3d::UnitX()) *
    //       Eigen::AngleAxisd(pt_in[0],base::Vector3d::UnitZ());

    rbs.orientation = Eigen::Quaterniond(Eigen::AngleAxisd(pt_in[0],base::Vector3d::UnitZ()) * Eigen::AngleAxisd(pt_in[1]/4.0,base::Vector3d::UnitY()));

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

    setDriver(&(mpImpl->mDriver));
    
    if (! TaskBase::configureHook())
        return false;
    
    return true;
}

bool Task::startHook()
{
    if (! TaskBase::startHook())
        return false;
    
    mpImpl->mDriver.initialize();
        
    if ( _pan_speed.get() > 0 )
        mpImpl->mDriver.setSpeedRad(ptu::PAN, _pan_speed);
        
    if ( _tilt_speed.get() > 0 )
        mpImpl->mDriver.setSpeedRad(ptu::TILT, _tilt_speed);

    // Disabling the software limits
    mpImpl->mDriver.write("LD ");
    // Throw away the answer
    mpImpl->mDriver.readAns();

    // Wait for the message to be thorwn away
    sleep(1);

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
    // Assuming x forward, a positive tilt means down, but the ptu takes positive tilt as up.
    if (new_tilt) mpImpl->mDriver.setPosRad(ptu::TILT, false, -pt[1]);

    pt << mpImpl->mDriver.getPosRad(ptu::PAN, false), 
       - mpImpl->mDriver.getPosRad(ptu::TILT, false);

    _pan_angle.write(pt[0]);
    _tilt_angle.write(pt[1]);

    base::samples::RigidBodyState lrbs_out = rbsFromPT(pt);
    lrbs_out.time = base::Time::now();
    lrbs_out.sourceFrame = _head_frame.get();
    lrbs_out.targetFrame = _base_frame.get();
    lrbs_out.position = Eigen::Vector3d::Zero();
    _orientation_samples.write(lrbs_out);
}

void Task::processIO()
{
    if (mDriver->hasPacket())
    {
        throw std::runtime_error("processIO called, and the driver reports a packet");
    }
}

void Task::exceptionHook()
{
    // In case the component goes into exception mode try to reset the PTU
    printf("ptu_directedperception: PTU error, resetting PTU\n");

    // Reconfigure component
    mpImpl->mDriver.openURI(_io_port.get());

    // Use a space as delimiter (can be either space or Enter)
    mpImpl->mDriver.write("R ");

    TaskBase::exceptionHook();

    // Timeout until the PTU resets itself, completely arbitrary number
    printf("ptu_directedperception: waiting for 60 seconds\n");
    sleep(60); 

    // Recover from reset, this sequence worked in rock-display
    recover();
    configure();
    cleanup();
    configure();
    start();

    printf("ptu_directedperception: Recovered\n");
}

void Task::stopHook()
{
    TaskBase::stopHook();
    mpImpl->mDriver.setHalt();
}

//void Task::cleanupHook()
//{
//    TaskBase::cleanupHook();
//}

