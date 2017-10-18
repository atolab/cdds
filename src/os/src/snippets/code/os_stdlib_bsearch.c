/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

void *
os_bsearch(const void *key, const void *base, size_t nmemb, size_t size,
	 int (*compar) (const void *, const void *))
{
    return bsearch (key,base,nmemb,size,compar);
}
