#define _USE_MATH_DEFINES
#include "geo.h"

namespace geo {
	bool Coordinates::operator==(const Coordinates& other) const {
		return std::abs(lat - other.lat) < EPSILON && std::abs(lng - other.lng) < EPSILON;
	}

	bool Coordinates::operator!=(const Coordinates& other) const {
		return !(*this == other);
	}

	double ComputeDistance(Coordinates from, Coordinates to) {
		using namespace std;
		if (from == to) {
			return 0.0;
		}
		static const double dr = 3.1415926535 / 180.;
		return abs(acos(sin(from.lat * dr) * sin(to.lat * dr)
			+ cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
			* R_Z);
	}

	bool IsZero(double value) {
		return std::abs(value) < EPSILON;
	}

	svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}
} // namespace geo

std::ostream& operator<<(std::ostream& out, const geo::Coordinates& coords) {
	using namespace std::literals;
	out << "lat = " << coords.lat << " "s << "lng = " << coords.lng;
	return out;
}
