#include <iomanip>
#include <random>
#include <signal.h>
#include <unistd.h>
#include "worker.h"


int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    loadStationsFromConfig();

    int listFlag = 0;
    std::string station_to_play;
    
    int opt;
    while ((opt = getopt(argc, argv, "lvs:h")) != -1) {
        switch (opt) {
            case 'l':
                listFlag = 1;
                break;
            case 'v':
                listFlag = 2;
                break;
            case 's':
                station_to_play = optarg;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }

    if (listFlag == 1) {
        listStations();
        return 0;
    }
    if (listFlag == 2) {
        listStationsVerbose();
        return 0;
    }

    if (!station_to_play.empty()) {
        if (stations.find(station_to_play) == stations.end()) {
            std::cerr << "Error: Station '" << station_to_play << "' not found in the configuration." << std::endl;
            return 1;
        }
        playStation(station_to_play);
    } else if (optind == 1) { 
        // No arguments, play a random station
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, stations.size() - 1);
        int index = dis(gen);
        auto it = std::begin(stations);
        std::advance(it, index);
        playStation(it->first);
    } else {
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
