#include <R.h>
#include <Rdefines.h>
#include "MSnbase.h"
#include <stdio.h>

/************************************************************
 * The Spectrum1 constructor in C.
 *
 */

static SEXP _new_Spectrum1(SEXP msLevel, SEXP peaksCount, SEXP rt,
			   SEXP acquisitionNum, SEXP scanIndex, SEXP tic,
			   SEXP mz, SEXP intensity, SEXP fromFile,
			   SEXP centroided, SEXP smoothed, SEXP polarity)
{
  SEXP classdef, ans;

  PROTECT(classdef = MAKE_CLASS("Spectrum1"));
  PROTECT(ans = NEW_OBJECT(classdef));
  /* Set the slot values */
  SET_SLOT(ans, install("msLevel"), msLevel);
  SET_SLOT(ans, install("peaksCount"), peaksCount);
  SET_SLOT(ans, install("rt"), rt);
  SET_SLOT(ans, install("acquisitionNum"), acquisitionNum);
  SET_SLOT(ans, install("scanIndex"), scanIndex);
  SET_SLOT(ans, install("tic"), tic);
  SET_SLOT(ans, install("mz"), mz);
  SET_SLOT(ans, install("intensity"), intensity);
  SET_SLOT(ans, install("fromFile"), fromFile);
  SET_SLOT(ans, install("centroided"), centroided);
  SET_SLOT(ans, install("smoothed"), smoothed);
  SET_SLOT(ans, install("polarity"), polarity);

  UNPROTECT(2);
  return ans;
}

SEXP Spectrum1_constructor(SEXP msLevel, SEXP peaksCount, SEXP rt,
			   SEXP acquisitionNum, SEXP scanIndex, SEXP tic,
			   SEXP mz, SEXP intensity, SEXP fromFile,
			   SEXP centroided, SEXP smoothed, SEXP polarity,
			   SEXP check)
{
  //int nvalues;
  SEXP ans;
  //const int *lengths_p;

  //nvalues = LENGTH(intensity);
  if (LOGICAL(check)[0]) {
    if ( LENGTH(mz) != LENGTH(intensity)) {
      error("'length(intensity)' != 'length(mz)'");
    }
  }

  PROTECT(ans = _new_Spectrum1(msLevel, peaksCount, rt, acquisitionNum,
			       scanIndex, tic, mz, intensity, fromFile,
			       centroided, smoothed, polarity));
  UNPROTECT(1);
  return(ans);
  return R_NilValue;
}

/* Constructor that automatically sorts M/Z and intensity values by M/Z.
 */
SEXP Spectrum1_constructor_mz_sorted(SEXP msLevel, SEXP peaksCount,
				     SEXP rt, SEXP acquisitionNum,
				     SEXP scanIndex, SEXP tic,
				     SEXP mz, SEXP intensity,
				     SEXP fromFile, SEXP centroided,
				     SEXP smoothed, SEXP polarity,
				     SEXP check)
{
  //int nvalues;
  SEXP ans, oMz, oInt;
  int nvals, i, calcTic;
  double theTic;

  calcTic = 0;
  theTic = asReal(tic);
  if (theTic == 0)
    calcTic = 1;
  //const int *lengths_p;
  nvals = LENGTH(mz);

  // Create pointers.
  double *pOint, *pOmz, *pIntensity, *pMz;

  //nvalues = LENGTH(intensity);
  if (LOGICAL(check)[0]) {
    if ( nvals != LENGTH(intensity)) {
      error("'length(intensity)' != 'length(mz)'");
    }
  }
  // initialize new M/Z, intensity vectors.
  PROTECT(oMz = NEW_NUMERIC(nvals));
  PROTECT(oInt = NEW_NUMERIC(nvals));
  // create pointers.
  pOmz = REAL(oMz);
  pOint = REAL(oInt);
  pMz = REAL(mz);
  pIntensity = REAL(intensity);
  int idx[nvals];
  _get_order_of_double_array(pMz, nvals, 0, idx, 0);
  // sort M/Z and intensity by M/Z
  for (i = 0; i < nvals; i++) {
    pOmz[i] = pMz[idx[i]];
    pOint[i] = pIntensity[idx[i]];
    if (calcTic)
      theTic += pOint[i];
  }
  PROTECT(ans = _new_Spectrum1(msLevel, peaksCount, rt, acquisitionNum,
			       scanIndex, ScalarReal(theTic), oMz, oInt,
			       fromFile, centroided, smoothed, polarity));
  UNPROTECT(3);
  return(ans);
  return R_NilValue;
}



/*************************************************************
 * Multi_Spectrum1_constructor_mz_sorted
 *
 * Simple C-function to create a list of Spectrum1 object based on the
 * submitted values.
 * All arguments (except mz, intensity) are supposed to be vectors of
 * length equal to the number of spectra. Argument nvalues defining the
 * number of M/Z and intensity values per spectrum.
 * This function is called by the R-function: Spectra1, argument checking is supposed
 * to take place there.
 * This constructor ensures that M/Z-intensity pairs are ordered by M/Z.
 */
SEXP Multi_Spectrum1_constructor_mz_sorted(SEXP msLevel, SEXP peaksCount,
					   SEXP rt, SEXP acquisitionNum,
					   SEXP scanIndex, SEXP tic,
					   SEXP mz, SEXP intensity,
					   SEXP fromFile, SEXP centroided,
					   SEXP smoothed, SEXP polarity,
					   SEXP nvalues, SEXP check)
{
  int n = length(nvalues);
  int currentN = 0;
  int startN = 0;
  SEXP cMz, cIntensity, currentMz;
  SEXP out = PROTECT(allocVector(VECSXP, n));
  double *pRt, *pTic;
  int *pPeaksCount, *pAcquisitionNum, *pScanIndex, *pPolarity,
    *pFromFile, *pCentroided, *pSmoothed, *pNvalues;
  double *pCurrentMz, *pCmz, *pCint, *pMz, *pInt;

  double currentTic;
  int calcTic;

  pRt = REAL(rt);
  pTic = REAL(tic);
  pPeaksCount = INTEGER(peaksCount);
  pAcquisitionNum = INTEGER(acquisitionNum);
  pScanIndex = INTEGER(scanIndex);
  pFromFile = INTEGER(fromFile);
  pPolarity = INTEGER(polarity);
  pNvalues = INTEGER(nvalues);
  pCentroided = LOGICAL(centroided);
  pSmoothed = LOGICAL(smoothed);
  pMz = REAL(mz);
  pInt = REAL(intensity);

  for (int i = 0; i < n; i++) {
    currentN = pNvalues[i];
    // Check if TIC is 0
    currentTic = pTic[i];
    calcTic = 0;
    if (currentTic == 0)
      calcTic = 1;
    // Creating the mz and intensity vectors.
    PROTECT(currentMz = NEW_NUMERIC(currentN));
    PROTECT(cMz = NEW_NUMERIC(currentN));
    PROTECT(cIntensity = NEW_NUMERIC(currentN));
    // Create pointers to enable faster access (http://adv-r.had.co.nz/C-interface.html#c-vectors)
    pCurrentMz = REAL(currentMz);
    pCmz = REAL(cMz);
    pCint = REAL(cIntensity);
    // Fill the vector with the M/Z values of the current spectrum
    for (int j = 0; j < currentN; j++) {
      pCurrentMz[j] = pMz[startN + j];
    }
    // Get the order of the M/Z values.
    int idx[currentN];
    _get_order_of_double_array(pCurrentMz, currentN, 0, idx, 0);
    // Sort the M/Z and intensity values for the current spectrum by M/Z
    for (int j = 0; j < currentN; j++) {
      pCmz[j] = pCurrentMz[idx[j]];
      pCint[j] = pInt[startN + (idx[j])];
      // Calculate TIC if provided one was 0
      if (calcTic > 0)
	currentTic += pCint[j];
    }
    // Create the new Spectrum1
    SET_VECTOR_ELT(out, i, _new_Spectrum1(msLevel,
					  ScalarInteger(pPeaksCount[i]),
					  ScalarReal(pRt[i]),
					  ScalarInteger(pAcquisitionNum[i]),
					  ScalarInteger(pScanIndex[i]),
					  /* ScalarReal(pTic[i]), */
					  ScalarReal(currentTic),
					  cMz,
					  cIntensity,
					  ScalarInteger(pFromFile[i]),
					  ScalarLogical(pCentroided[i]),
					  ScalarLogical(pSmoothed[i]),
					  ScalarInteger(pPolarity[i])));
    UNPROTECT(3);
    startN = startN + currentN;
  }

  UNPROTECT(1);
  return(out);
}

/*************************************************************
 *
 * DONT USE THIS ONE!!!
 *
 * Multi_Spectrum1_constructor
 *
 * Simple C-function to create a list of Spectrum1 object based on the
 * submitted values.
 * All arguments (except mz, intensity) are supposed to be vectors of
 * length equal to the number of spectra. Argument nvalues defining the
 * number of M/Z and intensity values per spectrum.
 * This function is called by the R-function: Spectra1, argument checking is supposed
 * to take place there.
 */
SEXP Multi_Spectrum1_constructor(SEXP msLevel, SEXP peaksCount, SEXP rt,
				 SEXP acquisitionNum, SEXP scanIndex, SEXP tic,
				 SEXP mz, SEXP intensity, SEXP fromFile,
				 SEXP centroided, SEXP smoothed, SEXP polarity,
				 SEXP nvalues, SEXP check)
{
  int n = length(nvalues);
  int currentN = 0;
  int startN = 0;
  SEXP cMz, cIntensity;
  SEXP out = PROTECT(allocVector(VECSXP, n));
  double *pRt, *pTic;
  int *pPeaksCount, *pAcquisitionNum, *pScanIndex, *pPolarity,
    *pFromFile, *pCentroided, *pSmoothed, *pNvalues;

  pRt = REAL(rt);
  pTic = REAL(tic);
  pPeaksCount = INTEGER(peaksCount);
  pAcquisitionNum = INTEGER(acquisitionNum);
  pScanIndex = INTEGER(scanIndex);
  pFromFile = INTEGER(fromFile);
  pPolarity = INTEGER(polarity);
  pNvalues = INTEGER(nvalues);
  pCentroided = LOGICAL(centroided);
  pSmoothed = LOGICAL(smoothed);

  for (int i = 0; i < n; i++) {
    // Creating the mz and intensity vectors.
    currentN = pNvalues[i];
    PROTECT(cMz = NEW_NUMERIC(currentN));
    PROTECT(cIntensity = NEW_NUMERIC(currentN));
    for (int j = 0; j < currentN; j++) {
      REAL(cMz)[j] = REAL(mz)[startN + j];
      REAL(cIntensity)[j] = REAL(intensity)[startN + j];
    }

    SET_VECTOR_ELT(out, i, _new_Spectrum1(msLevel,
					  ScalarInteger(pPeaksCount[i]),
					  ScalarReal(pRt[i]),
					  ScalarInteger(pAcquisitionNum[i]),
					  ScalarInteger(pScanIndex[i]),
					  ScalarReal(pTic[i]),
					  cMz,
					  cIntensity,
					  ScalarInteger(pFromFile[i]),
					  ScalarLogical(pCentroided[i]),
					  ScalarLogical(pSmoothed[i]),
					  ScalarInteger(pPolarity[i])));
    UNPROTECT(2);
    startN = startN + currentN;
  }

  UNPROTECT(1);
  return(out);
}
