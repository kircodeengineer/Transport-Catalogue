//#include "input_reader.h"
//#include "stat_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "json.h"
#include "svg.h"
#include "json_reader.h"
#include "domain.h"
#include "map_renderer.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <set>




int main() {
	/*
	 * Примерная структура программы:
	 *
	 * Считать JSON из stdin
	 * Построить на его основе JSON базу данных транспортного справочника
	 * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
	 * с ответами.
	 * Вывести в stdout ответы в виде JSON
	 */

	using namespace std::literals;

	// для отладки локально
#ifdef  _WIN32
	//std::cout << "windows\n"s;
	std::fstream file;
	file.open("input.json"s, std::ios::in);
	if (!file) {
		std::cout << "file read error!!!\n"s;
		return 0;
	}
	std::ofstream save_file("result.json");
	auto& out = save_file;
	std::ofstream save_file_svg("result.svg");
	//auto& out_svg = save_file_svg;
	// для тренажёра
#else
	//std::cout << "linux\n"s;
	auto& out = std::cout;
	//auto& out_svg = std::cout;
	std::istream& file = std::cin;
#endif
	json::Document doc = json::Load(file);
	JsonReader json_reader(doc);
	json::Document result_doc = json_reader.ProcessJsonDoc();
	json::Print(result_doc, out);
	return 0;
}
