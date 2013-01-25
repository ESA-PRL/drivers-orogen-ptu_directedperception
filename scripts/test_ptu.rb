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

    start_swp = 0.35355 # 22.5 deg
    end_swp = -0.35355 # -22.5 deg

    Readline.readline("Press <Enter> to test sweeping.") do
    end

    def tilt(rot,ptu)
        if rot
            ptu.ptFromRBS(rot).data[1]
        else
            nil
        end
    end

    pos = start_swp
    next_pos = end_swp
    while true
        rot = ptu.rbsFromPanTilt(0,pos)
        rot_writer.write(rot)
        

        tlt = tilt(rot_reader.read(),ptu)
        while ( pos >= 0 && tlt < pos ) || ( pos < 0 && tlt > pos)
            puts "tilt: #{tlt}"
            sleep(0.01)
            tlt = tilt(rot_reader.read(),ptu)
        end

        btwpos = pos
        pos = next_pos
        next_pos = btwpos
    end

    puts "Fini"
end
