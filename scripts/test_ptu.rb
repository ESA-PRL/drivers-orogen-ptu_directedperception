#! /usr/bin/env ruby

require 'orocos'
require 'readline'

include Orocos

ENV['BASE_LOG_LEVEL'] = 'INFO'
ENV['BASE_LOG_FORMAT'] = 'SHORT'

Orocos.initialize

Orocos.run 'ptu_directedperception::Task' => 'PTUTask' do

    #Orocos.log_all

    ptu = TaskContext.get 'PTUTask'

    port = ARGV[0] || "/dev/ttyS1"
    ptu.io_port = ["serial://", port,":9600"].join("")
    puts "connecting to #{ptu.io_port}"
    
    rot_writer = ptu.set_orientation.writer
    rot_reader = ptu.orientation_samples.reader

    ptu.configure
    ptu.start

    (1..10).each do |i|
        if rot = rot_reader.read()
            ea = ptu.ptFromRBS(rot)
            puts "pan: #{ea.data[0]} tilt: #{ea.data[1]}"
        end
        sleep(0.1)
    end
    

    Readline.readline("Press <Enter> to test movement.") do
    end

    puts "Going to 45 deg, 22.5 deg"
    rot = ptu.rbsFromPanTilt(0.7071,-0.35355)
    rot_writer.write(rot)
    
    t0 = Time.now
    while ( Time.now - t0 < 2.0 )
        if rot = rot_reader.read()
            ea = ptu.ptFromRBS(rot)
            puts "pan: #{ea.data[0]} tilt: #{ea.data[1]}"
        end
        sleep(0.01)
    end

    puts "Going back"
    rot = ptu.rbsFromPanTilt(0,0)
    rot_writer.write(rot)

    sleep(2.0)

    puts "Fini"
end
