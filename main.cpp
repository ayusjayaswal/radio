#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <random>

std::unordered_map<std::string, std::string> stations;
pid_t child_pid = -1;

void displayHelp() {
    std::cout << "Usage: radio" << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -l\t\t\tList available radio stations" << std::endl;
    std::cout << "  -s \"<station>\"\tPlay the specified radio station" << std::endl;
    std::cout << "  -h\t\t\tDisplay this help message" << std::endl;
    std::cout << "If no options are provided, a random station will be played." << std::endl;
}

void signalHandler(int signum) {
    if (child_pid != -1) {
        kill(child_pid, SIGTERM);
        child_pid = -1;
        std::cout << "Radio stopped." << std::endl;
    }
    exit(signum);
}

void loadStationsFromConfig() {
    std::string config_file = std::string(getenv("HOME")) + "/.config/radio/config.ini";
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open configuration file " << config_file << std::endl;
        return;
    }

    std::string line, station_name, station_url;
    while (std::getline(file, line)) {
        if (line.find("=") != std::string::npos) {
            size_t pos = line.find("=");
            station_name = line.substr(0, pos);
            station_url = line.substr(pos + 1);
            stations[station_name] = station_url;
        }
    }
    file.close();
}

void listStations() {
    std::cout << "Available radio stations:" << std::endl;
    int i = 1;
    for (const auto& [name, _] : stations) {
        std::cout << i << ". " << name << std::endl;
        i++;
    }
}

void playStation(const std::string& station_name) {
    std::string command = "ffplay -nodisp " + stations[station_name];
    child_pid = fork();
    if (child_pid == 0) {
        std::system(command.c_str());
        exit(0);
    } else if (child_pid > 0) {
        std::cout << "Playing radio station: " << station_name << std::endl;
        // Wait for the user to press Ctrl+C to stop the radio
        while (true) {
            pause();
        }
    } else {
        std::cerr << "Error: Failed to fork process." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    loadStationsFromConfig();

    if (argc == 1) {
        // Play a random station
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, stations.size() - 1);
        int index = dis(gen);
        auto it = std::begin(stations);
        std::advance(it, index);
        playStation(it->first);
    } else if (argc == 2) {
        if (std::string(argv[1]) == "-l") {
            listStations();
        } else if (std::string(argv[1]) == "-h") {
	    displayHelp();
	} else {
            std::cerr << "Usage: " << argv[0] << " [-l | -h | -s <station_name>]" << std::endl;
            return 1;
        }
    } else if (argc == 3 && std::string(argv[1]) == "-s") {
        std::string station_to_play = argv[2];
        if (stations.find(station_to_play) == stations.end()) {
            std::cerr << "Error: Station '" << station_to_play << "' not found in the configuration." << std::endl;
            return 1;
        }
        playStation(station_to_play);
    } else {
        std::cerr << "Usage: " << argv[0] << " [-l | -i | -s <station_name>]" << std::endl;
        return 1;
    }

    return 0;
}
