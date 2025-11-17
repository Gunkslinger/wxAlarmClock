// Volume control functions for pulse audio. get_old_vol() returns a std::string with the
// volume percentage before setting the volume to play the alarm at. This is so that the
// old volume can be restored after the alaem has stopped playing. set_vol() takes an
// int volume percentage param that was found in the config file, and is used to set
// the volume for the alarm.

// These two funcs are funcky. They merely build shell commands to to call pulse audio
// utilities.
// I decided to write them this way becuase I can't be sure that I'll always be using
// pulse audio and so I don't want to be bothered with learning the pulse audio API right now.
// They work for my purposes. Error handling is garbage, I know.

#include <iostream>
#include <string>
#include "pulse_audio.hpp"
int percentage;

int get_old_vol() {
    FILE* pipe = popen("pactl get-sink-volume @DEFAULT_SINK@", "r");
    if (!pipe)
    {
        std:std::cerr << "Can't open pipe in pulse_audio.cpp:get_old_vol()" << std::endl;
        return -1;
    }
    
    char buffer[128];
    std::string result;
    while (fgets(buffer, 128, pipe)) {
        result += buffer;
    }
    pclose(pipe);
    
    std::regex pattern("(\\d+)%");
    std::smatch match;
    if (std::regex_search(result, match, pattern)) {
        // Extract the percentage value
        percentage = std::stoi(match[1].str());
        std::cout << "Old volume percentage " << percentage << std::endl;
    } else {
        std::cout << "No percentage found." << std::endl;
    }
    return percentage;
}

void set_vol(int new_vol)
{
    std::cout << "New volume percentage " << new_vol << std::endl;
    std::string cmd = "pactl set-sink-volume @DEFAULT_SINK@ ";
    cmd.append(std::to_string(new_vol));
    cmd.append("%");
    system(cmd.c_str());
}