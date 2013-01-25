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

    ptu.tilt_speed = 2.0

    ptu.configure
    ptu.start

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
        last_tlt = tlt;
        while ( pos >= 0 && tlt < pos ) || ( pos < 0 && tlt > pos)
            tlt = tilt(rot_reader.read(),ptu)
            speed = (tlt-last_tlt)/0.01
            last_tlt = tlt
            puts "tilt: #{tlt} speed: #{speed}"
            sleep(0.01)
        end

        btwpos = pos
        pos = next_pos
        next_pos = btwpos
    end

    puts "Fini"
end
