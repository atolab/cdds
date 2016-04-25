#include "kernel/dds_domain.h"
#include "kernel/dds_tkmap.h"

static int dds_domain_compare (const int32_t * a, const int32_t * b)
{
  return (*a == *b) ? 0 : (*a < *b) ? -1 : 1;
}

const ut_avlTreedef_t dds_domaintree_def = UT_AVL_TREEDEF_INITIALIZER
(
  offsetof (dds_domain, m_node),
  offsetof (dds_domain, m_id),
  (int (*) (const void *, const void *)) dds_domain_compare,
  0
);

dds_domain * dds_domain_find_locked (dds_domainid_t id)
{
  return (dds_domain*) ut_avlLookup (&dds_domaintree_def, &dds_global.m_domains, &id);
}

dds_domain * dds_domain_create (dds_domainid_t id)
{
  dds_domain * domain;
  os_mutexLock (&dds_global.m_mutex);
  domain = dds_domain_find_locked (id);
  if (domain == NULL)
  {
    domain = dds_alloc (sizeof (*domain));
    domain->m_id = id;
    ut_avlInit (&dds_topictree_def, &domain->m_topics);
    ut_avlInsert (&dds_domaintree_def, &dds_global.m_domains, domain);
    domain->m_rd_tkmap = dds_tkmap_new ();
    domain->m_wr_tkmap = dds_tkmap_new ();
  }
  domain->m_refc++;
  os_mutexUnlock (&dds_global.m_mutex);
  return domain;
}

void dds_domain_free (dds_domain * domain)
{
  os_mutexLock (&dds_global.m_mutex);
  if (--domain->m_refc == 0)
  {
    ut_avlDelete (&dds_domaintree_def, &dds_global.m_domains, domain);
    dds_tkmap_free (domain->m_rd_tkmap);
    dds_tkmap_free (domain->m_wr_tkmap);
    dds_free (domain);
  }
  os_mutexUnlock (&dds_global.m_mutex);
}
