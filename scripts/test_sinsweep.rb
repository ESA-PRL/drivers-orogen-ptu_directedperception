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

    #Orocos.log_all

    ptu = TaskContext.get 'PTUTask'

    ptu.port = ARGV[0] || "/dev/ttyS1"
    
    rot_writer = ptu.set_orientation.writer
    rot_reader = ptu.orientation_samples.reader

    ptu.configure
    ptu.start


    start_swp = 0.35355 # 22.5 deg
    end_swp = -0.35355 # -22.5 deg
    period = 5.0 # seconds
    step_size = 0.05

    n = period / step_size

    mean = (start_swp + end_swp) / 2.0
    amp = (start_swp - end_swp) / 2.0
    
    t = (0..n-1).to_a.map { |v| v/n }
    ang = t.map { |v| amp * Math::sin(v * 2 * Math::PI) }   
    

    Readline.readline("Press <Enter> to test sweeping.") do
    end

    idx = 0
    while true
        rot = ptu.rbsFromPanTilt(0,ang[idx])
        rot_writer.write(rot)
        
        if rot = rot_reader.read()
            tilt = ptu.ptFromRBS(rot)
            puts "tilt: #{tilt.data[1]}"
        end

        sleep(step_size)
        idx +=1
        if idx >= n
            idx = 0
        end
    end

    puts "Fini"
end
