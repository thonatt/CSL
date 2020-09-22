#pragma once

//#include <boost/preprocessor/seq/for_each_i.hpp>
//#include <boost/preprocessor/variadic/to_seq.hpp>
//#include <boost/preprocessor/stringize.hpp>
//#include <boost/preprocessor/punctuation/comma_if.hpp>
//#include <boost/preprocessor/facilities/is_empty.hpp>

#define CSL_PP2_CONCAT_I(x, y) x ## y
#define CSL_PP2_CONCAT(x, y) CSL_PP2_CONCAT_I(x, y)

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
//#define CSL_PP_STR_I(x) #x
//#define CSL_PP2_STR(arg) BOOST_PP_STRINGIZE(arg)
//#define CSL_PP2_COMMA , //BOOST_PP_COMMA()
//#define CSL_PP2_COMMA_IF(arg) BOOST_PP_COMMA_IF(arg)
//
//#define CSL_PP2_ITERATE(macro, ...) \
//	BOOST_PP_SEQ_FOR_EACH_I(macro, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
//
//#define CSL_PP2_ITERATE_DATA(data, macro, ...) \
//	BOOST_PP_SEQ_FOR_EACH_I(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define CSL_PP_STR_I(x) #x
#define CSL_PP2_STR(arg) CSL_PP_STR_I(arg)

#define IF_0(t, f) f
#define IF_1(t, f) t
#define IF_II(b, t, f) IF_ ## b (t,f)
#define IF_I(b, t, f) IF_II(b, t, f)
#define IF(c, t, f) IF_I(BOOL(c),t,f)

#define COMMA() ,
#define EMPTY()
#define COMMA_IF(c) IF(c, COMMA, EMPTY)()

#define CSL_PP2_COMMA COMMA()
#define CSL_PP2_COMMA_IF(arg) COMMA_IF(arg)

#define ITERATE_II(macro, data, count, ...) ITERATE_ ## count(macro, data, 0, __VA_ARGS__)
#define ITERATE_I(macro, data, count, ...) ITERATE_II(macro, data, count, __VA_ARGS__)
#define CSL_PP2_ITERATE_DATA(data, macro, ...) ITERATE_I(macro, data, VARIADIC_SIZE(__VA_ARGS__), __VA_ARGS__)
#define CSL_PP2_ITERATE(macro, ...) ITERATE_I(macro, , VARIADIC_SIZE(__VA_ARGS__), __VA_ARGS__)
// Macro generator
//////////////////////////////
//#include <iostream>
//#include <sstream>
//
//int main()
//{
//    const int n = 64;
//    std::stringstream s;
//    s << "#define VARIADIC_SIZE_I(";
//    for (int i = 1; i <= n; ++i) {
//        s << (i == 1 ? "" : ",") << "_" << i;
//    }
//    s << ",N,...) N\n";
//    s << "#define VARIADIC_SIZE(...) VARIADIC_SIZE_I(__VA_ARGS__,";
//    for (int i = n; i >= 1; --i) {
//        s << i << ",";
//    }
//    s << ")\n\n";
//    for (int i = 1; i < n; ++i) {
//        s << "#define ITERATE_" << i + 1 << "(m, d, i, e, ...) m(d, i, e) ITERATE_" << i << "(m, d, INC(i), __VA_ARGS__)" << "\n";
//    }
//    s << "\n";
//    for (int i = 0; i < n; ++i) {
//        s << "#define INC_" << i << " " << i + 1 << "\n";
//    }
//    s << "\n";
//    for (int i = 0; i < n; ++i) {
//        s << "#define BOOL_" << i << " " << (i ? 1 : 0) << "\n";
//    }
//    std::cout << s.str() << std::endl;
//    return 0;
//}

#define VARIADIC_SIZE_I(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,N,...) N
#define VARIADIC_SIZE(...) VARIADIC_SIZE_I(__VA_ARGS__,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,)

#define ITERATE_1(m, d, i, e, ...) m(d,i,e)
#define ITERATE_2(m, d, i, e, ...) m(d,i,e) ITERATE_1(m, d, INC(i), __VA_ARGS__)
#define ITERATE_3(m, d, i, e, ...) m(d,i,e) ITERATE_2(m, d, INC(i), __VA_ARGS__)
#define ITERATE_4(m, d, i, e, ...) m(d,i,e) ITERATE_3(m, d, INC(i), __VA_ARGS__)
#define ITERATE_5(m, d, i, e, ...) m(d,i,e) ITERATE_4(m, d, INC(i), __VA_ARGS__)
#define ITERATE_6(m, d, i, e, ...) m(d,i,e) ITERATE_5(m, d, INC(i), __VA_ARGS__)
#define ITERATE_7(m, d, i, e, ...) m(d,i,e) ITERATE_6(m, d, INC(i), __VA_ARGS__)
#define ITERATE_8(m, d, i, e, ...) m(d,i,e) ITERATE_7(m, d, INC(i), __VA_ARGS__)
#define ITERATE_9(m, d, i, e, ...) m(d,i,e) ITERATE_8(m, d, INC(i), __VA_ARGS__)
#define ITERATE_10(m, d, i, e, ...) m(d,i,e) ITERATE_9(m, d, INC(i), __VA_ARGS__)
#define ITERATE_11(m, d, i, e, ...) m(d,i,e) ITERATE_10(m, d, INC(i), __VA_ARGS__)
#define ITERATE_12(m, d, i, e, ...) m(d,i,e) ITERATE_11(m, d, INC(i), __VA_ARGS__)
#define ITERATE_13(m, d, i, e, ...) m(d,i,e) ITERATE_12(m, d, INC(i), __VA_ARGS__)
#define ITERATE_14(m, d, i, e, ...) m(d,i,e) ITERATE_13(m, d, INC(i), __VA_ARGS__)
#define ITERATE_15(m, d, i, e, ...) m(d,i,e) ITERATE_14(m, d, INC(i), __VA_ARGS__)
#define ITERATE_16(m, d, i, e, ...) m(d,i,e) ITERATE_15(m, d, INC(i), __VA_ARGS__)
#define ITERATE_17(m, d, i, e, ...) m(d,i,e) ITERATE_16(m, d, INC(i), __VA_ARGS__)
#define ITERATE_18(m, d, i, e, ...) m(d,i,e) ITERATE_17(m, d, INC(i), __VA_ARGS__)
#define ITERATE_19(m, d, i, e, ...) m(d,i,e) ITERATE_18(m, d, INC(i), __VA_ARGS__)
#define ITERATE_20(m, d, i, e, ...) m(d,i,e) ITERATE_19(m, d, INC(i), __VA_ARGS__)
#define ITERATE_21(m, d, i, e, ...) m(d,i,e) ITERATE_20(m, d, INC(i), __VA_ARGS__)
#define ITERATE_22(m, d, i, e, ...) m(d,i,e) ITERATE_21(m, d, INC(i), __VA_ARGS__)
#define ITERATE_23(m, d, i, e, ...) m(d,i,e) ITERATE_22(m, d, INC(i), __VA_ARGS__)
#define ITERATE_24(m, d, i, e, ...) m(d,i,e) ITERATE_23(m, d, INC(i), __VA_ARGS__)
#define ITERATE_25(m, d, i, e, ...) m(d,i,e) ITERATE_24(m, d, INC(i), __VA_ARGS__)
#define ITERATE_26(m, d, i, e, ...) m(d,i,e) ITERATE_25(m, d, INC(i), __VA_ARGS__)
#define ITERATE_27(m, d, i, e, ...) m(d,i,e) ITERATE_26(m, d, INC(i), __VA_ARGS__)
#define ITERATE_28(m, d, i, e, ...) m(d,i,e) ITERATE_27(m, d, INC(i), __VA_ARGS__)
#define ITERATE_29(m, d, i, e, ...) m(d,i,e) ITERATE_28(m, d, INC(i), __VA_ARGS__)
#define ITERATE_30(m, d, i, e, ...) m(d,i,e) ITERATE_29(m, d, INC(i), __VA_ARGS__)
#define ITERATE_31(m, d, i, e, ...) m(d,i,e) ITERATE_30(m, d, INC(i), __VA_ARGS__)
#define ITERATE_32(m, d, i, e, ...) m(d,i,e) ITERATE_31(m, d, INC(i), __VA_ARGS__)
#define ITERATE_33(m, d, i, e, ...) m(d,i,e) ITERATE_32(m, d, INC(i), __VA_ARGS__)
#define ITERATE_34(m, d, i, e, ...) m(d,i,e) ITERATE_33(m, d, INC(i), __VA_ARGS__)
#define ITERATE_35(m, d, i, e, ...) m(d,i,e) ITERATE_34(m, d, INC(i), __VA_ARGS__)
#define ITERATE_36(m, d, i, e, ...) m(d,i,e) ITERATE_35(m, d, INC(i), __VA_ARGS__)
#define ITERATE_37(m, d, i, e, ...) m(d,i,e) ITERATE_36(m, d, INC(i), __VA_ARGS__)
#define ITERATE_38(m, d, i, e, ...) m(d,i,e) ITERATE_37(m, d, INC(i), __VA_ARGS__)
#define ITERATE_39(m, d, i, e, ...) m(d,i,e) ITERATE_38(m, d, INC(i), __VA_ARGS__)
#define ITERATE_40(m, d, i, e, ...) m(d,i,e) ITERATE_39(m, d, INC(i), __VA_ARGS__)
#define ITERATE_41(m, d, i, e, ...) m(d,i,e) ITERATE_40(m, d, INC(i), __VA_ARGS__)
#define ITERATE_42(m, d, i, e, ...) m(d,i,e) ITERATE_41(m, d, INC(i), __VA_ARGS__)
#define ITERATE_43(m, d, i, e, ...) m(d,i,e) ITERATE_42(m, d, INC(i), __VA_ARGS__)
#define ITERATE_44(m, d, i, e, ...) m(d,i,e) ITERATE_43(m, d, INC(i), __VA_ARGS__)
#define ITERATE_45(m, d, i, e, ...) m(d,i,e) ITERATE_44(m, d, INC(i), __VA_ARGS__)
#define ITERATE_46(m, d, i, e, ...) m(d,i,e) ITERATE_45(m, d, INC(i), __VA_ARGS__)
#define ITERATE_47(m, d, i, e, ...) m(d,i,e) ITERATE_46(m, d, INC(i), __VA_ARGS__)
#define ITERATE_48(m, d, i, e, ...) m(d,i,e) ITERATE_47(m, d, INC(i), __VA_ARGS__)
#define ITERATE_49(m, d, i, e, ...) m(d,i,e) ITERATE_48(m, d, INC(i), __VA_ARGS__)
#define ITERATE_50(m, d, i, e, ...) m(d,i,e) ITERATE_49(m, d, INC(i), __VA_ARGS__)
#define ITERATE_51(m, d, i, e, ...) m(d,i,e) ITERATE_50(m, d, INC(i), __VA_ARGS__)
#define ITERATE_52(m, d, i, e, ...) m(d,i,e) ITERATE_51(m, d, INC(i), __VA_ARGS__)
#define ITERATE_53(m, d, i, e, ...) m(d,i,e) ITERATE_52(m, d, INC(i), __VA_ARGS__)
#define ITERATE_54(m, d, i, e, ...) m(d,i,e) ITERATE_53(m, d, INC(i), __VA_ARGS__)
#define ITERATE_55(m, d, i, e, ...) m(d,i,e) ITERATE_54(m, d, INC(i), __VA_ARGS__)
#define ITERATE_56(m, d, i, e, ...) m(d,i,e) ITERATE_55(m, d, INC(i), __VA_ARGS__)
#define ITERATE_57(m, d, i, e, ...) m(d,i,e) ITERATE_56(m, d, INC(i), __VA_ARGS__)
#define ITERATE_58(m, d, i, e, ...) m(d,i,e) ITERATE_57(m, d, INC(i), __VA_ARGS__)
#define ITERATE_59(m, d, i, e, ...) m(d,i,e) ITERATE_58(m, d, INC(i), __VA_ARGS__)
#define ITERATE_60(m, d, i, e, ...) m(d,i,e) ITERATE_59(m, d, INC(i), __VA_ARGS__)
#define ITERATE_61(m, d, i, e, ...) m(d,i,e) ITERATE_60(m, d, INC(i), __VA_ARGS__)
#define ITERATE_62(m, d, i, e, ...) m(d,i,e) ITERATE_61(m, d, INC(i), __VA_ARGS__)
#define ITERATE_63(m, d, i, e, ...) m(d,i,e) ITERATE_62(m, d, INC(i), __VA_ARGS__)
#define ITERATE_64(m, d, i, e, ...) m(d,i,e) ITERATE_63(m, d, INC(i), __VA_ARGS__)

#define INC(x) INC_ ## x
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 10
#define INC_10 11
#define INC_11 12
#define INC_12 13
#define INC_13 14
#define INC_14 15
#define INC_15 16
#define INC_16 17
#define INC_17 18
#define INC_18 19
#define INC_19 20
#define INC_20 21
#define INC_21 22
#define INC_22 23
#define INC_23 24
#define INC_24 25
#define INC_25 26
#define INC_26 27
#define INC_27 28
#define INC_28 29
#define INC_29 30
#define INC_30 31
#define INC_31 32
#define INC_32 33
#define INC_33 34
#define INC_34 35
#define INC_35 36
#define INC_36 37
#define INC_37 38
#define INC_38 39
#define INC_39 40
#define INC_40 41
#define INC_41 42
#define INC_42 43
#define INC_43 44
#define INC_44 45
#define INC_45 46
#define INC_46 47
#define INC_47 48
#define INC_48 49
#define INC_49 50
#define INC_50 51
#define INC_51 52
#define INC_52 53
#define INC_53 54
#define INC_54 55
#define INC_55 56
#define INC_56 57
#define INC_57 58
#define INC_58 59
#define INC_59 60
#define INC_60 61
#define INC_61 62
#define INC_62 63
#define INC_63 64

#define BOOL(x) BOOL_ ## x
#define BOOL_0 0
#define BOOL_1 1
#define BOOL_2 1
#define BOOL_3 1
#define BOOL_4 1
#define BOOL_5 1
#define BOOL_6 1
#define BOOL_7 1
#define BOOL_8 1
#define BOOL_9 1
#define BOOL_10 1
#define BOOL_11 1
#define BOOL_12 1
#define BOOL_13 1
#define BOOL_14 1
#define BOOL_15 1
#define BOOL_16 1
#define BOOL_17 1
#define BOOL_18 1
#define BOOL_19 1
#define BOOL_20 1
#define BOOL_21 1
#define BOOL_22 1
#define BOOL_23 1
#define BOOL_24 1
#define BOOL_25 1
#define BOOL_26 1
#define BOOL_27 1
#define BOOL_28 1
#define BOOL_29 1
#define BOOL_30 1
#define BOOL_31 1
#define BOOL_32 1
#define BOOL_33 1
#define BOOL_34 1
#define BOOL_35 1
#define BOOL_36 1
#define BOOL_37 1
#define BOOL_38 1
#define BOOL_39 1
#define BOOL_40 1
#define BOOL_41 1
#define BOOL_42 1
#define BOOL_43 1
#define BOOL_44 1
#define BOOL_45 1
#define BOOL_46 1
#define BOOL_47 1
#define BOOL_48 1
#define BOOL_49 1
#define BOOL_50 1
#define BOOL_51 1
#define BOOL_52 1
#define BOOL_53 1
#define BOOL_54 1
#define BOOL_55 1
#define BOOL_56 1
#define BOOL_57 1
#define BOOL_58 1
#define BOOL_59 1
#define BOOL_60 1
#define BOOL_61 1
#define BOOL_62 1
#define BOOL_63 1
