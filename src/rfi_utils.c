#include "presto.h"
#include "rfifind.h"

rfi_obs *create_rfi_obs(rfi_instance rfi)
/* Create and initialize a rfi_obs structure */
{
  rfi_obs* new;

  new = (rfi_obs *)malloc(sizeof(rfi_obs));
  new->rfi = (rfi_instance *)malloc(sizeof(rfi_instance));
  *(new->rfi) = rfi;
  new->freq_avg = rfi.freq;
  new->freq_var = 0.0;
  new->number = 1;
  return new;
}

void free_rfi_obs(rfi_obs *rfi)
/* Free an rfi_obs structure and its contents */
{
  free(rfi->rfi);
  free(rfi);
}

void free_rfi_obs_vector(rfi_obs **rfi_vect, int num_rfi_vect)
/* Free a vector that holds rfi_obs structures */
{
  int ii=0;

  for(ii=0; ii<num_rfi_vect; ii++){
    free_rfi_obs(rfi_vect[ii]);
    ii++;
  }
  free(rfi_vect);
}

void add_rfi_instance(rfi_obs *old, rfi_instance new)
/* Add an instance of RFI to a rfi_obs structure */
{
  double davg, dvar;
  float *freqs;
  int ii;

  old->number++;
  old->rfi = (rfi_instance *)realloc(old->rfi, sizeof(rfi_instance) * 
				     old->number);
  old->rfi[old->number-1] = new;

  /* Update means and variances (extremely inefficient!) */

  freqs = gen_fvect(old->number);
  for (ii=0; ii<old->number; ii++)
    freqs[ii] = old->rfi[ii].freq;
  avg_var(freqs, old->number, &davg, &dvar);
  old->freq_avg = davg;
  old->freq_var = dvar;
  free(freqs);
}

int find_rfi(rfi_obs **rfi_vect, int num_rfi_vect, 
	     double freq, double fract_error)
/* Try to find a birdie in an rfi_obs vector.  Compare    */
/* all currently known birdies with the new freq.  If it  */
/* finds one with a freq within fractional error, it      */
/* returns the number of the birdie -- otherwise, -1.     */
{
  float err;
  int ii=0;

  for (ii=0; ii<num_rfi_vect; ii++){    
    err = fabs(rfi_vect[ii]->freq_avg - freq) / freq;
    if (err < fract_error)
      return ii;
  }
  return -1;
}

int compare_rfi_obs(void *ca, void *cb)
/*  Used as compare function for qsort() */
{
  rfi_obs *a, *b;

  a = (rfi_obs *) ca;
  b = (rfi_obs *) cb;
  if ((b->freq_avg - a->freq_avg) < 0.0)
    return -1;
  if ((b->freq_avg - a->freq_avg) > 0.0)
    return 1;
  return 0;
}

void percolate_rfi_obs(rfi_obs **list, int nlist)
/*  Pushes an rfi_obs structure as far up a sorted list of */
/*  structs as it needs to go to keep the list sorted.     */
/*  The new rfi_obs structure is in position nlist-1.      */
{
  int ct;
  rfi_obs *tempzz;

  for (ct=nlist-2; ct>=0; ct--){
    if (list[ct]->freq_avg < list[ct+1]->freq_avg) {
      SWAP(list[ct], list[ct+1]);
    } else {
      break;
    }
  }
}
