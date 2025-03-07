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
## Example
This Doxygen-generated man page
<pre>
do_not_use(3)              Library Functions Manual              do_not_use(3)

<b>NAME</b>
       do_not_use - Internal functions

<b>SYNOPSIS</b>
   <b>Functions</b>
       void <b>pktb_push</b> (struct pkt_buff *pktb, unsigned int len)
       void <b>pktb_pull</b> (struct pkt_buff *pktb, unsigned int len)
       void <b>pktb_put</b> (struct pkt_buff *pktb, unsigned int len)
       void <b>pktb_trim</b> (struct pkt_buff *pktb, unsigned int len)

<b>Detailed</b> <b>Description</b>
       Do not use these functions. Instead, always use the mangle function
       appropriate to the level at which you are working.
       <b>pktb_mangle()</b> uses all the below functions except <b>pktb_pull()</b>, which
       is not used by anything.

<b>Function</b> <b>Documentation</b>
   <b>void</b> <b>pktb_pull</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_pull - increment pointer to packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> Number of bytes to add to packet start address

       Definition at line <b>269</b> of file <b>pktbuff.c</b>.

   <b>void</b> <b>pktb_push</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_push - decrement pointer to packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> Number of bytes to subtract from packet start address

       Definition at line <b>257</b> of file <b>pktbuff.c</b>.

   <b>void</b> <b>pktb_put</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_put - add extra bytes to the tail of the packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> Number of bytes to add to packet tail (and length)

       Definition at line <b>281</b> of file <b>pktbuff.c</b>.

   <b>void</b> <b>pktb_trim</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_trim - set new length for this packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> New packet length (tail is adjusted to reflect this)

       Definition at line <b>292</b> of file <b>pktbuff.c</b>.

<b>Author</b>
       Generated automatically by Doxygen for Nice Man Pages From Doxygen from
       the source code.

Nice Man Pages From Doxygen       Version 0.1                    do_not_use(3)
</pre>
Changes to:
<pre>
do_not_use(3)              Library Functions Manual              do_not_use(3)

<b>NAME</b>
       pktb_push, pktb_pull, pktb_put, pktb_trim - Internal functions

<b>SYNOPSIS</b>
       <b>#include</b> <b>&lt;libmnl/libmnl.h&gt;</b>
       <b>#include</b> <b>&lt;libnetfilter_queue/pktbuff.h&gt;</b>

       void <b>pktb_push</b> (struct pkt_buff *pktb, unsigned int len)
       void <b>pktb_pull</b> (struct pkt_buff *pktb, unsigned int len)
       void <b>pktb_put</b> (struct pkt_buff *pktb, unsigned int len)
       void <b>pktb_trim</b> (struct pkt_buff *pktb, unsigned int len)

<b>Detailed</b> <b>Description</b>
       Do not use these functions. Instead, always use the mangle function
       appropriate to the level at which you are working.
        <b>pktb_mangle()</b> uses all the below functions except <b>pktb_pull()</b>, which
       is not used by anything.

<b>Function</b> <b>Documentation</b>
   <b>void</b> <b>pktb_pull</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_pull - increment pointer to packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> Number of bytes to add to packet start address

   <b>void</b> <b>pktb_push</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_push - decrement pointer to packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> Number of bytes to subtract from packet start address

   <b>void</b> <b>pktb_put</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_put - add extra bytes to the tail of the packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> Number of bytes to add to packet tail (and length)

   <b>void</b> <b>pktb_trim</b> <b>(struct</b> <b>pkt_buff</b> <b>*</b> <b>pktb,</b> <b>unsigned</b> <b>int</b> <b>len)</b>
       pktb_trim - set new length for this packet buffer

       <b>Parameters</b>
           <u>pktb</u> Pointer to userspace packet buffer
           <u>len</u> New packet length (tail is adjusted to reflect this)

<b>Author</b>
       Generated automatically by Doxygen for Nice Man Pages From Doxygen from
       the source code.

Nice Man Pages From Doxygen       Version 0.1                    do_not_use(3)
</pre>
