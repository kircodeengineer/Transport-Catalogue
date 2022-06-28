#include "svg.h"

namespace svg {

	using namespace std::literals;

	std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
		switch (stroke_line_cap) {
		case StrokeLineCap::BUTT:
			out << "butt"s;
			break;
		case StrokeLineCap::ROUND:
			out << "round"s;
			break;
		case StrokeLineCap::SQUARE:
			out << "square"s;
			break;
		}
		return out;
	}

	std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) {
		switch (stroke_line_join) {
		case StrokeLineJoin::ARCS:
			out << "arcs"s;
			break;
		case StrokeLineJoin::BEVEL:
			out << "bevel"s;
			break;
		case StrokeLineJoin::MITER:
			out << "miter"s;
			break;
		case StrokeLineJoin::MITER_CLIP:
			out << "miter-clip"s;
			break;
		case StrokeLineJoin::ROUND:
			out << "round"s;
			break;
		}
		return out;
	}

	RenderContext::RenderContext(std::ostream& out)
		: out(out) {
	}

	RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
		: out(out)
		, indent_step(indent_step)
		, indent(indent) {
	}

	RenderContext RenderContext::Indented() const {
		return { out, indent_step, indent + indent_step };
	}

	void RenderContext::RenderIndent() const {
		for (int i = 0; i < indent; ++i) {
			out.put(' ');
		}
	}

	void Object::Render(const RenderContext& context) const {
		context.RenderIndent();

		// Делегируем вывод тега своим подклассам
		RenderObject(context);

		context.out << std::endl;
	}



	// ---------- Circle ------------------
	Circle& Circle::SetCenter(Point center) {
		center_ = center;
		return *this;
	}

	Circle& Circle::SetRadius(double radius) {
		radius_ = radius;
		return *this;
	}

	void Circle::RenderObject(const RenderContext& context) const {
		auto& out = context.out;
		out << "\t<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
		out << "r=\""sv << radius_ << "\""sv;
		// Выводим атрибуты, унаследованные от PathProps
		RenderAttrs(context.out);
		out << "/>"sv;
	}

	// ---------- Polyline ------------------
	Polyline& Polyline::AddPoint(Point point) {
		points_.emplace_back(point);
		return *this;
	}

	void Polyline::RenderObject(const RenderContext& context) const {
		using namespace std::literals;
		auto& out = context.out;
		out << "\t<polyline points=\""sv;
		if (!points_.empty()) {
			auto it = points_.begin();
			out << it->x << ',' << it->y;
			for (it = std::next(it, 1); it != points_.end(); std::advance(it, 1)) {
				out << ' ' << it->x << ',' << it->y;
			}
		}
		out << "\""sv;
		// Выводим атрибуты, унаследованные от PathProps
		RenderAttrs(context.out);
		out << "/>"sv;
	}

	// ---------- Text ------------------
	Text& Text::SetPosition(Point pos) {
		pos_ = pos;
		return *this;
	}

	Text& Text::SetOffset(Point offset) {
		offset_ = offset;
		return *this;
	}

	Text& Text::SetFontSize(uint32_t size) {
		size_ = size;
		return *this;
	}

	Text& Text::SetFontFamily(std::string font_family) {
		font_family_ = font_family;
		return *this;
	}

	Text& Text::SetFontWeight(std::string font_weight) {
		font_weight_ = font_weight;
		return *this;
	}

	Text& Text::SetData(std::string data) {
		data_ = data;
		return *this;
	}

	void Text::RenderObject(const RenderContext& context) const {
		using namespace std::literals;
		auto& out = context.out;
		out << "\t<text"sv;
		RenderAttrs(out);
		out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y
			<< "\" dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y
			<< "\" font-size=\""sv << size_;
		if (!font_family_.empty())
			out << "\" font-family=\""sv << font_family_;
		if (!font_weight_.empty())
			out << "\" font-weight=\""sv << font_weight_;
		out << "\">";

		for (const auto& letter : data_) {
			if (letter == '<')
				out << "&lt;";
			else if (letter == '>')
				out << "&rt;";
			else if (letter == '\"')
				out << "&quot;";
			else if (letter == '\'')
				out << "&apos;";
			else if (letter == '&')
				out << "&amp;";
			else
				out << letter;
		}

		out << "</text>";
	}

	// ---------- Document ------------------
	void Document::AddPtr(std::unique_ptr<Object>&& obj) {
		objects_ptrs_.emplace_back(std::move(obj));
	}

	void Document::Render(std::ostream& out) const {
		using namespace std::literals;
		out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
		out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
		for (auto& object_ptr : objects_ptrs_) {
			object_ptr->Render({ out });
		}
		out << "</svg>"sv;
	}

} // namespace svg
