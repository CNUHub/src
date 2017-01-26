static int hk_off = 0;
static int dime_off = 40;
static int hist_off = 148;

static int header_soff[] = {
  hk_off+36,
  dime_off+0, dime_off+2, dime_off+4, dime_off+6, dime_off+8, dime_off+10,
  dime_off+12, dime_off+14, dime_off+24, dime_off+30, dime_off+32, dime_off+34,
  -1
};
static int header_ioff[] = {
  hk_off+0, hk_off+32,
  dime_off+92, dime_off+96, dime_off+100, dime_off+104,
  -1
};
static int header_foff[] = {
  dime_off+36, dime_off+40, dime_off+44, dime_off+48, dime_off+52, dime_off+56,
  dime_off+60, dime_off+64, dime_off+68, dime_off+72, dime_off+76, dime_off+80,
  dime_off+84, dime_off+88,
  hist_off+168, hist_off+172, hist_off+176, hist_off+180, hist_off+184,
  hist_off+188, hist_off+192, hist_off+196,
  -1
};

void swap_data(char *data, int *soff, int *loff, int *foff) {
  int *ptr;
  for (ptr = soff; *ptr >= 0; ptr++) swaper(data + *ptr)
  for (ptr = loff; *ptr >= 0; ptr++) lswaper(data + *ptr)
  for (ptr = foff; *ptr >= 0; ptr++) fswaper(data + *ptr)
}
