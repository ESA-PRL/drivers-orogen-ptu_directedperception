#! /usr/bin/env ruby

require 'orocos'
require 'readline'

include Orocos

if !ARGV[0]
    STDERR.puts "usage: test_ptu.rb PORT"
    exit 1
end

Orocos.initialize

Orocos.run 'ptu_directedperception::DirectedPerceptionTask' => 'PTUTask' do

    Orocos.log_all

    ptu = TaskContext.get 'PTUTask'

    ptu.port = ARGV[0] || "/dev/ttyS1"
    
    rot_writer = ptu.set_orientation.writer
    rot_reader = ptu.orientation_samples.reader

    ptu.configure
    ptu.start

    for i in 0..22
        if rot = rot_reader.read()
            ea = ptu.ptFromRBS(rot)
            puts "#{i} pan: #{ea.data[0]} tilt: #{ea.data[1]}"
        end
        sleep(0.1)
    end

    Readline.readline("Press <Enter> to test movement.") do
    end

    puts "Going to 45 deg, 22.5 deg"
    rot = ptu.rbsFromPanTilt(0.7071,0.35355)
    rot_writer.write(rot)
    
    sleep(10.0)

    puts "Going back"
    rot = ptu.rbsFromPanTilt(0,0)
    rot_writer.write(rot)

    sleep(5.0)
    puts "Fini"
end
