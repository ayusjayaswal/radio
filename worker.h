#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <mpv/client.h>

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string CYAN = "\033[36m";
const std::string BLUE = "\033[34m";
const std::string BORDER_COLOR = "\033[90m";

std::unordered_map<std::string, std::pair<std::string, std::string>> stations;
mpv_handle *mpv = nullptr;

void check_error(int status) {
    if (status < 0) {
        std::cerr << "MPV API error: " << mpv_error_string(status) << std::endl;
        exit(1);
    }
}

void signalHandler(int signum) {
    if (mpv) {
        mpv_terminate_destroy(mpv);
    }
    exit(signum);
}

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [-l] [-s <station_name>] [-h]" << std::endl;
    std::cerr << "Options:\n"
              << "  -l                 List all available stations\n"
      	      << "  -v                 Verbose Listing of Radio Stations along with desciptions\n"
              << "  -s <station_name>  Play the specified station\n"
              << "  -h                 Display this help message\n";
}

void loadStationsFromConfig() {
    std::string config_file = std::string(getenv("HOME")) + "/.config/radio/config.ini";
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open configuration file " << config_file << std::endl;
        return;
    }

    std::string line, station_name, station_url, station_desc;
    while (std::getline(file, line)) {
        if (line.find("=") != std::string::npos && line.find("#")==std::string::npos) {
            size_t pos1 = line.find("=");
            size_t pos2 = line.find("=", pos1+1);
            station_name = line.substr(0,pos1);
	    station_desc = line.substr(pos1 + 1, pos2 - pos1 -1);
            station_url = line.substr(pos2 + 1);
            stations[station_name].second = station_url;
            stations[station_name].first = station_desc;
        }
    }
    file.close();
}

void listStationsVerbose() {
    const int name_width = 30;
    const int info_width = 20;

    std::cout << BORDER_COLOR << std::string(name_width + info_width, '-') << RESET << "\n";
    
    std::cout << BOLD << std::left << std::setw(name_width) << "Station Name" 
              << std::setw(info_width) << "Description" << RESET << "\n";

    std::cout << BORDER_COLOR << std::string(name_width + info_width, '-') << RESET << "\n";

    for (const auto& [name, info] : stations) {
        std::cout << CYAN << std::left << std::setw(name_width) << name << RESET
                  << BLUE << std::left << std::setw(info_width) << info.first << RESET
                  << "\n";
    }

    std::cout << BORDER_COLOR << std::string(name_width + info_width, '-') << RESET << "\n";
}
/*
void listStationsVerbose() {
	const int name_width = 30;
       for (const auto& [name, _] : stations) {
        std::cout << std::left << std::setw(name_width)
                  << name
                  << _ .first << "\n";
    }
}
*/

void listStations(){
    for (const auto& [name, _] : stations){
        std::cout << name << '\n';
    }
}

void playStation(const std::string& station_name) {
    if (stations.find(station_name) == stations.end()) {
        std::cerr << "Error: Station '" << station_name << "' not found." << std::endl;
        return;
    }
    mpv = mpv_create();
    if (!mpv) {
        std::cerr << "Error: Failed to create MPV instance" << std::endl;
        return;
    }

    check_error(mpv_set_option_string(mpv, "video", "no"));
    check_error(mpv_set_option_string(mpv, "terminal", "yes"));
    check_error(mpv_set_option_string(mpv, "input-terminal", "yes"));
    check_error(mpv_set_option_string(mpv, "audio-display", "no"));

    check_error(mpv_initialize(mpv));

    const char *cmd[] = {"loadfile", stations[station_name].second.c_str(), NULL};
    check_error(mpv_command(mpv, cmd));

    std::cout << "Now playing: " << station_name << std::endl;
    std::cout << "Press Ctrl+C to stop playback" << std::endl;

    while (1) {
        mpv_event *event = mpv_wait_event(mpv, -1);
        if (event->event_id == MPV_EVENT_SHUTDOWN || 
            event->event_id == MPV_EVENT_END_FILE)
            break;
    }

    mpv_terminate_destroy(mpv);
    mpv = nullptr;
}
