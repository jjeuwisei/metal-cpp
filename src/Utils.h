#ifndef UTILS_H
#define UTILS_H

#define Assert( Expression ) \
  if( Expression ) {} \
  else { __builtin_trap(); }

#define ArrayCount(Array) sizeof(Array) / sizeof((Array)[0])  

#endif

