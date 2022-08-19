#pragma once

#define CSL_PP_CONCAT_I(x, y) x ## y
#define CSL_PP_CONCAT(x, y) CSL_PP_CONCAT_I(x, y)

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

#define CSL_PP_STR_I(x) #x
#define CSL_PP_STR(arg) CSL_PP_STR_I(arg)

// Next macros generator
//////////////////////////////
//#include <iostream>
//#include <sstream>
//
//int main()
//{
//    const int n = 64;
//    std::stringstream s;
//    s << "#define CSL_PP_VARIADIC_SIZE_I(";
//    for (int i = 1; i <= n; ++i)
//        s << (i == 1 ? "" : ",") << "_" << i;
//    s << ",N,...) N\n";
//    s << "#define CSL_PP_VARIADIC_SIZE(...) CSL_PP_VARIADIC_SIZE_I(__VA_ARGS__,";
//    for (int i = n; i >= 1; --i)
//        s << i << ",";
//    s << ")\n\n";
//	  s << "#define #define CSL_PP_INC(x) CSL_PP_INC_ ## x\n";
//    for (int i = 0; i < n; ++i)
//        s << "#define CSL_PP_INC_" << i << " " << i + 1 << "\n";
//    s << "\n";
//	  s << "#define CSL_PP_BOOL(x) CSL_PP_BOOL_ ## x" << "\n";
//    for (int i = 0; i < n; ++i)
//        s << "#define CSL_PP_BOOL_" << i << " " << (i ? 1 : 0) << "\n";
//    s << "\n";
//    for (int i = 1; i < n; ++i)
//        s << "#define CSL_PP_ITERATE_" << i + 1 << "(m, d, i, e, ...) m(d, i, e) CSL_PP_ITERATE_" << i << "(m, d, CSL_PP_INC(i), __VA_ARGS__)" << "\n";
//    std::cout << s.str() << std::endl;
//    return 0;
//}

#define CSL_PP_VARIADIC_SIZE_I(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,N,...) N
#define CSL_PP_VARIADIC_SIZE(...) CSL_PP_VARIADIC_SIZE_I(__VA_ARGS__,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,)

#define CSL_PP_INC(x) CSL_PP_INC_ ## x
#define CSL_PP_INC_0 1
#define CSL_PP_INC_1 2
#define CSL_PP_INC_2 3
#define CSL_PP_INC_3 4
#define CSL_PP_INC_4 5
#define CSL_PP_INC_5 6
#define CSL_PP_INC_6 7
#define CSL_PP_INC_7 8
#define CSL_PP_INC_8 9
#define CSL_PP_INC_9 10
#define CSL_PP_INC_10 11
#define CSL_PP_INC_11 12
#define CSL_PP_INC_12 13
#define CSL_PP_INC_13 14
#define CSL_PP_INC_14 15
#define CSL_PP_INC_15 16
#define CSL_PP_INC_16 17
#define CSL_PP_INC_17 18
#define CSL_PP_INC_18 19
#define CSL_PP_INC_19 20
#define CSL_PP_INC_20 21
#define CSL_PP_INC_21 22
#define CSL_PP_INC_22 23
#define CSL_PP_INC_23 24
#define CSL_PP_INC_24 25
#define CSL_PP_INC_25 26
#define CSL_PP_INC_26 27
#define CSL_PP_INC_27 28
#define CSL_PP_INC_28 29
#define CSL_PP_INC_29 30
#define CSL_PP_INC_30 31
#define CSL_PP_INC_31 32
#define CSL_PP_INC_32 33
#define CSL_PP_INC_33 34
#define CSL_PP_INC_34 35
#define CSL_PP_INC_35 36
#define CSL_PP_INC_36 37
#define CSL_PP_INC_37 38
#define CSL_PP_INC_38 39
#define CSL_PP_INC_39 40
#define CSL_PP_INC_40 41
#define CSL_PP_INC_41 42
#define CSL_PP_INC_42 43
#define CSL_PP_INC_43 44
#define CSL_PP_INC_44 45
#define CSL_PP_INC_45 46
#define CSL_PP_INC_46 47
#define CSL_PP_INC_47 48
#define CSL_PP_INC_48 49
#define CSL_PP_INC_49 50
#define CSL_PP_INC_50 51
#define CSL_PP_INC_51 52
#define CSL_PP_INC_52 53
#define CSL_PP_INC_53 54
#define CSL_PP_INC_54 55
#define CSL_PP_INC_55 56
#define CSL_PP_INC_56 57
#define CSL_PP_INC_57 58
#define CSL_PP_INC_58 59
#define CSL_PP_INC_59 60
#define CSL_PP_INC_60 61
#define CSL_PP_INC_61 62
#define CSL_PP_INC_62 63
#define CSL_PP_INC_63 64

#define CSL_PP_BOOL(x) CSL_PP_BOOL_ ## x
#define CSL_PP_BOOL_0 0
#define CSL_PP_BOOL_1 1
#define CSL_PP_BOOL_2 1
#define CSL_PP_BOOL_3 1
#define CSL_PP_BOOL_4 1
#define CSL_PP_BOOL_5 1
#define CSL_PP_BOOL_6 1
#define CSL_PP_BOOL_7 1
#define CSL_PP_BOOL_8 1
#define CSL_PP_BOOL_9 1
#define CSL_PP_BOOL_10 1
#define CSL_PP_BOOL_11 1
#define CSL_PP_BOOL_12 1
#define CSL_PP_BOOL_13 1
#define CSL_PP_BOOL_14 1
#define CSL_PP_BOOL_15 1
#define CSL_PP_BOOL_16 1
#define CSL_PP_BOOL_17 1
#define CSL_PP_BOOL_18 1
#define CSL_PP_BOOL_19 1
#define CSL_PP_BOOL_20 1
#define CSL_PP_BOOL_21 1
#define CSL_PP_BOOL_22 1
#define CSL_PP_BOOL_23 1
#define CSL_PP_BOOL_24 1
#define CSL_PP_BOOL_25 1
#define CSL_PP_BOOL_26 1
#define CSL_PP_BOOL_27 1
#define CSL_PP_BOOL_28 1
#define CSL_PP_BOOL_29 1
#define CSL_PP_BOOL_30 1
#define CSL_PP_BOOL_31 1
#define CSL_PP_BOOL_32 1
#define CSL_PP_BOOL_33 1
#define CSL_PP_BOOL_34 1
#define CSL_PP_BOOL_35 1
#define CSL_PP_BOOL_36 1
#define CSL_PP_BOOL_37 1
#define CSL_PP_BOOL_38 1
#define CSL_PP_BOOL_39 1
#define CSL_PP_BOOL_40 1
#define CSL_PP_BOOL_41 1
#define CSL_PP_BOOL_42 1
#define CSL_PP_BOOL_43 1
#define CSL_PP_BOOL_44 1
#define CSL_PP_BOOL_45 1
#define CSL_PP_BOOL_46 1
#define CSL_PP_BOOL_47 1
#define CSL_PP_BOOL_48 1
#define CSL_PP_BOOL_49 1
#define CSL_PP_BOOL_50 1
#define CSL_PP_BOOL_51 1
#define CSL_PP_BOOL_52 1
#define CSL_PP_BOOL_53 1
#define CSL_PP_BOOL_54 1
#define CSL_PP_BOOL_55 1
#define CSL_PP_BOOL_56 1
#define CSL_PP_BOOL_57 1
#define CSL_PP_BOOL_58 1
#define CSL_PP_BOOL_59 1
#define CSL_PP_BOOL_60 1
#define CSL_PP_BOOL_61 1
#define CSL_PP_BOOL_62 1
#define CSL_PP_BOOL_63 1

#define CSL_PP_ITERATE_1(m, d, i, e, ...) m(d,i,e)
#define CSL_PP_ITERATE_2(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_1(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_3(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_2(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_4(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_3(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_5(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_4(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_6(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_5(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_7(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_6(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_8(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_7(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_9(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_8(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_10(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_9(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_11(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_10(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_12(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_11(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_13(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_12(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_14(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_13(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_15(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_14(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_16(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_15(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_17(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_16(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_18(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_17(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_19(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_18(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_20(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_19(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_21(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_20(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_22(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_21(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_23(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_22(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_24(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_23(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_25(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_24(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_26(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_25(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_27(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_26(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_28(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_27(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_29(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_28(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_30(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_29(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_31(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_30(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_32(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_31(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_33(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_32(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_34(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_33(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_35(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_34(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_36(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_35(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_37(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_36(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_38(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_37(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_39(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_38(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_40(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_39(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_41(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_40(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_42(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_41(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_43(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_42(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_44(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_43(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_45(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_44(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_46(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_45(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_47(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_46(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_48(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_47(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_49(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_48(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_50(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_49(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_51(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_50(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_52(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_51(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_53(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_52(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_54(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_53(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_55(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_54(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_56(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_55(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_57(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_56(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_58(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_57(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_59(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_58(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_60(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_59(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_61(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_60(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_62(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_61(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_63(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_62(m, d, CSL_PP_INC(i), __VA_ARGS__)
#define CSL_PP_ITERATE_64(m, d, i, e, ...) m(d,i,e) CSL_PP_ITERATE_63(m, d, CSL_PP_INC(i), __VA_ARGS__)

//

#define CSL_PP_IF_0(t, f) f
#define CSL_PP_IF_1(t, f) t
#define CSL_PP_IF_II(b, t, f) CSL_PP_IF_ ## b (t,f)
#define CSL_PP_IF_I(b, t, f) CSL_PP_IF_II(b, t, f)
#define CSL_PP_IF(c, t, f) CSL_PP_IF_I(CSL_PP_BOOL(c),t,f)

#define CSL_PP_COMMA_I() ,
#define CSL_PP_EMPTY()

#define CSL_PP_COMMA CSL_PP_COMMA_I()
#define CSL_PP_COMMA_IF(arg) CSL_PP_IF(arg, CSL_PP_COMMA_I, CSL_PP_EMPTY)()

#define CSL_PP_ITERATE_II(macro, data, count, ...) CSL_PP_ITERATE_ ## count(macro, data, 0, __VA_ARGS__)
#define CSL_PP_ITERATE_I(macro, data, count, ...) CSL_PP_ITERATE_II(macro, data, count, __VA_ARGS__)

#define CSL_PP_ITERATE_DATA(data, macro, ...) CSL_PP_ITERATE_I(macro, data, CSL_PP_VARIADIC_SIZE(__VA_ARGS__), __VA_ARGS__)
#define CSL_PP_ITERATE(macro, ...) CSL_PP_ITERATE_I(macro, , CSL_PP_VARIADIC_SIZE(__VA_ARGS__), __VA_ARGS__)
