//sprint 9 itog понять и простить)
#include <iostream>


#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

//using namespace std;

int main() {
    TransportCatalogue catalogue;

    input::Reader data;
    data.ReadData(std::cin, catalogue);

    output::Reader stat;
    stat.ReadData(std::cin, catalogue);
}

