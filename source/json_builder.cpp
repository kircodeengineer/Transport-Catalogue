#include "json_builder.h"
namespace json {
	Builder::Builder() {
		nodes_stack_.emplace_back(&root_);
	}

	ArrayNode::ArrayNode(Builder&& builder) : builder_(builder) {}
	DictNode::DictNode(Builder&& builder) : builder_(builder) {}
	KeyNode::KeyNode(Builder&& builder) : builder_(builder) {}


	//--------------- KEY -------------//
	KeyNode Builder::Key(std::string key) {
		auto& top = nodes_stack_.back();
		auto& new_node = (const_cast<Dict&>(top->AsDict())[key]);
		nodes_stack_.emplace_back(&new_node);
		return std::move(*this);
	}

	KeyNode DictNode::Key(std::string key) {
		return builder_.Key(std::move(key));
	}
	//--------------- KEY -------------//

	//--------------- VALUE -------------//
	Builder& Builder::Value(Node value) {
		auto& top = nodes_stack_.back();
		if (top->IsArray()) {
			auto& arr = const_cast<Array&>(top->AsArray());
			arr.emplace_back(value);
		}
		else {
			*top = value;
			nodes_stack_.pop_back();
		}
		return *this;
	}

	ArrayNode ArrayNode::Value(Node value) {
		return std::move(builder_.Value(std::move(value)));
	}

	DictNode KeyNode::Value(Node value) {
		return std::move(builder_.Value(std::move(value)));
	}
	//--------------- VALUE -------------//

	//--------------- START_DICT -------------//
	DictNode Builder::StartDict() {
		auto& top = nodes_stack_.back();
		if (top->IsArray()) {
			auto& arr = const_cast<Array&>(top->AsArray());
			arr.emplace_back(Dict());
			auto& new_node = arr.back();
			nodes_stack_.emplace_back(&new_node);
		}
		else {
			*top = Dict();
		}
		return std::move(*this);
	}

	DictNode ArrayNode::StartDict() {
		return builder_.StartDict();
	}

	DictNode KeyNode::StartDict() {
		return builder_.StartDict();
	}
	//--------------- START_DICT -------------//

	//--------------- START_ARRAY -------------//
	ArrayNode Builder::StartArray() {
		auto& top = nodes_stack_.back();
		if (top->IsArray()) {
			auto& arr = const_cast<Array&>(top->AsArray());
			arr.emplace_back(Array());
			auto& new_node = arr.back();
			nodes_stack_.emplace_back(&new_node);
		}
		else {
			*top = Array();
		}
		return std::move(*this);
	}

	ArrayNode ArrayNode::StartArray() {
		return builder_.StartArray();
	}

	ArrayNode KeyNode::StartArray() {
		return builder_.StartArray();
	}
	//--------------- START_ARRAY -------------//

	//--------------- END -------------//
	Builder& Builder::EndDict() {
		nodes_stack_.pop_back();
		return *this;
	}

	Builder& DictNode::EndDict() {
		return builder_.EndDict();
	}

	Builder& Builder::EndArray() {
		nodes_stack_.pop_back();
		return *this;
	}

	Builder& ArrayNode::EndArray() {
		return builder_.EndArray();
	}
	//--------------- END -------------//

	json::Node Builder::Build() {
		return root_;
	}
}//namespace json
