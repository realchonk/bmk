# This is only available starting with autoconf-2.58
ifdef([AS_HELP_STRING], [], [
m4_define([AS_HELP_STRING],
[m4_pushdef([AS_Prefix], m4_default([$3], [                          ]))dnl
m4_pushdef([AS_Prefix_Format],
	   [  %-]m4_eval(m4_len(AS_Prefix) - 3)[s ])dnl [  %-23s ]
m4_text_wrap([$2], AS_Prefix, m4_format(AS_Prefix_Format, [$1]))dnl
m4_popdef([AS_Prefix_Format])dnl
m4_popdef([AS_Prefix])dnl
])])

