//sprint 12 itog

#include <clocale>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
    std::setlocale(LC_NUMERIC, "C");

    //std::istringstream input(inp_json);

    TransportCatalogue catalogue;
    TransportRouter router(catalogue);
    renderer::RendererMap render;
    RequestHandler request(catalogue, router, render);
   
    ReaderJson::Reader reader(catalogue, router, render, request);

    //reader.ReadData(input);
    reader.ReadData(std::cin);

    request.PrintStat();

}

