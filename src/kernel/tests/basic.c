#include "dds.h"
#include "cunitrunner/runner.h"

void test(void) 
{

  dds_entity_t participant;

  participant = dds_create_participant (DDS_DOMAIN_DEFAULT, NULL, NULL);
  CU_ASSERT(participant != NULL);

  /* TODO: CHAM-108: Add some simple read/write test(s). */

  dds_delete(participant);
}

int main (int argc, char *argv[])
{
    CU_pSuite pSuite;
    
	if(runner_init(argc, argv)){
		goto err_init;
	}

	/*add a suite to the registry*/
	if((pSuite = CU_add_suite("Basic C99 interface test", NULL, NULL)) == NULL){
		goto err;
	}

	/*add test cases to the test suite*/
	if(CU_add_test(pSuite, "C99 Interface",test) == NULL) {
		goto err;
	}
	runner_run();
err:
	runner_fini();
err_init:
	return CU_get_error();	    
}

