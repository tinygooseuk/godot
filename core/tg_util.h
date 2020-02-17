#pragma once
#include <core/class_db.h>

template <typename T>
constexpr T min(T x, T y) {
	return (x < y) ? x : y;
}
template <typename T>
constexpr T max(T x, T y) {
	return (x > y) ? x : y;
}

template <typename T>
constexpr T clamp(T value, T min_v, T max_v) {
	return max(min_v, min(max_v, value));
}

#define DECLARE_PROPERTY(cpp_type, name, default_value)    		\
	private:                                                    \
		cpp_type name = default_value;                          \
																\
	public:                                                     \
		void set_##name(cpp_type p_##name) { name = p_##name; } \
		cpp_type get_##name() const { return name; }


#define DECLARE_PUBLIC_PROPERTY(cpp_type, name, default_value) 	\
	public:                                                    	\
		cpp_type name = default_value;                          \
																\
		void set_##name(cpp_type p_##name) { name = p_##name; } \
		cpp_type get_##name() const { return name; }

#define IMPLEMENT_PROPERTY(cpp_class, gd_type, name)            \
	ClassDB::bind_method("get_" #name, &cpp_class::get_##name); \
	ClassDB::bind_method("set_" #name, &cpp_class::set_##name); \
	ClassDB::add_property(#cpp_class, PropertyInfo(Variant::gd_type, #name), "set_" #name, "get_" #name);

#define IMPLEMENT_PROPERTY_RESOURCE(cpp_class, type_hint, name) \
	ClassDB::bind_method("get_" #name, &cpp_class::get_##name);          \
	ClassDB::bind_method("set_" #name, &cpp_class::set_##name);          \
	ClassDB::add_property(#cpp_class, PropertyInfo(Variant::OBJECT, #name, PROPERTY_HINT_RESOURCE_TYPE, #type_hint), "set_" #name, "get_" #name);
