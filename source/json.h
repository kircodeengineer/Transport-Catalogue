#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

	class Node;
	// Сохраните объявления Dict и Array без изменения
	using Dict = std::map<std::string, Node>;
	using Array = std::vector<Node>;

	// Эта ошибка должна выбрасываться при ошибках парсинга JSON
	class ParsingError : public std::runtime_error {
	public:
		using runtime_error::runtime_error;
	};

	/// @brief Нода JSON документа
	class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>
	{
	public:
		using variant::variant;
		const variant& GetValue() const { return value_; }

		Node();
		Node(std::nullptr_t value);
		Node(Array array);
		Node(Dict map);
		Node(bool value);
		Node(int value);
		Node(double value);
		Node(std::string value);


		int AsInt() const;
		bool AsBool() const;
		double AsDouble() const;
		const std::string& AsString() const;
		const Array& AsArray() const;
		const Dict& AsDict() const;

		bool IsInt() const;
		bool IsDouble() const;
		bool IsPureDouble() const;
		bool IsBool() const;
		bool IsString() const;
		bool IsNull() const;
		bool IsArray() const;
		bool IsMap() const;

	private:
		variant value_;
	};

	/// @brief JSON документ
	class Document {
	public:
		explicit Document(Node root);

		const Node& GetRoot() const;

	private:
		/// @brief Корневая нода
		Node root_;
	};

	Document Load(std::istream& input);

	/// @brief Печать JSON документа
	/// @param doc JSON документ
	/// @param output потока вывода
	void Print(const Document& doc, std::ostream& output);

	bool operator==(const Node& left, const Node& right);
	bool operator!=(const Node& left, const Node& right);
	bool operator==(const Document& left, const Document& right);
	bool operator!=(const Document& left, const Document& right);
	std::ostream& operator<<(std::ostream& out, const Node& value);

	// Шаблон, подходящий для вывода double и int
	template <typename Value>
	void PrintValue(const Value& value, std::ostream& out);
	// Перегрузка функции PrintValue для вывода значений 
	void PrintValue(std::nullptr_t, std::ostream& out);
	void PrintValue(const std::string& value, std::ostream& out);
	void PrintValue(bool value, std::ostream& out);
	void PrintValue(const Array& value, std::ostream& out);
	void PrintValue(const Dict& value, std::ostream& out);
	void PrintNode(const Node& node, std::ostream& out);

}  // namespace json


template <typename Value>
void json::PrintValue(const Value& value, std::ostream& out) {
	out << value;
}
