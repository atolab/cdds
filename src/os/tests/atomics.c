#include "os/os.h"
#include "CUnit/Runner.h"

#define ENABLE_TRACING  0

uint32_t _osuint32 = 0;
uint64_t _osuint64 = 0;
// os_address is uintptr_t
uintptr_t _osaddress = 0;
ptrdiff_t _ptrdiff = 0;
void * _osvoidp = (uintptr_t *)0;

CUnit_Test(atomics, LD_ST)
{
   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(5);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(5);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(5);
   volatile os_atomic_voidp_t voidp = OS_ATOMIC_VOIDP_INIT((uintptr_t)5);

#if ENABLE_TRACING
   /* Test uint32 LD-ST */
   printf ("Starting tc_os_atomics_LD_ST_001\n");
#endif
   CU_ASSERT (os_atomic_ld32 (&uint32) == 5); /* Returns contents of uint32 */
   os_atomic_st32 (&uint32, _osuint32); /* Writes os_uint32 into uint32 */
   CU_ASSERT (os_atomic_ld32 (&uint32) == _osuint32);

#if ENABLE_TRACING
   /* Test uint64 LD-ST */
   printf ("Starting tc_os_atomics_LD_ST_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   CU_ASSERT (os_atomic_ld64 (&uint64) == 5);
   os_atomic_st64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == _osuint64);
#endif

#if ENABLE_TRACING
   /* Test uintptr LD-ST */
   printf ("Starting tc_os_atomics_LD_ST_003\n");
#endif
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 5);
   os_atomic_stptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == _osaddress);

#if ENABLE_TRACING
   /* Test uintvoidp LD-ST */
   printf ("Starting tc_os_atomics_LD_ST_004\n");
#endif
   CU_ASSERT (os_atomic_ldvoidp (&voidp) == (uintptr_t*)5);
   os_atomic_stvoidp (&voidp, _osvoidp);
   CU_ASSERT (os_atomic_ldvoidp (&voidp) == (uintptr_t*)_osvoidp);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_LD_ST\n");
#endif
}

CUnit_Test(atomics, CAS)
{
   /* Compare and Swap
    * if (ptr == expected) { ptr = newval; }
    */
   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(0);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(0);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(0);
   volatile os_atomic_voidp_t uintvoidp = OS_ATOMIC_VOIDP_INIT((uintptr_t)0);
   _osuint32 = 1;
   _osuint64 = 1;
   _osaddress = 1;
   _osvoidp = (uintptr_t *)1;
   uint32_t expected = 0, newval = 5;
   uintptr_t addr_expected = 0, addr_newval = 5;
   void *void_expected = (uintptr_t*)0;
   void *void_newval = (uintptr_t*)5;
   int ret = 0;

#if ENABLE_TRACING
   /* Test os_atomic_cas32 */
   printf ("Starting tc_os_atomics_CAS_001\n");
#endif
   ret = os_atomic_cas32 (&uint32, expected, newval);
   CU_ASSERT (os_atomic_ld32 (&uint32) == newval && ret == 1);
   os_atomic_st32 (&uint32, _osuint32);
   ret = os_atomic_cas32 (&uint32, expected, newval);
   CU_ASSERT (os_atomic_ld32 (&uint32) != newval && ret == 0);

#if ENABLE_TRACING
   /* Test os_atomic_cas64 */
   printf ("Starting tc_os_atomics_CAS_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   ret = os_atomic_cas64 (&uint64, expected, newval);
   CU_ASSERT (os_atomic_ld64 (&uint64) == newval && ret == 1);
   os_atomic_st64 (&uint64, _osuint64);
   ret = os_atomic_cas64 (&uint64, expected, newval);
   CU_ASSERT (os_atomic_ld64 (&uint64) != newval && ret == 0);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_casptr */
   printf ("Starting tc_os_atomics_CAS_003\n");
#endif
   ret = os_atomic_casptr (&uintptr, addr_expected, addr_newval);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == addr_newval && ret == 1);
   os_atomic_stptr (&uintptr, _osaddress);
   ret = os_atomic_casptr (&uintptr, addr_expected, addr_newval);
   CU_ASSERT (os_atomic_ldptr (&uintptr) != addr_newval && ret == 0);

#if ENABLE_TRACING
   /* Test os_atomic_casvoidp */
   printf ("Starting tc_os_atomics_CAS_003\n");
#endif
   ret = os_atomic_casvoidp (&uintvoidp, void_expected, void_newval);
   CU_ASSERT (os_atomic_ldvoidp (&uintvoidp) == (uintptr_t*)void_newval && ret == 1);
   os_atomic_stvoidp (&uintvoidp, _osvoidp);
   ret = os_atomic_casvoidp (&uintvoidp, void_expected, void_newval);
   CU_ASSERT (os_atomic_ldvoidp (&uintvoidp) == (uintptr_t*)1 && ret == 0);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_CAS\n");
#endif
}

CUnit_Test(atomics, INC)
{
   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(0);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(0);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(0);
   _osuint32 = 0;
   _osuint64 = 0;
   _osaddress = 0;
   _osvoidp = (uintptr_t *)0;

#if ENABLE_TRACING
   /* Test os_inc32 */
   printf ("Starting tc_os_atomics_INC_001\n");
#endif
   os_atomic_inc32 (&uint32);
   CU_ASSERT (os_atomic_ld32 (&uint32) == 1);

#if ENABLE_TRACING
   /* Test os_inc64 */
   printf ("Starting tc_os_atomics_INC_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_inc64 (&uint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == 1);
#endif

#if ENABLE_TRACING
   /* Test os_incptr */
   printf ("Starting tc_os_atomics_INC_003\n");
#endif
   os_atomic_incptr (&uintptr);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 1);

#if ENABLE_TRACING
   /* Test os_atomic_inc32_nv */
   printf ("Starting tc_os_atomics_INC_004\n");
#endif
   os_atomic_st32 (&uint32, _osuint32);
   CU_ASSERT (os_atomic_inc32_nv (&uint32) == 1);

#if ENABLE_TRACING
   /* Test os_atomic_inc64_nv */
   printf ("Starting tc_os_atomics_INC_005\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_st64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_inc64_nv (&uint64) == 1);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_incptr_nv */
   printf ("Starting tc_os_atomics_INC_006\n");
#endif
   os_atomic_stptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_incptr_nv(&uintptr) == 1);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_INC\n");
#endif
}

CUnit_Test(atomics, DEC)
{
   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(1);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(1);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(1);
   _osuint32 = 1;
   _osuint64 = 1;
   _osaddress = 1;
   _osvoidp = (uintptr_t *)1;

#if ENABLE_TRACING
   /* Test os_atomic_dec32 */
   printf ("Starting tc_os_atomics_DEC_001\n");
#endif
   os_atomic_dec32 (&uint32);
   CU_ASSERT (os_atomic_ld32 (&uint32) == 0);

#if ENABLE_TRACING
   /* Test os_atomic_dec64 */
   printf ("Starting tc_os_atomics_DEC_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_dec64 (&uint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == 0);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_decptr */
   printf ("Starting tc_os_atomics_DEC_003\n");
#endif
   os_atomic_decptr (&uintptr);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 0);

#if ENABLE_TRACING
   /* Test os_atomic_dec32_nv */
   printf ("Starting tc_os_atomics_DEC_004\n");
#endif
   os_atomic_st32 (&uint32, _osuint32);
   CU_ASSERT (os_atomic_dec32_nv (&uint32) == 0);

#if ENABLE_TRACING
   /* Test os_atomic_dec64_nv */
   printf ("Starting tc_os_atomics_DEC_005\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_st64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_dec64_nv (&uint64) == 0);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_decptr_nv */
   printf ("Starting tc_os_atomics_DEC_006\n");
#endif
   os_atomic_stptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_decptr_nv(&uintptr) == 0);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_DEC\n");
#endif
}

CUnit_Test(atomics, ADD)
{
   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(1);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(1);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(1);
   volatile os_atomic_voidp_t uintvoidp = OS_ATOMIC_VOIDP_INIT((uintptr_t)1);
   _osuint32 = 2;
   _osuint64 = 2;
   _osaddress = 2;
   _ptrdiff = 2;

#if ENABLE_TRACING
   /* Test os_atomic_add32 */
   printf ("Starting tc_os_atomics_ADD_001\n");
#endif
   os_atomic_add32 (&uint32, _osuint32);
   CU_ASSERT (os_atomic_ld32 (&uint32) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_add64 */
   printf ("Starting tc_os_atomics_ADD_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_add64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == 3);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_addptr */
   printf ("Starting tc_os_atomics_ADD_003\n");
#endif
   os_atomic_addptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_addvoidp */
   printf ("Starting tc_os_atomics_ADD_004\n");
#endif
   os_atomic_addvoidp (&uintvoidp, _ptrdiff);
   CU_ASSERT (os_atomic_ldvoidp (&uintvoidp) == (uintptr_t*)3);

#if ENABLE_TRACING
   /* Test os_atomic_add32_nv */
   printf ("Starting tc_os_atomics_ADD_005\n");
#endif
   os_atomic_st32 (&uint32, 1);
   CU_ASSERT (os_atomic_add32_nv (&uint32, _osuint32) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_add64_nv */
   printf ("Starting tc_os_atomics_ADD_006\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_st64 (&uint64, 1);
   CU_ASSERT (os_atomic_add64_nv (&uint64, _osuint64) == 3);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_addptr_nv */
   printf ("Starting tc_os_atomics_ADD_007\n");
#endif
   os_atomic_stptr (&uintptr, 1);
   CU_ASSERT (os_atomic_addptr_nv (&uintptr, _osaddress) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_addvoidp_nv */
   printf ("Starting tc_os_atomics_ADD_008\n");
#endif
   os_atomic_stvoidp (&uintvoidp, (uintptr_t*)1);
   CU_ASSERT (os_atomic_addvoidp_nv (&uintvoidp, _ptrdiff) == (uintptr_t*)3);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_ADD\n");
#endif
}

CUnit_Test(atomics, SUB)
{
   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(5);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(5);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(5);
   volatile os_atomic_voidp_t uintvoidp = OS_ATOMIC_VOIDP_INIT((uintptr_t)5);
   _osuint32 = 2;
   _osuint64 = 2;
   _osaddress = 2;
   _ptrdiff = 2;

#if ENABLE_TRACING
   /* Test os_atomic_sub32 */
   printf ("Starting tc_os_atomics_SUB_001\n");
#endif
   os_atomic_sub32 (&uint32, _osuint32);
   CU_ASSERT (os_atomic_ld32 (&uint32) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_sub64 */
   printf ("Starting tc_os_atomics_SUB_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_sub64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == 3);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_subptr */
   printf ("Starting tc_os_atomics_SUB_003\n");
#endif
   os_atomic_subptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_subvoidp */
   printf ("Starting tc_os_atomics_SUB_004\n");
#endif
   os_atomic_subvoidp (&uintvoidp, _ptrdiff);
   CU_ASSERT (os_atomic_ldvoidp (&uintvoidp) == (uintptr_t*)3);

#if ENABLE_TRACING
   /* Test os_atomic_sub32_nv */
   printf ("Starting tc_os_atomics_SUB_005\n");
#endif
   os_atomic_st32 (&uint32, 5);
   CU_ASSERT (os_atomic_sub32_nv (&uint32, _osuint32) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_sub64_nv */
   printf ("Starting tc_os_atomics_SUB_006\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_st64 (&uint64, 5);
   CU_ASSERT (os_atomic_sub64_nv (&uint64, _osuint64) == 3);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_subptr_nv */
   printf ("Starting tc_os_atomics_SUB_007\n");
#endif
   os_atomic_stptr (&uintptr, 5);
   CU_ASSERT (os_atomic_subptr_nv (&uintptr, _osaddress) == 3);

#if ENABLE_TRACING
   /* Test os_atomic_subvoidp_nv */
   printf ("Starting tc_os_atomics_SUB_008\n");
#endif
   os_atomic_stvoidp (&uintvoidp, (uintptr_t*)5);
   CU_ASSERT (os_atomic_subvoidp_nv (&uintvoidp, _ptrdiff) == (void *)3);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_SUB\n");
#endif
}

CUnit_Test(atomics, AND)
{
   /* AND Operation:

     150  010010110
     500  111110100

     148  010010100 */

   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(150);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(150);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(150);
   _osuint32 = 500;
   _osuint64 = 500;
   _osaddress = 500;

#if ENABLE_TRACING
   /* Test os_atomic_and32 */
   printf ("Starting tc_os_atomics_AND_001\n");
#endif
   os_atomic_and32 (&uint32, _osuint32);
   CU_ASSERT (os_atomic_ld32 (&uint32) == 148);

#if ENABLE_TRACING
   /* Test os_atomic_and64 */
   printf ("Starting tc_os_atomics_AND_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_and64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == 148);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_andptr */
   printf ("Starting tc_os_atomics_AND_003\n");
#endif
   os_atomic_andptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 148);

#if ENABLE_TRACING
   /* Test os_atomic_and32_ov */
   printf ("Starting tc_os_atomics_AND_004\n");
#endif
   CU_ASSERT (os_atomic_and32_ov (&uint32, _osuint32) == 148);

#if ENABLE_TRACING
   /* Test os_atomic_and64_ov */
   printf ("Starting tc_os_atomics_AND_005\n");
#endif
#if OS_ATOMIC64_SUPPORT
   CU_ASSERT (os_atomic_and64_ov (&uint64, _osuint64) == 148);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_andptr_ov */
   printf ("Starting tc_os_atomics_AND_006\n");
#endif
   CU_ASSERT (os_atomic_andptr_ov (&uintptr, _osaddress) == 148);

#if ENABLE_TRACING
   /* Test os_atomic_and32_nv */
   printf ("Starting tc_os_atomics_AND_007\n");
#endif
   CU_ASSERT (os_atomic_and32_nv (&uint32, _osuint32) == 148);

#if ENABLE_TRACING
   /* Test os_atomic_and64_nv */
   printf ("Starting tc_os_atomics_AND_008\n");
#endif
#if OS_ATOMIC64_SUPPORT
   CU_ASSERT (os_atomic_and64_nv (&uint64, _osuint64) == 148);
 #endif

#if ENABLE_TRACING
   /* Test os_atomic_andptr_nv */
   printf ("Starting tc_os_atomics_AND_009\n");
#endif
   CU_ASSERT (os_atomic_andptr_nv (&uintptr, _osaddress) == 148);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_AND\n");
#endif
}

CUnit_Test(atomics, OR)
{
   /* OR Operation:

     150  010010110
     500  111110100

     502  111110110 */

   volatile os_atomic_uint32_t uint32 = OS_ATOMIC_UINT32_INIT(150);
#if OS_ATOMIC64_SUPPORT
   volatile os_atomic_uint64_t uint64 = OS_ATOMIC_UINT64_INIT(150);
#endif
   volatile os_atomic_uintptr_t uintptr = OS_ATOMIC_UINTPTR_INIT(150);
   _osuint32 = 500;
   _osuint64 = 500;
   _osaddress = 500;

#if ENABLE_TRACING
   /* Test os_atomic_or32 */
   printf ("Starting tc_os_atomics_OR_001\n");
#endif
   os_atomic_or32 (&uint32, _osuint32);
   CU_ASSERT (os_atomic_ld32 (&uint32) == 502);

#if ENABLE_TRACING
   /* Test os_atomic_or64 */
   printf ("Starting tc_os_atomics_OR_002\n");
#endif
#if OS_ATOMIC64_SUPPORT
   os_atomic_or64 (&uint64, _osuint64);
   CU_ASSERT (os_atomic_ld64 (&uint64) == 502);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_orptr */
   printf ("Starting tc_os_atomics_OR_003\n");
#endif
   os_atomic_orptr (&uintptr, _osaddress);
   CU_ASSERT (os_atomic_ldptr (&uintptr) == 502);

#if ENABLE_TRACING
   /* Test os_atomic_or32_ov */
   printf ("Starting tc_os_atomics_OR_004\n");
#endif
   CU_ASSERT (os_atomic_or32_ov (&uint32, _osuint32) == 502);

#if ENABLE_TRACING
   /* Test os_atomic_or64_ov */
   printf ("Starting tc_os_atomics_OR_005\n");
#endif
#if OS_ATOMIC64_SUPPORT
   CU_ASSERT (os_atomic_or64_ov (&uint64, _osuint64) == 502);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_orptr_ov */
   printf ("Starting tc_os_atomics_OR_006\n");
#endif
   CU_ASSERT (os_atomic_orptr_ov (&uintptr, _osaddress) == 502);

#if ENABLE_TRACING
   /* Test os_atomic_or32_nv */
   printf ("Starting tc_os_atomics_OR_007\n");
#endif
   CU_ASSERT (os_atomic_or32_nv (&uint32, _osuint32) == 502);

#if ENABLE_TRACING
   /* Test os_atomic_or64_nv */
   printf ("Starting tc_os_atomics_OR_008\n");
#endif
#if OS_ATOMIC64_SUPPORT
   CU_ASSERT (os_atomic_or64_nv (&uint64, _osuint64) == 502);
#endif

#if ENABLE_TRACING
   /* Test os_atomic_orptr_nv */
   printf ("Starting tc_os_atomics_OR_009\n");
#endif
   CU_ASSERT (os_atomic_orptr_nv (&uintptr, _osaddress) == 502);

#if ENABLE_TRACING
   printf ("Ending tc_atomics_OR\n");
#endif
}
