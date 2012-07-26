/* Generated from orogen/lib/orogen/templates/tasks/Task.cpp */

#include <ptu_directedperception/Driver.h>

#include "DirectedPerceptionTask.hpp"

using namespace ptu_directedperception;

namespace ptu_directedperception {
class DriverImpl {
    public:
        ptu::Driver mDriver;
};
}

DirectedPerceptionTask::DirectedPerceptionTask(std::string const& name)
    : DirectedPerceptionTaskBase(name), mpImpl(new DriverImpl) 
{
}

DirectedPerceptionTask::DirectedPerceptionTask(std::string const& name, RTT::ExecutionEngine* engine)
    : DirectedPerceptionTaskBase(name, engine), mpImpl(new DriverImpl)
{
}

DirectedPerceptionTask::~DirectedPerceptionTask()
{
}

::base::Vector2d DirectedPerceptionTask::ptFromRBS(::base::samples::RigidBodyState const & rbs_in)
{
    return rbs_in.orientation.matrix().eulerAngles(0,1,2).segment<2>(1).reverse();
}

::base::samples::RigidBodyState DirectedPerceptionTask::rbsFromPT(::base::Vector2d const & pt_in)
{
    base::samples::RigidBodyState rbs;
    rbs.orientation = Eigen::AngleAxisd(pt_in[1],base::Vector3d::UnitY()) *
        Eigen::AngleAxisd(pt_in[0],base::Vector3d::UnitZ());
    return rbs;
}

bool DirectedPerceptionTask::configureHook()
{
    if (! DirectedPerceptionTaskBase::configureHook())
        return false;
    return true;
}
bool DirectedPerceptionTask::startHook()
{
    if (! DirectedPerceptionTaskBase::startHook())
        return false;

    if ( _baudrate.get() != 9600) 
        RTT::log(RTT::Warning) << "Other Baudrates then 9600 not supported by now." 
            << RTT::endlog();

    return mpImpl->mDriver.openSerial( _port.get(), _baudrate.get() );
}

void DirectedPerceptionTask::updateHook()
{
    DirectedPerceptionTaskBase::updateHook();

    base::samples::RigidBodyState lrbs;
    if ( _set_orientation.readNewest(lrbs) == RTT::NewData ) {

        base::Vector2d pt = ptFromRBS(lrbs);

        bool result = mpImpl->mDriver.setPosRad(ptu::PAN, false, pt[0]);
        result &= mpImpl->mDriver.setPosRad(ptu::TILT, false, pt[1]);

        RTT::log(RTT::Warning) << "Setting one of pan or tilt angle or both failed."
            << RTT::endlog();
    }

    base::Vector2d pt;
    pt << mpImpl->mDriver.getPosRad(ptu::PAN, false), 
       mpImpl->mDriver.getPosRad(ptu::TILT, false);

    base::samples::RigidBodyState lrbs_out = rbsFromPT(pt);
    lrbs_out.time = base::Time::now();
    _orientation_samples.write(lrbs_out);
    
}
// void DirectedPerceptionTask::errorHook()
// {
//     DirectedPerceptionTaskBase::errorHook();
// }
void DirectedPerceptionTask::stopHook()
{
    DirectedPerceptionTaskBase::stopHook();
    mpImpl->mDriver.close();
}
//void DirectedPerceptionTask::cleanupHook()
//{
//    DirectedPerceptionTaskBase::cleanupHook();
//}
