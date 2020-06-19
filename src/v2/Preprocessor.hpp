#pragma once

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>

#define CSL_PP2_CONCAT_INTERNAL(x, y) x ## y
#define CSL_PP2_CONCAT(x, y) CSL_PP2_CONCAT_INTERNAL(x, y)

// helpers for C++ reflexion from http://pfultz2.com/blog/2012/07/31/reflection-in-under-100-lines/
#define CSL_PP2_REM(...) __VA_ARGS__
#define CSL_PP2_EAT(...)
#define CSL_PP2_STRIP(x) CSL_PP2_EAT x		
#define CSL_PP2_PAIR(x) CSL_PP2_REM x

// 
#define TEST_FIRST(x,y) x
#define TEST_SECOND(x,y) y

// helpers for list args retrieval
#define CSL_PP2_FORWARD(x) x
#define CSL_PP2_REMOVE_PARENTHESIS(x) CSL_PP2_FORWARD( CSL_PP_REM x )

#define CSL_PP2_ADD_COMMA_INTERNAL(...) (__VA_ARGS__),
#define CSL_PP2_ADD_COMMA(x) CSL_PP2_ADD_COMMA_INTERNAL x

// helpers wrapping up BOOST_PP
#define CSL_PP2_STR(arg) BOOST_PP_STRINGIZE(arg)
#define CSL_PP2_IS_EMPTY(arg) BOOST_PP_IS_EMPTY(arg)
#define CSL_PP2_HEAD(elem) BOOST_PP_SEQ_HEAD(elem)
#define CSL_PP2_IF_EMPTY(arg,t,f) BOOST_PP_IIF(BOOST_PP_IS_EMPTY(arg),t,f)
#define CSL_PP2_NOT_EMPTY(arg) BOOST_PP_COMPL(BOOST_PP_IS_EMPTY(arg))
#define CSL_PP2_COMMA_IF(arg) BOOST_PP_COMMA_IF(arg)
#define CSL_PP2_COMMA_IF_NOT_EMPTY(arg) CSL_PP2_COMMA_IF(CSL_PP2_NOT_EMPTY(arg))

#define CSL_PP2_ITERATE(macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define CSL_PP2_ITERATE_DATA(data, macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

