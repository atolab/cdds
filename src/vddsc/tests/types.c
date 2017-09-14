#include <criterion/criterion.h>
#include <criterion/logging.h>
#include "dds.h"
#include "TypesArrayKey.h"

/* Very basic test; should be extended to read the written data as well. */
#define strfy(s) #s
#define VDDSC_ARRAYTYPEKEY_TEST(type, init, doalloc) \
    do { \
        dds_return_t status; \
        dds_entity_t par, top, wri; \
        TypesArrayKey_##type##_arraytypekey data; \
        TypesArrayKey_##type##_arraytypekey *datap = &data; \
        if(doalloc) { datap = TypesArrayKey_##type##_arraytypekey__alloc(); } \
        for ( unsigned i = 0; i < (sizeof data.key / sizeof *data.key); i++ ) { datap->key[i] = (init); } \
        \
        par = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL); \
        cr_assert_gt(par, 0); \
        if(doalloc) { \
            top = dds_create_topic(par, &TypesArrayKey_##type##_arraytypekey_desc, strfy(type)"_alloc", NULL, NULL); \
        } else { \
            top = dds_create_topic(par, &TypesArrayKey_##type##_arraytypekey_desc, strfy(type), NULL, NULL); \
        } \
        cr_assert_gt(top, 0); \
        wri = dds_create_writer(par, top, NULL, NULL); \
        cr_assert_gt(wri, 0); \
        \
        status = dds_write(wri, datap); \
        cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK); \
        \
        if(doalloc) { TypesArrayKey_##type##_arraytypekey_free(datap, DDS_FREE_ALL); } \
        dds_delete(wri); \
        dds_delete(top); \
        dds_delete(par); \
    } while (0)


Test(vddsc_types, enum_enumkey_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(enum_enumkey, 1, false);
}
Test(vddsc_types, enum_enumkey_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(enum_enumkey, 1, true);
}

Test(vddsc_types, long_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(long, 1, false);
}
Test(vddsc_types, long_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(long, 1, true);
}

Test(vddsc_types, longlong_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(longlong, 1, false);
}
Test(vddsc_types, longlong_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(longlong, 1, true);
}

Test(vddsc_types, unsignedshort_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(unsignedshort, 1, false);
}
Test(vddsc_types, unsignedshort_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(unsignedshort, 1, true);
}

Test(vddsc_types, unsignedlong_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(unsignedlong, 1, false);
}
Test(vddsc_types, unsignedlong_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(unsignedlong, 1, true);
}

Test(vddsc_types, unsignedlonglong_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(unsignedlonglong, 1, false);
}
Test(vddsc_types, unsignedlonglong_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(unsignedlonglong, 1, true);
}

Test(vddsc_types, float_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(float, 1.0f, false);
}
Test(vddsc_types, float_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(float, 1.0f, true);
}

Test(vddsc_types, double_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(double, 1.0f, false);
}
Test(vddsc_types, double_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(double, 1.0f, true);
}

Test(vddsc_types, char_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(char, '1', false);
}
Test(vddsc_types, char_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(char, '1', true);
}

Test(vddsc_types, boolean_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(boolean, true, false);
}
Test(vddsc_types, boolean_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(boolean, true, true);
}

Test(vddsc_types, octet_arraytypekey)
{
    VDDSC_ARRAYTYPEKEY_TEST(octet, 1, false);
}
Test(vddsc_types, octet_arraytypekey_alloc)
{
    VDDSC_ARRAYTYPEKEY_TEST(octet, 1, true);
}

Test(vddsc_types, alltypeskey)
{
    dds_return_t status;
    dds_entity_t par, top, wri;
    const TypesArrayKey_alltypeskey atk_data = {
        .l = -1,
        .ll = -1,
        .us = 1,
        .ul = 1,
        .ull = 1,
        .f = 1.0f,
        .d = 1.0f,
        .c = '1',
        .b = true,
        .o = 1,
        .s = "1"
    };

    par = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(par, 0);
    top = dds_create_topic(par, &TypesArrayKey_alltypeskey_desc, "AllTypesKey", NULL, NULL);
    cr_assert_gt(top, 0);
    wri = dds_create_writer(par, top, NULL, NULL);
    cr_assert_gt(wri, 0);

    status = dds_write(wri, &atk_data);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK);

    dds_delete(wri);
    dds_delete(top);
    dds_delete(par);
}
Test(vddsc_types, alltypeskey_alloc)
{
    dds_return_t status;
    dds_entity_t par, top, wri;
    TypesArrayKey_alltypeskey *atk_data = TypesArrayKey_alltypeskey__alloc();

    atk_data->l = -1;
    atk_data->ll = -1;
    atk_data->us = 1;
    atk_data->ul = 1;
    atk_data->ull = 1;
    atk_data->f = 1.0f;
    atk_data->d = 1.0f;
    atk_data->c = '1';
    atk_data->b = true;
    atk_data->o = 1;
    atk_data->s = dds_string_dup("1");

    par = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(par, 0);
    top = dds_create_topic(par, &TypesArrayKey_alltypeskey_desc, "AllTypesKey_alloc", NULL, NULL);
    cr_assert_gt(top, 0);
    wri = dds_create_writer(par, top, NULL, NULL);
    cr_assert_gt(wri, 0);

    status = dds_write(wri, atk_data);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK);

    TypesArrayKey_alltypeskey_free(atk_data, DDS_FREE_ALL);
    dds_delete(wri);
    dds_delete(top);
    dds_delete(par);
}

Test(vddsc_types, stringkey_alloc)
{
    dds_return_t status;
    dds_entity_t par, top, wri;
    TypesArrayKey_stringkey *sk_data = TypesArrayKey_stringkey__alloc();

    sk_data->payload = dds_string_dup("Not so long payload");
    sk_data->seq_payload._maximum = 20;
    sk_data->seq_payload._buffer = dds_alloc(sk_data->seq_payload._maximum * sizeof(char *));
    for ( sk_data->seq_payload._length = 0; sk_data->seq_payload._length < sk_data->seq_payload._maximum; sk_data->seq_payload._length++ ) {
        ((char **)sk_data->seq_payload._buffer)[sk_data->seq_payload._length] = dds_string_dup("Not so long sequence payload");
    }
    sk_data->seq_payload._release = true;
    sk_data->key = dds_string_dup("Not so long key");

    par = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
    cr_assert_gt(par, 0);
    top = dds_create_topic(par, &TypesArrayKey_stringkey_desc, "StringKey_alloc", NULL, NULL);
    cr_assert_gt(top, 0);
    wri = dds_create_writer(par, top, NULL, NULL);
    cr_assert_gt(wri, 0);

    status = dds_write(wri, sk_data);
    cr_assert_eq(dds_err_nr(status), DDS_RETCODE_OK);

    TypesArrayKey_stringkey_free(sk_data, DDS_FREE_ALL);
    dds_delete(wri);
    dds_delete(top);
    dds_delete(par);
}
