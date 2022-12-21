//sprint 14 itog

#include <fstream>
#include <iostream>
#include <string_view>

#include <clocale>
#include "serialization.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
    std::setlocale(LC_NUMERIC, "C");

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        ReaderJson::Reader reader(std::cin);

        TransportCatalogue catalogue;
        reader.FillCatalogue(catalogue);
        
        renderer::RendererMap render;
        render.SetRendererSettings(reader.GetRendererSettings());

        TransportRouter router(catalogue, reader.GetRoutingSettings());
        router.BuildGraph();

        std::ofstream fout(reader.GetSerializationSettings().file_name, std::ios::binary);
        if (fout.is_open()) {
            Serialize(catalogue, render, router, fout);
        }
    }
    else if (mode == "process_requests"sv) {
        ReaderJson::Reader reader(std::cin);
        std::ifstream fin(reader.GetSerializationSettings().file_name, std::ios::binary);
        if (fin.is_open()) {
            auto [catalogue, render, router, graph, stop_ids] = Deserialize(fin);
            RequestHandler request(catalogue, render, router);
            router.SetGraph(std::move(graph), std::move(stop_ids));
            reader.FillRequest(request);
            request.PrintStat();
        }
    }
    else {
        PrintUsage();
        return 1;
    }
}
