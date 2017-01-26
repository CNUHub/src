#include "psyhdr.h"

psyimgdblz::psyimgdblz(psyimg *psyimgptr, psytype pixeltype)
{
  psydims dim;
  psydims origin;
  psyres resolution;
  psyimgptr->getsize(&dim.x, &dim.y, &dim.z, &dim.i);
  psyimgptr->getorig(&origin.x, &origin.y, &origin.z, &origin.i);
  psyimgptr->getres(&resolution.x, &resolution.y, &resolution.z,
		    &resolution.i);
  dim.z *= 2;
  resolution.z /= 2.0;
  initpsyimglnkpxl(psyimgptr, dim.x, dim.y, dim.z, dim.i, pixeltype,
		   origin.x, origin.y, origin.z, origin.i, 0,
		   resolution.x, resolution.y, resolution.z, resolution.i,
		   psyimgptr->getwordres());
}

void psyimgdblz::getnextpixel(char *pixel)
{
  double dpixel, dpixel1, dpixel2;
// retrieve proper input pixel
  inputpsyimg->getpixel(pixel, type, getpixelloc.x, getpixelloc.y,
			getpixelloc.z/2, getpixelloc.i);

  if((getpixelloc.z%2) && (getpixelloc.z != end.z)) {
// odd z - lookup z falls half way between original image planes
// average with pixel from next plane
    type2double(pixel, type, (char *)&dpixel1);
    inputpsyimg->getpixel(pixel, type, getpixelloc.x, getpixelloc.y,
			  (getpixelloc.z/2)+1, getpixelloc.i);
    type2double(pixel, type, (char *)&dpixel2);
    dpixel = .5*(dpixel1 + dpixel2);
// convert back to desired output type
    pixeltypechg((char *)&dpixel, psydouble, pixel, type);
  }

  incgetpixel();
}

psyimghalfz::psyimghalfz(psyimg *psyimgptr)
{
  psydims dim;
  psydims origin;
  psyres resolution;
  psyimgptr->getsize(&dim.x, &dim.y, &dim.z, &dim.i);
  psyimgptr->getorig(&origin.x, &origin.y, &origin.z, &origin.i);
  psyimgptr->getres(&resolution.x, &resolution.y, &resolution.z,
		    &resolution.i);
  dim.z = (dim.z+1)/2; // round odd numbers up
  resolution.z *= 2.0;
  initpgbuff(psyimgptr,
	     dim.x, dim.y, dim.z, dim.i, psynotype,
	     origin.x, origin.y, origin.z, origin.i, 0,
	     resolution.x, resolution.y, resolution.z, resolution.i,
	     psyimgptr->getwordres(), 1);
}

void psyimghalfz::fillpage(char *buff, int z, int i)
{
 psypgbuff::fillpage(buff, z*2, i);
}

psyimgdupplanes::psyimgdupplanes(psyimg *psyimgptr,
				   int number_of_duplicates)
{
  psyimgdupplanes::number_of_duplicates=number_of_duplicates;
  psydims dim=psyimgptr->getsize();
  psydims origin=psyimgptr->getorig();
  psyres resolution=psyimgptr->getres();
  dim.z *= number_of_duplicates;
  resolution.z /= number_of_duplicates;
  initpsyimglnkpxl(psyimgptr,
		   dim.x, dim.y, dim.z, dim.i, psynotype,
		   origin.x, origin.y, origin.z, origin.i, 0,
		   resolution.x, resolution.y, resolution.z, resolution.i,
		   psyimgptr->getwordres());
}

void psyimgdupplanes::initgetpixel(psytype pixeltype, int x, int y,
				    int z, int i)
{
  psyimglnkpxl::initgetpixel(pixeltype, x, y, z, i);
  inputpsyimg->initgetpixel(pixeltype, x, y, z/number_of_duplicates, i);
}

void psyimgdupplanes::getnextpixel(char *pixel)
{
  inputpsyimg->getnextpixel(pixel);
  incgetpixel();
}

void psyimgdupplanes::freegetpixel() {
  inputpsyimg->freegetpixel();
  psyimglnkpxl::freegetpixel();
}

void psyimgdupplanes::copyblock(char *outbuff, int xorig, int yorig,
				 int zorig, int iorig,
				 int xend, int yend, int zend, int iend,
				 int out_xinc, int out_yinc,
				 int out_zinc, int out_iinc,
				 psytype pixeltype)
{
  if(zorig == zend) {
    int z=zorig/number_of_duplicates;
    inputpsyimg->copyblock(outbuff, xorig, yorig, z, iorig,
			   xend, yend, z, iend,
			   out_xinc, out_yinc, out_zinc, out_iinc,
			   pixeltype);
  }
  else {
    psyimg::copyblock(outbuff, xorig, yorig, zorig, iorig,
		      xend, yend, zend, iend,
		      out_xinc, out_yinc, out_zinc, out_iinc,
		      pixeltype);
  }
}

psyimgdupframes::psyimgdupframes(psyimg *psyimgptr, int number_of_duplicates)
{
  psydims dim;
  psydims origin;
  psyres resolution;
  psyimgdupframes::number_of_duplicates=number_of_duplicates;
  psyimgptr->getsize(&dim.x, &dim.y, &dim.z, &dim.i);
  psyimgptr->getorig(&origin.x, &origin.y, &origin.z, &origin.i);
  psyimgptr->getres(&resolution.x, &resolution.y, &resolution.z,
		    &resolution.i);
  dim.i *= number_of_duplicates;
  resolution.i /= number_of_duplicates;
  initpgbuff(psyimgptr,
	     dim.x, dim.y, dim.z, dim.i, psynotype,
	     origin.x, origin.y, origin.z, origin.i, 0,
	     resolution.x, resolution.y, resolution.z, resolution.i,
	     psyimgptr->getwordres(), 1);
}

void psyimgdupframes::fillpage(char *buff, int z, int i)
{
  i = i/number_of_duplicates;
  psypgbuff::fillpage(buff, z, i);
}

