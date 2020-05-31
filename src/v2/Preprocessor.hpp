#pragma once

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>

#define PP_CSL_CONCAT_INTERNAL(x, y) x ## y
#define PP_CSL_CONCAT(x, y) PP_CSL_CONCAT_INTERNAL(x, y)

//helpers for C++ reflexion from http://pfultz2.com/blog/2012/07/31/reflection-in-under-100-lines/
#define PP_CSL_REM(...) __VA_ARGS__
#define PP_CSL_EAT(...)
#define PP_CSL_STRIP(x) CSL_PP_EAT x		
#define PP_CSL_PAIR(x) CSL_PP_REM x

//helpers for list args retrieval
#define PP_CSL_FORWARD(x) x
#define PP_CSL_REMOVE_PARENTHESIS(x) PP_CSL_FORWARD( CSL_PP_REM x )

#define PP_CSL_ADD_COMMA_INTERNAL(...) (__VA_ARGS__),
#define PP_CSL_ADD_COMMA(x) PP_CSL_ADD_COMMA_INTERNAL x
#define PP_CSL_FIRST(x,...) x
#define PP_CSL_LIST_FIRST(...) PP_CSL_FORWARD( PP_CSL_FIRST(__VA_ARGS__) )

#define PP_CSL_GET_TYPE_PARENTHESIS(...) PP_CSL_LIST_FIRST( CSL_PP_ADD_COMMA(__VA_ARGS__) )
#define PP_CSL_GET_TYPE_LIST(x) PP_CSL_REMOVE_PARENTHESIS(PP_CSL_GET_TYPE_PARENTHESIS(x))

//helpers wrapping up BOOST_PP
#define PP_CSL_STR(arg) BOOST_PP_STRINGIZE(arg)
#define PP_CSL_IS_EMPTY(arg) BOOST_PP_IS_EMPTY(arg)
#define PP_CSL_HEAD(elem) BOOST_PP_SEQ_HEAD(elem)
#define PP_CSL_IF_EMPTY(arg,t,f) BOOST_PP_IIF(BOOST_PP_IS_EMPTY(arg),t,f)
#define PP_CSL_NOT_EMPTY(arg) BOOST_PP_COMPL(BOOST_PP_IS_EMPTY(arg))
#define PP_CSL_COMMA_IF_NOT_EMPTY(arg) BOOST_PP_COMMA_IF(PP_CSL_NOT_EMPTY(arg))

#define PP_CSL_ITERATE(macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define PP_CSL_ITERATE_DATA(data, macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

