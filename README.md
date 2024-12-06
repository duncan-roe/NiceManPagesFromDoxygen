# NMPFD
Nice Manual Pages From Doxygen
## Overview
**NMPFD** was developed to make nice man pages for library functions written in C.<br>
Its main component is the postprocessor shell script `build_man.sh` (hereafter referred to as *build_man*).<br>
*build_man* requires some extra Doxygen `\manonly` code in the source files.<br>
Doxyfile needs an input filter to generate man pages for functions declared `static inline` (affects all output formats).
## *build_man* actions
+ (optional) Create a man7 entry from the `\mainpage`
+ Rename each *topic*.3 man page to overwrite one of the *item*.3 pages that references it.
Replace each other *item*.3 page that references the <ins>original</ins> page with a symbolic link to the <ins>renamed</ins> page.
+ Post-process each renamed man page:
  + xxx 
