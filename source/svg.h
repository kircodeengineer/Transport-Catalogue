#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <deque>
#include <list>
#include <optional>

/// @brief графика
namespace svg {
	/// @brief тип формы конца линии
	enum class StrokeLineCap {
		BUTT,
		ROUND,
		SQUARE,
	};
	/// @brief тип формы соединения линий
	enum class StrokeLineJoin {
		ARCS,
		BEVEL,
		MITER,
		MITER_CLIP,
		ROUND,
	};

	std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap);

	std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

	using Color = std::string;

	inline const Color NoneColor{ "none" };

	class Object;

	template <typename Owner>
	class PathProps {
	public:
		Owner& SetFillColor(Color color);
		Owner& SetStrokeColor(Color color);

		/// @brief задаёт значение свойства stroke - width — толщину линии.По умолчанию свойство не выводится.
		Owner& SetStrokeWidth(double width);

		/// @brief задаёт значение свойства stroke - linecap — тип формы конца линии.По умолчанию свойство не выводится.
		Owner& SetStrokeLineCap(StrokeLineCap line_cap);

		/// @brief задаёт значение свойства stroke - linejoin — тип формы соединения линий.По умолчанию свойство не выводится.
		Owner& SetStrokeLineJoin(StrokeLineJoin line_join);

	protected:
		~PathProps() = default;

		void RenderAttrs(std::ostream& out) const;

	private:
		Owner& AsOwner() {
			// static_cast безопасно преобразует *this к Owner&,
			// если класс Owner — наследник PathProps
			return static_cast<Owner&>(*this);
		}

		std::optional<Color> fill_color_;
		std::optional<Color> stroke_color_;
		std::optional<double> width_;
		std::optional <StrokeLineCap> line_cap_;
		std::optional <StrokeLineJoin> line_join_;
	};

	template <typename Owner>
	Owner& PathProps<Owner>::SetFillColor(Color color) {
		fill_color_ = std::move(color);
		return AsOwner();
	}

	template <typename Owner>
	Owner& PathProps<Owner>::SetStrokeColor(Color color) {
		stroke_color_ = std::move(color);
		return AsOwner();
	}

	template <typename Owner>
	Owner& PathProps<Owner>::SetStrokeWidth(double width) {
		width_ = width;
		return AsOwner();
	}

	template <typename Owner>
	Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
		line_cap_ = line_cap;
		return AsOwner();
	}

	template <typename Owner>
	Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
		line_join_ = line_join;
		return AsOwner();
	}

	template <typename Owner>
	void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
		using namespace std::literals;

		if (fill_color_) {
			out << " fill=\""sv << *fill_color_ << "\""sv;
		}
		if (stroke_color_) {
			out << " stroke=\""sv << *stroke_color_ << "\""sv;
		}
		if (width_) {
			out << " stroke-width=\""sv << *width_ << "\""sv;
		}
		if (line_cap_) {
			out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
		}
		if (line_join_) {
			out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
		}
	}


	class ObjectContainer {
	protected:
		std::deque<std::unique_ptr<Object>> objects_ptrs_;
	public:
		virtual ~ObjectContainer() = default;
		virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

		template <typename Obj>
		void Add(Obj obj);
	};

	template <typename Obj>
	void ObjectContainer::Add(Obj obj) {
		AddPtr(std::move(std::make_unique<Obj>(std::move(obj))));
	}

	// Интерфейс Drawable задаёт объекты, которые можно нарисовать с помощью Graphics
	class Drawable {
	public:
		virtual void Draw(ObjectContainer& container) const = 0;
		virtual ~Drawable() = default;
	};

	struct Point {
		Point() = default;
		Point(double x, double y)
			: x(x)
			, y(y) {
		}
		double x = 0.0;
		double y = 0.0;
	};

	/// @brief Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
	/// @detail Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
	struct RenderContext {
		RenderContext(std::ostream& out);

		RenderContext(std::ostream& out, int indent_step, int indent = 0);

		RenderContext Indented() const;

		void RenderIndent() const;

		std::ostream& out;
		int indent_step = 0;
		int indent = 0;
	};

	/// @brief Абстрактный базовый класс Object служит для унифицированного хранения конкретных тегов SVG-документа
	/// @detail Реализует паттерн "Шаблонный метод" для вывода содержимого тега
	class Object {
	public:
		void Render(const RenderContext& context) const;

		virtual ~Object() = default;

	private:
		virtual void RenderObject(const RenderContext& context) const = 0;
	};

	/// @brief Класс Circle моделирует элемент <circle> для отображения круга
	/// @detail https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
	/// @detail Наследованием от PathProps<Circle> мы «сообщаем» родителю, что владельцем свойств является класс Circle
	class Circle final : public Object, public PathProps<Circle> {
	public:
		Circle& SetCenter(Point center);
		Circle& SetRadius(double radius);

	private:
		void RenderObject(const RenderContext& context) const override;

		Point center_;
		double radius_ = 1.0;
	};

	/// @brief Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
	/// @detail https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
	class Polyline : public Object, public PathProps<Polyline> {
	private:
		std::list<Point> points_;
	public:
		/// @brief Добавляет очередную вершину к ломаной линии
		/// @param point 
		/// @return 
		Polyline& AddPoint(Point point);

		void RenderObject(const RenderContext& context) const override;
	};

	/// @brief Класс Text моделирует элемент <text> для отображения текста
	/// @detail https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
	class Text : public Object, public PathProps<Text> {
	private:
		Point pos_ = { 0.0, 0.0 };
		Point offset_ = { 0.0, 0.0 };
		uint32_t size_ = 1;
		std::string font_family_;
		std::string font_weight_;
		std::string data_;

	public:
		/// @brief Задаёт координаты опорной точки (атрибуты x и y)
		/// @param pos 
		/// @return 
		Text& SetPosition(Point pos);

		/// @brief Задаёт смещение относительно опорной точки (атрибуты dx, dy)
		/// @param offset 
		/// @return 
		Text& SetOffset(Point offset);

		/// @brief Задаёт размеры шрифта (атрибут font-size)
		/// @param size 
		/// @return 
		Text& SetFontSize(uint32_t size);

		/// @brief Задаёт название шрифта (атрибут font-family)
		/// @param font_family 
		/// @return 
		Text& SetFontFamily(std::string font_family);

		/// @brief Задаёт толщину шрифта (атрибут font-weight)
		/// @param font_weight 
		/// @return 
		Text& SetFontWeight(std::string font_weight);

		/// @brief Задаёт текстовое содержимое объекта (отображается внутри тега text)
		/// @param data 
		/// @return 
		Text& SetData(std::string data);

		void RenderObject(const RenderContext& context) const override;
	};

	class Document : public ObjectContainer {

	public:
		Document() = default;

		/// @brief Добавляет в svg-документ объект-наследник svg::Object
		/// @param obj 
		void AddPtr(std::unique_ptr<Object>&& obj) override;

		/// @brief Выводит в ostream svg-представление документа
		/// @param out 
		void Render(std::ostream& out) const;
	};

}  // namespace svg
