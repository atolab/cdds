#include "ddsc/dds.h"
#include <criterion/criterion.h>
#include <criterion/logging.h>

Test(ddsc_err, str)
{
    cr_assert_str_eq(dds_err_str(1                                      ), "Success");
    cr_assert_str_eq(dds_err_str(-255                                   ), "Unknown");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_OK                     * -1), "Success");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_ERROR                  * -1), "Error");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_UNSUPPORTED            * -1), "Unsupported");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_BAD_PARAMETER          * -1), "Bad Parameter");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_PRECONDITION_NOT_MET   * -1), "Precondition Not Met");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_OUT_OF_RESOURCES       * -1), "Out Of Resources");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_NOT_ENABLED            * -1), "Not Enabled");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_IMMUTABLE_POLICY       * -1), "Immutable Policy");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_INCONSISTENT_POLICY    * -1), "Inconsistent Policy");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_ALREADY_DELETED        * -1), "Already Deleted");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_TIMEOUT                * -1), "Timeout");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_NO_DATA                * -1), "No Data");
    cr_assert_str_eq(dds_err_str(DDS_RETCODE_ILLEGAL_OPERATION      * -1), "Illegal Operation");
}
