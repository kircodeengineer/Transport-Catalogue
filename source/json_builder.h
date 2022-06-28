#include "json.h"
#include <string>
#include <vector>

namespace json {
	class Builder;

	class KeyNode;
	class DictNode;
	class ArrayNode;

	class Builder {
	private:
		/// @brief сам конструируемый объект
		Node root_ = Node{};
		/// @brief стек указателей на те вершины JSON, 
		///		которые ещё не построены: то есть текущее описываемое значение и цепочка его родителей. 
		/// Он поможет возвращаться в нужный контекст после вызова End-методов.
		std::vector<Node*> nodes_stack_;

	public:
		Builder();
		~Builder() = default;

		/// @brief При определении словаря задаёт строковое значение ключа для очередной пары ключ - значение.
		/// Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение
		///		 с помощью метода Value или начинать его определение с помощью StartDict или StartArray.
		/// @param
		virtual KeyNode Key(std::string key);

		/// @brief Задаёт значение, соответствующее ключу при определении словаря, 
		/// очередной элемент массива или, если вызвать сразу после конструктора json::Builder, 
		///		всё содержимое конструируемого JSON-объекта. 
		/// Может принимать как простой объект — число или строку — так и целый массив или словарь. 
		/// Здесь Node::Value — это синоним для базового класса Node, 
		///		шаблона variant с набором возможных типов-значений. Смотрите заготовку кода.
		/// @param  
		/// 
		Builder& Value(Node value);

		/// @brief Начинает определение сложного значения - словаря.
		/// Вызывается в тех же контекстах, что и Value.
		/// Следующим вызовом обязательно должен быть Key или EndDict.
		virtual DictNode StartDict();

		/// @brief Начинает определение сложного значения - массива.
		/// Вызывается в тех же контекстах, что и Value.
		/// Следующим вызовом обязательно должен быть EndArray или 
		///		любой, задающий новое значение : Value, StartDict или StartArray.
		virtual ArrayNode StartArray();

		/// @brief Завершает определение сложного значения - словаря.
		/// Последним незавершённым вызовом Start * должен быть StartDict.
		virtual Builder& EndDict();

		/// @brief Завершает определение сложного значения - массива.
		/// Последним незавершённым вызовом Start * должен быть StartArray.
		virtual Builder& EndArray();

		/// @brief Build(). Возвращает объект json::Node, содержащий JSON, описанный предыдущими вызовами методов. 
		/// К этому моменту для каждого Start* должен быть вызван соответствующий End*. 
		/// При этом сам объект должен быть определён, то есть вызов json::Builder{}.Build() недопустим.
		json::Node Build();

	};

	class DictNode final : Builder {
		/// Следующим вызовом обязательно должен быть Key или EndDict.
	public:
		DictNode(Builder&& builder);
		KeyNode Key(std::string key) override;
		Builder& EndDict() override;

	private:
		Builder& builder_;
	};

	class ArrayNode final : Builder {
		/// Следующим вызовом обязательно должен быть EndArray или
		///		любой, задающий новое значение : Value, StartDict или StartArray.
	public:
		ArrayNode(Builder&& builder);
		DictNode StartDict() override;
		ArrayNode Value(Node value);
		ArrayNode StartArray() override;
		Builder& EndArray() override;
	private:
		Builder& builder_;
	};

	class KeyNode final : Builder {
		/// Следующий вызов метода обязательно должен задавать соответствующее этому ключу значение
		///		 с помощью метода Value или начинать его определение с помощью StartDict или StartArray.
	public:
		KeyNode(Builder&& builder);
		DictNode StartDict() override;
		ArrayNode StartArray() override;
		DictNode Value(Node value);
	private:
		Builder& builder_;
	};


}
