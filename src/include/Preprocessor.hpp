#pragma once

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>

#define CSL_PP2_CONCAT_INTERNAL(x, y) x ## y
#define CSL_PP2_CONCAT(x, y) CSL_PP2_CONCAT_INTERNAL(x, y)

// helper for typed lists parsing
#define CSL_PP_VANISH
#define CSL_PP_VANCSL_PP_ISH
#define CSL_PP_ESCAPE_INTERNAL(...) CSL_PP_VAN ## __VA_ARGS__
#define CSL_PP_ESCAPE(...) CSL_PP_ESCAPE_INTERNAL(__VA_ARGS__)
#define CSL_PP_ISH(...) ISH __VA_ARGS__
#define CSL_PP_DEPARENTHESIS(...) CSL_PP_ESCAPE(CSL_PP_ISH __VA_ARGS__)

#define CSL_PP_FIRST_I(x,y) x
#define CSL_PP_SECOND_I(x,y) y
#define CSL_PP_FIRST(x) CSL_PP_FIRST_I x
#define CSL_PP_SECOND(x) CSL_PP_SECOND_I x

// helpers wrapping up BOOST_PP
#define CSL_PP2_STR(arg) BOOST_PP_STRINGIZE(arg)
#define CSL_PP2_IS_EMPTY(arg) BOOST_PP_IS_EMPTY(arg)
#define CSL_PP2_HEAD(elem) BOOST_PP_SEQ_HEAD(elem)
#define CSL_PP2_IF(arg, t, f) BOOST_PP_IIF(arg, t, f)
#define CSL_PP2_IF_EMPTY(arg,t,f) BOOST_PP_IIF(BOOST_PP_IS_EMPTY(arg),t,f)
#define CSL_PP2_NOT_EMPTY(arg) BOOST_PP_COMPL(BOOST_PP_IS_EMPTY(arg))
#define CSL_PP2_COMMA BOOST_PP_COMMA()
#define CSL_PP2_COMMA_IF(arg) BOOST_PP_COMMA_IF(arg)
#define CSL_PP2_COMMA_IF_NOT_EMPTY(arg) CSL_PP2_COMMA_IF(CSL_PP2_NOT_EMPTY(arg))
#define CSL_PP2_COUNT(...) BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)

#define CSL_PP2_ITERATE(macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, ,BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define CSL_PP2_ITERATE_DATA(data, macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, data,BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

