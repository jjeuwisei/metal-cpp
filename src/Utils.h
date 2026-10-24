#ifndef UTILS_H
#define UTILS_H

#define Assert( Expression ) \
  if( Expression ) {} \
  else { __builtin_trap(); }

#endif

