#! /usr/bin/env ruby

require 'orocos'

include OROCOS

if !ARGV[0]
    STDERR.puts "usage: test_ptu.rb PORT"
    exit 1
end

Orocos.initialize

Orocos.run 'ptu_directedperception::DirectedPerceptionTask' => 'PTUTask' do

    Orocos.log_all

    ptu = TaskContext.get 'PTUTask'

    ptu.port = "/dev/ttyS1"
    
    rot_writer = ptu.set_orientation.writer
    rot_reader = ptu.orientation_samples.read

    ptu.configure
    ptu.start

    for i in 0..99
        if rot = rot_reader.read()
            ea = ptu.ptFromRBS(rot)
            puts "#{i} pan: #{ea.data[0]} tilt: #{ea.data[1]}"
        end
        sleep(0.1)
    end

    puts "Going to 45 deg, 45 deg"
    ang = base::Vector2(0.7,0.7)
    rot = ptu.rbsFromPT(ang)
    rot_writer.write(rot)
    
    sleep(10.0)

    puts "Going back"
    ang = base::Vector2(0.0,0.0)
    rot = ptu.rbsFromPT(ang)
    rot_writer.write(rot)

    sleep(5.0)
    puts "Fini"
end
