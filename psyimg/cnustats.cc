#include "psyhdr.h"
#include <math.h>
#include "psytools.h"
#include "cnustats.h"

probfromtvalueimg::probfromtvalueimg(psyimg *timgptr, psyimg *dfimgptr,
				     double additionToDF,
				     psytype pixeltype, int keepNeg) :
				     proc2img(timgptr, dfimgptr, pixeltype)
{
  probfromtvalueimg::additionToDF = additionToDF;
  probfromtvalueimg::keepNeg = keepNeg;
}

void probfromtvalueimg::proc2pixels(char *in1, psytype in1type,
			  char *in2, psytype in2type,
			  char *out, psytype outtype)
{
  double prob, tvalue, df, signFactor;
  // convert to double to simplify math
  type2double(in1, in1type, (char *)&tvalue);
  type2double(in2, in2type, (char *)&df);
  signFactor = 1;
  if(keepNeg && (tvalue < 0.0)) signFactor = -1;
  // do actual processing
  df += additionToDF;
  if(df <= 0.0) prob = 1.0;
  else prob = betai( 0.5 * df, 0.5, df/(df + tvalue*tvalue) );
  prob *= signFactor;
  // convert back to desired output type
  pixeltypechg((char *)&prob, psydouble, out, outtype);
}
