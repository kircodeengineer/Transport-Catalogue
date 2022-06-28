#include "json.h"
#include <stdexcept>
#include <sstream>


using namespace std;

namespace json {

	namespace {

		Node LoadNode(istream& input);

		class ParsingError : public std::runtime_error {
		public:
			using runtime_error::runtime_error;
		};

		// Считывает содержимое строкового литерала JSON-документа
		// Функцию следует использовать после считывания открывающего символа ":
		std::string LoadStringTrainer(std::istream& input) {
			using namespace std::literals;

			auto it = std::istreambuf_iterator<char>(input);
			auto end = std::istreambuf_iterator<char>();
			std::string s;
			while (true) {
				if (it == end) {
					// Поток закончился до того, как встретили закрывающую кавычку?
					throw json::ParsingError("String parsing error");
				}
				const char ch = *it;
				if (ch == '"') {
					// Встретили закрывающую кавычку
					++it;
					break;
				}
				else if (ch == '\\') {
					// Встретили начало escape-последовательности
					++it;
					if (it == end) {
						// Поток завершился сразу после символа обратной косой черты
						throw ParsingError("String parsing error");
					}
					const char escaped_char = *(it);
					// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
					switch (escaped_char) {
					case 'n':
						s.push_back('\n');
						break;
					case 't':
						s.push_back('\t');
						break;
					case 'r':
						s.push_back('\r');
						break;
					case '"':
						s.push_back('"');
						break;
					case '\\':
						s.push_back('\\');
						break;
					default:
						// Встретили неизвестную escape-последовательность
						throw json::ParsingError("Unrecognized escape sequence \\"s + escaped_char);
					}
				}
				else if (ch == '\n' || ch == '\r') {
					// Строковый литерал внутри- JSON не может прерываться символами \r или \n
					throw json::ParsingError("Unexpected end of line"s);
				}
				else {
					// Просто считываем очередной символ и помещаем его в результирующую строку
					s.push_back(ch);
				}
				++it;
			}
			return s;
		}
		using Number = std::variant<int, double>;
		Number LoadNumber(std::istream& input) {
			using namespace std::literals;

			std::string parsed_num;

			// Считывает в parsed_num очередной символ из input
			auto read_char = [&parsed_num, &input] {
				parsed_num += static_cast<char>(input.get());
				if (!input) {
					throw ParsingError("Failed to read number from stream"s);
				}
			};

			// Считывает одну или более цифр в parsed_num из input
			auto read_digits = [&input, read_char] {
				if (!std::isdigit(input.peek())) {
					throw ParsingError("A digit is expected"s);
				}
				while (std::isdigit(input.peek())) {
					read_char();
				}
			};

			if (input.peek() == '-') {
				read_char();
			}
			// Парсим целую часть числа
			if (input.peek() == '0') {
				read_char();
				// После 0 в JSON не могут идти другие цифры
			}
			else {
				read_digits();
			}

			bool is_int = true;
			// Парсим дробную часть числа
			if (input.peek() == '.') {
				read_char();
				read_digits();
				is_int = false;
			}

			// Парсим экспоненциальную часть числа
			if (int ch = input.peek(); ch == 'e' || ch == 'E') {
				read_char();
				if (ch = input.peek(); ch == '+' || ch == '-') {
					read_char();
				}
				read_digits();
				is_int = false;
			}

			try {
				if (is_int) {
					// Сначала пробуем преобразовать строку в int
					try {
						return std::stoi(parsed_num);
					}
					catch (...) {
						// В случае неудачи, например, при переполнении,
						// код ниже попробует преобразовать строку в double
					}
				}
				return std::stod(parsed_num);
			}
			catch (...) {
				throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
			}
		}

		Node LoadString(istream& input) {
			return Node(move(LoadStringTrainer(input)));
		}

		Node LoadArray(istream& input) {
			if (input.peek() == -1) {
				throw json::ParsingError("LoadArray error"s);
			}
			Array result;
			char c;
			for (; input >> c && c != ']';) {
				if (c != ',') {
					input.putback(c);
				}
				result.push_back(LoadNode(input));
			}
			return Node(move(result));
		}

		Node LoadDict(istream& input) {
			if (input.peek() == -1) {
				throw json::ParsingError("LoadDict error"s);
			}
			Dict result;
			char c;
			for (; input >> c && c != '}';) {
				if (c == ',') {
					input >> c;
				}
				string key = LoadString(input).AsString();
				input >> c;
				result.insert({ move(key), LoadNode(input) });
			}

			return Node(move(result));
		}

		Node LoadNode(istream& input) {
			char c;
			input >> c;

			if (c == '[') {
				return LoadArray(input);
			}
			else if (c == ']') {
				return LoadArray(input);
			}
			else if (c == '{') {
				return LoadDict(input);
			}
			else if (c == '}') {
				return LoadDict(input);
			}
			else if (c == '"') {
				return LoadString(input);
			}
			else if (c == 'n') {
				static std::string name = "null"s;
				size_t letters_count = 1;
				while (input >> c) {
					if (name[letters_count] != c) {
						throw json::ParsingError("Null error"s);
					}
					++letters_count;
					if (name.size() == letters_count) {
						break;
					}
				}
				if (letters_count < name.size()) {
					throw json::ParsingError("Null error"s);
				}
				return Node(nullptr);
			}
			else if (c == 't') {
				static std::string name = "true"s;
				for (const auto& letter : name) {
					if (letter != c) {
						throw json::ParsingError("True error"s);
					}
					input >> c;
				}
				input.putback(c);
				return Node(true);
			}
			else if (c == 'f') {
				static std::string name = "false"s;
				for (const auto& letter : name) {
					if (letter != c) {
						throw json::ParsingError("False error"s);
					}
					input >> c;
				}
				input.putback(c);
				return Node(false);
			}
			else {
				input.putback(c);
				Number number = LoadNumber(input);
				//std::cout << "kek " << std::get<double>(number);
				if (std::holds_alternative<double>(number))
					return Node(std::get<double>(number));
				return Node(std::get<int>(number));
			}
		}

	}  // namespace

	Node::Node()
		: value_(nullptr) {
	}

	Node::Node(std::nullptr_t value)
		: value_(move(value)) {
	}

	Node::Node(Array array)
		: value_(move(array)) {
	}

	Node::Node(Dict map)
		: value_(move(map)) {
	}

	Node::Node(bool value)
		: value_(move(value)) {
	}

	Node::Node(int value)
		: value_(value) {
	}

	Node::Node(double value)
		: value_(move(value)) {
	}

	Node::Node(string value)
		: value_(move(value)) {
	}

	int Node::AsInt() const {
		using namespace std::literals;
		if (!IsInt()) {
			throw std::logic_error("logic error, odnako\n"s);
		}
		return std::get<int>(value_);
	}

	bool Node::AsBool() const {
		using namespace std::literals;
		if (!IsBool()) {
			throw std::logic_error("logic error, odnako\n"s);
		}
		return std::get<bool>(value_);
	}

	double Node::AsDouble() const {
		using namespace std::literals;
		if (!IsDouble()) {
			throw std::logic_error("logic error, odnako\n"s);
		}
		if (IsInt()) {
			return static_cast<double>(std::get<int>(value_));
		}
		return std::get<double>(value_);
	}

	const std::string& Node::AsString() const {
		using namespace std::literals;
		if (!IsString()) {
			throw std::logic_error("logic error, odnako\n"s);
		}
		return std::get<string>(value_);
	}

	const Array& Node::AsArray() const {
		using namespace std::literals;
		if (!IsArray()) {
			throw std::logic_error("logic error, odnako\n"s);
		}
		return std::get<Array>(value_);
	}

	const Dict& Node::AsDict() const {
		using namespace std::literals;
		if (!IsMap()) {
			throw std::logic_error("logic error, odnako\n"s);
		}
		return std::get<Dict>(value_);
	}

	bool Node::IsInt() const {
		return std::holds_alternative<int>(value_);
	}

	bool Node::IsDouble() const {
		return (std::holds_alternative<double>(value_) ||
			std::holds_alternative<int>(value_));
	}

	bool Node::IsPureDouble() const {
		return std::holds_alternative<double>(value_);
	}

	bool Node::IsBool() const {
		return std::holds_alternative<bool>(value_);
	}

	bool Node::IsString() const {
		return std::holds_alternative<std::string>(value_);
	}

	bool Node::IsNull() const {
		return std::holds_alternative<std::nullptr_t>(value_);
	}

	bool Node::IsArray() const {
		return std::holds_alternative<Array>(value_);
	}

	bool Node::IsMap() const {
		return std::holds_alternative<Dict>(value_);
	}

	Document::Document(Node root)
		: root_(move(root)) {
	}

	const Node& Document::GetRoot() const {
		return root_;
	}

	Document Load(istream& input) {
		return Document{ LoadNode(input) };
	}

	void Print(const Document& doc, std::ostream& output) {
		std::visit(
			[&output](const auto& value) { PrintValue(value, output); },
			doc.GetRoot().GetValue());
		// Реализуйте функцию самостоятельно
	}

	bool operator==(const Node& left, const Node& right) {
		if ((left.IsInt() && right.IsPureDouble()) || (left.IsPureDouble() && right.IsInt())) {
			return false;
		}
		if (left.IsDouble() && right.IsDouble()) {
			static const double EPSILON = 0.0000000001;
			double left_value = (left.IsInt()) ? std::get<int>(left.GetValue()) : std::get<double>(left.GetValue());
			double right_value = (right.IsInt()) ? std::get<int>(right.GetValue()) : std::get<double>(right.GetValue());
			return (std::abs(right_value - left_value) < EPSILON);
		}
		return (left.GetValue() == right.GetValue());
	}

	bool operator!=(const Node& left, const Node& right) {
		return !(left == right);
	}

	bool operator==(const Document& left, const Document& right) {
		return left.GetRoot() == right.GetRoot();
	}

	bool operator!=(const Document& left, const Document& right) {
		return !(left.GetRoot() == right.GetRoot());
	}

	//std::string Print(const Node& node) {
	//	std::ostringstream strm;
	//	std::visit(Node::NodePrinter{ strm }, node.GetValue());
	//	return strm.str();
	//};

	std::ostream& operator<<(std::ostream& out, const Node& value) {
		PrintValue(value, out);
		return out;
	}

	void PrintValue(std::nullptr_t, std::ostream& out) {
		out << "null"sv;
	}

	void PrintValue(bool value, std::ostream& out) {
		if (value) {
			out << "true"sv;
		}
		else {
			out << "false"sv;
		}
	}

	void PrintValue(const std::string& value, std::ostream& out) {
		out << '\"';
		for (const char& letter : value) {
			switch (letter) {
			case '"':
				out << '\\' << '"';
				break;
			case '\\':
				out << '\\' << letter;
				break;
			case '\t':
				out << "\\t"sv;
				break;
			case '\n':
				out << "\\n"sv;
				break;
			case '\r':
				out << "\\r"sv;
				break;
			case ']':
				out << "\\]"sv;
				break;
			case '}':
				out << "\\}"sv;
				break;
			default:
				out << letter;
				break;
			}
		}
		out << "\"";
	}

	void PrintValue(const Array& value, std::ostream& out) {
		out << "["sv;
		if (!value.empty()) {
			PrintNode(*value.begin(), out);
			for (auto it = std::next(value.begin(), 1); it != value.end(); std::advance(it, 1)) {
				out << ","sv;
				PrintNode(*it, out);
			}
		}
		out << "]"sv;
	}

	void PrintValue(const Dict& value, std::ostream& out) {
		out << "{ "sv;
		if (!value.empty()) {
			PrintNode(value.begin()->first, out);
			out << ": "sv;
			PrintNode(value.begin()->second, out);
			for (auto it = std::next(value.begin(), 1); it != value.end(); std::advance(it, 1)) {
				out << ", "sv;
				PrintNode(it->first, out);
				out << ": "sv;
				PrintNode(it->second, out);
			}
		}
		out << " }"sv;
	}

	void PrintNode(const Node& node, std::ostream& out) {
		std::visit(
			[&out](const auto& value) { PrintValue(value, out); },
			node.GetValue());
	}
}  // namespace json
