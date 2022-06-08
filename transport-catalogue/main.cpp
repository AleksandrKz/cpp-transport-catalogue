//sprint 10 itog

#include <clocale>

#include "map_renderer.h"
#include "json_reader.h"
#include "request_handler.h"

using namespace std;

int main() {
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
    std::setlocale(LC_NUMERIC, "C");

    string inp_json = 
    "{ \n \
  \"base_requests\": [ \n \
    { \n \
      \"type\": \"Bus\", \n \
      \"name\": \"14\", \n \
      \"stops\": [ \n \
        \"Ulitsa Lizy Chaikinoi\", \n \
        \"Elektroseti\", \n \
        \"Ulitsa Dokuchaeva\", \n \
        \"Ulitsa Lizy Chaikinoi\" \n \
      ], \n \
      \"is_roundtrip\": true \n \
    }, \n \
    { \n \
      \"type\": \"Bus\", \n \
      \"name\": \"114\", \n \
      \"stops\": [ \n \
        \"Morskoy vokzal\", \n \
        \"Rivierskiy most\" \n \
      ], \n \
      \"is_roundtrip\": false \n \
    }, \n \
    { \n \
      \"type\": \"Stop\", \n \
      \"name\": \"Rivierskiy most\", \n \
      \"latitude\": 43.587795, \n \
      \"longitude\": 39.716901, \n \
      \"road_distances\": { \n \
        \"Morskoy vokzal\": 850 \n \
      } \n \
    }, \n \
    { \n \
      \"type\": \"Stop\", \n \
      \"name\": \"Morskoy vokzal\", \n \
      \"latitude\": 43.581969, \n \
      \"longitude\": 39.719848, \n \
      \"road_distances\": { \n \
        \"Rivierskiy most\": 850 \n \
      } \n \
    }, \n \
    { \n \
      \"type\": \"Stop\", \n \
      \"name\": \"Elektroseti\", \n \
      \"latitude\": 43.598701, \n \
      \"longitude\": 39.730623, \n \
      \"road_distances\": { \n \
        \"Ulitsa Dokuchaeva\": 3000, \n \
        \"Ulitsa Lizy Chaikinoi\": 4300 \n \
      } \n \
    }, \n \
    { \n \
      \"type\": \"Stop\", \n \
      \"name\": \"Ulitsa Dokuchaeva\", \n \
      \"latitude\": 43.585586, \n \
      \"longitude\": 39.733879, \n \
      \"road_distances\": { \n \
        \"Ulitsa Lizy Chaikinoi\": 2000, \n \
        \"Elektroseti\": 3000 \n \
      } \n \
    }, \n \
    { \n \
      \"type\": \"Stop\", \n \
      \"name\": \"Ulitsa Lizy Chaikinoi\", \n \
      \"latitude\": 43.590317, \n \
      \"longitude\": 39.746833, \n \
      \"road_distances\": { \n \
        \"Elektroseti\": 4300, \n \
        \"Ulitsa Dokuchaeva\": 2000 \n \
      } \n \
    } \n \
  ], \n \
  \"render_settings\": { \n \
    \"width\": 600, \n \
    \"height\": 400, \n \
    \"padding\": 50, \n \
    \"stop_radius\": 5, \n \
    \"line_width\": 14, \n \
    \"bus_label_font_size\": 20, \n \
    \"bus_label_offset\": [ \n \
      7, \n \
      15 \n \
    ], \n \
    \"stop_label_font_size\": 20, \n \
    \"stop_label_offset\": [ \n \
      7, \n \
      -3 \n \
    ], \n \
    \"underlayer_color\": [ \n \
      255, \n \
      255, \n \
      255, \n \
      0.85 \n \
    ], \n \
    \"underlayer_width\": 3, \n \
    \"color_palette\": [ \n \
      \"green\", \n \
      [255, 160, 0], \n \
      \"red\" \n \
    ] \n \
  }, \n \
  \"stat_requests\": [ \n \
    { \"id\": 1, \"type\" : \"Map\" }, \n \
    { \"id\": 2, \"type\" : \"Stop\", \"name\" : \"Ривьерский мост\" }, \n \
    { \"id\": 3, \"type\" : \"Bus\", \"name\" : \"114\" } \n \
    ] \n \
}";

    std::istringstream input(inp_json);

    TransportCatalogue catalogue;
    renderer::RendererMap render;
    RequestHandler request(catalogue, render);
   
    ReaderJson::Reader reader(catalogue, render, request);

    //reader.ReadData(input);
    reader.ReadData(std::cin);

    request.PrintStat();

}

