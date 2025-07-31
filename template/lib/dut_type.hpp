#include <iostream>
#include <type_traits>
#include <tuple>
#pragma once

template <typename T>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> {
    using return_type = Ret;
    using args_tuple = std::tuple<Args...>;
};

template <typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Class, typename Ret, typename... Args>
struct function_traits<Ret(Class::*)(Args...)> : function_traits<Ret(Args...)> {};

template <typename Class, typename Ret, typename... Args>
struct function_traits<Ret(Class::*)(Args...) const> : function_traits<Ret(Args...)> {};

template <typename F>
struct function_traits : function_traits<decltype(&F::operator())> {};

template <typename F>
using return_type_t = typename function_traits<F>::return_type;

template <typename F>
using args_tuple_t = typename function_traits<F>::args_tuple;

#define DEFINE_PARAM_TYPE(ClassType, func, i, TypeName) \
    using TypeName = std::tuple_element_t< \
        i, \
        typename function_traits<decltype(&ClassType::func)>::args_tuple \
    >; \
    static_assert(i < std::tuple_size_v<typename function_traits<decltype(&ClassType::func)>::args_tuple>, \
                 "Index out of bounds for function parameters!");

#define DEFINE_RETURN_TYPE(ClassType, func, TypeName) \
    using TypeName = typename function_traits<decltype(&ClassType::func)>::return_type; \
    static_assert(!std::is_void_v<TypeName>, "Return type cannot be void (for demonstration purposes)");


// Generated code for GSIM:
{% if __SIMULATOR__ == "gsim" %}
{% for pin in __MODULE_EXTERNAL_PINS__ %}
{% if pin.type == "Out" %}
DEFINE_RETURN_TYPE(S{{__TOP_MODULE_NAME__}}, get_{{pin.name}}, type_{{pin.name}});
{% else %}
DEFINE_PARAM_TYPE(S{{__TOP_MODULE_NAME__}}, set_{{pin.name}}, 0, type_{{pin.name}});
{% endif %}
{% endfor %}
{% endif %}
