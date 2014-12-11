dnl libsecp25k1 helper checks
AC_DEFUN([SECP_INT128_CHECK],[
has_int128=$ac_cv_type___int128
if test x"$has_int128" != x"yes" && test x"$set_field" = x"64bit"; then
  AC_MSG_ERROR([$set_field field support explicitly requested but is not compatible with this host])
fi
if test x"$has_int128" != x"yes" && test x"$set_scalar" = x"64bit"; then
  AC_MSG_ERROR([$set_scalar scalar support explicitly requested but is not compatible with this host])
fi
])

dnl 
AC_DEFUN([SECP_64BIT_ASM_CHECK],[
AC_MSG_CHECKING(for x86_64 assembly availability)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
  #include <stdint.h>]],[[
  uint64_t a = 11, tmp;
  __asm__ __volatile__("movq $0x100000000,%1; mulq %%rsi" : "+a"(a) : "S"(tmp) : "cc", "%rdx");
  ]])],[has_64bit_asm=yes],[has_64bit_asm=no])
AC_MSG_RESULT([$has_64bit_asm])
if test x"$set_field" == x"64bit_asm"; then
  if test x"$has_64bit_asm" == x"no"; then
    AC_MSG_ERROR([$set_field field support explicitly requested but no x86_64 assembly available])
  fi
fi
])

dnl
AC_DEFUN([SECP_OPENSSL_CHECK],[
if test x"$use_pkgconfig" = x"yes"; then
    : #NOP
  m4_ifdef([PKG_CHECK_MODULES],[
    PKG_CHECK_MODULES([CRYPTO], [libcrypto], [has_libcrypto=yes],[has_libcrypto=no])
    if test x"$has_libcrypto" = x"yes"; then
      TEMP_LIBS="$LIBS"
      LIBS="$LIBS $CRYPTO_LIBS"
      AC_CHECK_LIB(crypto, main,[AC_DEFINE(HAVE_LIBCRYPTO,1,[Define this symbol if libcrypto is installed])],[has_libcrypto=no])
      LIBS="$TEMP_LIBS"
    fi
  ])
else
  AC_CHECK_HEADER(openssl/crypto.h,[AC_CHECK_LIB(crypto, main,[has_libcrypto=yes; CRYPTO_LIBS=-lcrypto; AC_DEFINE(HAVE_LIBCRYPTO,1,[Define this symbol if libcrypto is installed])]
)])
  LIBS=
fi
if test x"$has_libcrypto" == x"yes" && test x"$has_openssl_ec" = x; then
  AC_MSG_CHECKING(for EC functions in libcrypto)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
    #include <openssl/ec.h>
    #include <openssl/ecdsa.h>
    #include <openssl/obj_mac.h>]],[[
    EC_KEY *eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    ECDSA_sign(0, NULL, 0, NULL, NULL, eckey);
    ECDSA_verify(0, NULL, 0, NULL, 0, eckey);
    EC_KEY_free(eckey);
  ]])],[has_openssl_ec=yes],[has_openssl_ec=no])
  AC_MSG_RESULT([$has_openssl_ec])
fi
])

dnl
AC_DEFUN([SECP_GMP_CHECK],[
if test x"$has_gmp" != x"yes"; then
  CPPFLAGS_TEMP="$CPPFLAGS"
  CPPFLAGS="$GMP_CPPFLAGS $CPPFLAGS"
  LIBS_TEMP="$LIBS"
  LIBS="$GMP_LIBS $LIBS"
  AC_CHECK_HEADER(gmp.h,[AC_CHECK_LIB(gmp, __gmpz_init,[has_gmp=yes; GMP_LIBS="$GMP_LIBS -lgmp"; AC_DEFINE(HAVE_LIBGMP,1,[Define this symbol if libgmp is installed])])])
  CPPFLAGS="$CPPFLAGS_TEMP"
  LIBS="$LIBS_TEMP"
fi
if test x"$set_field" = x"gmp" && test x"$has_gmp" != x"yes"; then
    AC_MSG_ERROR([$set_field field support explicitly requested but libgmp was not found])
fi
if test x"$set_bignum" = x"gmp" && test x"$has_gmp" != x"yes"; then
    AC_MSG_ERROR([$set_bignum field support explicitly requested but libgmp was not found])
fi
])

