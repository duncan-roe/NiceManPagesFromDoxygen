# NiceManPagesFromDoxygen
Nice Manual Pages From Doxygen
## Overview
**NiceManPagesFromDoxygen** was developed to make nice man pages for library functions written in C.<br>
Its main component is the postprocessor shell script `build_man.sh` (hereafter referred to as *build_man*).<br>
*build_man* requires some extra Doxygen `\manonly` code in the source files.<br>
Doxyfile needs an input filter to generate man pages for functions declared `static inline` (affects all output formats).
## *build_man* actions
+ (optional) Create a man7 entry from the `\mainpage`
+ Rename each *topic*.3 man page to overwrite one of the *item*.3 pages that references it.
Replace each other *item*.3 page that references the <ins>original</ins> page with a symbolic link to the <ins>renamed</ins> page.
+ Post-process each renamed man page:
  + Remove the Doxygen-supplied synopsis up to and including any Topics, but only if the user supplied a `\manonly` synopsis (with required `#include` lines).
  + Fix the **NAME** entry to be a comma-separated list of documented items (functions and / or macros).
  This is the format that <b>mandb</b>(1) expects. After the next run of <b>mandb</b>, <b>apropos</b>(1) and <b>whatis</b>(1) will be able to report on the page.
  + **If** the user supplied a `\manonly` synopsis, move this synopsis to where the Doxygen-supplied one used to be,
  and compact the remaining synopsis by removing the **Functions** and / or **Macros** sub-headings
  while leaving a blank line after the last `#include`.
  + If the <b>Detailed Description</b> is empty, remove that heading.
  + If the source browser is on, remove the line starting `Definition at line` from each item.
  + Delete potential double blank lines (only needed for old versions of **man**(1)).
  + **If** the user created a man7 page, insert a **See Also** entry for it. 
